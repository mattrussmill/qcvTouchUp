#include "transformworker.h"
#include <QMutex>
#include <QString>
#include <QRect>
#include <opencv2/imgproc.hpp>
#include "signalsuppressor.h"
#include <QDebug>

TransformWorker::TransformWorker(const cv::Mat *masterImage, cv::Mat *previewImage, QMutex *mutex, QObject *parent) : QObject(parent)
{
    emit updateStatus("Transform Menu initializing...");
    autoCropforRotate_m = false;

    mutex_m = mutex;
    masterImage_m = masterImage;
    previewImage_m = previewImage;
    qDebug() << "Transform Worker Created! - Images:" << masterImage_m << previewImage_m;

    emit updateStatus("");

}

// destructor
TransformWorker::~TransformWorker()
{
    qDebug() << "TransformWorker destroyed";
}

/* This slot is used to update the member addresses for the master and preview images stored
 * in the controlling thread. If the Mat's become empty in the controlling thread this slot
 * should be signaled with nullptrs to signify they are empty. */
void TransformWorker::receiveImageAddresses(const cv::Mat *masterImage, cv::Mat *previewImage)
{
    masterImage_m = masterImage;
    previewImage_m = previewImage;
    qDebug() << "Transform Worker Images:" << masterImage_m << previewImage_m;
}

/* This member (slot) recieves the data from the controlling class (slow thread). The data
 * is sent as a pointer to the class itself who's member contains the data. To see how this
 * works see signalsuppressor.h/cpp. The format is tied to the associated menu object. */
void TransformWorker::receiveSuppressedSignal(SignalSuppressor *dataContainer)
{
    doRotateComputation(dataContainer->getNewData().toInt());
}

/* This slot performs a cropping computation on the image. It is passed a ROI, which is assumed
 * to already be in bounds from the signal, assigns the new ROI to the dstRGBImage_m buffer. The
 * postImageOperationMutex is not used as the image on screen should not be updated (signal) until
 * the crop selection is complete according to the user so that adjustments to the ROI can be made.*/
void TransformWorker::doCropComputation(QRect roi)
{
    emit updateStatus("Working...");
    if(mutex_m) mutex_m->lock();
    if(masterImage_m == nullptr || previewImage_m == nullptr)
    {
        if(mutex_m) mutex_m->unlock();
        qDebug() << "Cannot perform Crop, image not attached";
        return;
    }

    cv::Rect region(roi.topLeft().x(), roi.topLeft().y(), roi.width(), roi.height());
    *previewImage_m = cv::Mat(*masterImage_m, region);
    //after computation is complete, push image and histogram to GUI if changes were made
    if(mutex_m) mutex_m->unlock();
    emit updateStatus("");
}

/* This slot performs the rotation computation on the image. It is passed a value that represents the
 * number of degrees to rotate the image. After the image is rotated using an adjusted rotation matrix,
 * the new image is warped to fit inside the rectangular boundary of the rotated region. If auto-crop is
 * enabled the method attempts to crop the image after rotation to a useful size within the rotated region,
 * but excluding any black corners that were not part of the original image. In the notes a suggestion for
 * a better solution is discussed for a further update */
