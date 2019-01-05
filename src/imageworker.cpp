#include "imageworker.h"
#include "histogramwidget.h"
#include "bufferwrappersqcv.h"
#include "adjustmenu.h"
#include "filtermenu.h"
#include <cmath>
#include <QDebug>
#include <QImage>
#include <QDir>
#include <QSize>
#include <QMutex>
#include <QString>
#include <QRect>
#include <algorithm>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgcodecs/imgcodecs.hpp>

ImageWorker::ImageWorker(QMutex &m)
{
    mutex_m = &m;
    masterRGBImage_m = nullptr;
    srcRGBImage_m = nullptr;
    tmpImage_m = nullptr;
    dstRGBImage_m = nullptr;
    imageWrapper_m = nullptr;
    splitChannelsTmp_m.push_back(cv::Mat());
    splitChannelsTmp_m.push_back(cv::Mat());
    splitChannelsTmp_m.push_back(cv::Mat());
}

// Deletes the dynamically allocated data before object is destroyed
ImageWorker::~ImageWorker()
{
    doClearImageBuffer();
    dstRGBHisto_m = nullptr; //managed externally
    if(srcRGBHisto_m)
    {
        delete srcRGBHisto_m[HistogramWidget::Blue];
        delete srcRGBHisto_m[HistogramWidget::Green];
        delete srcRGBHisto_m[HistogramWidget::Red];
        delete srcRGBHisto_m;
        srcRGBHisto_m = nullptr;
    }
}

////////////////////////--- Image Worker Assisting Functions ---////////////////////////

// Opens an Image from a file path. Images always stored in 24-bit BGR format when loading.
void ImageWorker::doOpenImage(const QString imagePath)
{
    mutex_m->lock();
    emit updateStatus("Opening...");
    clearImageBuffers();

    //attempt to open file from path, if empty emit signal with null image and stop work
    masterRGBImage_m = new cv::Mat(cv::imread(imagePath.toStdString(), cv::IMREAD_COLOR));

    if(masterRGBImage_m->empty())
    {
        if(masterRGBImage_m)
        {
            delete masterRGBImage_m;
            masterRGBImage_m = nullptr;
        }
        mutex_m->unlock();
        emit updateStatus("");
        //if fails, returns a nullptr to the main thread and stops
        emit resultImageSet(nullptr);
        return;
    }

    //convert image to RGB Color space
    cv::cvtColor(*masterRGBImage_m, *masterRGBImage_m, cv::COLOR_BGR2RGB);
    imageWrapper_m = new QImage(qcv::cvMatToQImage(*masterRGBImage_m));
    deriveWorkingBuffersFromMaster();

    //generate the histogram to be displayed in the GUI
    HistogramWidget::generateHistogram(*imageWrapper_m, dstRGBHisto_m);

    //finish initializing the buffers used by ImageWorker for use after an image is loaded asdf adsf asdf and send signal to display image while working
    HistogramWidget::copy(const_cast<const uint**>(dstRGBHisto_m), srcRGBHisto_m); //necessary? Src Histo first then copy to dest - avoid race condition - same in adjust menu
    updateStatus("");
    mutex_m->unlock();
    emit resultImageSet(imageWrapper_m);
    emit resultHistoUpdate();
}

//Public, mutexed method used to signal a manual buffer clear
void ImageWorker::doClearImageBuffer()
{
    mutex_m->lock();
    clearImageBuffers();
    mutex_m->unlock();
}

/* This slot should only be called once, during initialization after the worker thread event loop has
 * initialized. This function sets the destination address for the histogram as the address maintained
 * on the heap for the mainwindow histogram of the displayed image. It also creates a buffer for the
 * source histogram to be used during image manipulations.*/
void ImageWorker::doSetHistogramDstAddress(uint **histo)
{
    dstRGBHisto_m = histo;
    srcRGBHisto_m = new uint *[3];
    srcRGBHisto_m[HistogramWidget::Red] = new uint[HISTO_SIZE];
    srcRGBHisto_m[HistogramWidget::Green] = new uint[HISTO_SIZE];
    srcRGBHisto_m[HistogramWidget::Blue] = new uint[HISTO_SIZE];

}

