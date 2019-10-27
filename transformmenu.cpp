/***********************************************************************
* FILENAME :    transformmenu.cpp
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
#include "mousewheeleatereventfilter.h"
#include "focusindetectoreventfilter.h"
#include "transformmenu.h"
#include "imagewidget.h"
#include "transformworker.h"
#include "ui_transformmenu.h"
#include "imagelabel.h"
#include <cmath>
#include <QButtonGroup>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QRect>
#include <QPoint>
#include <QMessageBox>
#include <QString>

#include <QDebug>

/* Constructor installs the necessary filters for different objects, groups the buttons together
 * to easily search for the selected button, and establishes all signals/slots necessary.*/
TransformMenu::TransformMenu(QMutex *mutex, QWidget *parent) :
    QScrollArea(parent),
    ui(new Ui::TransformMenu)
{
    ui->setupUi(this);
    masterImage_m = nullptr;
    previewImage_m = nullptr;
    workerMutex_m = mutex;
    transformWorker_m = nullptr;

    MouseWheelEaterEventFilter *wheelFilter = new MouseWheelEaterEventFilter(this);
    FocusInDetectorEventFilter *cropFocusFilter = new FocusInDetectorEventFilter(this);
    FocusInDetectorEventFilter *rotateFocusFilter = new FocusInDetectorEventFilter(this);
    FocusInDetectorEventFilter *scaleFocusFilter = new FocusInDetectorEventFilter(this);

    //fix radio buttons to work in separate group boxes (for asthetics)
    buttonGroup_m = new QButtonGroup(this);
    buttonGroup_m->addButton(ui->radioButton_CropEnable);
    buttonGroup_m->addButton(ui->radioButton_RotateEnable);
    buttonGroup_m->addButton(ui->radioButton_ScaleEnable);

    //setup crop menu options
    ui->lineEdit_CropRoiStart->installEventFilter(cropFocusFilter);
    ui->lineEdit_CropRoiEnd->installEventFilter(cropFocusFilter);
    connect(cropFocusFilter, SIGNAL(focusDetected(bool)), ui->radioButton_CropEnable, SLOT(setChecked(bool)));
    connect(ui->radioButton_CropEnable, SIGNAL(toggled(bool)), this, SLOT(setSelectInImage(bool)));
    connect(ui->radioButton_CropEnable, SIGNAL(toggled(bool)), ui->line_Crop, SLOT(setVisible(bool)));
    connect(ui->lineEdit_CropRoiStart, SIGNAL(textEdited(QString)), this, SLOT(setImageInternalROI()));
    connect(ui->lineEdit_CropRoiEnd, SIGNAL(textEdited(QString)), this, SLOT(setImageInternalROI()));
    connect(ui->radioButton_CropEnable, SIGNAL(toggled(bool)), this, SLOT(changeSampleImage(bool)));

    //setup rotate menu options
    ui->horizontalSlider_Rotate->installEventFilter(wheelFilter);
    ui->spinBox_RotateDegrees->installEventFilter(wheelFilter);
    ui->horizontalSlider_Rotate->installEventFilter(rotateFocusFilter);
    ui->spinBox_RotateDegrees->installEventFilter(rotateFocusFilter);
    ui->checkBox_rotateAutoCrop->installEventFilter(rotateFocusFilter);
    connect(rotateFocusFilter, SIGNAL(focusDetected(bool)), ui->radioButton_RotateEnable, SLOT(setChecked(bool)));
    connect(ui->spinBox_RotateDegrees, SIGNAL(valueChanged(int)), ui->horizontalSlider_Rotate, SLOT(setValue(int)));
    connect(ui->horizontalSlider_Rotate, SIGNAL(valueChanged(int)), ui->spinBox_RotateDegrees, SLOT(setValue(int)));
    connect(ui->spinBox_RotateDegrees, SIGNAL(valueChanged(int)), this, SLOT(resendImageRotateSignal()));
    connect(ui->checkBox_rotateAutoCrop, SIGNAL(toggled(bool)), this, SIGNAL(setAutoCropOnRotate(bool)));
    connect(ui->checkBox_rotateAutoCrop, SIGNAL(toggled(bool)), this, SLOT(resendImageRotateSignal()));
    connect(ui->radioButton_RotateEnable, SIGNAL(toggled(bool)), this, SLOT(changeSampleImage(bool)));

    //setup scale menu options
    ui->spinBox_ScaleHeight->installEventFilter(wheelFilter);
    ui->spinBox_ScaleWidth->installEventFilter(wheelFilter);
    ui->checkBox_ScaleLinked->installEventFilter(scaleFocusFilter);
    ui->spinBox_ScaleHeight->installEventFilter(scaleFocusFilter);
    ui->spinBox_ScaleWidth->installEventFilter(scaleFocusFilter);
    connect(scaleFocusFilter, SIGNAL(focusDetected(bool)), ui->radioButton_ScaleEnable, SLOT(setChecked(bool)));
    connect(scaleFocusFilter, SIGNAL(focusDetected(bool)), this, SLOT(setImageInterSizeOnFocusIn(bool)));
    connect(ui->radioButton_ScaleEnable, SIGNAL(toggled(bool)), ui->label_ScaleInstruction, SLOT(setVisible(bool)));
    connect(ui->radioButton_ScaleEnable, SIGNAL(toggled(bool)), ui->line_Scale, SLOT(setVisible(bool)));
    connect(ui->spinBox_ScaleHeight, SIGNAL(valueChanged(int)), this, SLOT(setImageInternalSizeHeight(int)));
    connect(ui->spinBox_ScaleWidth, SIGNAL(valueChanged(int)), this, SLOT(setImageInternalSizeWidth(int)));
    connect(ui->spinBox_ScaleHeight, SIGNAL(editingFinished()), this, SLOT(performImageScalePreview()));
    connect(ui->spinBox_ScaleWidth, SIGNAL(editingFinished()), this, SLOT(performImageScalePreview()));
    connect(ui->radioButton_ScaleEnable, SIGNAL(toggled(bool)), this, SLOT(changeSampleImage(bool)));

    imageSize_m = QRect(-1, -1, -1, -1);
    initializeSliders();

    //NOTE: rotate and warp will need silentEnable like in Filter
}

