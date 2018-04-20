#ifndef FILTERMENU_H
#define FILTERMENU_H

#include <QScrollArea>

namespace Ui {
class FilterMenu;
}

class FilterMenu : public QScrollArea
{
    Q_OBJECT

public:
    explicit FilterMenu(QWidget *parent = 0);
    ~FilterMenu();

private slots:
    void on_pushButton_Smooth_released();

private:
    Ui::FilterMenu *ui;
    float *menuValues;
};

#endif // FILTERMENU_H
