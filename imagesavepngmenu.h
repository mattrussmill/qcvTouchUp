#ifndef IMAGESAVEPNGMENU_H
#define IMAGESAVEPNGMENU_H

#include <QWidget>

namespace Ui {
class ImageSavePngMenu;
}

class ImageSavePngMenu : public QWidget
{
    Q_OBJECT

public:
    explicit ImageSavePngMenu(QWidget *parent = nullptr);
    ~ImageSavePngMenu();

private:
    Ui::ImageSavePngMenu *ui;
};

#endif // IMAGESAVEPNGMENU_H
