#include "mymainwindow.h"
#include "ui_mymainwindow.h"
#include <QMessageBox>
#include <QFile>
#include <QFileDialog>

//#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>
//#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>

#include "layer.h"
#include "net.h"
#include <float.h>
#include <stdio.h>
#include <vector>

#include <cstdlib>
#include <ctime>

//随机生成BGR颜色

std::vector<int> getRandomColor(){
    std::vector<int> color;
    srand((int)time(0));  // 产生随机种子  把0换成NULL也行
    for (int i = 0; i < 3; i++)
    {
        int temp_color = rand()%256;
        color.push_back(temp_color);

    }

    return color;
}

// ncnn nanodet
struct Object
{
    cv::Rect_<float> rect;
    int label;
    float prob;
};

static inline float intersection_area(const Object& a, const Object& b)
{
    cv::Rect_<float> inter = a.rect & b.rect;
    return inter.area();
}

static void qsort_descent_inplace(std::vector<Object>& faceobjects, int left, int right)
{
    int i = left;
    int j = right;
    float p = faceobjects[(left + right) / 2].prob;

    while (i <= j)
    {
        while (faceobjects[i].prob > p)
            i++;

        while (faceobjects[j].prob < p)
            j--;

        if (i <= j)
        {
            // swap
            std::swap(faceobjects[i], faceobjects[j]);

            i++;
            j--;
        }
    }

    #pragma omp parallel sections
    {
        #pragma omp section
        {
            if (left < j) qsort_descent_inplace(faceobjects, left, j);
        }
        #pragma omp section
        {
            if (i < right) qsort_descent_inplace(faceobjects, i, right);
        }
    }
}

static void qsort_descent_inplace(std::vector<Object>& faceobjects)
{
    if (faceobjects.empty())
        return;

    qsort_descent_inplace(faceobjects, 0, faceobjects.size() - 1);
}

static void nms_sorted_bboxes(const std::vector<Object>& faceobjects, std::vector<int>& picked, float nms_threshold)
{
    picked.clear();

    const int n = faceobjects.size();

    std::vector<float> areas(n);
    for (int i = 0; i < n; i++)
    {
        areas[i] = faceobjects[i].rect.width * faceobjects[i].rect.height;
    }

    for (int i = 0; i < n; i++)
    {
        const Object& a = faceobjects[i];

        int keep = 1;
        for (int j = 0; j < (int)picked.size(); j++)
        {
            const Object& b = faceobjects[picked[j]];

            // intersection over union
            float inter_area = intersection_area(a, b);
            float union_area = areas[i] + areas[picked[j]] - inter_area;
            // float IoU = inter_area / union_area
            if (inter_area / union_area > nms_threshold)
                keep = 0;
        }

        if (keep)
            picked.push_back(i);
    }
}

static void generate_proposals(const ncnn::Mat& cls_pred, const ncnn::Mat& dis_pred, int stride, const ncnn::Mat& in_pad, float prob_threshold, std::vector<Object>& objects)
{
    const int num_grid = cls_pred.h;

    int num_grid_x;
    int num_grid_y;
    if (in_pad.w > in_pad.h)
    {
        num_grid_x = in_pad.w / stride;
        num_grid_y = num_grid / num_grid_x;
    }
    else
    {
        num_grid_y = in_pad.h / stride;
        num_grid_x = num_grid / num_grid_y;
    }

    const int num_class = cls_pred.w;
    const int reg_max_1 = dis_pred.w / 4;

    for (int i = 0; i < num_grid_y; i++)
    {
        for (int j = 0; j < num_grid_x; j++)
        {
            const int idx = i * num_grid_x + j;

            const float* scores = cls_pred.row(idx);

            // find label with max score
            int label = -1;
            float score = -FLT_MAX;
            for (int k = 0; k < num_class; k++)
            {
                if (scores[k] > score)
                {
                    label = k;
                    score = scores[k];
                }
            }

            if (score >= prob_threshold)
            {
                ncnn::Mat bbox_pred(reg_max_1, 4, (void*)dis_pred.row(idx));
                {
                    ncnn::Layer* softmax = ncnn::create_layer("Softmax");

                    ncnn::ParamDict pd;
                    pd.set(0, 1); // axis
                    pd.set(1, 1);
                    softmax->load_param(pd);

                    ncnn::Option opt;
                    opt.num_threads = 1;
                    opt.use_packing_layout = false;

                    softmax->create_pipeline(opt);

                    softmax->forward_inplace(bbox_pred, opt);

                    softmax->destroy_pipeline(opt);

                    delete softmax;
                }

                float pred_ltrb[4];
                for (int k = 0; k < 4; k++)
                {
                    float dis = 0.f;
                    const float* dis_after_sm = bbox_pred.row(k);
                    for (int l = 0; l < reg_max_1; l++)
                    {
                        dis += l * dis_after_sm[l];
                    }

                    pred_ltrb[k] = dis * stride;
                }

                float pb_cx = (j + 0.5f) * stride;
                float pb_cy = (i + 0.5f) * stride;

                float x0 = pb_cx - pred_ltrb[0];
                float y0 = pb_cy - pred_ltrb[1];
                float x1 = pb_cx + pred_ltrb[2];
                float y1 = pb_cy + pred_ltrb[3];

                Object obj;
                obj.rect.x = x0;
                obj.rect.y = y0;
                obj.rect.width = x1 - x0;
                obj.rect.height = y1 - y0;
                obj.label = label;
                obj.prob = score;

                objects.push_back(obj);
            }
        }
    }
}

