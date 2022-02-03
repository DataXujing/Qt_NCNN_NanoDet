#include "mymainwindow.h"

#include <QApplication>
#include <QFile>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 加载QSS
    //QFile qss_(":/pic/assets/NeonButtons.qss");
    QFile qss_(":/pic/assets/ManjaroMix.qss");
    if( qss_.open(QFile::ReadOnly)){
        qDebug("open success");
        QString style = QLatin1String(qss_.readAll());
        a.setStyleSheet(style);
        qss_.close();
    }
    else{
        qDebug("Open failed");
    }


    MyMainWindow w;
    w.show();
    return a.exec();
}
