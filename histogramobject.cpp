#include "histogramobject.h"
#include <QImage>
#include "histogramdata.h"
#include <QDebug>

/* default constructor, creates Histogram Data */
HistogramObject::HistogramObject() : data(new HistogramData) {}

/* constructor fills newly created histogram data from image and emits the signal declaring successful. */
HistogramObject::HistogramObject(const QImage &image) : data(new HistogramData)
{
    if (update(image))
        emit updated();
}

/* constructor that passes the shared data ptr to the shared data object */
HistogramObject::HistogramObject(const HistogramObject &hobj) : data(hobj.data) {}

/* default destructor */
HistogramObject::~HistogramObject() {}

/* creates a deep copy of this object and its data through calling detach on the shared data */
HistogramObject HistogramObject::clone()
{
    data.detach();
    return *this;
}

/* Overloads the [] operator so that the value in the histogram is returned when the [] operator is used.*/
uint32_t HistogramObject::operator[](int index)
{
    return data->histogram.at(index);
}

/* returns the index to access the desired value given the image channel and intensity to access. Returns -1
 * if out of range */
int HistogramObject::getIndex(int channel, int pixelIntensity)
{
    int index = channel * 255 + pixelIntensity;
    if(index +1 > data->histogram.size())
        return -1;
    return index;
}

/* returns the number of channels in which the 8-bit depth histogram represents */
int HistogramObject::getNumChannels()
{
    return data->histogram.size() % 256;
}

bool HistogramObject::update(const QImage &image)
{
    switch(image.format())
    {
        case QImage::Format_Grayscale8:
            data->histogram.fill(0, 256);
            //histo operation
            break;
        case QImage::Format_RGB888:
            data->histogram.fill(0, 256 * 3);
            //histo operation
            break;
        default:
            return false;
    }

    return true;


}
