#ifndef HR_CAT_2_H
#define HR_CAT_2_H
#include <QTimer>
#include <QPainter>
#include <QPixmap>
#include <QLabel>
#include <QImage>
#include <QDebug>
#include <opencv2/opencv.hpp>
#include <QMainWindow>


void mouseWrapper( int event, int x, int y, int flags, void* param );
namespace Ui {
class HR_cat_2;
}

class HR_cat_2 : public QMainWindow
{
    Q_OBJECT

public:
    QImage  image,image_origin;
    explicit HR_cat_2(QWidget *parent = 0);
    QImage MatToQImage(const cv::Mat& mat);
    ~HR_cat_2();


public slots:
    void onMouse(int EVENT, int x, int y, int flags, void* userdata);//注意要放到public中；
private slots:
int btn1_clicked();


private:
    Ui::HR_cat_2 *ui;

};

#endif // HR_CAT_2_H
