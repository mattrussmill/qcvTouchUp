/***********************************************************************
* FILENAME :    histogramobject.cpp
*
* LICENSE:
*       qcvTouchUp provides an image processing toolset for editing
*       photographs, purposed and packaged for use in a desktop application
*       user environment. Copyright (C) 2018,  Matthew R. Miller
*
*       This program is free software: you can redistribute it and/or modify
*       it under the terms of the GNU General Public License as published by
*       the Free Software Foundation (version 3 of the License) and the
*       3-clause BSD License as agreed upon through the use of the Qt toolkit
*       and OpenCV libraries in qcvTouchUp development, respectively. Copies
*       of the appropriate license files for qcvTouchup, and its source code,
*       can be found in LICENSE.Qt.txt and LICENSE.CV.txt.
*
*       This program is distributed in the hope that it will be useful,
*       but WITHOUT ANY WARRANTY; without even the implied warranty of
*       MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*       GNU General Public License for more details.
*
*       You should have received a copy of the GNU General Public License and
*       3-clause BSD License along with this program.  If not, please see
*       <http://www.gnu.org/licenses/> and <https://opencv.org/license.html>.
*
*       If you wish to contact the developer about this project, please do so
*       through their account at <https://github.com/mattrussmill>
*
* DESCRIPTION :
*       This object maintains and calculates a multi-channel histogram for an
*       image, including handles all of the shared data operations for implicit
*       histogram buffer sharing.
*
*
* AUTHOR :  Matthew R. Miller       START DATE :    June 6, 2019
*
* CHANGES : N/A - N/A
*
* VERSION       DATE                    WHO                     DETAIL
* 0.1           June 17, 2019       Matthew R. Miller       Initial Rev
*
************************************************************************/
#include "histogramobject.h"
#include <QImage>
#include "histogramdata.h"
#include <QDebug>

/* default constructor, creates Histogram Data */
HistogramObject::HistogramObject() : data_m(new HistogramData) {}

/* constructor fills newly created histogram data from image and emits the signal declaring successful. */
HistogramObject::HistogramObject(const QImage &image) : data_m(new HistogramData)
{
    if (update(image))
        emit updated();
}

/* constructor that passes the shared data ptr to the shared data object */
HistogramObject::HistogramObject(const HistogramObject &hobj) : data_m(hobj.data_m) {}

/* default destructor */
HistogramObject::~HistogramObject() {}

/* creates a deep copy of this object and its data through calling detach on the shared data */
HistogramObject HistogramObject::clone()
{
    data_m.detach();
    return *this;
}

/* Overloads the [] operator so that the value in the histogram is returned when the [] operator is used.*/
uint32_t HistogramObject::operator[](int index)
{
    return data_m->histogram.at(index);
}

/* returns the index to access the desired value given the image channel and intensity to access. Returns -1
 * if out of range */
int HistogramObject::getIndex(int channel, int pixelIntensity)
{
    int index = channel * 255 + pixelIntensity;
    if(index +1 > data_m->histogram.size())
        return -1;
    return index;
}

/* returns the number of channels in which the 8-bit depth histogram represents */
int HistogramObject::getNumChannels()
{
    return data_m->histogram.size() % 256;
}

bool HistogramObject::update(const QImage &image)
{
    int size;
    switch(image.format())
    {
        //adds the grayscale values to a single channel histogram
        case QImage::Format_Grayscale8:
        {
            data_m->histogram.fill(0, 256);
            size = image.width() * image.height();
            for(int i = 0; i < size; i++)
                data_m->histogram[*(image.bits() + i)]++;
            break;
        }

        /* Low level solution to align data accesses for 24-bit depth (and aligned) RGB image buffer.
         * Traverses the image in a 32 bit sequence where the RGBR GBRG BRGB sequence can repeat itself.
         * When the sequence can no longer repeate itself, the extra bytes are added to the histogram
         * if they exist. */
        case QImage::Format_RGB888:
        {
            data_m->histogram.fill(0, 256 * 3); //channels: r * 1, g * 2, b * 3
            const uint32_t *bits = reinterpret_cast<const uint32_t*>(image.bits());
            size = image.width() * image.height() * 3;


            //appropriately traverse and insert 32-bit aligned pixel data into histogram channels
            //and set appropriate size for 32-bit traversing of 3 channel 24-bit pixels
            const uint32_t *max_address = bits + (size / 4) - 3; //-3 because +3 every time
            while(bits <= max_address)
            {
                //RGBR
                data_m->histogram[(*bits & 0xFF000000) >> 24]++;          //red
                data_m->histogram[((*bits & 0x00FF0000) >> 16) * 2]++;    //green
                data_m->histogram[((*bits & 0x0000FF00) >> 8) * 3]++;     //blue
                data_m->histogram[*bits & 0x000000FF]++;                  //red
                bits++;

                //GBRG
                data_m->histogram[((*bits & 0xFF000000) >> 24) * 2]++;    //green
                data_m->histogram[((*bits & 0x00FF0000) >> 16) * 3]++;    //blue
                data_m->histogram[(*bits & 0x0000FF00) >> 8]++;           //red
                data_m->histogram[(*bits & 0x000000FF) * 2]++;            //green
                bits++;

                //BRGB
                data_m->histogram[((*bits & 0xFF000000) >> 24) * 3]++;    //blue
                data_m->histogram[(*bits & 0x00FF0000) >> 16]++;          //red
                data_m->histogram[((*bits & 0x0000FF00) >> 8) * 2]++;     //green
                data_m->histogram[(*bits & 0x000000FF) * 3]++;            //blue
                bits++;
            }

            //get trailing bits that aren't 32-bit aligned, capture them too
            if(size % 4)
            {
                const uint8_t *unalignedBits = reinterpret_cast<const uint8_t*>(max_address + 3);
                switch(size % 4)
                {
                    case 3:
                    {
                        data_m->histogram[*(unalignedBits + 6)]++;        //last RGB red
                        data_m->histogram[*(unalignedBits + 7) * 2]++;    //last RGB green
                        data_m->histogram[*(unalignedBits + 8) * 3]++;    //last RGB blue
                        //no break, flow into 21 after 3 if more than 2 px must be evaluated
                    }
                    case 2:
                    {
                        data_m->histogram[*(unalignedBits + 3)]++;        //2nd last RGB red
                        data_m->histogram[*(unalignedBits + 4) * 2]++;    //2nd last RGB green
                        data_m->histogram[*(unalignedBits + 5) * 3]++;    //2nd last RGB blue
                        //no break, flow into 1 after 2 if more than 1 px must be evaluated
                    }

                    case 1:
                    {
                        data_m->histogram[*unalignedBits];              //3rd last RGB red
                        data_m->histogram[*(unalignedBits + 1) * 2];    //3rd last RGB green
                        data_m->histogram[*(unalignedBits + 2) * 3];    //3rd last RGB blue
                        break;
                    }

                    default:
                    {
                        qDebug() << "too many pixels left over when calculating the histogram";
                    }
                }
            }
            break;
        }

        //if image type not added, do nothing and return false
        default:
        {
            qDebug() << "no histogram present for image type";
            return false;
        }
    }
    emit updated();
    return true;
}
