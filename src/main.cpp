#include "qttubeapplication.h"
#include "ui/forms/mainwindow.h"

int main(int argc, char *argv[])
{
    QtTubeApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
