#include "colorslicemenu.h"
#include "ui_colorslicemenu.h"

ColorSliceMenu::ColorSliceMenu(QWidget *parent) :
    QScrollArea(parent),
    ui(new Ui::ColorSliceMenu)
{
    ui->setupUi(this);
}

ColorSliceMenu::~ColorSliceMenu()
{
    delete ui;
}
