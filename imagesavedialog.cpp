#include "imagesavedialog.h"
#include "ui_imagesavedialog.h"
#include <QFileDialog>

#include <QGridLayout>
#include <QDebug>
#include <QHBoxLayout>

ImageSaveDialog::ImageSaveDialog(QImage &image, QWidget *parent)
    : QFileDialog(parent)
{
    setOption(QFileDialog::DontUseNativeDialog);
    setAcceptMode(QFileDialog::AcceptSave);
    setNameFilter("All Files (*);;Bitmap (*.bmp *.dib);;JPEG(*.jpeg *.jpg *.jpe);;PNG (*.png)");
    appendImageOptionsWidget(nullptr);


}

ImageSaveDialog::ImageSaveDialog(QImage &image, QWidget *parent, const QString &caption, const QString &directory)
    : QFileDialog (parent, caption, directory, "All Files (*);;Bitmap (*.bmp *.dib);;JPEG(*.jpeg *.jpg *.jpe);;PNG (*.png)")
{
    //must set to not use native dialog so that we can access the dialog's layout
    setOption(QFileDialog::DontUseNativeDialog);
    setAcceptMode(QFileDialog::AcceptSave);
    appendImageOptionsWidget(nullptr);
}

ImageSaveDialog::~ImageSaveDialog()
{

}

void ImageSaveDialog::appendImageOptionsWidget(QWidget *widget)
{
    QGridLayout* mainLayout = dynamic_cast <QGridLayout*>(this->layout());

    if(!mainLayout){
        qDebug()<<"mainLayout is unavailable";
    }else{

        QHBoxLayout *hbl =new QHBoxLayout(this);
        QPushButton *pb;
        pb =new QPushButton(QString("My checkbox"));
        hbl->addWidget(pb);
        int num_rows = mainLayout->rowCount();
        qDebug()<<"num_rows: "<<num_rows;
        mainLayout->addLayout(hbl, num_rows, 0, 1, -1);
    }

}
