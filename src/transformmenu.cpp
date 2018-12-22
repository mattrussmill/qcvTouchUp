#include "mousewheeleatereventfilter.h"
#include "mouseclickdetectoreventfilter.h"
#include "transformmenu.h"
#include "imagewidget.h"
#include "ui_transformmenu.h"
#include <QButtonGroup>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QRect>
#include <QPoint>
#include <QString>

#include <QDebug>

TransformMenu::TransformMenu(QWidget *parent) :
    QScrollArea(parent),
    ui(new Ui::TransformMenu)
{
    ui->setupUi(this);
    MouseWheelEaterEventFilter *wheelFilter = new MouseWheelEaterEventFilter(this);
    MouseClickDetectorEventFilter *cropClickFilter = new MouseClickDetectorEventFilter(this);
    MouseClickDetectorEventFilter *rotateClickFilter = new MouseClickDetectorEventFilter(this);

    //fix radio buttons to work in separate group boxes (for asthetics)
    buttonGroup_m = new QButtonGroup(this);
    buttonGroup_m->addButton(ui->radioButton_CropEnable);
    buttonGroup_m->addButton(ui->radioButton_RotateEnable);
    buttonGroup_m->addButton(ui->radioButton_ScaleEnable);
    buttonGroup_m->addButton(ui->radioButton_WarpEnable);

    //setup crop menu options
    ui->lineEdit_CropRoiStart->installEventFilter(cropClickFilter);
    ui->lineEdit_CropRoiEnd->installEventFilter(cropClickFilter);
    connect(cropClickFilter, SIGNAL(clickDetected(bool)), ui->radioButton_CropEnable, SLOT(setChecked(bool)));
    connect(ui->radioButton_CropEnable, SIGNAL(toggled(bool)), this, SLOT(setSelectInImage(bool)));
    connect(ui->radioButton_CropEnable, SIGNAL(toggled(bool)), ui->line_Crop, SLOT(setVisible(bool)));
    connect(ui->lineEdit_CropRoiStart, SIGNAL(textEdited(QString)), this, SLOT(setImageInternalROI()));
    connect(ui->lineEdit_CropRoiEnd, SIGNAL(textEdited(QString)), this, SLOT(setImageInternalROI()));

    //setup rotate menu options
    ui->horizontalSlider_Rotate->installEventFilter(wheelFilter);
    ui->spinBox_RotateDegrees->installEventFilter(wheelFilter);
    ui->spinBox_RotateDegrees->installEventFilter(rotateClickFilter);
    //DOES NOTHING, NO EVENT FOR MOUSE? CHAGE TO FOCUS DETECTOR? <- PROBABLY THAT
    connect(rotateClickFilter, SIGNAL(clickDetected(bool)), ui->radioButton_RotateEnable, SLOT(setChecked(bool)));
    //silent enable signal slot necessary
    connect(ui->spinBox_RotateDegrees, SIGNAL(valueChanged(int)), ui->horizontalSlider_Rotate, SLOT(setValue(int)));
    connect(ui->horizontalSlider_Rotate, SIGNAL(valueChanged(int)), ui->spinBox_RotateDegrees, SLOT(setValue(int)));

    imageSize_m = QRect(-1, -1, -1, -1);
    initializeMenu();

    //NOTE: rotate and warp will need silentEnable like in Filter
}

TransformMenu::~TransformMenu()
{
    delete ui;
}

void TransformMenu::initializeMenu()
{
    blockSignals(true);

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
        ui->lineEdit_CropRoiEnd->setText("("+ QString::number(imageSize_m.width()) +", "
                                         + QString::number(imageSize_m.height()) +")");

        //scale
        ui->spinBox_ScaleHeight->setValue(imageSize_m.height());
        ui->spinBox_ScaleWidth->setValue(imageSize_m.width());
    }

    //crop
    ui->label_CropInstruction->setText("");
    ui->line_Crop->setVisible(false);

    //rotate
    ui->horizontalSlider_Rotate->setValue(0);
    ui->spinBox_RotateDegrees->setValue(0);

    //scale
    ui->checkBox_ScaleLinked->setChecked(true);

    //warp (needs more)

    blockSignals(false);
    emit setGetCoordinateMode(ImageWidget::CoordinateMode::NoClick);
}

// setImageResolution sets the default image resolution for initializing menu items
void TransformMenu::setImageResolution(QRect imageSize)
{
    imageSize_m = imageSize;
    initializeMenu();
}

//boundCheck checks that the ROI passed to it is within the image. True if yes, false if out of bounds
bool TransformMenu::boundCheck(const QRect &ROI)
{
    int x1, y1, x2, y2;
    ROI.getCoords(&x1, &y1, &x2, &y2);
    if(x1 >= x2 || y1 >= y2)
        return false;
    else if(x1 < 0 || y1 < 0 || x2 > imageSize_m.width() || y2 > imageSize_m.height())
        return false;
    return true;
}

/* setImageROI is a public slot that when signaled, sets the menu line edits and stored ROI values
 * from an external source. It is assumed the ROI is correct on arrival and no further checks are made*/
void TransformMenu::setImageROI(QRect ROI)
{
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
    if(!visible)
        initializeMenu();
    QWidget::setVisible(visible);
}

/* setSelectInImage is an internal method used to emit the appropriate signals to enable and disable region selection
 * through emitting the proper signals to the ImageWidget object*/
void TransformMenu::setSelectInImage(bool checked)
{
    if(checked)
    {
        emit setGetCoordinateMode(ImageWidget::CoordinateMode::RectROI);
        ui->label_CropInstruction->setText("Select ROI");
    }
    else
    {
        emit setGetCoordinateMode(ImageWidget::CoordinateMode::NoClick);
        ui->label_CropInstruction->setText("");
        emit cancelRoiSelection();
    }
}