static int detect_nanodet(const cv::Mat& bgr, std::vector<Object>& objects)
{
    ncnn::Net nanodet;

    nanodet.opt.use_vulkan_compute = true;
    // nanodet.opt.use_bf16_storage = true;

    // original pretrained model from https://github.com/RangiLyu/nanodet
    // the ncnn model https://github.com/nihui/ncnn-assets/tree/master/models

    QFile::copy("assets:/pic/nanodet_m.param", "nanodet_m.param");
    QFile::copy("assets:/pic/nanodet_m.bin", "nanodet_m.bin");
    nanodet.load_param("nanodet_m.param");
    nanodet.load_model("nanodet_m.bin");

    int width = bgr.cols;
    int height = bgr.rows;

    const int target_size = 320;
    const float prob_threshold = 0.4f;
    const float nms_threshold = 0.5f;

    // pad to multiple of 32
    int w = width;
    int h = height;
    float scale = 1.f;
    if (w > h)
    {
        scale = (float)target_size / w;
        w = target_size;
        h = h * scale;
    }
    else
    {
        scale = (float)target_size / h;
        h = target_size;
        w = w * scale;
    }

    ncnn::Mat in = ncnn::Mat::from_pixels_resize(bgr.data, ncnn::Mat::PIXEL_BGR, width, height, w, h);

    // pad to target_size rectangle
    int wpad = (w + 31) / 32 * 32 - w;
    int hpad = (h + 31) / 32 * 32 - h;
    ncnn::Mat in_pad;
    ncnn::copy_make_border(in, in_pad, hpad / 2, hpad - hpad / 2, wpad / 2, wpad - wpad / 2, ncnn::BORDER_CONSTANT, 0.f);

    const float mean_vals[3] = {103.53f, 116.28f, 123.675f};
    const float norm_vals[3] = {0.017429f, 0.017507f, 0.017125f};
    in_pad.substract_mean_normalize(mean_vals, norm_vals);

    ncnn::Extractor ex = nanodet.create_extractor();

    ex.input("input.1", in_pad);

    std::vector<Object> proposals;

    // stride 8
    {
        ncnn::Mat cls_pred;
        ncnn::Mat dis_pred;
        ex.extract("792", cls_pred);
        ex.extract("795", dis_pred);

        std::vector<Object> objects8;
        generate_proposals(cls_pred, dis_pred, 8, in_pad, prob_threshold, objects8);

        proposals.insert(proposals.end(), objects8.begin(), objects8.end());
    }

    // stride 16
    {
        ncnn::Mat cls_pred;
        ncnn::Mat dis_pred;
        ex.extract("814", cls_pred);
        ex.extract("817", dis_pred);

        std::vector<Object> objects16;
        generate_proposals(cls_pred, dis_pred, 16, in_pad, prob_threshold, objects16);

        proposals.insert(proposals.end(), objects16.begin(), objects16.end());
    }

    // stride 32
    {
        ncnn::Mat cls_pred;
        ncnn::Mat dis_pred;
        ex.extract("836", cls_pred);
        ex.extract("839", dis_pred);

        std::vector<Object> objects32;
        generate_proposals(cls_pred, dis_pred, 32, in_pad, prob_threshold, objects32);

        proposals.insert(proposals.end(), objects32.begin(), objects32.end());
    }

    // sort all proposals by score from highest to lowest
    qsort_descent_inplace(proposals);

    // apply nms with nms_threshold
    std::vector<int> picked;
    nms_sorted_bboxes(proposals, picked, nms_threshold);

    int count = picked.size();

    objects.resize(count);
    for (int i = 0; i < count; i++)
    {
        objects[i] = proposals[picked[i]];

        // adjust offset to original unpadded
        float x0 = (objects[i].rect.x - (wpad / 2)) / scale;
        float y0 = (objects[i].rect.y - (hpad / 2)) / scale;
        float x1 = (objects[i].rect.x + objects[i].rect.width - (wpad / 2)) / scale;
        float y1 = (objects[i].rect.y + objects[i].rect.height - (hpad / 2)) / scale;

        // clip
        x0 = std::max(std::min(x0, (float)(width - 1)), 0.f);
        y0 = std::max(std::min(y0, (float)(height - 1)), 0.f);
        x1 = std::max(std::min(x1, (float)(width - 1)), 0.f);
        y1 = std::max(std::min(y1, (float)(height - 1)), 0.f);

        objects[i].rect.x = x0;
        objects[i].rect.y = y0;
        objects[i].rect.width = x1 - x0;
        objects[i].rect.height = y1 - y0;
    }

    return 0;
}


