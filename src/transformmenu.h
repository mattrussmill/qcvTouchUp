/***********************************************************************
* FILENAME :    transformmenu.h
*
* LICENSE:
*       qcvTouchUp provides an image processing toolset for editing
*       photographs, purposed and packaged for use in a desktop application
*       user environment. Copyright (C) 2018,  Matthew R. Miller
*
*       This program is free software: you can redistribute it and/or modify
*       it under the terms of the GNU General Public License as published by
*       the Free Software Foundation (version 3 of the License) and the
*       3-clause BSD License as agreed upon through the use of the Qt toolkit
*       and OpenCV libraries in qcvTouchUp development, respectively. Copies
*       of the appropriate license files for qcvTouchup, and its source code,
*       can be found in LICENSE.Qt.txt and LICENSE.CV.txt.
*
*       This program is distributed in the hope that it will be useful,
*       but WITHOUT ANY WARRANTY; without even the implied warranty of
*       MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*       GNU General Public License for more details.
*
*       You should have received a copy of the GNU General Public License and
*       3-clause BSD License along with this program.  If not, please see
*       <http://www.gnu.org/licenses/> and <https://opencv.org/license.html>.
*
*       If you wish to contact the developer about this project, please do so
*       through their account at <https://github.com/mattrussmill>
*
* DESCRIPTION :
*       This menu is used for image transformations and warping operations.
*       These operations are, and should be, the only operations in this
*       application that changes the size of the destination image.
*
* NOTES :
*
* AUTHOR :  Matthew R. Miller       START DATE :    December 10, 2018
*
* CHANGES : N/A - N/A
*
* VERSION       DATE            WHO                     DETAIL
* 0.1           01/25/2018      Matthew R. Miller       Initial Rev
*
************************************************************************/
#ifndef TRANSFORMMENU_H
#define TRANSFORMMENU_H

#include <QScrollArea>
#include <QThread>
#include "signalsuppressor.h"
#include <opencv2/core.hpp>
class QString;
class QButtonGroup;
class QRect;
class TransformWorker;

namespace Ui {
class TransformMenu;
}

class TransformMenu : public QScrollArea
{
    Q_OBJECT

public:
    explicit TransformMenu(QMutex *mutex, QWidget *parent = 0);
    ~TransformMenu();
    enum ParameterIndex
    {
        Affine      = 0,
        Perspective = 1
    };
    QRect getSizeOfScale() const;

public slots:
    void initializeSliders();
    void receiveImageAddresses(const cv::Mat *masterImage, cv::Mat *previewImage);
    void setMenuTracking(bool enable);
    void setImageResolution(QRect imageSize);
    void setImageROI(QRect ROI);
    void setVisible(bool visible) override;
    void showEvent(QShowEvent *event) override;

signals:
    void updateDisplayedImage();
    void distributeImageBufferAddresses(const cv::Mat*, cv::Mat*);
    void updateStatus(QString);
    void enableCropImage(bool); //mainwindow sends ROI to here, when performImageCrop is emitted, then sends the ROI value to worker when apply is selected.
    void performImageCrop(QRect ROI); //if same size as image nothing happens. -> after apply is hit this is released
    void giveImageROI(QRect ROI);
    void setGetCoordinateMode(uint);
    void cancelRoiSelection();
    void setAutoCropOnRotate(bool);
    void performImageScale(QRect size);

protected:
    const cv::Mat *masterImage_m;
    cv::Mat *previewImage_m;
    QMutex *workerMutex_m;
    QThread worker_m;
    TransformWorker *transformWorker_m;

protected slots:
    void manageWorker(bool life);

private:
    bool boundCheck(const QRect &ROI);
    Ui::TransformMenu *ui;
    QButtonGroup *buttonGroup_m;
    QRect imageSize_m;
    QRect croppedROI_m;
    QVector<int> menuValues_m;
    SignalSuppressor workSignalSuppressor;

private slots:
    void setSelectInImage(bool checked);
    void setImageInternalROI();
    void resendImageRotateSignal();
    void setImageInternalSizeWidth(int width);
    void setImageInternalSizeHeight(int height);
    void setImageInterSizeOnFocusIn(bool focus);
    void performImageScalePreview();
    void changeSampleImage(bool detected);
};

#endif // TRANSFORMMENU_H