//autogenerated destructor
TransformMenu::~TransformMenu()
{
    if(transformWorker_m)
    {
        worker_m.terminate();
        worker_m.wait();
        delete transformWorker_m;
        transformWorker_m = nullptr;
    }
    delete ui;
}

/* This slot is used to update the member addresses for the master and preview images stored
 * in the parent object. If the Mat's become empty in the parent object this slot
 * should be signaled with nullptrs to signify they are empty. */
void TransformMenu::receiveImageAddresses(const cv::Mat *masterImage, cv::Mat *previewImage)
{
    masterImage_m = masterImage;
    previewImage_m = previewImage;
    qDebug() << "Filter Menu Images:" << masterImage_m << previewImage_m;
    emit distributeImageBufferAddresses(masterImage, previewImage);
}

// Enables or disables tracking for the appropriate menu widgets
void TransformMenu::setMenuTracking(bool enable)
{
    ui->horizontalSlider_Rotate->setTracking(enable);
}

// Function initializes the necessary widget values to their starting values.
void TransformMenu::initializeSliders()
{
    ui->lineEdit_CropRoiStart->blockSignals(true);
    ui->lineEdit_CropRoiEnd->blockSignals(true);
    ui->spinBox_ScaleHeight->blockSignals(true);
    ui->spinBox_ScaleWidth->blockSignals(true);
    ui->horizontalSlider_Rotate->blockSignals(true);
    ui->spinBox_RotateDegrees->blockSignals(true);
    ui->checkBox_rotateAutoCrop->blockSignals(true);
    ui->checkBox_ScaleLinked->blockSignals(true);

    //reinitialize buttons to unchecked
    QAbstractButton *checkedButton = buttonGroup_m->checkedButton();
    if(checkedButton != nullptr)
    {
        buttonGroup_m->setExclusive(false);
        checkedButton->setChecked(false);
        buttonGroup_m->setExclusive(true);
    }

    //if not initialized with image's dimensions, init with defaults
    if(imageSize_m == QRect(-1, -1, -1, -1))
    {
        //crop
        ui->lineEdit_CropRoiStart->setText("(x1, y1)");
        ui->lineEdit_CropRoiEnd->setText("(x2, y2)");

        //scale
        ui->spinBox_ScaleHeight->setValue(0);
        ui->spinBox_ScaleWidth->setValue(0);
    }
    else
    {
        //crop
        ui->lineEdit_CropRoiStart->setText("(0, 0)");
        ui->lineEdit_CropRoiEnd->setText("("+ QString::number(imageSize_m.bottomRight().x()) +", "
                                         + QString::number(imageSize_m.bottomRight().y()) +")");

        //scale
        ui->spinBox_ScaleHeight->setValue(imageSize_m.height());
        ui->spinBox_ScaleWidth->setValue(imageSize_m.width());
        ui->label_RatioWidth->setText("100%");
        ui->label_RatioHeight->setText("100%");
    }

    //crop
    ui->label_CropInstruction->setText("");
    ui->line_Crop->setVisible(false);

    //rotate
    ui->horizontalSlider_Rotate->setValue(0);
    ui->spinBox_RotateDegrees->setValue(0);

    //scale
    ui->label_ScaleInstruction->setText("Return to Preview");
    ui->label_ScaleInstruction->setVisible(false);
    ui->line_Scale->setVisible(false);
    ui->checkBox_ScaleLinked->setChecked(true);
    blockSignals(false);

    //signal must be emitted on reset
    ui->checkBox_rotateAutoCrop->setChecked(false);

    ui->lineEdit_CropRoiStart->blockSignals(false);
    ui->lineEdit_CropRoiEnd->blockSignals(false);
    ui->spinBox_ScaleHeight->blockSignals(false);
    ui->spinBox_ScaleWidth->blockSignals(false);
    ui->horizontalSlider_Rotate->blockSignals(false);
    ui->spinBox_RotateDegrees->blockSignals(false);
    ui->checkBox_rotateAutoCrop->blockSignals(false);
    ui->checkBox_ScaleLinked->blockSignals(false);

    ui->label_SampleImage->setPixmap(QPixmap::fromImage(QImage(":/img/icons/masterIcons/rgb.png")));
    emit setGetCoordinateMode(ImageLabel::CoordinateMode::NoClick);
}

