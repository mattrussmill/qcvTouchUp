/***********************************************************************
* FILENAME :    histogramwindow.cpp
*
* LICENSE:
*       qcvTouchUp provides an image processing toolset for editing
*       photographs, purposed and packaged for use in a desktop application 
*       user environment. Copyright (C) 2018,  Matthew R. Miller
*
*       This program is free software: you can redistribute it and/or modify
*       it under the terms of the GNU General Public License as published by 
*       the Free Software Foundation (version 3 of the License) and the 
*       3-clause BSD License as agreed upon through the use of the Qt toolkit
*       and OpenCV libraries in qcvTouchUp development, respectively. Copies
*       of the appropriate license files for qcvTouchup, and its source code,
*       can be found in LICENSE.Qt.txt and LICENSE.CV.txt.
*
*       This program is distributed in the hope that it will be useful,
*       but WITHOUT ANY WARRANTY; without even the implied warranty of
*       MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*       GNU General Public License for more details.
*
*       You should have received a copy of the GNU General Public License and
*       3-clause BSD License along with this program.  If not, please see 
*       <http://www.gnu.org/licenses/> and <https://opencv.org/license.html>.
*    
*       If you wish to contact the developer about this project, please do so
*       through their account at <https://github.com/mattrussmill>
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
