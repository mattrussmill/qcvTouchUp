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
    ui->horizontalSlider_SmoothWeight->installEventFilter(wheelFilter);
    connect(ui->radioButton_SmoothEnable, SIGNAL(released()), this, SLOT(collectBlurParameters()));
    connect(ui->comboBox_Smooth, SIGNAL(currentIndexChanged(int)), this, SLOT(collectBlurParameters()));
    connect(ui->horizontalSlider_SmoothWeight, SIGNAL(valueChanged(int)), this, SLOT(collectBlurParameters()));

    //setup sharpen menu options
    ui->comboBox_Sharpen->addItem("Unsharpen"); //comboBox index 0 (default)
    ui->comboBox_Sharpen->addItem("Laplacian"); //comboBox index 1
    ui->comboBox_Sharpen->installEventFilter(wheelFilter);
    ui->horizontalSlider_SharpenWeight->installEventFilter(wheelFilter);
    connect(ui->radioButton_SharpenEnable, SIGNAL(released()), this, SLOT(collectSharpenParameters()));
    connect(ui->comboBox_Sharpen, SIGNAL(currentIndexChanged(int)), this, SLOT(adjustSharpenSliderRange(int)));
    connect(ui->comboBox_Sharpen, SIGNAL(currentIndexChanged(int)), this, SLOT(collectSharpenParameters()));
    connect(ui->horizontalSlider_SharpenWeight, SIGNAL(valueChanged(int)), this, SLOT(collectSharpenParameters()));

    //setup edge detect menu options
    ui->comboBox_Edge->addItem("Canny");        //comboBox index 0 (default)
    ui->comboBox_Edge->addItem("Laplacian");    //comboBox index 1 - must match above
    ui->comboBox_Edge->addItem("Sobel");        //comboBox index 2
    ui->comboBox_Edge->installEventFilter(wheelFilter);
    connect(ui->radioButton_EdgeEnable, SIGNAL(released()), this, SLOT(collectEdgeDetectParameters()));
    connect(ui->comboBox_Edge, SIGNAL(currentIndexChanged(int)), this, SLOT(adjustEdgeSliderRange(int)));
    connect(ui->comboBox_Edge, SIGNAL(currentIndexChanged(int)), this, SLOT(collectEdgeDetectParameters()));
    connect(ui->horizontalSlider_EdgeWeight, SIGNAL(valueChanged(int)), this, SLOT(collectEdgeDetectParameters()));


    menuValues.resize(5);
}

FilterMenu::~FilterMenu()
{
    delete ui;
}

void FilterMenu::initializeMenu()
{

}

//Changes the slider range for the SharpenSlider based on the needs of the filter selected from the combo box.
void FilterMenu::adjustSharpenSliderRange(int value)
{
    //if not default filter (unsharpen), switch to maximum value of 50 for no gap in slider values
    if(value == FilterLaplacian)
    {
        ui->horizontalSlider_SharpenWeight->setValue(ui->horizontalSlider_SharpenWeight->value() / 2);
        ui->horizontalSlider_SharpenWeight->setMaximum(50);
        ui->horizontalSlider_SharpenWeight->setPageStep(2);
    }
    else
    {
        ui->horizontalSlider_SharpenWeight->setMaximum(100);
        ui->horizontalSlider_SharpenWeight->setValue(ui->horizontalSlider_SharpenWeight->value() * 2);
        ui->horizontalSlider_SharpenWeight->setPageStep(3);
    }
}

//Changes the slider range for the EdgeSlider based on the needs of the filter selected from the combo box.
void FilterMenu::adjustEdgeSliderRange(int value)
{
    //if canny filter, switch to minimum value to 1 (1*2+1=3) in passing slot for no gap in slider values
    if(value == FilterCanny)
    {
        ui->horizontalSlider_EdgeWeight->setValue(1);
        ui->horizontalSlider_EdgeWeight->setMinimum(1);
    }
    else
    {
        ui->horizontalSlider_EdgeWeight->setMinimum(0);
    }
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

//Populates the menuValues parameter and passes it to a worker slot for the Edge Detect operation.
void FilterMenu::collectEdgeDetectParameters()
{
    //if filter not enabled, do nothing
    if(!ui->radioButton_EdgeEnable->isChecked()) return;

    //sends values of 1/3/5/7 for opencv functions. Slider ranges from 0 to 3
    menuValues[KernelType] = ui->comboBox_Edge->currentIndex();
    menuValues[KernelWeight] = ui->horizontalSlider_EdgeWeight->value() * 2 + 1;

    emit performImageEdgeDetect(menuValues);
}
