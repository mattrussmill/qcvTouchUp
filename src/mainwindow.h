#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "imageworker.h"
#include "adjustmenu.h"
#include "filtermenu.h"
#include <opencv2/core/core.hpp>
#include <QThread>
#include <QMutex>
#include <QDir>
class QImage;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    QThread workerThread;

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
    ImageWorker *imageWorker;
    AdjustMenu *adjustMenu;
    FilterMenu *filterMenu;

};

#endif // MAINWINDOW_H