// Function checks if the working image buffers are allocated. If so, they are deallocated
inline void ImageWorker::clearImageBuffers()
{
    if(masterRGBImage_m) {delete masterRGBImage_m; masterRGBImage_m = nullptr;}
    if(srcRGBImage_m) {delete srcRGBImage_m; srcRGBImage_m = nullptr;}
    if(tmpImage_m) {delete tmpImage_m; tmpImage_m = nullptr;}
    if(dstRGBImage_m) {delete dstRGBImage_m; dstRGBImage_m = nullptr;}
    if(imageWrapper_m) {delete imageWrapper_m; imageWrapper_m = nullptr;}
}

// Creates or resets all buffers to match the master buffer.
inline void ImageWorker::deriveWorkingBuffersFromMaster()
{
    if(!masterRGBImage_m || masterRGBImage_m->empty()) {qWarning("no master buffer allocated"); return;}

    if(!srcRGBImage_m)
        srcRGBImage_m = new cv::Mat(masterRGBImage_m->clone());
    else
        *srcRGBImage_m = masterRGBImage_m->clone();

    if(!dstRGBImage_m)
        dstRGBImage_m = new cv::Mat(masterRGBImage_m->clone());
    else
        *dstRGBImage_m = masterRGBImage_m->clone();

    if(!tmpImage_m)
        tmpImage_m = new cv::Mat(masterRGBImage_m->rows, masterRGBImage_m->cols, masterRGBImage_m->type());
}

// Applies the most recent destination RGB buffer information into the master buffer and src Histogram
void ImageWorker::doCopyWorkerBufferToMasterBuffer()
{
    if(!masterRGBImage_m || !dstRGBImage_m) return;
    mutex_m->lock();
    *masterRGBImage_m = dstRGBImage_m->clone(); //are these addresses all staying the same or is there a MEMORY LEAK!? CHECK THIS
    *imageWrapper_m = qcv::cvMatToQImage(*masterRGBImage_m);
    HistogramWidget::copy(const_cast<const uint**>(dstRGBHisto_m), srcRGBHisto_m);
    mutex_m->unlock();
    emit resultImageSet(imageWrapper_m);
}

// Reverts the image and histogram back to the most previously un-applied form (the master image
// and source histogram) and displays them.
void ImageWorker::doDisplayMasterBuffer()
{
    if(!masterRGBImage_m || !dstRGBImage_m) return;
    mutex_m->lock();
    *imageWrapper_m = qcv::cvMatToQImage(*masterRGBImage_m);
    HistogramWidget::copy(const_cast<const uint**>(srcRGBHisto_m), dstRGBHisto_m);
    mutex_m->unlock();
    emit resultImageUpdate(imageWrapper_m);
    emit resultHistoUpdate();
}

/* Function does the pre-image checks and locks for the corresponding menu functions in the worker
 * thread. First the destination and master buffers are checked for allocation. If they are the
 * Mutex is locked and the statusbar updated to indicate work is being performed. Returns false if
 * allocation did not yet occur.*/
bool ImageWorker::preImageOperationMutex()
{
    if(dstRGBImage_m == nullptr || masterRGBImage_m == nullptr)
        return false;

    mutex_m->lock();
    emit updateStatus("Applying changes...");
    return true;
}

/* Post image worker thread computation actions for corresponding menu functions. Push image in the
 * destination buffer to the GUI after wrapping in compatible QImage. Generate histogram. Then
 * unlock mutex and push both buffers to GUI to display changes.*/
void ImageWorker::postImageOperationMutex()
{
    *imageWrapper_m = qcv::cvMatToQImage(*dstRGBImage_m);
    HistogramWidget::generateHistogram(*imageWrapper_m, dstRGBHisto_m); //this should not be in here anymore
    emit updateStatus("");
    mutex_m->unlock();
    emit resultImageUpdate(imageWrapper_m);
    emit resultHistoUpdate();
}


