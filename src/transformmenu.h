#ifndef TRANSFORMMENU_H
#define TRANSFORMMENU_H

#include <QScrollArea>

namespace Ui {
class TransformMenu;
}

class TransformMenu : public QScrollArea
{
    Q_OBJECT

public:
    explicit TransformMenu(QWidget *parent = 0);
    ~TransformMenu();

private:
    Ui::TransformMenu *ui;
};

#endif // TRANSFORMMENU_H
