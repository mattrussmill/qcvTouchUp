#ifndef IMAGESAVEDIALOG_H
#define IMAGESAVEDIALOG_H

#include <QFileDialog>
class QStackedWidget;
class QPushButton;
class ImageSaveJpegMenu;
class ImageSavePngMenu;
class ImageSaveWebpMenu;

class ImageSaveDialog : public QFileDialog
{
    Q_OBJECT

public:    
    explicit ImageSaveDialog(QImage &image, QWidget *parent = nullptr);
    ImageSaveDialog(QImage &image, QWidget *parent = nullptr, const QString &caption = QString(),
                    const QString &directory = QString());
    ~ImageSaveDialog();
protected:
    void saveJPEG(QImage &image);
    void saveBitmap(QImage &image);
    void savePNG(QImage &image);

private slots:
    void advancedOptionsToggled();

private:
    enum ImageType{JPEG, PNG, WEBP};
    void appendImageOptionsWidget();
    void appendAdvancedOptionsButton();
    QPushButton *buttonAdvancedOptions_m = nullptr;
    QStackedWidget *saveOptionsWidget_m = nullptr;
    ImageSaveJpegMenu *jpegMenu_m = nullptr;
    ImageSavePngMenu *pngMenu_m = nullptr;
    ImageSaveWebpMenu *webpMenu_m = nullptr;

};

#endif // IMAGESAVEDIALOG_H
