#ifndef FILTERMENU_H
#define FILTERMENU_H

#include <QScrollArea>

namespace Ui {
class FilterMenu;
}

class FilterMenu : public QScrollArea
{
    Q_OBJECT

public:
    explicit FilterMenu(QWidget *parent = 0);
    ~FilterMenu();

private:
    Ui::FilterMenu *ui;
};

#endif // FILTERMENU_H
