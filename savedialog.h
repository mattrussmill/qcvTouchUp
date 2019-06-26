#ifndef SAVEDIALOG_H
#define SAVEDIALOG_H

#include <QFileDialog>

class SaveDialog : public QFileDialog
{
    Q_OBJECT
public:
    explicit SaveDialog(QWidget *parent = nullptr);

signals:

public slots:
};

#endif // SAVEDIALOG_H
