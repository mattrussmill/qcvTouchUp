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
        FilterAverage   = 0,
        FilterGaussian  = 1,
        FilterMedian    = 2,
        FilterSharpen   = 0,
        FilterUnsharpen = 1,

        KernelType      = 0,
        KernelWeight    = 1



    };

public slots:
    void initializeMenu();
//    void setVisible(bool visible) override;

signals:
    performImageBlur(QVector<int>);
    performImageSharpen(QVector<int>);
    performImageEdgeDetect(QVector<int>);
    performImageNoiseRemove(QVector<int>);
    performImageReconstruct(QVector<int>);
    cancelAdjustments();
    applyAdjustments();

private slots:
    void collectBlurParameters();
    void collectSharpenParameters();

private:
    Ui::FilterMenu *ui;
    QVector<int> menuValues;
};

#endif // FILTERMENU_H