///////////////////////////////--- Adjust Menu Computations ---///////////////////////////////

/* Performs the image adjustment operations from the Adjust menu in the GUI. If the images
 * exist in memory the function locks the mutex and copies the necessary parameters before
 * performing the desired operations only for the corresponding sliders in the .ui file that
 * have changed from their default value. Using a QVector forces a copy when passing information*/
void ImageWorker::doAdjustmentsComputation(QVector<float> parameter)
{
    if(!preImageOperationMutex()) return;

    //clone necessary because internal checks will prevent GUI image from cycling.
    *dstRGBImage_m = masterRGBImage_m->clone();

    //--adjust the number of colors available of not at initial value of 255
    if(parameter.at(AdjustMenu::Depth) < 255)
    {
        //create and normalize LUT from 0 to largest intensity value, then scale from 0 to 255
        float scaleFactor = parameter.at(AdjustMenu::Depth) / 255;
        cv::Mat lookUpTable(1, 256, CV_8U);
        for(int i = 0; i < 256; i++)
            lookUpTable.data[i] = round(round(i * scaleFactor) / scaleFactor);

        //replace pixel intensities based on their LUT value
        cv::LUT(*dstRGBImage_m, lookUpTable, *dstRGBImage_m);
    }


    //--perform operations on hue, intensity, and saturation color space if values are not set to initial
    if(parameter.at(AdjustMenu::Hue) != 0.0 || parameter.at(AdjustMenu::Intensity) != 0.0
            || parameter.at(AdjustMenu::Saturation) != 0.0 || parameter.at(AdjustMenu::Gamma) != 1.0
            || parameter.at(AdjustMenu::Highlight) != 0.0 || parameter.at(AdjustMenu::Shadows) != 0.0)
    {
        cv::cvtColor(*dstRGBImage_m, *tmpImage_m, cv::COLOR_RGB2HLS);
        cv::split(*tmpImage_m, splitChannelsTmp_m);

        /* openCv hue is stored as 360/2 since uchar cannot store above 255 so a LUT is populated
         * from 0 to 180 and phase shifted between -180 and 180 based on slider input. */
        if(parameter.at(AdjustMenu::Hue) != 0.0)
        {
            //populate LUT to map the current values to correct phase (only 180 cells used)
            int hueShifted;
            cv::Mat lookUpTable(1, 256, CV_8UC1);
            for(int i = 0; i < 180; i++)
            {
                hueShifted = i + parameter.at(AdjustMenu::Hue);
                if(hueShifted < 0)
                    hueShifted += 180;
                else if(hueShifted > 179)
                    hueShifted -=180;
                lookUpTable.data[i] = hueShifted;
            }
            cv::LUT(splitChannelsTmp_m.at(0), lookUpTable, splitChannelsTmp_m[0]);
        }

        //adjust the intensity
        if(parameter.at(AdjustMenu::Intensity) != 0)
            splitChannelsTmp_m.at(1).convertTo(splitChannelsTmp_m[1], -1, 1.0, parameter.at(AdjustMenu::Intensity));

        //adjust the saturation
        if(parameter.at(AdjustMenu::Saturation) != 0)
            splitChannelsTmp_m.at(2).convertTo(splitChannelsTmp_m[2], -1, 1.0, parameter.at(AdjustMenu::Saturation));

        //adjust gamma by 255(i/255)^(1/gamma) where gamma 0.5 to 3.0
        if(parameter.at(AdjustMenu::Gamma) != 1.0 || parameter.at(AdjustMenu::Highlight) != 0.0
                || parameter.at(AdjustMenu::Shadows) != 0.0)
        {
            //fill LUT for gamma adjustment
            float tmpGamma;
            cv::Mat lookUpTable(1, 256, CV_8U);
            for(int i = 0; i < 256; i++)
            {
                //adjust gamma
                tmpGamma = 255.0 * pow(i / 255.0, 1.0 / parameter.at(AdjustMenu::Gamma));

                /* Bound the shadow adjustment to all pixels below 149 such that the x axis is not
                 * crossed (output is not fliped) in the adjustment equation -(x/50.0)^4 + x and is
                 * handled as a step function. Function is deisned to taper towards zero as the bound
                 * is approached in conjunction with the highlight adjustment. Equation inverted for
                 * subtraction. Shadow operates on the gamma adjusted LUT from a range of -80 to 80*/
                if(i < 149 && parameter.at(AdjustMenu::Shadows) != 0.0)
                {
                    float tmpShadow;
                    if(parameter.at(AdjustMenu::Shadows) > 0.0)
                    {
                        //-(x/50.0)^4 + shadow
                        tmpShadow = -1 * pow(i / 50.0, 4) + parameter.at(AdjustMenu::Shadows);
                        if(tmpShadow > 0)
                            tmpGamma += tmpShadow;
                    }
                    else
                    {
                        //(x/50.0)^4 - shadow .. (shadow is negative)
                        tmpShadow = pow(i / 50.0, 4) + parameter.at(AdjustMenu::Shadows);
                        if(tmpShadow < 0)
                            tmpGamma += tmpShadow;
                    }
                }

                /* Bound the highlight adjustment to all pixels above 106 such that the x axis is not
                 * crossed (output is not fliped) in the adjustment equation -(x/50.0 - 5.1)^4 + x and is
                 * handled as a step function. Function is deisned to taper towards zero as the bound
                 * is approached in conjunction with the highlight adjustment.Equation inverted for
                 * subtraction. Highlight operates on the gamma adjusted LUT from a range of -80 to 80*/
                if(i > 106 && parameter.at(AdjustMenu::Highlight) != 0.0)
                {
                    float tmpHighlight;
                    if(parameter.at(AdjustMenu::Highlight) > 0.0)
                    {
                        //-(x/50.0 - 5.1)^4 + highlight
                        tmpHighlight = -1 * pow(i / 50.0 - 5.1, 4) + parameter.at(AdjustMenu::Highlight);
                        if(tmpHighlight > 0)
                            tmpGamma += tmpHighlight;
                    }
                    else
                    {
                        //(x/50.0 - 5.1)^4 - highlight .. (highlight is negative)
                        tmpHighlight = pow(i / 50.0 - 5.1, 4) + parameter.at(AdjustMenu::Highlight);
                        if(tmpHighlight < 0)
                            tmpGamma += tmpHighlight;
                    }
                }

                //limit gamma adjusted values
                if(tmpGamma > 255)
                    tmpGamma = 255;
                else if(tmpGamma < 0)
                    tmpGamma = 0;

                lookUpTable.data[i] = tmpGamma;
            }

            //replace pixel values based on their LUT value
            cv::LUT(splitChannelsTmp_m.at(1), lookUpTable, splitChannelsTmp_m[1]);
        }

        cv::merge(splitChannelsTmp_m, *tmpImage_m);
        cv::cvtColor(*tmpImage_m, *dstRGBImage_m, cv::COLOR_HLS2RGB);
    }


    //--convert from color to grayscale if != 1.0
    if(parameter.at(AdjustMenu::Color) != 1.0)
    {
        cv::cvtColor(*dstRGBImage_m, splitChannelsTmp_m[0], cv::COLOR_RGB2GRAY);
        splitChannelsTmp_m.at(0).copyTo(splitChannelsTmp_m.at(1));
        splitChannelsTmp_m.at(0).copyTo(splitChannelsTmp_m.at(2));
        cv::merge(splitChannelsTmp_m, *dstRGBImage_m);
    }


    //--perform contrast and brightness operation if sliders are not at initial positions
    if (parameter.at(AdjustMenu::Brightness) != 0.0 || parameter.at(AdjustMenu::Contrast) != 1.0)
    {
        float alpha = parameter.at(AdjustMenu::Contrast);
        float beta = parameter.at(AdjustMenu::Brightness);

        //calculate brightness correction
        if(alpha >= 1)
            beta += -72.8 * log2(alpha);
        else
            beta += 127 * -log2(alpha) / sqrt(1 / alpha);

        //perform contrast computation and prime source buffer
        dstRGBImage_m->convertTo(*dstRGBImage_m, -1, alpha, beta);
    }


    //after computation is complete, push image and histogram to GUI if changes were made
    postImageOperationMutex();
} //end doAdjustmentsComputation


