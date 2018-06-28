#include "temperaturemenu.h"
#include "ui_temperaturemenu.h"

TemperatureMenu::TemperatureMenu(QWidget *parent) :
    QScrollArea(parent),
    ui(new Ui::TemperatureMenu)
{
    ui->setupUi(this);
}

TemperatureMenu::~TemperatureMenu()
{
    delete ui;
}