// setImageResolution sets the default image resolution for initializing menu items
void TransformMenu::setImageResolution(const QRect &imageSize)
{
    imageSize_m = imageSize;
    initializeSliders();
}

//boundCheck checks that the ROI passed to it is within the image. True if yes, false if out of bounds
bool TransformMenu::boundCheck(const QRect &ROI)
{
    int x1, y1, x2, y2;
    ROI.getCoords(&x1, &y1, &x2, &y2);
    if(x1 >= x2 || y1 >= y2)
        return false;
    else if(x1 < 0 || y1 < 0 || x2 > imageSize_m.bottomRight().x() || y2 > imageSize_m.bottomRight().y())
        return false;
    return true;
}

/* setImageROI is a public slot that when signaled, sets the menu line edits and stored ROI values
 * from an external source.*/
void TransformMenu::setImageROI(QRect ROI)
{
    //if out of bounds, fix bounds
    if(ROI.topLeft().y() < 0) ROI.setTopLeft(QPoint(ROI.topLeft().x(), 0));
    if(ROI.topLeft().x() < 0) ROI.setTopLeft(QPoint(0, ROI.topLeft().y()));
    if(ROI.bottomRight().y() > imageSize_m.bottomRight().y()) ROI.setBottomRight(
                QPoint(ROI.bottomRight().x(), imageSize_m.bottomRight().y()));
    if(ROI.bottomRight().x() > imageSize_m.bottomRight().x()) ROI.setBottomRight(
                QPoint(imageSize_m.bottomRight().x(), ROI.bottomRight().y()));

    ui->lineEdit_CropRoiStart->setText("("+ QString::number(ROI.topLeft().x()) +", "
                                       + QString::number(ROI.topLeft().y()) +")");


    ui->lineEdit_CropRoiEnd->setText("("+ QString::number(ROI.bottomRight().x()) +", "
                                     + QString::number(ROI.bottomRight().y()) +")");

    ui->label_CropInstruction->setText("Select ROI");

    croppedROI_m = ROI;
    emit performImageCrop(ROI);
}

