#include "colorselectionwidget.h"
#include "ui_colorselectionwidget.h"

ColorSelectionWidget::ColorSelectionWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ColorSelectionWidget)
{
    ui->setupUi(this);
}

ColorSelectionWidget::~ColorSelectionWidget()
{
    delete ui;
}
