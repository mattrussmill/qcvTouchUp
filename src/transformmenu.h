#ifndef TRANSFORMMENU_H
#define TRANSFORMMENU_H

#include <QScrollArea>
#include <QVector>
class QButtonGroup;
class QRect;

namespace Ui {
class TransformMenu;
}

class TransformMenu : public QScrollArea
{
    Q_OBJECT

public:
    explicit TransformMenu(QWidget *parent = 0);
    ~TransformMenu();
    enum ParameterIndex
    {
        Affine      = 0,
        Perspective = 1
    };

public slots:
    void initializeMenu();
    void setImageResolution(QRect imageSize);
    void setImageROI(QRect ROI);
    void setVisible(bool visible) override;

signals:
    void enableCropImage(bool); //mainwindow sends ROI to here, when performImageCrop is emitted, then sends the ROI value to worker when apply is selected.
    void performImageCrop(QRect ROI); //if same size as image nothing happens. -> after apply is hit this is released
    void giveImageROI(QRect ROI);
    void setGetCoordinateMode(uint);
    void cancelRoiSelection();
    void performImageRotate(int);
    void setAutoCropOnRotate(bool);
    void giveScaledRect(QRect ROI); //unused

private:
    bool boundCheck(const QRect &ROI);
    Ui::TransformMenu *ui;
    QButtonGroup *buttonGroup_m;
    QRect imageSize_m;
    QRect croppedROI_m;
    QVector<int> menuValues_m;

private slots:
    void setSelectInImage(bool checked);
    void setImageInternalROI();
    void resendImageRotateSignal();
    void setImageInternalScale(int); //unused (useCroppedROI?)
};

#endif // TRANSFORMMENU_H