/* setImageInternalROI is a private slot that when signaled, pulls the string from the crop line edits
 * and checks them for correctness using regular expressions. If the sequence is correct, and the values
 * extracted in bounds, the croppedROI_m member is updated and giveImageROI emitted which is intended to
 * update the widget used to paint the selected region on the displayed image.*/
void TransformMenu::setImageInternalROI()
{
    QRegularExpression re("^\\((\\d+),\\s?(\\d+)\\)$");
    QRegularExpressionMatch rectTopLeft = re.match(ui->lineEdit_CropRoiStart->text());
    QRegularExpressionMatch rectBottomRight = re.match(ui->lineEdit_CropRoiEnd->text());

    if(rectTopLeft.hasMatch() && rectBottomRight.hasMatch())
    {
        //extract values from match
        QRect ROI(QPoint(rectTopLeft.captured(1).toInt(), rectTopLeft.captured(2).toInt()),
                  QPoint(rectBottomRight.captured(1).toInt(), rectBottomRight.captured(2).toInt()));

        if(boundCheck(ROI) && boundCheck(ROI))
        {
            ui->label_CropInstruction->setText("Select ROI");
            croppedROI_m = ROI;
            qDebug() << ROI;
            emit giveImageROI(ROI);
            emit performImageCrop(ROI);
        }
        else
        {
            ui->label_CropInstruction->setText("Invalid ROI");
        }
    }
    else
    {
        ui->label_CropInstruction->setText("Invalid ROI");
    }

}

//overloads setVisible to signal the worker thread to cancel any adjustments that weren't applied when minimized
void TransformMenu::setVisible(bool visible)
{
    manageWorker(visible);
    QWidget::setVisible(visible);
}

//overloads show event to initialize the visible menu widgets before being seen
void TransformMenu::showEvent(QShowEvent *event)
{
    initializeSliders();
    QWidget::showEvent(event);
}

/* setSelectInImage is an internal method used to emit the appropriate signals to enable and disable region selection
 * through emitting the proper signals to the ImageWidget object*/
void TransformMenu::setSelectInImage(bool checked)
{
    if(checked)
    {
        emit setGetCoordinateMode(ImageLabel::CoordinateMode::RectROI);
        ui->label_CropInstruction->setText("Select ROI");
    }
    else
    {
        emit setGetCoordinateMode(ImageLabel::CoordinateMode::NoClick);
        ui->label_CropInstruction->setText("");
        emit cancelRoiSelection();
    }
}

//sends a signal to perform the image rotate operation. Intended to send value again to kick off operation @ toggle event
void TransformMenu::resendImageRotateSignal()
{
    workRotateSignalSuppressor.receiveNewData(ui->spinBox_RotateDegrees->value());
}

/* setImageInternalSizeHeight sets the member storing the desired Scaled Image Size according to the new value in
 * spinBox_ScaleHeight. If the ScaleLinked checkbox is checked, the width is dynamically adjusted to maintain the
 * aspect ratio of the image. The labels indicating the % the image is scaled are also updated. */
