#include "imagesavedialog.h"
#include "imagesavejpegmenu.h"
#include "imagesavepngmenu.h"
#include "imagesavewebpmenu.h"
#include <QStackedWidget>
#include <QGridLayout>
#include <QPushButton>
#include <QVector>
#include <QString>
#include <QMessageBox>
#include <QRegularExpression>
#include <QDebug>

ImageSaveDialog::ImageSaveDialog(cv::Mat &image, QWidget *parent, const QString &caption, const QString &directory)
    : QFileDialog (parent, caption, directory, "JPEG(*.jpeg *.jpg *.jpe);;PNG (*.png);;WebP (*.webp);;All Files (*)"),
      image_m(&image)
{
    //must set to not use native dialog so that we can access the dialog's layout
    setOption(QFileDialog::DontUseNativeDialog);
    setAcceptMode(QFileDialog::AcceptSave);
    appendAdvancedOptionsButton();
    connect(this, SIGNAL(accepted()), this, SLOT(saveAccepted()));
}

ImageSaveDialog::~ImageSaveDialog()
{

}

/* method saves the file as a jpg image according to OpenCV 3.3.2, using the
 * parameters from webpMenu - default if not specified.*/
void ImageSaveDialog::saveJPEG(QString &filePath)
{
    /* if menu was not able to be appended to the dialog (and thus not added)
     * create it to acquire its default values */
    if(!jpegMenu_m)
    {
        jpegMenu_m = new ImageSaveJpegMenu(this);
        jpegMenu_m->setVisible(false);
    }
    QVector<int> saveParameters;
    saveParameters.append(cv::IMWRITE_JPEG_QUALITY);
    saveParameters.append(jpegMenu_m->getQuality());
    saveParameters.append(cv::IMWRITE_JPEG_PROGRESSIVE);
    saveParameters.append(jpegMenu_m->getProgressiveScan());
    saveParameters.append(cv::IMWRITE_JPEG_OPTIMIZE);
    saveParameters.append(jpegMenu_m->getBaselineOptimized());
    saveParameters.append(cv::IMWRITE_JPEG_RST_INTERVAL);
    saveParameters.append(jpegMenu_m->getRestartInterval());
    saveParameters.append(cv::IMWRITE_JPEG_LUMA_QUALITY);
    saveParameters.append(jpegMenu_m->getLumaQuality());
    saveParameters.append(cv::IMWRITE_JPEG_CHROMA_QUALITY);
    saveParameters.append(jpegMenu_m->getChromaQuality());

    //catch exeception and display so doesnt crash - add regex later
    try {
        cv::imwrite(filePath.toStdString(), *image_m, saveParameters.toStdVector());
    } catch (cv::Exception e) {
        QMessageBox::warning(this, "Error", QString::fromStdString(e.msg));
    }
}

/* method saves the file as a png image according to OpenCV 3.3.2, using the
 * parameters from webpMenu - default if not specified.*/
void ImageSaveDialog::savePNG(QString &filePath)
{
    /* if menu was not able to be appended to the dialog (and thus not added)
     * create it to acquire its default values */
    if(!pngMenu_m)
    {
        pngMenu_m = new ImageSavePngMenu(this);
        pngMenu_m->setVisible(false);
    }
    QVector<int> saveParameters;
    saveParameters.append(cv::IMWRITE_PNG_COMPRESSION);
    saveParameters.append(pngMenu_m->getCompression());
    saveParameters.append(cv::IMWRITE_PNG_STRATEGY);
    saveParameters.append(pngMenu_m->getStrategy());
    saveParameters.append(cv::IMWRITE_PNG_BILEVEL);
    saveParameters.append(pngMenu_m->getBinaryLevel());

    //catch exeception and display so doesnt crash - add regex later
    try {
        cv::imwrite(filePath.toStdString(), *image_m, saveParameters.toStdVector());
    } catch (cv::Exception e) {
        QMessageBox::warning(this, "Error", QString::fromStdString(e.msg));
    }
}

/* method saves the file as a webp image according to OpenCV 3.3.2, using the
 * parameters from webpMenu - default if not specified.*/
void ImageSaveDialog::saveWebP(QString &filePath)
{
    /* if menu was not able to be appended to the dialog (and thus not added)
     * create it to acquire its default values */
    if(!webpMenu_m)
    {
        webpMenu_m = new ImageSaveWebpMenu(this);
        webpMenu_m->setVisible(false);
    }
    QVector<int> saveParameters;
    saveParameters.append(cv::IMWRITE_WEBP_QUALITY);
    saveParameters.append(webpMenu_m->getQuality());

    //catch exeception and display so doesnt crash - add regex later
    try {
        cv::imwrite(filePath.toStdString(), *image_m, saveParameters.toStdVector());
    } catch (cv::Exception e) {
        QMessageBox::warning(this, "Error", QString::fromStdString(e.msg));
    }
}

