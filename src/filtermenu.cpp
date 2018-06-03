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
    //ui->horizontalSlider_SmoothRadius->installEventFilter(wheelFilter);
    ui->horizontalSlider_SmoothWeight->installEventFilter(wheelFilter);
    connect(ui->radioButton_SmoothEnable, SIGNAL(released()), this, SLOT(collectBlurParameters()));
    connect(ui->comboBox_Smooth, SIGNAL(currentIndexChanged(int)), this, SLOT(collectBlurParameters()));
   //connect(ui->horizontalSlider_SmoothRadius, SIGNAL(valueChanged(int)), this, SLOT(collectBlurParameters()));
    connect(ui->horizontalSlider_SmoothWeight, SIGNAL(valueChanged(int)), this, SLOT(collectBlurParameters()));

    //setup sharpen menu options
    ui->comboBox_Sharpen->addItem("Unsharpen"); //comboBox index 0 (default)
    ui->comboBox_Sharpen->addItem("Laplacian"); //comboBox index 1
    ui->comboBox_Sharpen->installEventFilter(wheelFilter);
    ui->horizontalSlider_SharpenWeight->installEventFilter(wheelFilter);
    connect(ui->radioButton_SharpenEnable, SIGNAL(released()), this, SLOT(collectSharpenParameters()));
    connect(ui->comboBox_Sharpen, SIGNAL(currentIndexChanged(int)), this, SLOT(collectSharpenParameters()));
    connect(ui->horizontalSlider_SharpenWeight, SIGNAL(valueChanged(int)), this, SLOT(collectSharpenParameters()));

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
    menuValues[KernelWeight] = ui->horizontalSlider_SmoothWeight->value();

    emit performImageBlur(menuValues);
}

//Populates the menuValues parameter and passes it to a worker slot for the Sharpen operation.
void FilterMenu::collectSharpenParameters()
{
    //if filter not enabled, do nothing
    if(!ui->radioButton_SharpenEnable->isChecked()) return;

    menuValues[KernelType] = ui->comboBox_Sharpen->currentIndex();
    menuValues[KernelWeight] = ui->horizontalSlider_SharpenWeight->value();

    emit performImageSharpen(menuValues);
}
