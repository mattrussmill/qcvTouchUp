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
    virtual void setColor(const QColor *color);

signals:
    void colorSelected(QColor);

protected slots:
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;

private:
    void paintCursorOnPalette();
    Ui::ColorSelectionWidget *ui;
    QImage valueDisplay_m;
    QPoint palettePoint_m;
    QColor selectedColor_m;

private slots:
    void populateColorValues();
    void setColorFromPalette();
    void setColorFromRGB();
    void setColorFromHSV();
    void setColorFromHTML();
    void setCursorFromColor();
};

#endif // COLORSELECTIONWIDGET_H