void TransformMenu::setImageInternalSizeHeight(int height)
{
    int width = ui->spinBox_ScaleWidth->value();
    if(ui->checkBox_ScaleLinked->isChecked())
    {
        double ratio = height / static_cast<double>(imageSize_m.height());
        width = round(imageSize_m.width() * ratio);
        ui->spinBox_ScaleWidth->blockSignals(true);
        ui->spinBox_ScaleWidth->setValue(width);
        ui->spinBox_ScaleWidth->blockSignals(false);
    }
    ui->label_RatioWidth->setText(QString::number(width / static_cast<double>(imageSize_m.width()) * 100, 'f', 2) + '%');
    ui->label_RatioHeight->setText(QString::number(height / static_cast<double>(imageSize_m.height()) * 100, 'f', 2) + '%');
}

/* setImageInternalSizeWidth sets the member storing the desired Scaled Image Size according to the new value in
 * spinBox_ScaleWidth. If the ScaleLinked checkbox is checked, the width is dynamically adjusted to maintain the
 * aspect ratio of the image. The labels indicating the % the image is scaled are also updated. */
void TransformMenu::setImageInternalSizeWidth(int width)
{
    int height = ui->spinBox_ScaleHeight->value();
    if(ui->checkBox_ScaleLinked->isChecked())
    {
        double ratio = width / static_cast<double>(imageSize_m.width());
        height = round(imageSize_m.height() * ratio);
        ui->spinBox_ScaleHeight->blockSignals(true);
        ui->spinBox_ScaleHeight->setValue(height);
        ui->spinBox_ScaleHeight->blockSignals(false);
    }
    ui->label_RatioWidth->setText(QString::number(width / static_cast<double>(imageSize_m.width()) * 100, 'f', 2) + '%');
    ui->label_RatioHeight->setText(QString::number(height / static_cast<double>(imageSize_m.height()) * 100, 'f', 2) + '%');
}

/* Emits the values stored in the spinboxes associated with scaling/resizing the image. This should be
 * connected to the FocusIn event on any Scale Image widgets*/
void TransformMenu::setImageInterSizeOnFocusIn(bool focus)
{
    if(focus)
    {
        croppedROI_m.setWidth(ui->spinBox_ScaleWidth->value());
        croppedROI_m.setHeight(ui->spinBox_ScaleHeight->value());
    }
}

// member function that allows access to the currently stored size of the new desired image for a Scale operation
QRect TransformMenu::getSizeOfScale() const
{
    return croppedROI_m;
}

/* Preloads the Scale signal suppressor. This should only be called from this object when previewing the image. A different
 * trigger will signal the worker thread to perform the final image transformation so it is not doing extra work each
 * time a value is adjusted*/
void TransformMenu::performImageScalePreview()
{
    workScaleSignalSuppressor.receiveNewData(QRect(0,0, ui->spinBox_ScaleWidth->value(), ui->spinBox_ScaleHeight->value()));

    //emit performImageScale(QRect(0,0, ui->spinBox_ScaleWidth->value(), ui->spinBox_ScaleHeight->value()));
}

//Sets the sample image based on the menu item selected, and signals the displaying object to reset the image shown.
void TransformMenu::changeSampleImage(bool detected)
{
    //check so only enabled button is called
    if(detected)
    {
        if(ui->radioButton_CropEnable->isChecked())
            ui->label_SampleImage->setPixmap(QPixmap::fromImage(QImage(":/img/icons/transformMenu/crop.png")));
        else if(ui->radioButton_ScaleEnable->isChecked())
            ui->label_SampleImage->setPixmap(QPixmap::fromImage(QImage(":/img/icons/transformMenu/scale.png")));
        else if(ui->radioButton_RotateEnable->isChecked())
            ui->label_SampleImage->setPixmap(QPixmap::fromImage(QImage(":/img/icons/transformMenu/rotate.png")));
        emit displayMaster();
    }
}

//displays a warning dialog box with the appropriate message if an exception is caught within opencv operations
void TransformMenu::exceptionDialog(QString message)
{
    QMessageBox::warning(this, "Error", message);
}

/* This method determines when the worker thread should be created or destroyed so
 * that the worker thread (with event loop) is only running if it is required (in
 * this case if the menu is visible). This thread manages the creation, destruction,
 * connection, and disconnection of the thread and its signals / slots.*/
