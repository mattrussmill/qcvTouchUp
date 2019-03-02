#include "adjustworker.h"
#include "signalsuppressor.h"
#include "adjustmenu.h"
#include <QMutex>
#include <opencv2/imgproc/imgproc.hpp>
#include <QDebug>

AdjustWorker::AdjustWorker(QMutex *mutex, QObject *parent) : QObject(parent)
{
    qDebug() << "AdjustWorker created";
    mutex_m = mutex;
    masterImage_m = nullptr;
    previewImage_m = nullptr;
    lookUpTable_m = cv::Mat(1, 256, CV_8U);
}

AdjustWorker::~AdjustWorker()
{
    qDebug() << "AdjustWorker destroyed";
}

void AdjustWorker::receiveSuppressedSignal(SignalSuppressor *dataContainer)
{
    data = dataContainer->getNewData().toByteArray();
    float *parameters = reinterpret_cast<float*>(data.data());
    performImageAdjustments(parameters);
    emit updateDisplayedImage();
}

void AdjustWorker::receiveImageAddresses(const cv::Mat *masterImage, cv::Mat *previewImage)
{
    masterImage_m = masterImage;
    previewImage_m = previewImage;
    qDebug() << "Adjust Worker Images:" << masterImage_m << previewImage_m;
}

/* Performs the image adjustment operations from the Adjust menu in the GUI. If the images
 * exist in memory the function locks the mutex and copies the necessary parameters before
 * performing the desired operations only for the corresponding sliders in the .ui file that
 * have changed from their default value. Using a QVector forces a copy when passing information*/
