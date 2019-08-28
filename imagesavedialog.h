#ifndef IMAGESAVEDIALOG_H
#define IMAGESAVEDIALOG_H

#include <QFileDialog>
#include <opencv2/imgcodecs.hpp>
class QStackedWidget;
class QPushButton;
class QString;
class ImageSaveJpegMenu;
class ImageSavePngMenu;
class ImageSaveWebpMenu;

class ImageSaveDialog : public QFileDialog
{
    Q_OBJECT

public:    
    explicit ImageSaveDialog(cv::Mat &image, QWidget *parent = nullptr);
    ImageSaveDialog(cv::Mat &image, QWidget *parent = nullptr, const QString &caption = QString(),
                    const QString &directory = QString());
    ~ImageSaveDialog();

protected slots:
    void saveAccepted();

protected:
    void saveJPEG(QString &filePath);
    void saveWebP(QString &filePath);
    void savePNG(QString &filePath);

private slots:
    void advancedOptionsToggled();
    void displayFilteredSaveOptions();

private:
    enum ImageType{JPEG = 0, PNG = 1, WEBP = 2};
    void appendImageOptionsWidget();
    void appendAdvancedOptionsButton();
    cv::Mat *image_m;
    QPushButton *buttonAdvancedOptions_m = nullptr;
    QStackedWidget *saveOptionsWidget_m = nullptr;
    ImageSaveJpegMenu *jpegMenu_m = nullptr;
    ImageSavePngMenu *pngMenu_m = nullptr;
    ImageSaveWebpMenu *webpMenu_m = nullptr;

};

#endif // IMAGESAVEDIALOG_H
