#include "mousewheeleatereventfilter.h"
#include "transformmenu.h"
#include "ui_transformmenu.h"

TransformMenu::TransformMenu(QWidget *parent) :
    QScrollArea(parent),
    ui(new Ui::TransformMenu)
{
    ui->setupUi(this);
    MouseWheelEaterEventFilter *wheelFilter = new MouseWheelEaterEventFilter(this);

    //fix radio buttons to work in separate group boxes (for asthetics)
    buttonGroup_m = new QButtonGroup(this);
    buttonGroup_m->addButton(ui->radioButton_CropEnable);
    buttonGroup_m->addButton(ui->radioButton_RotateEnable);
    buttonGroup_m->addButton(ui->radioButton_ScaleEnable);
    buttonGroup_m->addButton(ui->radioButton_WarpEnable);

    //setup crop menu options

    //setup rotate menu options

    //setup scale menu options

    //setup warp menu options
}

TransformMenu::~TransformMenu()
{
    delete ui;
}

void TransformMenu::initializeMenu()
{
    blockSignals(true);

    //reset signals


    blockSignals(false);
}

//overloads setVisible to signal the worker thread to cancel any adjustments that weren't applied when minimized
void TransformMenu::setVisible(bool visible)
{
    if(!visible)
        initializeMenu();
    QWidget::setVisible(visible);
}