void AdjustWorker::performImageAdjustments(float * parameter)
{

    if(mutex_m)
        mutex_m->lock();
    if(!(masterImage_m || previewImage_m))
    {
        if(mutex_m)
            mutex_m->unlock();
        qDebug() << "Cannot perform Adjustments, image not attached";
        return;
    }


    //clone necessary because internal checks will prevent GUI image from cycling.
    masterImage_m->copyTo(*previewImage_m); //get rid of clone use copyTo

    //--adjust the number of colors available of not at initial value of 255
    if(parameter[AdjustMenu::Depth] < 255)
    {
        //create and normalize LUT from 0 to largest intensity value, then scale from 0 to 255
        float scaleFactor = parameter[AdjustMenu::Depth] / 255;
        for(int i = 0; i < 256; i++)
            lookUpTable_m.data[i] = round(round(i * scaleFactor) / scaleFactor);

        //replace pixel intensities based on their LUT value
        cv::LUT(*previewImage_m, lookUpTable_m, *previewImage_m);
    }


    //--perform operations on hue, intensity, and saturation color space if values are not set to initial
    if(parameter[AdjustMenu::Hue] != 0.0 || parameter[AdjustMenu::Intensity] != 0.0
            || parameter[AdjustMenu::Saturation] != 0.0 || parameter[AdjustMenu::Gamma] != 1.0
            || parameter[AdjustMenu::Highlight] != 0.0 || parameter[AdjustMenu::Shadows] != 0.0)
    {
        cv::cvtColor(*previewImage_m, *previewImage_m, cv::COLOR_RGB2HLS);
        cv::split(*previewImage_m, splitChannelsTmp_m);

        /* openCv hue is stored as 360/2 since uchar cannot store above 255 so a LUT is populated
             * from 0 to 180 and phase shifted between -180 and 180 based on slider input. */
        if(parameter[AdjustMenu::Hue] != 0.0)
        {
            //populate LUT to map the current values to correct phase (only 180 cells used)
            int hueShifted;
            cv::Mat lookUpTable(1, 256, CV_8UC1);
            for(int i = 0; i < 180; i++)
            {
                hueShifted = i + parameter[AdjustMenu::Hue];
                if(hueShifted < 0)
                    hueShifted += 180;
                else if(hueShifted > 179)
                    hueShifted -=180;
                lookUpTable.data[i] = hueShifted;
            }
            cv::LUT(splitChannelsTmp_m.at(0), lookUpTable, splitChannelsTmp_m[0]);
        }

        //adjust the intensity
        if(parameter[AdjustMenu::Intensity] != 0)
            splitChannelsTmp_m.at(1).convertTo(splitChannelsTmp_m[1], -1, 1.0, parameter[AdjustMenu::Intensity]);

        //adjust the saturation
        if(parameter[AdjustMenu::Saturation] != 0)
            splitChannelsTmp_m.at(2).convertTo(splitChannelsTmp_m[2], -1, 1.0, parameter[AdjustMenu::Saturation]);

        //adjust gamma by 255(i/255)^(1/gamma) where gamma 0.5 to 3.0
        if(parameter[AdjustMenu::Gamma] != 1.0 || parameter[AdjustMenu::Highlight] != 0.0
                || parameter[AdjustMenu::Shadows] != 0.0)
        {
            //fill LUT for gamma adjustment
            float tmpGamma;
            for(int i = 0; i < 256; i++)
            {
                //adjust gamma
                tmpGamma = 255.0 * pow(i / 255.0, 1.0 / parameter[AdjustMenu::Gamma]);

                /* Bound the shadow adjustment to all pixels below 149 such that the x axis is not
                     * crossed (output is not fliped) in the adjustment equation -(x/50.0)^4 + x and is
                     * handled as a step function. Function is deisned to taper towards zero as the bound
                     * is approached in conjunction with the highlight adjustment. Equation inverted for
                     * subtraction. Shadow operates on the gamma adjusted LUT from a range of -80 to 80*/
                if(i < 149 && parameter[AdjustMenu::Shadows] != 0.0)
                {
                    float tmpShadow;
                    if(parameter[AdjustMenu::Shadows] > 0.0)
                    {
                        //-(x/50.0)^4 + shadow
                        tmpShadow = -1 * pow(i / 50.0, 4) + parameter[AdjustMenu::Shadows];
                        if(tmpShadow > 0)
                            tmpGamma += tmpShadow;
                    }
                    else
                    {
                        //(x/50.0)^4 - shadow .. (shadow is negative)
                        tmpShadow = pow(i / 50.0, 4) + parameter[AdjustMenu::Shadows];
                        if(tmpShadow < 0)
                            tmpGamma += tmpShadow;
                    }
                }

                /* Bound the highlight adjustment to all pixels above 106 such that the x axis is not
                     * crossed (output is not fliped) in the adjustment equation -(x/50.0 - 5.1)^4 + x and is
                     * handled as a step function. Function is deisned to taper towards zero as the bound
                     * is approached in conjunction with the highlight adjustment.Equation inverted for
                     * subtraction. Highlight operates on the gamma adjusted LUT from a range of -80 to 80*/
                if(i > 106 && parameter[AdjustMenu::Highlight] != 0.0)
                {
                    float tmpHighlight;
                    if(parameter[AdjustMenu::Highlight] > 0.0)
                    {
                        //-(x/50.0 - 5.1)^4 + highlight
                        tmpHighlight = -1 * pow(i / 50.0 - 5.1, 4) + parameter[AdjustMenu::Highlight];
                        if(tmpHighlight > 0)
                            tmpGamma += tmpHighlight;
                    }
                    else
                    {
                        //(x/50.0 - 5.1)^4 - highlight .. (highlight is negative)
                        tmpHighlight = pow(i / 50.0 - 5.1, 4) + parameter[AdjustMenu::Highlight];
                        if(tmpHighlight < 0)
                            tmpGamma += tmpHighlight;
                    }
                }

                //limit gamma adjusted values
                if(tmpGamma > 255)
                    tmpGamma = 255;
                else if(tmpGamma < 0)
                    tmpGamma = 0;

                lookUpTable_m.data[i] = tmpGamma;
            }

            //replace pixel values based on their LUT value
            cv::LUT(splitChannelsTmp_m.at(1), lookUpTable_m, splitChannelsTmp_m[1]);
        }

        cv::merge(splitChannelsTmp_m, *previewImage_m);
        cv::cvtColor(*previewImage_m, *previewImage_m, cv::COLOR_HLS2RGB);
    }


    //--convert from color to grayscale if != 1.0
    if(parameter[AdjustMenu::Color] != 1.0)
    {
        cv::cvtColor(*previewImage_m, splitChannelsTmp_m[0], cv::COLOR_RGB2GRAY);
        splitChannelsTmp_m.at(0).copyTo(splitChannelsTmp_m.at(1));
        splitChannelsTmp_m.at(0).copyTo(splitChannelsTmp_m.at(2));
        cv::merge(splitChannelsTmp_m, *previewImage_m);
    }


    //--perform contrast and brightness operation if sliders are not at initial positions
    if (parameter[AdjustMenu::Brightness] != 0.0 || parameter[AdjustMenu::Contrast] != 1.0)
    {
        float alpha = parameter[AdjustMenu::Contrast];
        float beta = parameter[AdjustMenu::Brightness];

        //calculate brightness correction
        if(alpha >= 1)
            beta += -72.8 * log2(alpha);
        else
            beta += 127 * -log2(alpha) / sqrt(1 / alpha);

        //perform contrast computation and prime source buffer
        previewImage_m->convertTo(*previewImage_m, -1, alpha, beta);
    }


    //after computation is complete, push image and histogram to GUI if changes were made
    if(mutex_m)
        mutex_m->unlock();
}
