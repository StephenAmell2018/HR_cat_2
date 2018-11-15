#ifndef HR_CAT_2_H
#define HR_CAT_2_H

#include <QMainWindow>

namespace Ui {
class HR_cat_2;
}

class HR_cat_2 : public QMainWindow
{
    Q_OBJECT

public:
    explicit HR_cat_2(QWidget *parent = 0);
    ~HR_cat_2();

private:
    Ui::HR_cat_2 *ui;
};

#endif // HR_CAT_2_H
