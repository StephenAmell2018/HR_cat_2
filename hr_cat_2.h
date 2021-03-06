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
#include <vector>
#include "QValueAxis"
#include <complex>
#include <opencv2/face.hpp>
#include <QWidget>
#include "qcustomplot.h"
#include <QtCharts/QChartView>
#include <QSplineSeries>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing/render_face_detections.h>
#include <dlib/image_processing.h>
#include <dlib/gui_widgets.h>
#include <dlib/image_io.h>
#include <iostream>

using namespace std;
using namespace cv;

void mouseWrapper( int event, int x, int y, int flags, void* param );
namespace Ui {
class HR_cat_2;
}

class HR_cat_2 : public QMainWindow
{
    Q_OBJECT

public:
    QImage  image,image_origin;
    QTimer *timer;




    //corr2_array用来存储互相关函数运算后得到的0-1之间的数值，这里不用指明长度因为是动态数组；


    //index用来指明corr2_array的规模，初始值为0，到达200开始做傅立叶变换；
//    int size;

    explicit HR_cat_2(QWidget *parent = 0);
    QImage MatToQImage(const cv::Mat& mat);
    double corr2(Mat a,Mat b);

//    struct Complex_{
//        double real;
//        double imagin;
//    };
//    typedef struct Complex_ Complex;

    vector<double> corr2_array;





    void ShowVec(const vector<double>& valList);
    ~HR_cat_2();

    //画图尝试
    void setupQuadraticDemo(QCustomPlot *customPlot);
    void timerEvent(QTimerEvent *event);
    vector<double> generateGaussianTemplate(vector<double> window, int center, double sigma);
public slots:
    void onMouse(int EVENT, int x, int y, int flags, void* userdata);//注意要放到public中；
private slots:
int btn1_clicked();
int btn2_clicked();
void video_dealing_chosing();
void UIupdate();
void btn3_clicked();
void drawPolyline
    (
      Mat &im,
      const vector<Point2f> &landmarks,
      const int start,
      const int end,
      bool isClosed = false
    );

void drawLandmarks(Mat &im, vector<Point2f> &landmarks);




private:
    Ui::HR_cat_2 *ui;
    Ptr<face::Facemark> facemark;
    QTimer    *timer_face;

    VideoCapture cap;
    CascadeClassifier face_cascade;



};

#endif // HR_CAT_2_H


