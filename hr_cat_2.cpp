#include "hr_cat_2.h"
#include "ui_hr_cat_2.h"

HR_cat_2::HR_cat_2(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::HR_cat_2)
{
    ui->setupUi(this);
}

HR_cat_2::~HR_cat_2()
{
    delete ui;
}
