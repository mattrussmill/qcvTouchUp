#ifndef COLORSELECTIONWIDGET_H
#define COLORSELECTIONWIDGET_H

#include <QWidget>
class QImage;
class QPixmap;
class QColor;
class QMouseEvent;
class QPoint;

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
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void setColor(const QColor *color);

private:
    void paintCursorOnPalette();
    Ui::ColorSelectionWidget *ui;
    QImage valueDisplay_m;
    QPoint palettePoint_m;
    QColor selectedColor_m;

private slots:
    void setColorFromPalette();
    void populateColorValues();
};

#endif // COLORSELECTIONWIDGET_H
