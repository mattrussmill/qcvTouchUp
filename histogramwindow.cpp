/***********************************************************************
* FILENAME :    histogramwindow.cpp
*
* DESCRIPTION :
*       This widget plots a graph of HistogramWidget in a dialog box.
*
* NOTES :
*       Associated with histogramwindow.ui
*
* AUTHOR :  Matthew R. Miller       START DATE :    Feburary 7, 2018
*
* CHANGES : N/A - N/A
*
* VERSION       DATE            WHO                     DETAIL
* 0.1           02/07/2018      Matthew R. Miller       Initial Rev
*
************************************************************************/
#include "histogramwindow.h"
#include "ui_histogramwindow.h"
#include <QString>
#include <QImage>
#include <QLabel>
#include <QColor>
#include <QHBoxLayout>

/* Constructor initializes the dialog with a blank plot of the histogram */
HistogramWindow::HistogramWindow(QWidget *parent) :
    QDialog(parent, Qt::WindowCloseButtonHint), ui(new Ui::HistogramWindow)
{
    ui->setupUi(this);
    ui->hw->setBackgroundColor(QColor(0xa0, 0xa0, 0xa0));
    ui->hw->setBackgroundRole(QPalette::Dark);
    this->setWindowTitle("qcvTouchUp - Histogram of Preview");
}

/* Constructor initializes the dialog generating a plot from the passed image
 * address. If the image address is null, it generates a blank plot.*/
HistogramWindow::HistogramWindow(QImage &image, QWidget *parent) :
    QDialog(parent, Qt::WindowCloseButtonHint), ui(new::Ui::HistogramWindow)
{
    ui->setupUi(this);
    ui->hw->setBackgroundColor(QColor(0xa0, 0xa0, 0xa0));
    ui->hw->setBackgroundRole(QPalette::Dark);
    this->setWindowTitle("qcvTouchUp - Histogram of Preview");
    if(&image != nullptr)
    {
        ui->hw->setLineWidth(3.5);
        ui->hw->setClickable(true);
        ui->hw->setHistogramData(image);
        ui->label_yMax->setText("(0, " + QString::number(ui->hw->getLargestPeak()) + ")");
    }
    else
    {
        QLabel *notifier = new QLabel("No Data Available", ui->hw);
        notifier->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        notifier->setAlignment(Qt::AlignCenter);
        QHBoxLayout *layout = new QHBoxLayout(ui->hw);
        layout->addWidget(notifier);
        ui->hw->setLayout(layout);
    }
}

HistogramWindow::~HistogramWindow()
{
    delete ui;
}
