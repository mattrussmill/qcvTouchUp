#ifndef COLORSELECTIONWIDGET_H
#define COLORSELECTIONWIDGET_H

#include <QWidget>
class QImage;

namespace Ui {
class ColorSelectionWidget;
}

class ColorSelectionWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ColorSelectionWidget(QWidget *parent = 0);
    ~ColorSelectionWidget();

private:
    Ui::ColorSelectionWidget *ui;
    QImage valueDisplay_m;
    QImage paletteDisplay_m;
};

#endif // COLORSELECTIONWIDGET_H