void TransformWorker::doRotateComputation(int degree)
{
    emit updateStatus("Working...");
    if(mutex_m) mutex_m->lock();
    if(masterImage_m == nullptr || previewImage_m == nullptr)
    {
        if(mutex_m) mutex_m->unlock();
        qDebug() << "Cannot perform Rotate, image not attached";
        return;
    }

    //@@@@@@@@@@@@@@@@@@@ TRY UMAT SPEEDUP @@@@@@@@@@@@@@@@@

    //center of rotation, rotation matrix, and containing size for rotation
    degree *= -1;
    cv::Point2f center = cv::Point2f((masterImage_m->cols -1) / 2.0, (masterImage_m->rows -1) / 2.0);
    cv::Mat rotationMatrix = cv::getRotationMatrix2D(center, degree, 1);
    cv::RotatedRect rotatedRegion(center, masterImage_m->size(), degree);
    cv::Rect boundingRegion = rotatedRegion.boundingRect();

    //adjust the rotation matrix
    rotationMatrix.at<double>(0, 2) += boundingRegion.width / 2.0 - masterImage_m->cols / 2.0;
    rotationMatrix.at<double>(1, 2) += boundingRegion.height / 2.0 - masterImage_m->rows / 2.0;

    //perform transform
    *previewImage_m = cv::Mat(boundingRegion.size(), masterImage_m->type());
    cv::warpAffine(*masterImage_m, *previewImage_m, rotationMatrix, boundingRegion.size());

    /* crop image so that no black edges due to rotation are showing if not square
     * NOTE: maybe in a later update use the opposite line equations here to calculate the optimal position
     * of image within the original frame, then warp to fit instead of trying to adjust the frame to the
     * optimal size within the rotated region*/
    if(autoCropforRotate_m && degree != 0 && abs(degree) != 90 && abs(degree) != 180)
    {
        cv::Point2f corners[4];
        rotatedRegion.points(corners);

        //offset for points in boundingRegion
        for(int i = 0; i < 4; i ++)
        {
            corners[i].x += boundingRegion.width / 2.0 - masterImage_m->cols / 2.0;
            corners[i].y += boundingRegion.height / 2.0 - masterImage_m->rows / 2.0;
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

        //for testing purposes to visualize points (good place to visualize points if debugging)
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
        }//(good place to visualize points if debugging)


        //using the location of the opposite intercept points, attempt to create a ROI large as possible while excluding black edges
        double x, y, width, height;
        if(interceptOppositeOfTopCorner.x < interceptOppositeOfBottomCorner.x)
        {
            x = (interceptOppositeOfRightCorner.x + interceptOppositeOfTopCorner.x) / 2.0;
            width = (interceptOppositeOfLeftCorner.x + interceptOppositeOfBottomCorner.x) / 2.0 - x;
        }
        else
        {
            x = (interceptOppositeOfRightCorner.x + interceptOppositeOfBottomCorner.x) / 2.0;
            width = (interceptOppositeOfLeftCorner.x + interceptOppositeOfTopCorner.x) / 2.0 - x;
        }

        if(interceptOppositeOfLeftCorner.y < interceptOppositeOfRightCorner.y)
        {
            y = (interceptOppositeOfBottomCorner.y + interceptOppositeOfLeftCorner.y) / 2.0;
            height = (interceptOppositeOfTopCorner.y + interceptOppositeOfRightCorner.y) / 2.0 - y;
        }
        else
        {
            y = (interceptOppositeOfBottomCorner.y + interceptOppositeOfRightCorner.y) / 2.0;
            height = (interceptOppositeOfTopCorner.y + interceptOppositeOfLeftCorner.y) / 2.0 - y;
        }

        //Hagrid tells you you're a wizard and you draw/crop this rectangle (good place to visualize rect region if debugging)
        cv::Rect cropRegion(x, y, width, height);
        //cv::rectangle(*dstRGBImage_m, cropRegion, cv::Scalar( 255, 0, 0 ), 3);
        cv::Mat(*previewImage_m, cropRegion).copyTo(*previewImage_m);
    }

    //after computation is complete, push image and histogram to GUI if changes were made
    if(mutex_m) mutex_m->unlock();
    emit updateDisplayedImage();
    emit updateStatus("");
}

//sets the autoCropForRotate member used in the doRotateComputation function to enable or disable the auto crop attempt.
void TransformWorker::setAutoCropForRotate(bool value)
{
    autoCropforRotate_m = value;
}

//performs a resize operation using 4x4 interpolation
void TransformWorker::doScaleComputation(QRect newSize)
{
    emit updateStatus("Working...");
    if(mutex_m) mutex_m->lock();
    if(masterImage_m == nullptr || previewImage_m == nullptr)
    {
        if(mutex_m) mutex_m->unlock();
        qDebug() << "Cannot perform Rotate, image not attached";
        return;
    }
    cv::resize(*masterImage_m, *previewImage_m, cv::Size(newSize.width(), newSize.height()), 0, 0, cv::INTER_CUBIC);
    if(mutex_m) mutex_m->unlock();
    emit updateDisplayedImage();
    emit updateStatus("");
}
