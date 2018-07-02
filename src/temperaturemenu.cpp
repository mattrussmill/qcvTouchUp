#include "temperaturemenu.h"
#include "ui_temperaturemenu.h"
#include <QString>
#include <QRadioButton>
#include <QDebug>

TemperatureMenu::TemperatureMenu(QWidget *parent) :
    QScrollArea(parent),
    ui(new Ui::TemperatureMenu)
{
    ui->setupUi(this);

    connect(ui->radioButton_1000K, SIGNAL(clicked(bool)), this, SLOT(moveSliderToButton(bool)));
    connect(ui->radioButton_2000K, SIGNAL(clicked(bool)), this, SLOT(moveSliderToButton(bool)));
    connect(ui->radioButton_3000K, SIGNAL(clicked(bool)), this, SLOT(moveSliderToButton(bool)));
    connect(ui->radioButton_4000K, SIGNAL(clicked(bool)), this, SLOT(moveSliderToButton(bool)));
    connect(ui->radioButton_5000K, SIGNAL(clicked(bool)), this, SLOT(moveSliderToButton(bool)));
    connect(ui->radioButton_6000K, SIGNAL(clicked(bool)), this, SLOT(moveSliderToButton(bool)));
    connect(ui->radioButton_7000K, SIGNAL(clicked(bool)), this, SLOT(moveSliderToButton(bool)));
    connect(ui->radioButton_8000K, SIGNAL(clicked(bool)), this, SLOT(moveSliderToButton(bool)));
    connect(ui->radioButton_9000K, SIGNAL(clicked(bool)), this, SLOT(moveSliderToButton(bool)));
    connect(ui->radioButton_10000K, SIGNAL(clicked(bool)), this, SLOT(moveSliderToButton(bool)));

    connect(ui->horizontalSlider_Temperature, SIGNAL(valueChanged(int)), this, SLOT(selectRadioButtonFromSlider(int)));

}

TemperatureMenu::~TemperatureMenu()
{
    delete ui;
}

/* If adjustSlider is true (QRadioButton enabled), the function retrieves the sender's name
 * and prunes its name such that only the numeric representation of the temperature the radio
 * button is associated with remains. That value is converted to an integer and moves the
 * temperature slider appropriately.*/
void TemperatureMenu::moveSliderToButton(bool adjustSlider)
{
    if(adjustSlider)
    {
        //retrieve sender name and prune object name to only the number
        QString senderName(sender()->objectName());
        senderName.chop(1);
        senderName.remove(0,12);
        ui->horizontalSlider_Temperature->setValue(senderName.toInt() / 100);
    }
    qDebug() << "Button Moved Slider";
}


void TemperatureMenu::selectRadioButtonFromSlider(int value)
{


}