cv::Mat draw_objects(const cv::Mat& bgr, const std::vector<Object>& objects)
{
    static const char* class_names[] = {
        "person", "bicycle", "car", "motorcycle", "airplane", "bus", "train", "truck", "boat", "traffic light",
        "fire hydrant", "stop sign", "parking meter", "bench", "bird", "cat", "dog", "horse", "sheep", "cow",
        "elephant", "bear", "zebra", "giraffe", "backpack", "umbrella", "handbag", "tie", "suitcase", "frisbee",
        "skis", "snowboard", "sports ball", "kite", "baseball bat", "baseball glove", "skateboard", "surfboard",
        "tennis racket", "bottle", "wine glass", "cup", "fork", "knife", "spoon", "bowl", "banana", "apple",
        "sandwich", "orange", "broccoli", "carrot", "hot dog", "pizza", "donut", "cake", "chair", "couch",
        "potted plant", "bed", "dining table", "toilet", "tv", "laptop", "mouse", "remote", "keyboard", "cell phone",
        "microwave", "oven", "toaster", "sink", "refrigerator", "book", "clock", "vase", "scissors", "teddy bear",
        "hair drier", "toothbrush"
    };

    cv::Mat image = bgr.clone();

    for (size_t i = 0; i < objects.size(); i++)
    {
        std::vector<int> CurColor = getRandomColor();
        const Object& obj = objects[i];

        fprintf(stderr, "%d = %.5f at %.2f %.2f %.2f x %.2f\n", obj.label, obj.prob,
                obj.rect.x, obj.rect.y, obj.rect.width, obj.rect.height);

        int tl_ = (int)(0.002*image.rows);
        if (tl_ == 0){
            tl_ = 1;
        }
        cv::rectangle(image, obj.rect, cv::Scalar(CurColor[0], CurColor[1], CurColor[2]),tl_);

        char text[256];
        sprintf(text, "%s %.1f%%", class_names[obj.label], obj.prob * 100);
        qDebug() << class_names[obj.label] << "," << obj.prob * 100;

        int baseLine = 0;
        cv::Size label_size = cv::getTextSize(text, cv::FONT_HERSHEY_SIMPLEX, (float)tl_/2, tl_,&baseLine);

        int x = obj.rect.x;
        int y = obj.rect.y - label_size.height - baseLine;
        if (y < 0)
            y = 0;
        if (x + label_size.width > image.cols)
            x = image.cols - label_size.width;

        cv::rectangle(image, cv::Rect(cv::Point(x, y), cv::Size(label_size.width, label_size.height + baseLine)),
                      cv::Scalar(CurColor[0], CurColor[1], CurColor[2]), -1);

        cv::putText(image, text, cv::Point(x, y + label_size.height),
                    cv::FONT_HERSHEY_SIMPLEX, (float)tl_/2, cv::Scalar(0, 0, 0),tl_);
    }

//    cv::imshow("image", image);
//    cv::waitKey(0);

    return image;
}




