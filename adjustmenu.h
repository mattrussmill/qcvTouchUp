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
