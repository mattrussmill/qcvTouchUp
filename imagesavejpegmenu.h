#ifndef IMAGESAVEJPEGMENU_H
#define IMAGESAVEJPEGMENU_H

#include <QWidget>

namespace Ui {
class ImageSaveJpegMenu;
}

class ImageSaveJpegMenu : public QWidget
{
    Q_OBJECT

public:
    explicit ImageSaveJpegMenu(QWidget *parent = nullptr);
    ~ImageSaveJpegMenu();

private:
    Ui::ImageSaveJpegMenu *ui;
};

#endif // IMAGESAVEJPEGMENU_H
