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

    setupQuadraticDemo(ui->qcustomplot); //关联了动态

    connect(ui->btn1,SIGNAL(clicked()),this,SLOT(btn1_clicked()));//打开摄像头按钮
    connect(ui->btn2,SIGNAL(clicked()),this,SLOT(btn2_clicked()));
    connect(ui->comboBox,SIGNAL(currentIndexChanged(QString)),this,SLOT(offline_video_dealing()));
}



//画图函数、、可以将具体的处理逻辑写在这里，然后在按钮函数那里调用！！
void HR_cat_2::setupQuadraticDemo(QCustomPlot *customPlot)
{
  // generate some data:
  QVector<double> x(101), y(101); // initialize with entries 0..100
  for (int i=0; i<101; ++i)
  {
    x[i] = i/50.0 - 1; // x goes from -1 to 1
    y[i] = x[i]*x[i];  // let's plot a quadratic function
  }
  // create graph and assign data to it:
  ui->qcustomplot->addGraph();
  ui->qcustomplot->graph(0)->setData(x, y);
  // give the axes some labels:
  ui->qcustomplot->xAxis->setLabel("x");
  ui->qcustomplot->yAxis->setLabel("y");
  // set axes ranges, so we see all data:
  ui->qcustomplot->xAxis->setRange(-1, 1);
  ui->qcustomplot->yAxis->setRange(0, 1);
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

void HR_cat_2::DFT(double src[],Complex  dst[],int size){
//    clock_t start,end;
//    start=clock();

    for(int m=0;m<size;m++){
        double real=0.0;
        double imagin=0.0;
        for(int n=0;n<size;n++){
            double x=M_PI*2*m*n;
            real+=src[n]*cos(x/size);
            imagin+=src[n]*(-sin(x/size));
        }
        dst[m].imagin=imagin;
        dst[m].real=real;
        if(imagin>=0.0){
//            printf("%lf+%lfj\n",real,imagin);
        }
        else
        {
//            printf("%lf%lfj\n",real,imagin,sqrt(pow(dst[m].imagin,2)+pow(dst[m].real,2)));
        }
     printf("%lf\n",sqrt(pow(real,2)+pow(imagin,2)));
    }
//    end=clock();
//    printf("DFT use time :%lf for Datasize of:%d\n",(double)(end-start)/CLOCKS_PER_SEC,size);
}


int HR_cat_2::btn2_clicked(){

    VideoCapture cap;
    cap.open("/Users/yanyupeng/Desktop/speckleVideo/180106/greenSpeckleVideo/greenSpeckleVideo_0002.avi");
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
   vector<double> dst;
   //用来存储相邻两针图像矩阵
   Mat cur,next;
   //**很重要！！！！*
   //这里原本是用index来做标示，超过200 开始傅立叶变换但是发现会闪退，可能是vector std库函数的原因
   //用了vecto.size()做标示，可以

   Mat tmp,last;
   last = NULL;
   int frameNum=0;
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
               }else if(value_A!=0 && corr2_array.size()>0 && corr2_array.size()<600){
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
                   Complex dst[corr2_array.size()];
                   if   (!corr2_array.empty())   {
                   DFT(&corr2_array[0],dst,corr2_array.size());
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


       if( waitKey(100) == 27 )//ESC键退出 ，一个while循环的运行事件和30ms相比是不是可以忽略？
           //考虑用定时器重构，但是很多架构都要改变
           stop = true;
          end=clock();
          std::cout<<"time is:"<<(double)(end -start)/CLOCKS_PER_SEC<<std::endl;
   }
   return 0;
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
   vector<double> dst;
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
               }else if(value_A!=0 && corr2_array.size()>0 && corr2_array.size()<128){
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
                   Complex dst[corr2_array.size()];
                   if   (!corr2_array.empty())   {
                   DFT(&corr2_array[0],dst,corr2_array.size());
                    }
//                   dft(corr2_array,dst,0,0);
//                   ShowVec(dst);
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
    for (int i = 0; i < count;i++)
    {
        cout << valList[i] << endl;
    }
}

HR_cat_2::~HR_cat_2()
{
    delete ui;
}



