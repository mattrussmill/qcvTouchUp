#ifndef COLORSELECTIONWIDGET_H
#define COLORSELECTIONWIDGET_H

#include <QColorDialog>

class ColorSelectionWidget : public QColorDialog
{
    Q_OBJECT
public:
    explicit ColorSelectionWidget(QWidget *parent = nullptr);

signals:

public slots:
};

#endif // COLORSELECTIONWIDGET_H