///////////////////////////////--- Filter Menu Computations ---///////////////////////////////
/* Sets the kernel radius for a filter to a maximum of 0.015 times the smallest image dimension.
 * The kernel size is then scaled between 1 and 100% of its maximum size through weightPercent.
 * The result of this function must also always be odd. */
int ImageWorker::kernelSize(QSize image, int weightPercent)
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
cv::Mat ImageWorker::makeLaplacianKernel(int size)
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

/* Performs the smoothing operations from the Filter menu in the GUI. Switch statement
 * selects the type of smoothing that will be applied to the image in the master buffer.
 * The parameterArray passes all the necessary parameters to the worker thread based on
 * the openCV functions it calls.*/
void ImageWorker::doSmoothFilterComputation(QVector<int> parameter)
{
    //check to make sure all working arrays are allocated
    if(!preImageOperationMutex()) return;

    int ksize = kernelSize(QSize(masterRGBImage_m->cols, masterRGBImage_m->rows),
                          parameter.at(FilterMenu::KernelWeight));

    switch (parameter.at(FilterMenu::KernelType))
    {

    case FilterMenu::FilterGaussian:
    {
        //For Gaussian, sigma should be 1/4 size of kernel.
        cv::GaussianBlur(*masterRGBImage_m, *dstRGBImage_m, cv::Size(ksize, ksize), ksize * 0.25);
        break;
    }
    case FilterMenu::FilterMedian:
    {
        cv::medianBlur(*masterRGBImage_m, *dstRGBImage_m, ksize);
        break;
    }
    default: //FilterMenu::FilterAverage
    {
        cv::blur(*masterRGBImage_m, *dstRGBImage_m, cv::Size(ksize, ksize));
        break;
    }
    }

    //after computation is complete, push image and histogram to GUI if changes were made
    postImageOperationMutex();

}