//获取android 相册
QString selectedFileName;
cv::Mat Img2Det;

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT void JNICALL
Java_com_amin_QtAndroidGallery_QtAndroidGallery_fileSelected(JNIEnv */*env*/,
                                                             jobject /*obj*/,
                                                             jstring results)
{
    selectedFileName = QAndroidJniObject(results).toString();
}

#ifdef __cplusplus
}
#endif


//label显示Mat
void LabelDisplayMat(QLabel *label, cv::Mat &mat){

    cv::Mat Rgb;
    QImage Img;
    int w = label->rect().width();
    int h = label->rect().height();

    if (mat.channels() == 3){
        cv::cvtColor(mat,Rgb,cv::COLOR_BGR2RGB);
        cv::resize(Rgb,Rgb,cv::Size(w,h));

        Img = QImage((const uchar*)(Rgb.data), Rgb.cols,Rgb.rows,Rgb.cols * Rgb.channels(),QImage::Format_RGB888);
    }
    else{
        cv::Mat grayImg;
        cv::resize(mat,grayImg,cv::Size(w,h));
        Img = QImage((const uchar*)(grayImg.data), grayImg.cols,grayImg.rows,grayImg.cols * grayImg.channels(),QImage::Format_Indexed8);
    }

    label->setPixmap(QPixmap::fromImage(Img));
    label->setScaledContents(true);  //自适应大小

}



MyMainWindow::MyMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MyMainWindow)
{
    ui->setupUi(this);
}

MyMainWindow::~MyMainWindow()
{
    delete ui;
}


void MyMainWindow::on_pushButton_clicked()
{

    //在android中是不work的
    //    QString fileName = QFileDialog::getOpenFileName(this, tr("打开图片"), "assets:/pic",  tr("Image Files(*.jpg *.png *.bmp *.pgm *.pbm);;All(*.*)"));
    //    qDebug() << "fileName: " << fileName ;
    //    QMessageBox::information(NULL, "选择的图片路径", fileName, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

    selectedFileName = "#";
    QAndroidJniObject::callStaticMethod<void>("com/amin/QtAndroidGallery/QtAndroidGallery",
                                              "openAnImage",
                                              "()V");
    while(selectedFileName == "#")
            qApp->processEvents();

    qDebug()<<"file name:"<<selectedFileName;
    if(QFile(selectedFileName).exists())
    {
        //        QImage img(selectedFileName);
        //        ui->label_2->setPixmap(QPixmap::fromImage(img));

        cv::Mat img = cv::imread(selectedFileName.toStdString());
        LabelDisplayMat(ui->label_2,img);
        Img2Det = img.clone();
    }



}

void MyMainWindow::on_pushButton_2_clicked()
{
    //    QFile::copy("assets:/pic/ai.png", "ai.png");
    //    cv::Mat img = cv::imread("ai.png");
    //    LabelDisplayMat(ui->label_2,img);

    selectedFileName = "#";
    QAndroidJniObject::callStaticMethod<void>("com/amin/QtAndroidGallery/QtAndroidGallery",
                                              "captureAnImage",
                                              "()V");
    while(selectedFileName == "#")
        qApp->processEvents();

    if(selectedFileName != "#")
    {
        if(QFile(selectedFileName).exists()){
            cv::Mat img = cv::imread(selectedFileName.toStdString());
            LabelDisplayMat(ui->label_2,img);
            Img2Det = img.clone();
        }

    }

}

void MyMainWindow::on_pushButton_3_clicked()
{
    //    QFile::copy("assets:/pic/bus.jpg", "bus.jpg");
    //    cv::Mat m = cv::imread("bus.jpg", 1);
    qDebug() << "开始识别";

    if (!Img2Det.empty()){
        std::vector<Object> objects;
        detect_nanodet(Img2Det, objects);

        cv::Mat afterImg;
        afterImg = draw_objects(Img2Det, objects);

        LabelDisplayMat(ui->label_2,afterImg);
        Img2Det.release();
    }
    else{
        QMessageBox::information(NULL, "请选择图片", "暂时没有被选中的图片资源!", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
    }


}
