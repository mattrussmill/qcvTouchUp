#ifndef COLORSLICEMENU_H
#define COLORSLICEMENU_H

#include <QScrollArea>
class QButtonGroup;
class QBitmap;
class QImage;

namespace Ui {
class ColorSliceMenu;
}

class ColorSliceMenu : public QScrollArea
{
    Q_OBJECT

public:
    explicit ColorSliceMenu(QWidget *parent = 0);
    ~ColorSliceMenu();

public slots:
    void initializeMenu();
    void setImageReference(const QImage *image);

signals:
    void performColorSlice(int); //worker will calculate the offset from the hue selected and shift all masked pix so the averate matches the selection
    void setCursorBrushRadius(int);
    void setSelectionToWand(bool);
    void setSelectionToBrush(bool);
    void cancelPainting();
    void paintMask(QPixmap*);
    void displaySelectionOverlay(bool);

private:
    void selectNeighborsOfPoint(int hueVariance);
    void checkAndUpdateOverlayState();
    Ui::ColorSliceMenu *ui;
    QButtonGroup *buttonGroup_m;
    QBitmap *imageMask_m;
    const QImage *imageReference_m;
    bool displaySelectionOverlayState_m;
};

#endif // COLORSLICEMENU_H
