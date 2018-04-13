/***********************************************************************
* FILENAME :    imagewidget.h
*
* DESCRIPTION :
*       This widget creates a scrollable area in which a QImage can be
*       viewed and resized without distortion. The area can also return
*       a selected point from within the image (scaled appropriately)
*       to be used in selecting colors within the displayed image.
*
* NOTES :
*       QWidget was subclassed to ImageWidget instead of QScrollArea so
*       that most of the QScrollArea members remain encapsulated and
*       private.
*
* AUTHOR :  Matthew R. Miller       START DATE :    January 7, 2018
*
* CHANGES : N/A - N/A
*
* VERSION       DATE            WHO                     DETAIL
* 0.1           01/22/2018      Matthew R. Miller       Initial Rev
*
************************************************************************/

#ifndef IMAGEWIDGET_H
#define IMAGEWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QScrollArea>
#include <QImage>
#include <QMenu>
#include <QPoint>
#include <QMutex>

class ImageWidget : public QWidget
{
    Q_OBJECT

public:
    ImageWidget(QWidget *parent = nullptr);
    QPoint lastPointSelected() const;
    Qt::ScrollBarPolicy verticalScrollBarPolicy() const;
    Qt::ScrollBarPolicy horizontalScrollBarPolicy() const;
    void setVerticalScrollBarPolicy(Qt::ScrollBarPolicy sbp = Qt::ScrollBarAsNeeded);
    void setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy sbp = Qt::ScrollBarAsNeeded);
    void setImage(const QImage &image);
    void setFillWidget(bool fill = true);
    double currentScale() const;
    bool imageAttached() const;
    bool fillWidgetStatus() const;
    const QImage* displayedImage();
    void setMutex(QMutex &m);


signals:
    void imageSet();
    void imageCleared();
    void imageNull();
    void imagePointSelected(QPoint selectedPoint);
    void fillWidgetChanged(bool fillScrollArea);

public slots:
    void setImage(const QImage *image);
    void clearImage();
    void zoomIn();
    void zoomOut();
    void zoomFit();
    void zoomActual();
    void updateDisplayedImage();
    void updateDisplayedImage(const QImage *image);


protected:
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void contextMenuEvent(QContextMenuEvent *event) override;

private:
    QPoint selectedPoint;
    QAction *zoomInAction;
    QAction *zoomOutAction;
    QAction *zoomFitAction;
    QAction *zoomActualAction;
    QLabel *imageLabel;
    QScrollArea *scrollArea;
    QMutex *mutex;
    const QImage *attachedImage = nullptr;
    float scalar;
    bool fillScrollArea = true;
    bool retrieveColor = false;
};

#endif // IMAGEWIDGET_H
