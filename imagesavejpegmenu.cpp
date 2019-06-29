#include "imagesavejpegmenu.h"
#include "ui_imagesavejpegmenu.h"
#include <opencv2/imgcodecs.hpp>

ImageSaveJpegMenu::ImageSaveJpegMenu(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ImageSaveJpegMenu)
{
    ui->setupUi(this);
}

ImageSaveJpegMenu::~ImageSaveJpegMenu()
{
    delete ui;
}
