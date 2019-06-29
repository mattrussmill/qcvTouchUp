#ifndef IMAGESAVEDIALOG_H
#define IMAGESAVEDIALOG_H

#include <QFileDialog>
class QStackedLayout;
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

private:
    enum ImageType{JPEG, PNG, WEBP};
    void appendImageOptionsWidget();
    QStackedLayout *saveOptionsLayout_m;
    ImageSaveJpegMenu *jpegMenu_m;
    ImageSavePngMenu *pngMenu_m;
    ImageSaveWebpMenu *webpMenu_m;

};

#endif // IMAGESAVEDIALOG_H
