/***********************************************************************
* FILENAME :    filtermenu.h
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
*       This menu is used for filtering the image through 2D kernels by
*       performing high and low pass filtering. This filtering includes
*       sharpening, smoothing/blurring, and edge detection. It collects
*       all necessary slider values in a QVector and copies the object
*       through the signal/signal mechanism to provide the necessary
*       parameters for image processing.
*
*
* NOTES :
*       This class is directly tied to filtermenu.ui and has functionality
*       tied to the ImageWorker class which uses the values selected here
*       as a basis to control calculations on the image.
*
*
* AUTHOR :  Matthew R. Miller       START DATE :    April 04/16/2018
*
* CHANGES : N/A - N/A
*
* VERSION       DATE            WHO                     DETAIL
* 0.1           06/23/2018      Matthew R. Miller       Initial Rev
*
************************************************************************/

#ifndef FILTERMENU_H
#define FILTERMENU_H

#include <QScrollArea>
#include <QVector>
class QButtonGroup;

namespace Ui {
class FilterMenu;
}

class FilterMenu : public QScrollArea
{
    Q_OBJECT

public:
    explicit FilterMenu(QWidget *parent = 0);
    ~FilterMenu();
    enum ParameterIndex
    {
        FilterAverage       = 0,
        FilterGaussian      = 1,
        FilterMedian        = 2,

        FilterUnsharpen     = 0,
        FilterLaplacian     = 1,

        FilterCanny         = 0,
        FilterSobel         = 2,

        KernelType          = 0,
        KernelWeight        = 1
    };

public slots:
    void initializeSliders();
    void setVisible(bool visible) override;

signals:
    void performImageBlur(QVector<int>);
    void performImageSharpen(QVector<int>);
    void performImageEdgeDetect(QVector<int>);
    void performImageNoiseRemove(QVector<int>);

private slots:
    void adjustSharpenSliderRange(int value);
    void adjustEdgeSliderRange(int value);
    void collectBlurParameters();
    void collectSharpenParameters();
    void collectEdgeDetectParameters();
    void changeSampleImage();

private:
    Ui::FilterMenu *ui;
    QVector<int> menuValues_m;
    QButtonGroup *buttonGroup_m;
};

#endif // FILTERMENU_H
