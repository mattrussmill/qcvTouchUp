#include "imagesavedialog.h"
#include "imagesavejpegmenu.h"
#include "imagesavepngmenu.h"
#include "imagesavewebpmenu.h"
#include <QStackedWidget>
#include <QGridLayout>
#include <QPushButton>
#include <QDebug>
#include <opencv2/imgcodecs.hpp>

ImageSaveDialog::ImageSaveDialog(QImage &image, QWidget *parent)
    : QFileDialog(parent)
{
    setOption(QFileDialog::DontUseNativeDialog);
    setAcceptMode(QFileDialog::AcceptSave);
    setNameFilter("JPEG(*.jpeg *.jpg *.jpe);;PNG (*.png);;WebP (*.webp);;All Files (*)");
    appendAdvancedOptionsButton();

}

ImageSaveDialog::ImageSaveDialog(QImage &image, QWidget *parent, const QString &caption, const QString &directory)
    : QFileDialog (parent, caption, directory, "JPEG(*.jpeg *.jpg *.jpe);;PNG (*.png);;WebP (*.webp);;All Files (*)")
{
    //must set to not use native dialog so that we can access the dialog's layout
    setOption(QFileDialog::DontUseNativeDialog);
    setAcceptMode(QFileDialog::AcceptSave);
    appendAdvancedOptionsButton();
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

/* If the advanced options do not exist, create them. If they do exist,
 * toggle their visibility*/
void ImageSaveDialog::advancedOptionsToggled()
{
    if(!saveOptionsWidget_m)
    {
        appendImageOptionsWidget();
        buttonAdvancedOptions_m->setText("Hide...");
    }
    else if(!saveOptionsWidget_m->isVisible())
    {
        saveOptionsWidget_m->setVisible(true);
        buttonAdvancedOptions_m->setText("Hide...");
    }
    else
    {
        saveOptionsWidget_m->setVisible(false);
        buttonAdvancedOptions_m->setText("Advanced...");
    }
}

/* this method is used by the constructor and appends the "Advanced..." dialog
 * button to the dialog in the bottom right corner of the layout below "Cancel"*/
void ImageSaveDialog::appendAdvancedOptionsButton()
{
    //get main layout of QFileDialog
    QGridLayout* mainLayout = dynamic_cast<QGridLayout*>(this->layout());

    if(!mainLayout)
    {
        qDebug()<<"mainLayout is unavailable";
    }
    else
    {
        buttonAdvancedOptions_m = new QPushButton("Advanced...", this);
        int rows = mainLayout->rowCount();
        int columns = mainLayout->columnCount();
        mainLayout->addWidget(buttonAdvancedOptions_m, rows, columns - 1, 1, -1);

        //connect the button functionality
        connect(buttonAdvancedOptions_m, SIGNAL(released()), this, SLOT(advancedOptionsToggled()));
    }
}

/* this method generates and adds the advanced options widgets to the save dialog for the
 * supported image formats */
void ImageSaveDialog::appendImageOptionsWidget()
{
    //get main layout of QFileDialog
    QGridLayout* mainLayout = dynamic_cast<QGridLayout*>(this->layout());

    if(!mainLayout)
    {
        qDebug()<<"mainLayout is unavailable";
    }
    else
    {
        saveOptionsWidget_m = new QStackedWidget(this); //do not put in initialization list
        jpegMenu_m = new ImageSaveJpegMenu(this);
        pngMenu_m = new ImageSavePngMenu(this);
        webpMenu_m = new ImageSaveWebpMenu(this);
        saveOptionsWidget_m->addWidget(jpegMenu_m);
        saveOptionsWidget_m->addWidget(pngMenu_m);
        saveOptionsWidget_m->addWidget(webpMenu_m);
        int rows = mainLayout->rowCount();
        qDebug() << "save dialog g-layout rows: " << rows;
        mainLayout->addWidget(saveOptionsWidget_m, rows, 0, 1, -1);
    }
}
