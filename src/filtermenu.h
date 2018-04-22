#ifndef FILTERMENU_H
#define FILTERMENU_H

#include <QScrollArea>

namespace Ui {
class FilterMenu;
}

class FilterMenu : public QScrollArea
{
    Q_OBJECT

public:
    explicit FilterMenu(QWidget *parent = 0);
    ~FilterMenu();

public slots:
    void initializeMenu();
//    void setVisible(bool visible) override;

signals:
    performImageBlur(int*);
    performImageSharpen(int*);
    performImageEdgeDetect(int*);
    performImageNoiseRemove(int*);
    performImageReconstruct(int*);
    cancelAdjustments();
    applyAdjustments();

private slots:
    void collectBlurParameters();

    void on_comboBox_Smooth_currentIndexChanged(int index);

private:
    Ui::FilterMenu *ui;
    int *menuValues;
};

#endif // FILTERMENU_H
