#include "imageworker.h"
#include "histogramwidget.h"
#include "bufferwrappersqcv.h"
#include <QImage>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <cmath>
#include <QDebug>

ImageWorker::ImageWorker(QMutex &m)
{
    mutex = &m;
    masterRGBImage = nullptr;
    srcRGBImage = nullptr;
    srcTmpImage = nullptr;
    dstRGBImage = nullptr;
    dstTmpImage = nullptr;
    imageWrapper = nullptr;
    splitChannelsTmp.push_back(cv::Mat());
    splitChannelsTmp.push_back(cv::Mat());
    splitChannelsTmp.push_back(cv::Mat());
}

// Deletes the dynamically allocated data before object is destroyed
ImageWorker::~ImageWorker()
{
    doClearImageBuffer();
    dstRGBHisto = nullptr; //managed externally
    if(srcRGBHisto)
    {
        delete srcRGBHisto[HistogramWidget::Blue];
        delete srcRGBHisto[HistogramWidget::Green];
        delete srcRGBHisto[HistogramWidget::Red];
        delete srcRGBHisto;
        srcRGBHisto = nullptr;
    }
}

////////////////////////--- Image Worker Assisting Functions ---////////////////////////

// Opens an Image from a file path. Images always stored in 24-bit BGR format when loading.
void ImageWorker::doOpenImage(const QString imagePath)
{
    mutex->lock();
    emit updateStatus("Opening...");
    clearImageBuffers();

    //attempt to open file from path, if empty emit signal with null image and stop work
    masterRGBImage = new cv::Mat(cv::imread(imagePath.toStdString(), cv::IMREAD_COLOR));

    if(masterRGBImage->empty())
    {
        if(masterRGBImage)
        {
            delete masterRGBImage;
            masterRGBImage = nullptr;
        }
        mutex->unlock();
        emit updateStatus("");
        //if fails, returns a nullptr to the main thread and stops
        emit resultImageSet(nullptr);
        return;
    }

    //convert image to RGB Color space
    cv::cvtColor(*masterRGBImage, *masterRGBImage, cv::COLOR_BGR2RGB);
    imageWrapper = new QImage(qcv::cvMatToQImage(*masterRGBImage));
    deriveWorkingBuffersFromMaster();

    //generate the histogram to be displayed in the GUI
    HistogramWidget::generateHistogram(*imageWrapper, dstRGBHisto);   

    //finish initializing the buffers used by ImageWorker for use after an image is loaded asdf adsf asdf and send signal to display image while working
    HistogramWidget::copy(const_cast<const uint**>(dstRGBHisto), srcRGBHisto); //necessary? Src Histo first then copy to dest - avoid race condition - same in adjust menu
    updateStatus("");
    mutex->unlock();
    emit resultImageSet(imageWrapper);
    emit resultHistoUpdate();
}

//Public, mutexed method used to signal a manual buffer clear
void ImageWorker::doClearImageBuffer()
{
    mutex->lock();
    clearImageBuffers();
    mutex->unlock();
}

/* This slot should only be called once, during initialization after the worker thread event loop has
 * initialized. This function sets the destination address for the histogram as the address maintained
 * on the heap for the mainwindow histogram of the displayed image. It also creates a buffer for the
 * source histogram to be used during image manipulations.*/
void ImageWorker::doSetHistogramDstAddress(uint **histo)
{
    dstRGBHisto = histo;
    srcRGBHisto = new uint *[3];
    srcRGBHisto[HistogramWidget::Red] = new uint[HISTO_SIZE];
    srcRGBHisto[HistogramWidget::Green] = new uint[HISTO_SIZE];
    srcRGBHisto[HistogramWidget::Blue] = new uint[HISTO_SIZE];

}

