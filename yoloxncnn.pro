QT       += core gui
QT += androidextras

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mymainwindow.cpp

HEADERS += \
    mymainwindow.h

FORMS += \
    mymainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    pic.qrc

DISTFILES += \
    android/AndroidManifest.xml \
    android/build.gradle \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew \
    android/gradlew.bat \
    android/res/values/libs.xml \
    android/src/com/amin/QtAndroidGallery/QtAndroidGallery.java \
    model/yolact.bin \
    model/yolact.param

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

QMAKE_CXXFLAGS += -fopenmp -static-openmp
QMAKE_LFLAGS += -fopenmp -static-openmp
LIBS += -fopenmp -static-openmp

android {
ANDROID_OPENCV = D:/QT/OpenCV-android-sdk/sdk/native
#ANDROID_OPENCV = D:/NCNN/opencv-mobile-4.5.1-android/sdk/native
INCLUDEPATH += \
$$ANDROID_OPENCV/jni/include/opencv2 \
$$ANDROID_OPENCV/jni/include \
D:/NCNN/ncnn-20211208-android/armeabi-v7a/include \
D:/NCNN/ncnn-20211208-android/armeabi-v7a/include/ncnn


LIBS += \
$$ANDROID_OPENCV/staticlibs/armeabi-v7a/libopencv_ml.a \
$$ANDROID_OPENCV/staticlibs/armeabi-v7a/libopencv_objdetect.a \
$$ANDROID_OPENCV/staticlibs/armeabi-v7a/libopencv_calib3d.a \
$$ANDROID_OPENCV/staticlibs/armeabi-v7a/libopencv_video.a \
$$ANDROID_OPENCV/staticlibs/armeabi-v7a/libopencv_features2d.a \
$$ANDROID_OPENCV/staticlibs/armeabi-v7a/libopencv_highgui.a \
$$ANDROID_OPENCV/staticlibs/armeabi-v7a/libopencv_flann.a \
$$ANDROID_OPENCV/staticlibs/armeabi-v7a/libopencv_imgproc.a \
$$ANDROID_OPENCV/staticlibs/armeabi-v7a/libopencv_dnn.a \
$$ANDROID_OPENCV/staticlibs/armeabi-v7a/libopencv_core.a \
$$ANDROID_OPENCV/3rdparty/libs/armeabi-v7a/libcpufeatures.a \
$$ANDROID_OPENCV/3rdparty/libs/armeabi-v7a/libIlmImf.a \
#$$ANDROID_OPENCV/3rdparty/libs/armeabi-v7a/liblibjasper.a \
$$ANDROID_OPENCV/3rdparty/libs/armeabi-v7a/liblibjpeg-turbo.a \
$$ANDROID_OPENCV/3rdparty/libs/armeabi-v7a/liblibpng.a \
$$ANDROID_OPENCV/3rdparty/libs/armeabi-v7a/liblibprotobuf.a \
$$ANDROID_OPENCV/3rdparty/libs/armeabi-v7a/liblibtiff.a \
$$ANDROID_OPENCV/3rdparty/libs/armeabi-v7a/liblibwebp.a \
$$ANDROID_OPENCV/3rdparty/libs/armeabi-v7a/libquirc.a \
$$ANDROID_OPENCV/3rdparty/libs/armeabi-v7a/libtbb.a \
$$ANDROID_OPENCV/3rdparty/libs/armeabi-v7a/libtegra_hal.a \
$$ANDROID_OPENCV/libs/armeabi-v7a/libopencv_java4.so \
D:/NCNN/ncnn-20211208-android/armeabi-v7a/lib/libncnn.a \
D:/QT/android-ndk-r21/platforms/android-28/arch-arm/usr/lib/libandroid.so \
D:/QT/android-ndk-r21/toolchains/llvm/prebuilt/windows-x86_64/lib64/clang/9.0.8/lib/linux/arm/libomp.a
D:/QT/android-ndk-r21/toolchains/llvm/prebuilt/windows-x86_64/lib64/clang/9.0.8/lib/linux/arm/libomp.so
#$$ANDROID_OPENCV/staticlibs/armeabi-v7a/libopencv_core.a \
#$$ANDROID_OPENCV/staticlibs/armeabi-v7a/libopencv_features2d.a \
#$$ANDROID_OPENCV/staticlibs/armeabi-v7a/libopencv_highgui.a \
#$$ANDROID_OPENCV/staticlibs/armeabi-v7a/libopencv_imgproc.a \
#$$ANDROID_OPENCV/staticlibs/armeabi-v7a/libopencv_photo.a \
#$$ANDROID_OPENCV/staticlibs/armeabi-v7a/libopencv_video.a \

# add other file
#data.files += ssd_mobilenet_v1_coco_11_06_2017/frozen_inference_graph.pb
#data.files += ssd_mobilenet_v1_coco_11_06_2017/ssd_mobilenet_v1_coco.pbtxt

data.files += pic/nanodet.png
data.files += pic/ai.png
data.files += pic/carmera.png
data.files += pic/picture.png
data.files += pic/bus.jpg
data.files += model/nanodet_m.param
data.files += model/nanodet_m.bin

data.path = /assets/pic
INSTALLS += data

#QMAKE_CFLAGS += -fno-rtti
#QMAKE_CXXFLAGS += -fno-rtti



}

ANDROID_EXTRA_LIBS += D:/QT/OpenCV-android-sdk/sdk/native/libs/armeabi-v7a/libopencv_java4.so \
                      D:/QT/android-ndk-r21/platforms/android-28/arch-arm/usr/lib/libandroid.so \
                      D:/QT/android-ndk-r21/toolchains/llvm/prebuilt/windows-x86_64/lib64/clang/9.0.8/lib/linux/arm/libomp.so




