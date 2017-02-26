#include "mainwindow.h"
#include <QApplication>
#include <QDesktopWidget>

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    QDesktopWidget desktop;
    int width =  w.width();
    int height =  w.height();
    w.move((desktop.width() - width)/2, (desktop.height() - height)/2);
    w.show();
    
    return a.exec();
}
