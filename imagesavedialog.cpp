#include "imagesavedialog.h"
#include "imagesavejpegmenu.h"
#include "imagesavepngmenu.h"
#include "imagesavewebpmenu.h"
#include <QStackedLayout>
#include <QGridLayout>
#include <QDebug>

ImageSaveDialog::ImageSaveDialog(QImage &image, QWidget *parent)
    : QFileDialog(parent)
{
    setOption(QFileDialog::DontUseNativeDialog);
    setAcceptMode(QFileDialog::AcceptSave);
    setNameFilter("All Files (*);;Bitmap (*.bmp *.dib);;JPEG(*.jpeg *.jpg *.jpe);;PNG (*.png)");
    appendImageOptionsWidget();


}

ImageSaveDialog::ImageSaveDialog(QImage &image, QWidget *parent, const QString &caption, const QString &directory)
    : QFileDialog (parent, caption, directory, "All Files (*);;JPEG(*.jpeg *.jpg *.jpe);;PNG (*.png);;WebP (*.webp)")
{
    //must set to not use native dialog so that we can access the dialog's layout
    setOption(QFileDialog::DontUseNativeDialog);
    setAcceptMode(QFileDialog::AcceptSave);
    appendImageOptionsWidget();
}

ImageSaveDialog::~ImageSaveDialog()
{

}

void ImageSaveDialog::saveJPEG(QImage &image)
{
    //https://docs.opencv.org/3.1.0/d4/da8/group__imgcodecs.html
    //qiality // set default 95
}

void ImageSaveDialog::savePNG(QImage &image)
{
    //compression level // set default 3
}

void ImageSaveDialog::saveBitmap(QImage &image)
{

}

void ImageSaveDialog::appendImageOptionsWidget()
{
    QGridLayout* mainLayout = dynamic_cast<QGridLayout*>(this->layout());

    if(!mainLayout)
    {
        qDebug()<<"mainLayout is unavailable";
    }
    else
    {
        saveOptionsLayout_m = new QStackedLayout(this); //do not put in initialization list
        //webpUi_m = new Ui::ImageSaveWebpMenu; ??
        //saveOptionsLayout_m->addWidget(webpMenu_m);
        int rows = mainLayout->rowCount();
        qDebug() << "save dialog g-layout rows: " << rows;
        mainLayout->addLayout(saveOptionsLayout_m, rows, 0, 1, -1);
    }

}
