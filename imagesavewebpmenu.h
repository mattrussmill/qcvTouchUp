#ifndef IMAGESAVEWEBPMENU_H
#define IMAGESAVEWEBPMENU_H

#include <QWidget>

namespace Ui {
class ImageSaveWebpMenu;
}

class ImageSaveWebpMenu : public QWidget
{
    Q_OBJECT

public:
    explicit ImageSaveWebpMenu(QWidget *parent = nullptr);
    ~ImageSaveWebpMenu();

private:
    Ui::ImageSaveWebpMenu *ui;
};

#endif // IMAGESAVEWEBPMENU_H
