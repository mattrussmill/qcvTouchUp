#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDir>
#include <QString>
#include <QImage>
#include <QMutex>
#include "workereventloop.h"
#include "adjustmenu.h"

#include <opencv2/core/core.hpp>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void loadSubMenu(int menuIndex);
    void imageOpenOperationFailed();
    void updateImageInformation(const QImage *image);
    void initializeWorkerThreadData();
    void updateHistogram();

    //Generated from QDesigner
    void on_actionOpen_triggered();
    void on_actionAbout_triggered();
    void on_actionHistogram_triggered();

private:
    void clearImageBuffers();
    Ui::MainWindow *ui;
    QDir userImagePath;
    QMutex mutex;
    WorkerEventLoop *imageWorker;
    AdjustMenu *adjustMenu;

};

#endif // MAINWINDOW_H
