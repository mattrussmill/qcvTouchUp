#include "filterworker.h"
#include "signalsuppressor.h"
#include "filtermenu.h"
#include <QMutex>
#include <QString>
#include <opencv2/imgproc.hpp>
#include <opencv2/core/ocl.hpp>
#include <QDebug>

/* Constructor initializes the appropriate member variables for the worker object. If
 * an OpenCL device is detected as available on the system, a pre-initialization step is
 * performed to increase initial performance. */
FilterWorker::FilterWorker(const cv::Mat *masterImage, cv::Mat *previewImage, QMutex *mutex, QObject *parent)
    : QObject(parent)
{
    emit updateStatus("Filter Menu initializing...");
    //QElapsedTimer timer;
    //timer.start();

    //OpenCL initialization step to build the OpenCL calls in GPU before the worker is called with an attached image
    cv::ocl::Context ctx = cv::ocl::Context::getDefault();
    if (ctx.ptr())
    {
        cv::Mat tmpMat(100, 100, CV_8UC3);
        cv::randu(tmpMat, cv::Scalar(0, 0, 0), cv::Scalar(255, 255, 255));
        int tmpParameters[] = {0, 0, 0};

        mutex_m = nullptr;
        masterImage_m = &tmpMat;
        previewImage_m = &tmpMat;

        performImageFiltering(tmpParameters);
    }

    mutex_m = mutex;
    masterImage_m = masterImage;
    previewImage_m = previewImage;
    qDebug() << "Filter Worker Created! - Images:" << masterImage_m << previewImage_m;

    //qDebug() << "The slow operation took" << timer.elapsed() << "milliseconds";
    emit updateStatus("");

}

// destructor
FilterWorker::~FilterWorker()
{
    qDebug() << "FilterWorker destroyed";
}

/* This member (slot) recieves the data from the controlling class (slow thread). The data
 * is sent as a pointer to the class itself who's member contains the data. To see how this
 * works see signalsuppressor.h/cpp. The format is tied to the associated menu object. */
void FilterWorker::receiveSuppressedSignal(SignalSuppressor *dataContainer)
{
    data_m = dataContainer->getNewData().toByteArray();
    int *parameters = reinterpret_cast<int*>(data_m.data());
    performImageFiltering(parameters);
    emit updateDisplayedImage();
}

/* This slot is used to update the member addresses for the master and preview images stored
 * in the controlling thread. If the Mat's become empty in the controlling thread this slot
 * should be signaled with nullptrs to signify they are empty. */
void FilterWorker::receiveImageAddresses(const cv::Mat *masterImage, cv::Mat *previewImage)
{
    masterImage_m = masterImage;
    previewImage_m = previewImage;
    qDebug() << "Filter Worker Images:" << masterImage_m << previewImage_m;
}

///////////////////////////////--- Filter Menu Computations ---///////////////////////////////
/* Sets the kernel radius for a filter to a maximum of 0.015 times the smallest image dimension.
 * The kernel size is then scaled between 1 and 100% of its maximum size through weightPercent.
 * The result of this function must also always be odd. */
int FilterWorker::kernelSize(QSize image, int weightPercent)
{
    int ksize;
    if(image.width() > image.height())
        ksize = image.height();
    else
        ksize = image.width();

    if(weightPercent > 100)
        weightPercent = 100;
    else if(weightPercent < 1)
        weightPercent = 1;

    ksize *= 0.015 * (weightPercent / 100.0);

    return ksize | 1;
}

/* Generates a 2D Laplacian kernel for use with OpenCV's Filter2D function. If the desired size is
 * negative, the kernel will output a Mat with one element of 1. If passed an even size, the next
 * greatest odd size is used.*/
cv::Mat FilterWorker::makeLaplacianKernel(int size)
{
    if(size < 1)
        size = 1;
    size |= 1; //must be odd
    int matCenter = size >> 1;

    //fill new kernel with zeroes
    cv::Mat newKernel = cv::Mat::zeros(size, size, CV_32F);

    //fill matrix from center; traverse approx 1/4 elements
    int kernelPoint;
    int kernelSum = 0;
    for(int i = 0; i < matCenter + 1; i++)
    {
        for(int j = 0; j < matCenter + 1; j++)
        {
            kernelPoint = -(1 + i + j - matCenter);
            if (kernelPoint > 0) kernelPoint = 0;

            //top left
            newKernel.at<float>(cv::Point(i, j)) = kernelPoint;

            //bottom right
            newKernel.at<float>(cv::Point(size - i - 1, size - j - 1)) = kernelPoint;

            //do not write & count multiple times and to sum properly
            if(i != size >> 1 && j != size >> 1)
            {
                kernelSum += kernelPoint * 4;

                //top right
                newKernel.at<float>(cv::Point(size - i - 1, j)) = kernelPoint;

                //bottom left
                newKernel.at<float>(cv::Point(i, size - j - 1)) = kernelPoint;
            }
            else
            {
                kernelSum += kernelPoint * 2;
            }
        }
    }

    //adjust the kernel sum to exclude the center point. Invert and set as center.
    kernelSum -= newKernel.at<float>(cv::Point(matCenter, matCenter)) * 2;
    newKernel.at<float>(cv::Point(matCenter, matCenter)) = -kernelSum;
    return newKernel;
}

void FilterWorker::performImageFiltering(int *parameter)
{

}
