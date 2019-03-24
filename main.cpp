#include "hr_cat_2.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    HR_cat_2 w;
 w.setStyleSheet("QMainWindow{background-image: url(:/3.jpg)}");

    w.show();


    return a.exec();
}
