#include "hr_cat_2.h"
#include "ui_hr_cat_2.h"
#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include "QValueAxis"
#include<QDateTime>
#include<QTime>
#include <gsl/gsl_sf_bessel.h>
#include <complex>
#include<fftw3.h>
#define real 0
#define imag 1
#define PI 3.1415926
using namespace std;
using namespace cv;
using namespace QtCharts;

Mat frame;//保存帧图像
Point origin;//用于保存鼠标选择第一次单击时点的位置
Rect selection;//用于保存鼠标选择的矩形框
bool selectObject = false;//代表是否在选要跟踪的初始目标，true表示正在用鼠标选择
QSplineSeries* series;
QSplineSeries* series_fft;

 int timerId;
 int timerId_time;
 int timeId_fft;



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
                           //corr2在线波形显示
                           series = new QSplineSeries;
                           QChart *chart = new QChart();
                           chart->legend()->hide();
                           chart->addSeries(series);
                           for(int i=0;i<512;++i){
                              series->append(i,0);
                           }
                           // Customize chart background
                           QLinearGradient backgroundGradient;
                           backgroundGradient.setStart(QPointF(0, 0));
                           backgroundGradient.setFinalStop(QPointF(0, 1));
                           backgroundGradient.setColorAt(0.0, QRgb(0xd2d0d1));
                           backgroundGradient.setColorAt(1.0, QRgb(0x4c4547));
                           backgroundGradient.setCoordinateMode(QGradient::ObjectBoundingMode);
                           chart->setBackgroundBrush(backgroundGradient);
                           chart->setTitle("corr2_t");

                           ui->graphicsView->setChart(chart);
//                         ui->graphicsView->setRenderHint(QPainter::Antialiasing);
                           QValueAxis *axisX = new QValueAxis;
                           axisX->setRange(0,512);
                           axisX->setLabelFormat("%g");
                           axisX->setTitleText("axisX");

                           QValueAxis *axisY = new QValueAxis;
                           axisY->setRange(0,1);
                           axisY->setTitleText("axisY");


                           // Customize axis label font
                           QFont labelsFont("Times",16,false);
                           labelsFont.setPixelSize(12);
                           axisX->setLabelsFont(labelsFont);
                           axisY->setLabelsFont(labelsFont);

                           // Customize axis colors
                           QPen axisPen(QRgb(0xd18952));
                           axisPen.setWidth(2);
                           axisX->setLinePen(axisPen);
                           axisY->setLinePen(axisPen);

                           chart->setAxisX(axisX,series);
                           chart->setAxisY(axisY,series);
                           chart->legend()->hide();
                           chart->setTitle("Correlation Coefficient Waveform");
                           timerId = startTimer(10);
                           timerId_time=startTimer(20);
                           timeId_fft=startTimer(20);

                           //fft变换的实时波形显示
                           series_fft = new QSplineSeries;
                           QChart *chart_fft = new QChart();
                           chart_fft->legend()->hide();
                           chart_fft->addSeries(series_fft);
                           for(int i=0;i<512;++i){
                              series_fft->append(i,0);
                           }

                           chart_fft->setTitle("Frequency Spectrogram");
                           chart_fft->setBackgroundBrush(backgroundGradient);
                           ui->fft_display->setChart(chart_fft);
//                         ui->graphicsView->setRenderHint(QPainter::Antialiasing);
                           QValueAxis *axisX_fft = new QValueAxis;
                           axisX_fft->setRange(0,50);
                           axisX_fft->setTickCount(11);
                           axisX_fft->setLabelFormat("%g");
                           axisX_fft->setTitleText("axisX");

                           QValueAxis *axisY_fft = new QValueAxis;
                           axisY_fft->setRange(0,0.02);
                           axisY_fft->setTitleText("axisY");


                           labelsFont.setPixelSize(12);
                           axisX_fft->setLabelsFont(labelsFont);
                           axisY_fft->setLabelsFont(labelsFont);

                           // Customize axis colors

                           axisPen.setWidth(2);
                           axisX_fft->setLinePen(axisPen);
                           axisY_fft->setLinePen(axisPen);


                           chart_fft->setAxisX(axisX_fft,series_fft);
                           chart_fft->setAxisY(axisY_fft,series_fft);
                           chart_fft->legend()->hide();