/* Performs the Sharpening operations from the Filter menu in the GUI. Switch statement
 * selects the type of smoothing that will be applied to the image in the master buffer.
 * The parameterArray passes all the necessary parameters to the worker thread based on
 * the openCV functions it calls.*/
void ImageWorker::doSharpenFilterComputation(QVector<int> parameter)
{
    //check to make sure all working arrays are allocated
    if(!preImageOperationMutex()) return;

    int ksize = kernelSize(QSize(masterRGBImage_m->cols, masterRGBImage_m->rows),
                          parameter.at(FilterMenu::KernelWeight));

    switch (parameter.at(FilterMenu::KernelType))
    {

    case FilterMenu::FilterLaplacian:
    {
        //blur first to reduce noise
        cv::GaussianBlur(*masterRGBImage_m, *tmpImage_m, cv::Size(3, 3), 0);
        cv::filter2D(*tmpImage_m, *tmpImage_m, CV_8U,
                     makeLaplacianKernel(parameter.at(FilterMenu::KernelWeight)));
        cv::addWeighted(*masterRGBImage_m, .9, *tmpImage_m, .1, 255 * 0.1, *dstRGBImage_m, masterRGBImage_m->depth());
        break;
    }
    default: //FilterMenu::FilterUnsharpen
    {
        cv::GaussianBlur(*masterRGBImage_m, *tmpImage_m, cv::Size(ksize, ksize), ksize * 0.25);
        cv::addWeighted(*masterRGBImage_m, 1.5, *tmpImage_m, -0.5, 0, *dstRGBImage_m, masterRGBImage_m->depth());
        break;
    }
    }

    //after computation is complete, push image and histogram to GUI if changes were made
    postImageOperationMutex();
}

