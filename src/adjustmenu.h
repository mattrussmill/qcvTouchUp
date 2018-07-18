/***********************************************************************
* FILENAME :    adjustmenu.h
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
*       This menu is used for adjusting the color, brightness, contrast,
*       pixel depth, hue, saturation, intensity including high and low
*       adjustments to gamma correction. It collects all necessary slider
*       values in a QVector and copies the object through the signal/signal
*       mechanism to provide the necessary parameters for image processing.
*       The slider values are primed here as necessary for the worker thread.
*
* NOTES :
*       This class is directly tied to adjustmenu.ui and has functionality
*       tied to the ImageWorker class which uses the values selected here
*       as a basis to control calculations on the image.
*
*
* AUTHOR :  Matthew R. Miller       START DATE :    January 02/15/2018
*
* CHANGES : N/A - N/A
*
* VERSION       DATE            WHO                     DETAIL
* 0.1           04/18/2018      Matthew R. Miller       Initial Rev
*
************************************************************************/

#ifndef ADJUSTMENU_H
#define ADJUSTMENU_H

#include <QScrollArea>
#include <QVector>

namespace Ui {
class AdjustMenu;
}

class AdjustMenu : public QScrollArea
{
    Q_OBJECT

public:
    explicit AdjustMenu(QWidget *parent = 0);
    ~AdjustMenu();
    enum ParameterIndex
    {
        Brightness  = 0,
        Contrast    = 1,
        Depth       = 2,
        Hue         = 3,
        Saturation  = 4,
        Intensity   = 5,
        Gamma       = 6,
        Highlight   = 7,
        Shadows     = 8,
        Color       = 9
    };

public slots:
    void initializeSliders();
    void setVisible(bool visible) override;

signals:
    void performImageAdjustments(QVector<float>);
    void cancelAdjustments();
    void applyAdjustments();


private:

    Ui::AdjustMenu *ui;
    QVector<float> sliderValues;


private slots:
    void on_horizontalSlider_Contrast_valueChanged(int value);
    void on_horizontalSlider_Brightness_valueChanged(int value);
    void on_horizontalSlider_Depth_valueChanged(int value);
    void on_horizontalSlider_Hue_valueChanged(int value);
    void on_horizontalSlider_Saturation_valueChanged(int value);
    void on_horizontalSlider_Intensity_valueChanged(int value);
    void on_pushButton_Cancel_released();
    void on_radioButton_Color_released();
    void on_radioButton_Grayscale_released();
    void on_pushButton_Apply_released();
    void on_horizontalSlider_Gamma_valueChanged(int value);
    void on_horizontalSlider_Highlight_valueChanged(int value);
    void on_horizontalSlider_Shadows_valueChanged(int value);
};

#endif // ADJUSTMENU_H
