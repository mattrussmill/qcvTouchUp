/***********************************************************************
* FILENAME :    adjustmenu.h
*
* DESCRIPTION :
*       This menu is used for adjusting the color, brightness, contrast,
*       pixel depth, hue, saturation, intensity including high and low
*       adjustments to gamma correction. It collects all necessary slider
*       values in a dynamically allocated array and passes the address to
*       that array via a Qt signal to provide the necessary parameters for
*       image processing.
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

namespace Ui {
class AdjustMenu;
}

class AdjustMenu : public QScrollArea
{
    Q_OBJECT

public:
    explicit AdjustMenu(QWidget *parent = 0);
    ~AdjustMenu();

public slots:
    void initializeSliders();
    void setVisible(bool visible) override;

signals:
    performImageAdjustments(float*);
    cancelAdjustments();
    applyAdjustments();


private:

    Ui::AdjustMenu *ui;
    float *sliderValues;


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
