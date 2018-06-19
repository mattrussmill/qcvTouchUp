#ifndef FILTERMENU_H
#define FILTERMENU_H

#include <QScrollArea>
#include <QVector>

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
    performImageBlur(QVector<int>);
    performImageSharpen(QVector<int>);
    performImageEdgeDetect(QVector<int>);
    performImageNoiseRemove(QVector<int>);
    cancelAdjustments();
    applyAdjustments();

private slots:
    void adjustSharpenSliderRange(int value);
    void adjustEdgeSliderRange(int value);
    void collectBlurParameters();
    void collectSharpenParameters();
    void collectEdgeDetectParameters();
    void changeSampleImage();

    void on_pushButton_Apply_released();

    void on_pushButton_Cancel_released();

private:
    Ui::FilterMenu *ui;
    QVector<int> menuValues;
};

#endif // FILTERMENU_H
