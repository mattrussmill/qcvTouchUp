#include "imagesavepngmenu.h"
#include "ui_imagesavepngmenu.h"
#include <opencv2/imgcodecs.hpp>

ImageSavePngMenu::ImageSavePngMenu(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ImageSavePngMenu)
{
    ui->setupUi(this);
}

ImageSavePngMenu::~ImageSavePngMenu()
{
    delete ui;
}
