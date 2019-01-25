#ifndef COLORSLICEMENU_H
#define COLORSLICEMENU_H

#include <QScrollArea>

namespace Ui {
class ColorSliceMenu;
}

class ColorSliceMenu : public QScrollArea
{
    Q_OBJECT

public:
    explicit ColorSliceMenu(QWidget *parent = 0);
    ~ColorSliceMenu();

private:
    Ui::ColorSliceMenu *ui;
};

#endif // COLORSLICEMENU_H
