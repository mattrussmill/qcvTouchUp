#ifndef TRANSFORMMENU_H
#define TRANSFORMMENU_H

#include <QScrollArea>
class QButtonGroup;

namespace Ui {
class TransformMenu;
}

class TransformMenu : public QScrollArea
{
    Q_OBJECT

public:
    explicit TransformMenu(QWidget *parent = 0);
    ~TransformMenu();
    enum ParameterIndex
    {
        Affine      = 0,
        Perspective = 1
    };

public slots:
    void initializeMenu();
    void setVisible(bool visible) override;

private:
    Ui::TransformMenu *ui;
    QButtonGroup *buttonGroup_m;
};

#endif // TRANSFORMMENU_H