/* Performs the Edge Detect operations from the Filter menu in the GUI. Switch statement
 * selects the type of edge detectopm that will be applied to the image in the master buffer.
 * The parameterArray passes all the necessary parameters to the worker thread based on
 * the openCV functions it calls.*/
void ImageWorker::doEdgeFilterComputation(QVector<int> parameter)
{
    if(!preImageOperationMutex()) return;

    //blur first to reduce noise for high pass filter
    cv::GaussianBlur(*masterRGBImage_m, *tmpImage_m, cv::Size(3, 3), 0);

    switch (parameter.at(FilterMenu::KernelType))
    {

    //these opencv functions can have aperature size of 1/3/5/7
    case FilterMenu::FilterLaplacian:
    {
        cv::Laplacian(*tmpImage_m, *dstRGBImage_m, CV_8U, parameter.at(FilterMenu::KernelWeight));
        break;
    }

    case FilterMenu::FilterSobel:
    {
        cv::Sobel(*tmpImage_m, *dstRGBImage_m, CV_8U, 1, 0, parameter.at(FilterMenu::KernelWeight));
        cv::Sobel(*tmpImage_m, *tmpImage_m, CV_8U, 0, 1, parameter.at(FilterMenu::KernelWeight));
        cv::addWeighted(*tmpImage_m, 0.5, *dstRGBImage_m, 0.5, 0, *dstRGBImage_m, masterRGBImage_m->depth());
        break;
    }

    default: //FilterMenu::FilterCanny
    {
        cv::Canny(*tmpImage_m, *dstRGBImage_m, 80, 200, parameter.at(FilterMenu::KernelWeight));
        qDebug() << "channels:" << QString::number(dstRGBImage_m->channels());
        break;
    }
    }
    //after computation is complete, push image and histogram to GUI if changes were made
    postImageOperationMutex();
}


/////////////////////////////--- Temperature Menu Computations ---/////////////////////////////
/* Performs the Light Source Temperature Adjustment operations from the Filter menu in the GUI. Through
 * the polynomials derived using mycurvefit.com from a light source temperature LUT such that all temperature
 * are covered between 1000K and 10000K. The RGB scalar values are calculated as a function of temperature (K)
 * passed to the doTemperatureComputation function. Each channel in the Master Image is multiplied by its scalar
 * to make the adjustment.*/
void ImageWorker::doTemperatureComputation(int parameter)
{
    if(!preImageOperationMutex()) return;

    float yred, ygreen, yblue;
    double x = parameter / 100.0;

    //step function for red and green channels
    if(parameter > 6500)
    {
        yred = 479.7143 - 4.757143 * x + 0.02 * pow(x, 2);
        ygreen = 369.3214 - 2.502381 * x + 0.01 * pow(x, 2);
    }
    else
    {
        yred = 255.0;
        ygreen = -50.34577 + 13.21698 * x - 0.2250017 * pow(x, 2.0) + 0.001430717 * pow(x, 3.0);
    }

    //step function for blue channel
    if(parameter < 1500)
    {
        yblue = 0.0;
    }
    else if(parameter > 7000)
    {
        yblue = 255.0;
    }
    else
    {
        yblue = 260.1294 - 51.0587 * x + 3.321611 * pow(x, 2.0) - 0.08684615 * pow(x, 3.0)
                + 0.001048834 * pow(x, 4.0) - 0.000004820513 * pow(x, 5.0);
    }

    //split each channel and manipulate each channel individually
    cv::split(*masterRGBImage_m, splitChannelsTmp_m);
    splitChannelsTmp_m.at(0) *= yred / 255;
    splitChannelsTmp_m.at(1) *= ygreen / 255;
    splitChannelsTmp_m.at(2) *= yblue / 255;
    cv::merge(splitChannelsTmp_m, *dstRGBImage_m);

    //after computation is complete, push image and histogram to GUI if changes were made
    postImageOperationMutex();
}