//                           chart_fft->setTitle("fft_display");

    connect(ui->btn1,SIGNAL(clicked()),this,SLOT(btn1_clicked()));//打开摄像头按钮
    connect(ui->btn2,SIGNAL(clicked()),this,SLOT(btn2_clicked()));
    connect(ui->comboBox,SIGNAL(currentIndexChanged(QString)),this,SLOT(offline_video_dealing()));
    connect( ui->exit, SIGNAL(clicked()),qApp, SLOT(closeAllWindows()) );
}



//画图函数、、可以将具体的处理逻辑写在这里，然后在按钮函数那里调用！！


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

//用于心率核心提取的相关算法
double HR_cat_2::corr2(Mat a,Mat b){
//    Mat mean_A,mean_B;
//    Mat stddev_1,stddev_2;
//    meanStdDev(a,mean_A,stddev_1);
//    meanStdDev(b,mean_B,stddev_2);
//    float value_A =mean_A.at<double>(1);
//    float value_B = mean_B.at<double>(1);
      Scalar mean1 = cv::mean( a );
      double value_A = mean1.val[1];
      Scalar mean2 = cv::mean( b );
      double value_B = mean2.val[1];

    cout<<"meanA = "<<value_A<<endl;
    cout<<"meanB = "<<value_B<<endl;
    int height= a.cols;//列
    int width=a.rows;//行
    cout<<"width="<<width<<endl;
    cout<<"heigth="<<height<<endl;
    double s=0.0;
    double q=0.0;
    double p=0.0;
    for(int i=0;i<width;i++){
        for(int j=0;j<height;j++){
         s+=(a.at<Vec3b>(i,j)[1]-value_A)*(b.at<Vec3b>(i,j)[1]-value_B);
         p+=pow(a.at<Vec3b>(i,j)[1]-value_A,2);
         q+=pow(b.at<Vec3b>(i,j)[1]-value_B,2);
        }
    }
  cout<<"s="<<s<<endl;
  cout<<"sqrt(q*p)="<<sqrt(q*p)<<endl;
  double r=s/sqrt(q*p);
  cout<<"我们认为r="<<r<<endl;
  return r;
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

//void HR_cat_2::DFT(vector<double>src,complex<double>  dst[],int size){
////    clock_t start,end;
////    start=clock();

//    for(int m=0;m<size;m++){
//        double real=0.0;
//        double imagin=0.0;
//        for(int n=0;n<size;n++){
//            double x=M_PI*2*m*n;
//            real+=src[n]*cos(x/size);
//            imagin+=src[n]*(-sin(x/size));
//        }
//        dst[m].imag()=imagin;
//        dst[m].real()=real;
//        if(imagin>=0.0){
////            printf("%lf+%lfj\n",real,imagin);
//        }
//        else
//        {
////            printf("%lf%lfj\n",real,imagin,sqrt(pow(dst[m].imagin,2)+pow(dst[m].real,2)));
//        }
//     printf("%lf\n",sqrt(pow(real,2)+pow(imagin,2)));
//    }
////    end=clock();
////    printf("DFT use Complex :%lf for Datasize of:%d\n",(double)(end-start)/CLOCKS_PER_SEC,size);
//}





int HR_cat_2::btn2_clicked(){

    VideoCapture cap;
    cap.open("/Users/yanyupeng/Desktop/speckleVideo/180325/greenSpeckleVideo/greenSpeckleVideo_0008.avi");
    //获取帧率
    double rate = cap.get(CV_CAP_PROP_FPS);//60.002
    cout<<"帧率为:"<<rate<<endl;
    int numFrames =  cap.get(CV_CAP_PROP_FRAME_COUNT);
    cout<<"总帧数为"<<numFrames<<endl;
    bool stop = false;
    Mat ROI,ROI_next;


    if(!cap.isOpened())
    {
        return -1;
    }

    namedWindow("video",0);
    setMouseCallback( "video",  mouseWrapper, 0 );//消息响应机制

   //处理步骤应该在while循环中完成因为这是一个实时流的处理，而且应该是在提取到的roi矩阵部分操作
   //因为roi是在if中提取的所以要把流程定位在if中，后续步骤只是用来显示；

   //通道分离；channels用来存储三个颜色通道，mat用来承载需要的通道；
   //接下来的处理针对imageBlueChannel就可以了；
   std::vector<Mat> channels;
   Mat  imageBlueChannel;

   //用来存储相邻两针图像矩阵
   Mat cur,next;
   //**很重要！！！！*
   //这里原本是用index来做标示，超过200 开始傅立叶变换但是发现会闪退，可能是vector std库函数的原因
   //用了vecto.size()做标示，可以

   Mat tmp,last;
   last = NULL;
   int frameNum=0;
   Mat test;
   bool flag1= cap.read(frame);

   cout<<"捕获当前帧:"<<++frameNum<<endl;


   while(!stop)
   {
              clock_t start,end;
              start=clock();
               ROI = frame(selection);//这句话是将frame帧图片中的选中矩形区域的地址指向ROI
               //对于内存而言，frame和ROI是公用内存的，所以下面这句实际
               //是将frame帧图像中的选中矩形区域块图像进行操作，而不是新创建
               //一个内存来进行操作
               //当然所截图的矩形区域ROI，可以使用imwrite函数来保存
               //这里只是为了显示的比较直观
               //bitwise_not(ROI, ROI);//bitwise_not为将每一个bit位取反
               Mat mean_A;
               Mat stddev_1;
               meanStdDev(ROI,mean_A,stddev_1);
               float value_A =mean_A.at<double>(1);
               if(value_A==0)
               {
                 cout<<"请截图确定待测区域！"<<endl;
               }else if(value_A!=0 &&  corr2_array.size()==0){
                   cur=next=ROI;
                   corr2_array.push_back(1);
                   cout<<"the first 相关系数个数为："<<corr2_array.size()<<endl;
                   //corr2 画图 第一个值
               }else if(value_A!=0 && corr2_array.size()>0 && corr2_array.size()<512){
                   cur=next.clone();// = capy clone 在内存地址的操作很不一样!!!十天
                   bool flag2= cap.read(frame);
                   cout<<"捕获下一帧:"<<++frameNum<<endl;
                   ROI_next = frame(selection);
                   next=ROI_next;
                   double index=corr2(cur,next);
                   corr2_array.push_back(index);
                   cout<<"相关系数个数为："<<corr2_array.size()<<endl;
               } else{
                   //这里的逻辑应该是把第一个元素去掉 加进来第200号元素，变换
                   corr2_array.erase(corr2_array.begin());
                   cur=next.clone();// = capy clone 在内存地址的操作很不一样!!!十天
                   bool flag3= cap.read(frame);
                   cout<<"捕获下一帧:"<<flag3<<++frameNum<<endl;
                   ROI_next = frame(selection);
                   next=ROI_next;
                   corr2_array.push_back(corr2(cur,next));
                   //傅立叶变换
                   cout<<"相关系数个数为："<<corr2_array.size()<<endl;



                   ShowVec(corr2_array);
                   if   (!corr2_array.empty())   {
//                   DFT(corr2_array,dst,corr2_array.size());
                    }
//                   dft(corr2_array,dst,0,0);
//                   ShowVec(dst);
               }


      imshow("video", frame);
      image_origin=MatToQImage(frame);
      ui->label->setScaledContents(true);//很重要，通过这个设置可以使label自适应显示图片
      ui->label->setPixmap(QPixmap::fromImage(image_origin));//将视频显示到label上
      image=MatToQImage(cur);
      ui->label2->setScaledContents(true);//很重要，通过这个设置可以使label自适应显示图片
      ui->label2->setPixmap(QPixmap::fromImage(image));//将视频显示到label上


       if( waitKey(10) == 27 )//ESC键退出 ，一个while循环的运行事件和30ms相比是不是可以忽略？
           //考虑用定时器重构，但是很多架构都要改变

           stop = true;
          end=clock();
          std::cout<<"本次for循环使用时间为:"<<(double)(end -start)/CLOCKS_PER_SEC<<std::endl;
   }


   return 0;
}

vector<double> HR_cat_2::generateGaussianTemplate(vector<double> window, int center, double sigma){

    for(int i=0;i<window.size();i++){
       double m=window[i]*exp(-0.5*pow(i-center,2)/pow(sigma,2));
       m/=sigma*sqrt(2*PI);
       window[i]=m;
    }

return window;

}


void HR_cat_2::timerEvent(QTimerEvent *event) {
     if(event->timerId()== timerId)
     {//定时器到时间,//模拟数据填充
        // 模拟不停的接收到新数据
        QVector<QPointF> points;
        //先去除信号的直流成分再画图和处理

        for(int i=0;i<corr2_array.size();i++){

            points.append(QPointF(i,1-corr2_array[i]));
        }
        series->replace(points);



    }else if(event->timerId()== timerId_time){
         ui->curTime->setText(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ddd"));
     }else if(event->timerId()==timeId_fft){
         //完成fft原始赋值，和一些条件
         int size=corr2_array.size();
         //添加了规模判断以后不再闪退，可能是刚开始的null使得傅立叶失败
         vector<double> ave_corr2;
         double value=accumulate(corr2_array.begin(),corr2_array.end(),0.0)/size;
        for(int i=0;i<corr2_array.size();i++){
         ave_corr2.push_back(corr2_array[i]-value);
        }

         if(ave_corr2.size()>511){
         fftw_complex x[size];
         fftw_complex y[size];
         vector<double> amplitude;
         for(int i=0;i<size;i++){
             x[i][real]=ave_corr2[i];
             x[i][imag]=0;
         }
         //完成fft的内存开辟和fft变换
         fftw_plan plan= fftw_plan_dft_1d(size,x,y,FFTW_FORWARD,FFTW_ESTIMATE);
          fftw_execute(plan);
          fftw_destroy_plan(plan);
          fftw_cleanup();
          cout<<"FFT"<<endl;
          //用来测试傅立叶变换是否成功
          // for(int i=0;i<size;i++){
         // cout<<y[i][real]<<"---"<<y[i][imag]<<"i"<<endl;
          for(int i=0;i<size;i++){
              amplitude.push_back(sqrt(y[i][real]*y[i][real]+y[i][imag]*y[i][imag])/size);
          }
          vector<double> amplitude_1=generateGaussianTemplate(amplitude,13,2);
          vector<double>::iterator biggest = std::max_element(std::begin(amplitude_1), std::end(amplitude_1));
          cout << "Max element is " << *biggest<< " at position " << std::distance(std::begin(amplitude_1), biggest) << std::endl;
          double HR_rate= distance(std::begin(amplitude_1), biggest)*60*60/512;
          ui->HR->setText(QString::number(HR_rate,'f', 1));
          QVector<QPointF> points_fft;
          //先去除信号的直流成分再画图和处理

          for(int i=0;i<amplitude.size();i++){

              points_fft.append(QPointF(i,amplitude_1[i]));
          }
          series_fft->replace(points_fft);
          amplitude.clear();
          amplitude_1.clear();
          ave_corr2.clear();

}


     }


}




//所有的选择分支都可以用comboBox来做
void HR_cat_2::offline_video_dealing(){
    if(ui->comboBox->currentIndex()==1){
      //单独调用结构清晰，功能独立完成，具体在btn2_clicked中完成
       btn2_clicked();

    }else if(ui->comboBox->currentIndex()==2){
        //单独调用结构清晰，功能独立完成，具体在btn1_clicked中完成
         btn1_clicked();

      }
}

int HR_cat_2::btn1_clicked(){
    bool stop = false;
    Mat ROI,ROI_next;
    VideoCapture cap(0);

    if(!cap.isOpened())
    {
        return -1;
    }

    namedWindow("frame",0);
    setMouseCallback( "frame",  mouseWrapper, 0 );//消息响应机制

   //处理步骤应该在while循环中完成因为这是一个实时流的处理，而且应该是在提取到的roi矩阵部分操作
   //因为roi是在if中提取的所以要把流程定位在if中，后续步骤只是用来显示；

   //通道分离；channels用来存储三个颜色通道，mat用来承载需要的通道；
   //接下来的处理针对imageBlueChannel就可以了；
   std::vector<Mat> channels;
   Mat  imageBlueChannel;

   //用来存储相邻两针图像矩阵
   Mat cur,next;
   //**很重要！！！！*
   //这里原本是用index来做标示，超过200 开始傅立叶变换但是发现会闪退，可能是vector std库函数的原因
   //用了vecto.size()做标示，可以

   Mat tmp,last;
   last = NULL;
//   while(!stop)
//   {
//       cap.read(tmp);
//       cur = tmp(selection);
//       double cor;
//       if(corr2_array.size()!=0)
//       {
//           cor = corr2(last,cur);
//           corr2_array.push_back(cor);
//       }
//       else
//       {
//           corr2_array.push_back(1.0);
//       }
//       last = cur.clone();
//       cout<<cor<<endl;
//   }

   while(!stop)
   {
              clock_t start,end;
              start=clock();

               bool flag1= cap.read(frame);
               cout<<"捕获当前帧:"<<flag1<<endl;
               ROI = frame(selection);//这句话是将frame帧图片中的选中矩形区域的地址指向ROI，
               //对于内存而言，frame和ROI是公用内存的，所以下面这句实际
               //是将frame帧图像中的选中矩形区域块图像进行操作，而不是新创建
               //一个内存来进行操作
               //当然所截图的矩形区域ROI，可以使用imwrite函数来保存
               //这里只是为了显示的比较直观
               bitwise_not(ROI, ROI);//bitwise_not为将每一个bit位取反
               Mat mean_A;
               Mat stddev_1;
               meanStdDev(ROI,mean_A,stddev_1);
               float value_A =mean_A.at<double>(1);
               if(value_A==0)
               {
                 cout<<"请截图确定待测区域！"<<endl;
               }else if(value_A!=0 &&  corr2_array.size()==0){
                   cur=next=ROI;
                   corr2_array.push_back(1);
                   cout<<"the first 相关系数个数为："<<corr2_array.size()<<endl;
               }else if(value_A!=0 && corr2_array.size()>0 && corr2_array.size()<512){
                   cur=next.clone();// = capy clone 在内存地址的操作很不一样!!!十天
                   bool flag2= cap.read(frame);
                   cout<<"捕获下一帧:"<<flag2<<endl;
                   ROI_next = frame(selection);
                   next=ROI_next;
                   int index=corr2(cur,next);
                   corr2_array.push_back(index);
                   cout<<"相关系数个数为："<<corr2_array.size()<<endl;
               } else{
                   //这里的逻辑应该是把第一个元素去掉 加进来第200号元素，变换
                   corr2_array.erase(corr2_array.begin());
                   cur=next.clone();// = capy clone 在内存地址的操作很不一样!!!十天
                   bool flag3= cap.read(frame);
                   cout<<"捕获下一帧:"<<flag3<<endl;
                   ROI_next = frame(selection);
                   next=ROI_next;
                   corr2_array.push_back(corr2(cur,next));
                   //傅立叶变换
                   cout<<"相关系数个数为："<<corr2_array.size()<<endl;
                   //接下来准备做傅立叶
                   //减均值,先不做 容易导致corr2_array超载
}




      imshow("frame", frame);
      image_origin=MatToQImage(frame);
      ui->label->setScaledContents(true);//很重要，通过这个设置可以使label自适应显示图片
      ui->label->setPixmap(QPixmap::fromImage(image_origin));//将视频显示到label上
      image=MatToQImage(cur);
      ui->label2->setScaledContents(true);//很重要，通过这个设置可以使label自适应显示图片
      ui->label2->setPixmap(QPixmap::fromImage(image));//将视频显示到label上




      end=clock();
      std::cout<<"time is:"<<(double)(end -start)/CLOCKS_PER_SEC<<std::endl;
       if( waitKey(20) == 27 )//ESC键退出 ，一个while循环的运行事件和30ms相比是不是可以忽略？
           //考虑用定时器重构，但是很多架构都要改变
           stop = true;
   }


   return 0;
}


void HR_cat_2:: ShowVec(const vector<double>& valList)
{
    int count = valList.size();
     ofstream f("/Users/yanyupeng/Desktop/data.txt");//打开out.txt文件。
    for (int i = 0; i < count;i++)
    {
         // cout << valList[i] << endl;
            f << corr2_array[i] << endl; //写入每个元素，每个元素单独一行。
    }
}


HR_cat_2::~HR_cat_2()
{
    delete ui;
}



