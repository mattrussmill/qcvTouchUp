#ifndef TEMPERATUREMENU_H
#define TEMPERATUREMENU_H

#include <QScrollArea>

namespace Ui {
class TemperatureMenu;
}

class TemperatureMenu : public QScrollArea
{
    Q_OBJECT

public:
    explicit TemperatureMenu(QWidget *parent = 0);
    ~TemperatureMenu();

private:
    Ui::TemperatureMenu *ui;
};

#endif // TEMPERATUREMENU_H
