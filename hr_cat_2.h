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
    vector<double> corr2_array;

    //index用来指明corr2_array的规模，初始值为0，到达200开始做傅立叶变换；
//    int size;

    explicit HR_cat_2(QWidget *parent = 0);
    QImage MatToQImage(const cv::Mat& mat);
    double corr2(Mat a,Mat b);

    void ShowVec(const vector<double>& valList);
    ~HR_cat_2();


public slots:
    void onMouse(int EVENT, int x, int y, int flags, void* userdata);//注意要放到public中；
private slots:
int btn1_clicked();


private:
    Ui::HR_cat_2 *ui;

};

#endif // HR_CAT_2_H
