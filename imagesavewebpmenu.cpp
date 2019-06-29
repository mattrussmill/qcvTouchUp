#include "imagesavewebpmenu.h"
#include "ui_imagesavewebpmenu.h"
#include <opencv2/imgcodecs.hpp>

ImageSaveWebpMenu::ImageSaveWebpMenu(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ImageSaveWebpMenu)
{
    ui->setupUi(this);
}

ImageSaveWebpMenu::~ImageSaveWebpMenu()
{
    delete ui;
}
