#include "hr_cat_2.h"
#include "ui_hr_cat_2.h"
#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>

using namespace std;
using namespace cv;

Mat frame;//保存帧图像
Point origin;//用于保存鼠标选择第一次单击时点的位置
Rect selection;//用于保存鼠标选择的矩形框
bool selectObject = false;//代表是否在选要跟踪的初始目标，true表示正在用鼠标选择

void mouseWrapper( int event, int x, int y, int flags, void* param )
{
    HR_cat_2 * mainWin = (HR_cat_2 *)(param);
    mainWin->onMouse(event,x,y,flags,param);
}


HR_cat_2::HR_cat_2(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::HR_cat_2)
{
    ui->setupUi(this);
    connect(ui->btn1,SIGNAL(clicked()),this,SLOT(btn1_clicked()));//打开摄像头按钮
}



void HR_cat_2::onMouse( int event, int x, int y, int, void* )
{
    if( selectObject )//只有当鼠标左键按下去时才有效，然后通过if里面代码就可以确定所选择的矩形区域selection了
    {
        selection.x = MIN(x, origin.x);//矩形左上角顶点坐标
        selection.y = MIN(y, origin.y);
        selection.width = std::abs(x - origin.x);//矩形宽
        selection.height = std::abs(y - origin.y);//矩形高

        selection &= Rect(0, 0, frame.cols, frame.rows);//用于确保所选的矩形区域在图片范围内
    }

    switch( event )
    {
    case CV_EVENT_LBUTTONDOWN:
        origin = Point(x,y);
        selection = Rect(x,y,0,0);//鼠标刚按下去时初始化了一个矩形区域
        selectObject = true;
        break;
    case CV_EVENT_LBUTTONUP:
        selectObject = false;
        if( selection.width > 0 && selection.height > 0 )
        break;
    }
}

QImage HR_cat_2::MatToQImage(const cv::Mat& mat)
{
    // 8-bits unsigned, NO. OF CHANNELS = 1
    if(mat.type() == CV_8UC1)
    {
        QImage image(mat.cols, mat.rows, QImage::Format_Indexed8);
        // Set the color table (used to translate colour indexes to qRgb values)
        image.setColorCount(256);
        for(int i = 0; i < 256; i++)
        {
            image.setColor(i, qRgb(i, i, i));
        }
        // Copy input Mat
        uchar *pSrc = mat.data;
        for(int row = 0; row < mat.rows; row ++)
        {
            uchar *pDest = image.scanLine(row);
            memcpy(pDest, pSrc, mat.cols);
            pSrc += mat.step;
        }
        return image;
    }
    // 8-bits unsigned, NO. OF CHANNELS = 3
    else if(mat.type() == CV_8UC3)
    {
        // Copy input Mat
        const uchar *pSrc = (const uchar*)mat.data;
        // Create QImage with same dimensions as input Mat
        QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
        return image.rgbSwapped();
    }
    else if(mat.type() == CV_8UC4)
    {
        qDebug() << "CV_8UC4";
        // Copy input Mat
        const uchar *pSrc = (const uchar*)mat.data;
        // Create QImage with same dimensions as input Mat
        QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_ARGB32);
        return image.copy();
    }
    else
    {
        qDebug() << "ERROR: Mat could not be converted to QImage.";
        return QImage();
    }
}

int HR_cat_2::btn1_clicked(){
    bool stop = false;
    Mat ROI;
    VideoCapture cap(0);

    if(!cap.isOpened())
    {
        return -1;
    }

    namedWindow("frame",0);
   setMouseCallback( "frame",  mouseWrapper, 0 );//消息响应机制

   while(!stop)
   {
       cap>>frame;
       if( selectObject && selection.width > 0 && selection.height > 0 )
       {
           ROI = frame(selection);//这句话是将frame帧图片中的选中矩形区域的地址指向ROI，
                                  //对于内存而言，frame和ROI是公用内存的，所以下面这句实际
                                  //是将frame帧图像中的选中矩形区域块图像进行操作，而不是新创建
                                  //一个内存来进行操作
           //当然所截图的矩形区域ROI，可以使用imwrite函数来保存
           bitwise_not(ROI, ROI);//bitwise_not为将每一个bit位取反
       }
      imshow("frame",frame);
       image=MatToQImage(ROI);
       ui->label2->setScaledContents(true);//很重要，通过这个设置可以使label自适应显示图片
       ui->label2->setPixmap(QPixmap::fromImage(image));//将视频显示到label上

       image_origin=MatToQImage(frame);
       ui->label->setScaledContents(true);//很重要，通过这个设置可以使label自适应显示图片
       ui->label->setPixmap(QPixmap::fromImage(image_origin));//将视频显示到label上


       if( waitKey(30) == 27 )//ESC键退出
           stop = true;
   }
   return 0;
}

HR_cat_2::~HR_cat_2()
{
    delete ui;
}



