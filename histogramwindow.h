/***********************************************************************
* FILENAME :    histogramwindow.h
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
#ifndef HISTOGRAMWINDOW_H
#define HISTOGRAMWINDOW_H

#include <QDialog>

namespace Ui {
class HistogramWindow;
}

class HistogramWindow : public QDialog
{
    Q_OBJECT

public:
    HistogramWindow(QWidget *parent = 0);
    HistogramWindow(QImage &image, QWidget *parent = 0);
    ~HistogramWindow();
    void setHistogramData(QImage &image);

private:
    Ui::HistogramWindow *ui;
};

#endif // HISTOGRAMWINDOW_H