// Function checks if the working image buffers are allocated. If so, they are deallocated
inline void ImageWorker::clearImageBuffers()
{
    if(masterRGBImage) {delete masterRGBImage; masterRGBImage = nullptr;}
    if(srcRGBImage) {delete srcRGBImage; srcRGBImage = nullptr;}
    if(srcTmpImage) {delete srcTmpImage; srcTmpImage = nullptr;}
    if(dstRGBImage) {delete dstRGBImage; dstRGBImage = nullptr;}
    if(dstTmpImage) {delete dstTmpImage; dstTmpImage = nullptr;}
    if(imageWrapper) {delete imageWrapper; imageWrapper = nullptr;}
}

// Creates or resets all buffers to match the master buffer.
inline void ImageWorker::deriveWorkingBuffersFromMaster()
{
    if(!masterRGBImage || masterRGBImage->empty()) {qWarning("no master buffer allocated"); return;}

    if(!srcRGBImage)
        srcRGBImage = new cv::Mat(masterRGBImage->clone());
    else
        *srcRGBImage = masterRGBImage->clone();

    if(!dstRGBImage)
        dstRGBImage = new cv::Mat(masterRGBImage->clone());
    else
        *dstRGBImage = masterRGBImage->clone();

    if(!srcTmpImage)
        srcTmpImage = new cv::Mat(masterRGBImage->rows, masterRGBImage->cols, masterRGBImage->type());

    if(!dstTmpImage)
        dstTmpImage = new cv::Mat(masterRGBImage->rows, masterRGBImage->cols, masterRGBImage->type());
}

// Applies the most recent destination RGB buffer information into the master buffer and src Histogram
void ImageWorker::doCopyRGBBufferToMasterBuffer()
{
    if(!masterRGBImage || !dstRGBImage) return;
    mutex->lock();
    *masterRGBImage = dstRGBImage->clone();
    HistogramWidget::copy(const_cast<const uint**>(dstRGBHisto), srcRGBHisto);
    mutex->unlock();
}

// Reverts the image and histogram back to the most previously un-applied form (the master image
// and source histogram) and displays them.
void ImageWorker::doDisplayMasterBuffer()
{
    if(!masterRGBImage || !dstRGBImage) return;
    mutex->lock();
    *imageWrapper = qcv::cvMatToQImage(*masterRGBImage);
    HistogramWidget::copy(const_cast<const uint**>(srcRGBHisto), dstRGBHisto);
    mutex->unlock();
    emit resultImageUpdate(imageWrapper);
    emit resultHistoUpdate();
}



///////////////////////////////--- Adjust Menu Computations ---///////////////////////////////

/* Performs the image adjustment operations from the Adjust menu in the GUI. If the images
 * exist in memory the function locks the mutex and copies the necessary parameters before
 * performing the desired operations only for the corresponding sliders in the .ui file that
 * have changed from their default value.*/
