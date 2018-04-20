#include "filtermenu.h"
#include "ui_filtermenu.h"

FilterMenu::FilterMenu(QWidget *parent) :
    QScrollArea(parent),
    ui(new Ui::FilterMenu)
{
    ui->setupUi(this);
    ui->comboBox_Smooth->addItem("Average");
    ui->comboBox_Smooth->addItem("Gaussian");
    ui->comboBox_Smooth->addItem("Median");
    ui->comboBox_Smooth->addItem("Bilateral");

    menuValues = new float [5];
}

FilterMenu::~FilterMenu()
{
    if(menuValues)
        delete menuValues;
    delete ui;
}

void FilterMenu::on_pushButton_Smooth_released()
{

}
