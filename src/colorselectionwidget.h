#ifndef COLORSELECTIONWIDGET_H
#define COLORSELECTIONWIDGET_H

#include <QWidget>
class QImage;
class QPixmap;
class QColor;

namespace Ui {
class ColorSelectionWidget;
}

class ColorSelectionWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ColorSelectionWidget(QWidget *parent = 0);
    virtual ~ColorSelectionWidget();

public slots:
    virtual void setColor(QColor color);

private:
    Ui::ColorSelectionWidget *ui;
    QImage valueDisplay_m;
    QPixmap colorPreview_m;
};

#endif // COLORSELECTIONWIDGET_H