void TransformMenu::manageWorker(bool life)
{
    if(life)
    {
        if(!transformWorker_m)
        {
            //If worker is still trying to exit, wait and process other events until its done
            if(worker_m.isRunning())
            {
                QApplication::setOverrideCursor(Qt::WaitCursor);
                qDebug() << "Waiting for thread to exit";
                while(!worker_m.isFinished())
                {
                    QApplication::processEvents(QEventLoop::AllEvents, 100);
                }
                QApplication::restoreOverrideCursor();
            }

            transformWorker_m = new TransformWorker(masterImage_m, previewImage_m, workerMutex_m);
            transformWorker_m->moveToThread(&worker_m);
            //signal slot connections (might be able to do them in constructor?)
            connect(this, SIGNAL(distributeImageBufferAddresses(const cv::Mat*,cv::Mat*)), transformWorker_m, SLOT(receiveImageAddresses(const cv::Mat*, cv::Mat*)));
            connect(&workRotateSignalSuppressor, SIGNAL(suppressedSignal(SignalSuppressor*)), transformWorker_m, SLOT(receiveRotateSuppressedSignal(SignalSuppressor*)));
            connect(&workScaleSignalSuppressor, SIGNAL(suppressedSignal(SignalSuppressor*)), transformWorker_m, SLOT(receiveScaleSuppressedSignal(SignalSuppressor*)));
            //other worker signals slots
            connect(transformWorker_m, SIGNAL(updateDisplayedImage()), this, SIGNAL(updateDisplayedImage()));
            connect(transformWorker_m, SIGNAL(updateStatus(QString)), this, SIGNAL(updateStatus(QString)));
            connect(this, SIGNAL(performImageCrop(QRect)), transformWorker_m, SLOT(doCropComputation(QRect)));
            connect(this, SIGNAL(setAutoCropOnRotate(bool)), transformWorker_m, SLOT(setAutoCropForRotate(bool)));
            connect(transformWorker_m, SIGNAL(handleExceptionMessage(QString)), this, SLOT(exceptionDialog(QString)));
            worker_m.start();
        }
    }
    else
    {
        //while the worker event loop is running, tell it to delete itself once loop is empty.
        if(transformWorker_m)
        {
            /* All signals to and from the object are automatically disconnected (string based, not functor),
             * and any pending posted events for the object are removed from the event queue. This is done incase functor signal/slots used later*/
            disconnect(this, SIGNAL(distributeImageBufferAddresses(const cv::Mat*,cv::Mat*)), transformWorker_m, SLOT(receiveImageAddresses(const cv::Mat*, cv::Mat*)));
            disconnect(&workRotateSignalSuppressor, SIGNAL(suppressedSignal(SignalSuppressor*)), transformWorker_m, SLOT(receiveRotateSuppressedSignal(SignalSuppressor*)));
            disconnect(&workScaleSignalSuppressor, SIGNAL(suppressedSignal(SignalSuppressor*)), transformWorker_m, SLOT(receiveScaleSuppressedSignal(SignalSuppressor*)));
            //other worker signals slots
            disconnect(transformWorker_m, SIGNAL(updateDisplayedImage()), this, SIGNAL(updateDisplayedImage()));
            disconnect(transformWorker_m, SIGNAL(updateStatus(QString)), this, SIGNAL(updateStatus(QString)));
            disconnect(this, SIGNAL(performImageCrop(QRect)), transformWorker_m, SLOT(doCropComputation(QRect)));
            disconnect(this, SIGNAL(setAutoCropOnRotate(bool)), transformWorker_m, SLOT(setAutoCropForRotate(bool)));
            disconnect(transformWorker_m, SIGNAL(handleExceptionMessage(QString)), this, SLOT(exceptionDialog(QString)));
            transformWorker_m->deleteLater();
            transformWorker_m = nullptr;
            worker_m.quit();
        }
    }
}

