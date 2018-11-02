#include "transformmenu.h"
#include "ui_transformmenu.h"

TransformMenu::TransformMenu(QWidget *parent) :
    QScrollArea(parent),
    ui(new Ui::TransformMenu)
{
    ui->setupUi(this);
}

TransformMenu::~TransformMenu()
{
    delete ui;
}
