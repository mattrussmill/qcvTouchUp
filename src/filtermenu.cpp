#include "mousewheeleatereventfilter.h"
#include "filtermenu.h"
#include "ui_filtermenu.h"
#include <QDebug>


FilterMenu::FilterMenu(QWidget *parent) :
    QScrollArea(parent),
    ui(new Ui::FilterMenu)
{
    ui->setupUi(this);
    MouseWheelEaterEventFilter *wheelFilter = new MouseWheelEaterEventFilter(this);

    //setup smooth menu options
    ui->comboBox_Smooth->addItem("Average");    //comboBox index 0 (default)
    ui->comboBox_Smooth->addItem("Gaussian");   //comboBox index 1
    ui->comboBox_Smooth->addItem("Median");     //comboBox index 2
    ui->comboBox_Smooth->installEventFilter(wheelFilter);
    ui->horizontalSlider_SmoothRadius->installEventFilter(wheelFilter);
    ui->horizontalSlider_SmoothWeight->installEventFilter(wheelFilter);
    connect(ui->radioButton_SmoothEnable, SIGNAL(released()), this, SLOT(collectBlurParameters()));
    connect(ui->comboBox_Smooth, SIGNAL(currentIndexChanged(int)), this, SLOT(collectBlurParameters()));
    connect(ui->horizontalSlider_SmoothRadius, SIGNAL(valueChanged(int)), this, SLOT(collectBlurParameters()));
    connect(ui->horizontalSlider_SmoothWeight, SIGNAL(valueChanged(int)), this, SLOT(collectBlurParameters()));

    //setup sharpen menu options
    ui->comboBox_Sharpen->addItem("Sharpen");   //comboBox index 0 (default)
    ui->comboBox_Sharpen->addItem("Unsharpen"); //comboBox index 1

    menuValues.resize(5);
}

FilterMenu::~FilterMenu()
{
    delete ui;
}

void FilterMenu::initializeMenu()
{

}

//Populates the menuValues parameter and passes it to a worker slot for the Smooth operation.
void FilterMenu::collectBlurParameters()
{
    //if filter not enabled, do nothing
    if(!ui->radioButton_SmoothEnable->isChecked()) return;

    menuValues[KernelType] = ui->comboBox_Smooth->currentIndex();
    menuValues[KernelRadius] = ui->horizontalSlider_SmoothRadius->value() | 1; //r must be odd
    menuValues[KernelWeight] = ui->horizontalSlider_SmoothWeight->value();

    emit performImageBlur(menuValues);
}

//Sets the SmoothWeight (sigma) slider to be disabled when Gaussian Blur is not selected.
void FilterMenu::on_comboBox_Smooth_currentIndexChanged(int index)
{
    if(index == FilterGaussian) // 1
        ui->horizontalSlider_SmoothWeight->setEnabled(true);
    else
        ui->horizontalSlider_SmoothWeight->setEnabled(false);
}
