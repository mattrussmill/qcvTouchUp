#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <opencv2/core/core.hpp>
#include <QThread>
#include <QMutex>
#include <QDir>
class QImage;
class QString;
class ImageWorker;
class AdjustMenu;
class FilterMenu;
class TemperatureMenu;

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
    void openImage();
    void openImage(QString imagePath);

    //Generated from QDesigner
    void on_actionHistogram_triggered();

private:
    void clearImageBuffers();
    Ui::MainWindow *ui;
    QDir userImagePath;
    QMutex mutex;
    ImageWorker *imageWorker;
    AdjustMenu *adjustMenu;
    FilterMenu *filterMenu;
    TemperatureMenu *temperatureMenu;

};

#endif // MAINWINDOW_H
