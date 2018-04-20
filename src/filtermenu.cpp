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
}

FilterMenu::~FilterMenu()
{
    delete ui;
}