/////////////////////////////--- Transform Menu Computations ---/////////////////////////////
/* This slot performs a cropping computation on the image. It is passed a ROI, which is assumed
 * to already be in bounds from the signal, assigns the new ROI to the dstRGBImage_m buffer. The
 * postImageOperationMutex is not used as the image on screen should not be updated (signal) until
 * the crop selection is complete according to the user so that adjustments to the ROI can be made.*/
void ImageWorker::doCropComputation(QRect roi)
{
    if(!preImageOperationMutex()) return;
    cv::Rect region(roi.topLeft().x(), roi.topLeft().y(), roi.width(), roi.height());
    *dstRGBImage_m = cv::Mat(*masterRGBImage_m, region);
    emit updateStatus("");
    mutex_m->unlock();
}

void ImageWorker::doRotateComputation(int degree)
{
    if(!preImageOperationMutex()) return;

    //center of rotation, rotation matrix, and containing size for rotation
    degree *= -1;
    cv::Point2f center = cv::Point2f((masterRGBImage_m->cols -1) / 2.0, (masterRGBImage_m->rows -1) / 2.0);
    cv::Mat rotationMatrix = cv::getRotationMatrix2D(center, degree, 1);
    cv::RotatedRect rotatedRegion(center, masterRGBImage_m->size(), degree);
    cv::Rect boundingRegion = rotatedRegion.boundingRect();

    //adjust the rotation matrix
    rotationMatrix.at<double>(0, 2) += boundingRegion.width / 2.0 - masterRGBImage_m->cols / 2.0;
    rotationMatrix.at<double>(1, 2) += boundingRegion.height / 2.0 - masterRGBImage_m->rows / 2.0;

    //perform transform
    *dstRGBImage_m = cv::Mat(boundingRegion.size(), masterRGBImage_m->type());
    cv::warpAffine(*masterRGBImage_m, *dstRGBImage_m, rotationMatrix, boundingRegion.size());

    //crop image so that no black edges due to rotation are showing if not square
    //float ratio = static_cast<float>(masterRGBImage_m->cols) / static_cast<float>(masterRGBImage_m->rows); //needed?
    if(degree != 0 && abs(degree) != 90 && abs(degree) != 180)
    { 
        cv::Point2f corners[4];
        rotatedRegion.points(corners);

        //offset for points in boundingRegion
        for(int i = 0; i < 4; i ++)
        {
            corners[i].x += boundingRegion.width / 2.0 - masterRGBImage_m->cols / 2.0;
            corners[i].y += boundingRegion.height / 2.0 - masterRGBImage_m->rows / 2.0;
        }

        //determine which points are which on the rotated image
        cv::Point2f *top = &corners[0];
        cv::Point2f *bottom = &corners[0];
        cv::Point2f *left = &corners[0];
        cv::Point2f *right = &corners[0];

        //find position-most point of rotated rect
        for(int i = 0; i < 4; i++) //one loop, set initial values first
        {
            //should not be called if points are square (ignore bounding case for now)
            if(top->y < corners[i].y)
                top = &corners[i];
            if(bottom->y > corners[i].y)
                bottom = &corners[i];
            if(left->x > corners[i].x)
                left = &corners[i];
            if(right->x < corners[i].x)
                right = &corners[i];
        }
        //points are flipped in rotatedRegion.points() call (something weird with corners on rotate)
        std::swap(right->y, left->y);
        std::swap(top->y, bottom->y);

        //for testing purposes to visualize points
        qDebug() << "T:"<< top->x << top->y << "B:" << bottom->x << bottom->y << "L:" << left->x << left->y << "R:" << right->x << right->y;

        //bottom lines are parallel
        double slopeTopLeft = (top->y - left->y) / (top->x - left->x);
        double slopeTopRight = (top->y - right->y) / (top->x - right->x);

        //find line intercepts for each side (y is inverted for lower intercepts so they are adjusted accordingly)
        double yInterceptTopLeft = -1 * (slopeTopLeft * top->x);
        double yInterceptTopRight = -1 * (slopeTopRight * top->x);
        double yInterceptBottomLeft = left->y;
        double yInterceptBottomRight = -1 * (slopeTopLeft * bottom->x) + boundingRegion.height;

        //find point on opposite intercepting lines from image corners straight up or across
        cv::Point2f interceptOppositeOfTopCorner;
        cv::Point2f interceptOppositeOfBottomCorner;
        cv::Point2f interceptOppositeOfLeftCorner;
        cv::Point2f interceptOppositeOfRightCorner;

        //calculates the intercepts on the edge of the rectangle opposite its corners
        interceptOppositeOfBottomCorner.x = bottom->x;
        interceptOppositeOfTopCorner.x = top->x;
        if(top->x < bottom->x)
        {
            interceptOppositeOfBottomCorner.y = slopeTopRight * interceptOppositeOfBottomCorner.x + yInterceptTopRight;
            interceptOppositeOfTopCorner.y = slopeTopRight * interceptOppositeOfTopCorner.x + yInterceptBottomLeft;
        }
        else
        {
            interceptOppositeOfBottomCorner.y = slopeTopLeft * interceptOppositeOfBottomCorner.x + yInterceptTopLeft;
            interceptOppositeOfTopCorner.y = slopeTopLeft * interceptOppositeOfTopCorner.x + yInterceptBottomRight;
        }

        interceptOppositeOfRightCorner.y = right->y;
        interceptOppositeOfLeftCorner.y = left->y;
        if(left->y < right->y)
        {
            interceptOppositeOfLeftCorner.x = (interceptOppositeOfLeftCorner.y - yInterceptTopRight) / slopeTopRight;
            interceptOppositeOfRightCorner.x = (interceptOppositeOfRightCorner.y - yInterceptBottomLeft) / slopeTopRight;
        }
        else
        {
            interceptOppositeOfLeftCorner.x = (interceptOppositeOfLeftCorner.y - yInterceptBottomRight) / slopeTopLeft;
            interceptOppositeOfRightCorner.x = (interceptOppositeOfRightCorner.y - yInterceptTopLeft) / slopeTopLeft;
        }

        cv::rectangle(*dstRGBImage_m, cv::Rect(interceptOppositeOfTopCorner.x, interceptOppositeOfTopCorner.y, 5, 5), cv::Scalar( 255, 255, 0 ), 5);
        cv::rectangle(*dstRGBImage_m, cv::Rect(interceptOppositeOfBottomCorner.x, interceptOppositeOfBottomCorner.y, 5, 5), cv::Scalar( 255, 255, 0 ), 5);
        cv::rectangle(*dstRGBImage_m, cv::Rect(interceptOppositeOfLeftCorner.x, interceptOppositeOfLeftCorner.y, 5, 5), cv::Scalar( 255, 0, 0 ), 5);
        cv::rectangle(*dstRGBImage_m, cv::Rect(interceptOppositeOfRightCorner.x, interceptOppositeOfRightCorner.y, 5, 5), cv::Scalar( 255, 255, 0 ), 5);

        //WORKING HERE NOW
        //x,y,width,height
        cv::Rect rectasdf = cv::Rect(interceptOppositeOfRightCorner.x, interceptOppositeOfBottomCorner.y,
                                     interceptOppositeOfLeftCorner.x - interceptOppositeOfRightCorner.x,
                                     interceptOppositeOfTopCorner.y - interceptOppositeOfBottomCorner.y);

        //Hagrid tells you you're a wizard and you draw this rectangle
        cv::rectangle(*dstRGBImage_m, rectasdf, cv::Scalar( 255, 0, 0 ), 3);
    }

    postImageOperationMutex();
}