void ImageWorker::doAdjustmentsComputation(float *parameterArray)
{
    //check to make sure all working arrays are allocated
    if(dstRGBImage == nullptr || masterRGBImage == nullptr)
        return;

    mutex->lock();
    emit updateStatus("Applying changes..."); //THIS IS NOT WORKING
    //create local copies of all parameters so a race condition does not exist mid-operation
    float alpha = parameterArray[1];
    float beta = parameterArray[0];
    float depth = parameterArray[2];
    float hue = parameterArray[3];
    float saturation = parameterArray[4];
    float intensity = parameterArray[5];
    float gamma = parameterArray[6];
    float highlight = parameterArray[7];
    float shadow = parameterArray[8];
    float color = parameterArray[9];

    *dstRGBImage = masterRGBImage->clone();

    //adjust the number of colors available of not at initial value of 255 --- PUT THIS AT BEGINNING?
    if(depth < 255)
    {
        //create and normalize LUT from 0 to largest intensity value, then scale from 0 to 255
        float scaleFactor = depth / 255;
        cv::Mat lookUpTable(1, 256, CV_8U);
        for(int i = 0; i < 256; i++)
            lookUpTable.data[i] = round(round(i * scaleFactor) / scaleFactor);

        //replace pixel intensities based on their LUT value
        cv::LUT(*dstRGBImage, lookUpTable, *dstRGBImage);
    }

    //perform operations on hue, intensity, and saturation channels.
    if(hue != 0.0 || intensity != 0.0 || saturation != 0.0 || gamma != 1.0
            || highlight != 0.0 || shadow != 0.0)
    {
        cv::cvtColor(*dstRGBImage, *srcTmpImage, cv::COLOR_RGB2HLS);
        cv::split(*srcTmpImage, splitChannelsTmp);

        /* openCv hue is stored as 360/2 since uchar cannot store above 255 so a LUT is populated
         * from 0 to 180 and phase shifted between -180 and 180 based on slider input. */
        if(hue != 0.0)
        {
            //populate LUT to map the current values to correct phase (only 180 cells used)
            int hueShifted;
            cv::Mat lookUpTable(1, 256, CV_8UC1);
            for(int i = 0; i < 180; i++)
            {
                hueShifted = i + hue;
                if(hueShifted < 0)
                    hueShifted += 180;
                else if(hueShifted > 179)
                    hueShifted -=180;
                lookUpTable.data[i] = hueShifted;
            }
            cv::LUT(splitChannelsTmp.at(0), lookUpTable, splitChannelsTmp[0]);
        }

        //adjust the intensity
        if(intensity != 0)
            splitChannelsTmp.at(1).convertTo(splitChannelsTmp[1], -1, 1.0, intensity); 

        //adjust the saturation
        if(saturation != 0)
            splitChannelsTmp.at(2).convertTo(splitChannelsTmp[2], -1, 1.0, saturation);

        //adjust gamma by 255(i/255)^(1/gamma) where gamma 0.5 to 3.0
        if(gamma != 1.0 || highlight != 0.0 || shadow != 0.0)
        {
            //fill LUT for gamma adjustment
            float tmpGamma;
            cv::Mat lookUpTable(1, 256, CV_8U);
            for(int i = 0; i < 256; i++)
            {
                //adjust gamma
                tmpGamma = 255.0 * pow(i / 255.0, 1.0 / gamma);

                /* Bound the shadow adjustment to all pixels below 149 such that the x axis is not
                 * crossed (output is not fliped) in the adjustment equation -(x/50.0)^4 + x and is
                 * handled as a step function. Function is deisned to taper towards zero as the bound
                 * is approached in conjunction with the highlight adjustment. Equation inverted for
                 * subtraction. Shadow operates on the gamma adjusted LUT from a range of -80 to 80*/
                if(i < 149 && shadow != 0.0)
                {
                    float tmpShadow;
                    if(shadow > 0.0)
                    {
                        //-(x/50.0)^4 + shadow
                        tmpShadow = -1 * pow(i / 50.0, 4) + shadow;
                        if(tmpShadow > 0)
                            tmpGamma += tmpShadow;
                    }
                    else
                    {
                        //(x/50.0)^4 - shadow .. (shadow is negative)
                        tmpShadow = pow(i / 50.0, 4) + shadow;
                        if(tmpShadow < 0)
                            tmpGamma += tmpShadow;
                    }
                }

                /* Bound the highlight adjustment to all pixels above 106 such that the x axis is not
                 * crossed (output is not fliped) in the adjustment equation -(x/50.0 - 5.1)^4 + x and is
                 * handled as a step function. Function is deisned to taper towards zero as the bound
                 * is approached in conjunction with the highlight adjustment.Equation inverted for
                 * subtraction. Highlight operates on the gamma adjusted LUT from a range of -80 to 80*/
                if(i > 106 && highlight != 0.0)
                {
                    float tmpHighlight;
                    if(highlight > 0.0)
                    {
                        //-(x/50.0 - 5.1)^4 + highlight
                        tmpHighlight = -1 * pow(i / 50.0 - 5.1, 4) + highlight;
                        if(tmpHighlight > 0)
                            tmpGamma += tmpHighlight;
                    }
                    else
                    {
                        //(x/50.0 - 5.1)^4 - highlight .. (highlight is negative)
                        tmpHighlight = pow(i / 50.0 - 5.1, 4) + highlight;
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
            cv::LUT(splitChannelsTmp.at(1), lookUpTable, splitChannelsTmp[1]);
        }


        cv::merge(splitChannelsTmp, *dstTmpImage);
        cv::cvtColor(*dstTmpImage, *dstRGBImage, cv::COLOR_HLS2RGB);
    }

    //convert from color to grayscale if != 1.0
    if(color != 1.0)
    {
        cv::cvtColor(*dstRGBImage, splitChannelsTmp[0], cv::COLOR_RGB2GRAY);
        splitChannelsTmp.at(0).copyTo(splitChannelsTmp.at(1));
        splitChannelsTmp.at(0).copyTo(splitChannelsTmp.at(2));
        cv::merge(splitChannelsTmp, *dstRGBImage);
    }

    //perform contrast and brightness operation if sliders are not at initial positions
    if (beta != 0.0 || alpha != 1.0)
    {
        //calculate brightness correction
        if(alpha >= 1)
            beta += -72.8 * log2(alpha);
        else
            beta += 127 * -log2(alpha) / sqrt(1 / alpha);

        //perform contrast computation and prime source buffer
        dstRGBImage->convertTo(*dstRGBImage, -1, alpha, beta);
    }

    //after computation is complete, push image and histogram to GUI if changes were made
    *imageWrapper = qcv::cvMatToQImage(*dstRGBImage);
    HistogramWidget::generateHistogram(*imageWrapper, dstRGBHisto);
    emit updateStatus(""); //THIS IS NOT WORKING
    mutex->unlock();
    emit resultImageUpdate(imageWrapper);
    emit resultHistoUpdate();
} //end doAdjustmentsComputation


///////////////////////////////--- Filter Menu Computations ---///////////////////////////////

/* Performs the smoothing operations from the Filter menu in the GUI. Switch statement
 * selects the type of smoothing that will be applied to the image in the master buffer.
 * The parameterArray passes all the necessary parameters to the worker thread based on
 * the openCV functions it calls. The kernel size cannot be larger than 21 px, limited
 * by the slider in the FilterMenu.*/
void ImageWorker::doSmoothFilterComputation(int *parameterArray)
{
    //check to make sure all working arrays are allocated
    if(dstRGBImage == nullptr || masterRGBImage == nullptr)
        return;

    mutex->lock();
    emit updateStatus("Applying changes...");

    switch (parameterArray[0]) //index 0 indicates the filter to use
    {

    case 1:
    {
        //For Gaussian, parameterArray[2] ranges from 1 to 100. Divide by 10 to get sigma.
        cv::GaussianBlur(*masterRGBImage, *dstRGBImage, cv::Size(parameterArray[1],parameterArray[1]),
                parameterArray[2] / 10.0, parameterArray[2] / 10.0);
        break;
    }
    case 2:
    {
        cv::medianBlur(*masterRGBImage, *dstRGBImage, parameterArray[1]);
        break;
    }
    default:
    {
        cv::blur(*masterRGBImage, *dstRGBImage, cv::Size(parameterArray[1],parameterArray[1]));
        break;
    }
    }

    //after computation is complete, push image and histogram to GUI if changes were made
    *imageWrapper = qcv::cvMatToQImage(*dstRGBImage);
    HistogramWidget::generateHistogram(*imageWrapper, dstRGBHisto);
    emit updateStatus("");
    mutex->unlock();
    emit resultImageUpdate(imageWrapper);
    emit resultHistoUpdate();

}

void ImageWorker::doSharpenFilterComputation(int *parameterArray)
{

}

void ImageWorker::doEdgeFilterComputation(int *parameterArray)
{

}

void ImageWorker::doNoiseFilterComputation(int *parameterArray)
{

}

void ImageWorker::doReconstructFilterComputation(int *parameterArray)
{

}













///////////////////////////////--- Adjust Menu Computations ---///////////////////////////////

/* This member function is used when a histogram buffer must be directly operated on instead of
 * regenerating the histogram from a source image. Given the source and destination histogram
 * buffers of equal size, this function accepts a sorting function as a parameter which will
 * generate a destination index based on the source index for copying histogram information
 * from a source bucket (number of pixels at the init value) and moving it into a destination bucket
 * (number of pixels at the new value) based on the newIntensityFunction result. Said function should
 * expect pixel values of 0 to 255 (8-bit) color depth. An additional value, if the function
 * requires a value to be passed, is included and must be cast to the appropriate type within
 * newIntensityFunction. The passed function shall output a floating point value as the new bucket
 * index (pixel intensity). This function is used to avoid rewriting iterative code in accessing the
 * histograms.*/
void ImageWorker::sortHistogramBuckets(float (ImageWorker::*newIntensityFunction)(int, void*), uint **source,
                                       uint** destination, void* fnPtrParameter, int numberOfChannels)
{
    float newIndexF;
    uint newIndex;
    if (!destination || numberOfChannels < 1)
        return;

    HistogramWidget::clear(destination, numberOfChannels);
    for(int index = 0; index < HISTO_SIZE; index++)
    {
        // Tests bound of pixel intensity so it truncates inside the range of the histogram buffer.
        newIndexF = (this->*newIntensityFunction)(index, fnPtrParameter);
        if(newIndexF < 0)
            newIndex = 0;
        else if(newIndexF > HISTO_SIZE - 1)
            newIndex = HISTO_SIZE - 1;
        else
            newIndex = newIndexF;

        // Adds the contents in the histogram source bucket to the destination bucket based on newIndex
        for(int c = 0; c < numberOfChannels; c++)
        {
            destination[c][newIndex] += source[c][index];
        }
    }
}

/* Function performs a contrast operation on the RGB image (i' = ai + b where i = image matrix) given an
 * alpha value. The alpha value expected is between 0.1 and 2.4. The function calculates the brightness
 * correction factor (beta) according to the alpha value for an 8-bit / channel image. If the image is >= 1
 * beta = -72.8 * log2(alpha) else beta = 127 * -log2(alpha) / sqrt(1 / alpha). These curves level out as
 * alpha reaches its extremes of 0.1 and 2.4 such that the brightness correction factor increases at a less
 * extreme rate. The function also sorts the histogram to reflect the contrast adjustment and emits the signals
 * to display the buffers on the gui when finished. */
//void ImageWorker::doContrastComputation(float alpha)


/* A sorting function for sortHistogramBuckets. The parameter pixel is the pixel to be sorted.
 * The pointer to alphaBeta is a pointer to an array which lives in the calling function which
 * stores the alpha and beta values for the pixel intensity sorting in 'float alphaBeta[2]'
 * respectively and as cast as such for use.*/
float ImageWorker::histogramContrastSorter(int pixel, void* alphaBeta)
{
        return pixel * (static_cast<float*>(alphaBeta)[0]) + (static_cast<float*>(alphaBeta)[1]);
}

/* Function performs a brightness operation on the RGB image (i' = i + b where i = image matrix) given an
 * alpha value. The function also sorts the histogram to reflect the brightness adjustment and emits the signals
 * to display the buffers on the gui when finished.*/
//void ImageWorker::doBrightnessComputation(int beta)


/* A sorting function for sortHistogramBuckets. The parameter pixel is the pixel to be sorted.
 * The pointer to beta is a pointer to an integer lives in the calling function and
 * stores the beta value for the pixel intensity sorting and cast appropriately.*/
float ImageWorker::histogramBrightnessSorter(int pixel, void *beta)
{
    return pixel + *static_cast<int*>(beta);
}




