/* the saveAccepted slot retreives the specified file path to save the file from the QFileDialog
 * based on the index from the specified name filter and calls the appropriate save method based
 * on the desired format. Before calling the save function, appends file extension if nonexistant */
void ImageSaveDialog::saveAccepted()
{
    QString filePath = selectedFiles().at(0);

    //if the file extension is incorrect, append a correct one.
    QRegularExpression re;
    re.setPatternOptions(QRegularExpression::CaseInsensitiveOption);

    switch(nameFilters().indexOf(selectedNameFilter()))
    {
        case PNG:
            savePNG(filePath);
            re.setPattern("^.*(.png)$");
            if(!re.match(&filePath).hasMatch())
                filePath += ".png";
            break;
        case WEBP:
            re.setPattern("^.*(.webp)$");
            if(!re.match(&filePath).hasMatch())
                filePath += ".webp";
            saveWebP(filePath);
            break;
        default:
            re.setPattern("^.*(.jpg|.jpeg)$");
            if(!re.match(&filePath).hasMatch())
                filePath += ".jpg";
            saveJPEG(filePath);
            break;
    }
    qDebug() << filePath;
}

/* If the advanced options do not exist, create them. If they do exist,
 * toggle their visibility. */
void ImageSaveDialog::advancedOptionsToggled()
{
    if(!saveOptionsWidget_m)
    {
        appendImageOptionsWidget();
        buttonAdvancedOptions_m->setText("Hide...");
    }
    else if(!saveOptionsWidget_m->isVisible())
    {
        saveOptionsWidget_m->setVisible(true);
        buttonAdvancedOptions_m->setText("Hide...");
    }
    else
    {
        saveOptionsWidget_m->setVisible(false);
        buttonAdvancedOptions_m->setText("Advanced...");
    }
    displayFilteredSaveOptions();
}

/* private slot sets the index of the stackedWidget to display the appropriately selected
 * image save options when the advanced options are displayed based on the dialog filter.*/
void ImageSaveDialog::displayFilteredSaveOptions()
{
    if(saveOptionsWidget_m)
        saveOptionsWidget_m->setCurrentIndex(nameFilters().indexOf(selectedNameFilter()));
}

/* this method is used by the constructor and appends the "Advanced..." dialog
 * button to the dialog in the bottom right corner of the layout below "Cancel"*/
void ImageSaveDialog::appendAdvancedOptionsButton()
{
    //get main layout of QFileDialog
    QGridLayout* mainLayout = dynamic_cast<QGridLayout*>(this->layout());

    if(!mainLayout)
    {
        qDebug()<<"mainLayout is unavailable";
    }
    else
    {
        buttonAdvancedOptions_m = new QPushButton("Advanced...", this);
        int rows = mainLayout->rowCount();
        int columns = mainLayout->columnCount();
        mainLayout->addWidget(buttonAdvancedOptions_m, rows, columns - 1, 1, -1);

        //connect the button functionality
        connect(buttonAdvancedOptions_m, SIGNAL(released()), this, SLOT(advancedOptionsToggled()));
    }
}

/* this method generates and adds the advanced options widgets to the save dialog for the
 * supported image formats */
void ImageSaveDialog::appendImageOptionsWidget()
{
    //get main layout of QFileDialog
    QGridLayout* mainLayout = dynamic_cast<QGridLayout*>(this->layout());

    if(!mainLayout)
    {
        qDebug()<<"mainLayout is unavailable";
    }
    else
    {
        saveOptionsWidget_m = new QStackedWidget(this); //do not put in initialization list
        jpegMenu_m = new ImageSaveJpegMenu(this);
        pngMenu_m = new ImageSavePngMenu(this);
        webpMenu_m = new ImageSaveWebpMenu(this);
        saveOptionsWidget_m->addWidget(jpegMenu_m);
        saveOptionsWidget_m->addWidget(pngMenu_m);
        saveOptionsWidget_m->addWidget(webpMenu_m);
        int rows = mainLayout->rowCount();
        qDebug() << "save dialog g-layout rows: " << rows;
        mainLayout->addWidget(saveOptionsWidget_m, rows, 0, 1, -1);

        //connect the filter selected to the display save options based on the file filter
        connect(this, SIGNAL(filterSelected(const QString&)), this, SLOT(displayFilteredSaveOptions()));
    }
}
