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
    QMutex mutex;

private slots:
    void imageOpenOperationFailed();
    void updateImageInformation(const QImage *image);
    void initializeWorkerThreadData();
    void updateHistogram();
    void openImage();
    void openImage(QString imagePath);
    void loadHistogramTool();

private:
    void clearImageBuffers();
    Ui::MainWindow *ui;
    QDir userImagePath_m;
    ImageWorker *imageWorker_m;
    AdjustMenu *adjustMenu_m;
    FilterMenu *filterMenu_m;
    TemperatureMenu *temperatureMenu_m;

};

#endif // MAINWINDOW_H
