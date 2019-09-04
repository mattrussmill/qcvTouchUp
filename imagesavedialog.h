/***********************************************************************
* FILENAME :    imagesavedialog.h
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
*       This dialog provides the user a prompt and the desired functionality
*       to save images edited by the application and contained in an OpenCV
*       Mat container by extending the non-native file dialog provided by Qt
*       and implementing the OpenCV imwrite functionality.
*
* NOTES :
*       For each image format desired as a save option, an individual
*       widget must be created - see imagesave[format]menu objects.
*
*
* AUTHOR :  Matthew R. Miller       START DATE :    March 03/04/2019
*
* CHANGES : N/A - N/A
*
* VERSION       DATE            WHO                     DETAIL
* 0.1           08/29/2019      Matthew R. Miller       Initial Rev
*
************************************************************************/
#ifndef IMAGESAVEDIALOG_H
#define IMAGESAVEDIALOG_H

#include <QFileDialog>
#include <opencv2/imgcodecs.hpp>
class QStackedWidget;
class QPushButton;
class QString;
class ImageSaveJpegMenu;
class ImageSavePngMenu;
class ImageSaveWebpMenu;

class ImageSaveDialog : public QFileDialog
{
    Q_OBJECT

public:    
    explicit ImageSaveDialog(cv::Mat &image, QWidget *parent = nullptr);
    ImageSaveDialog(cv::Mat &image, QWidget *parent = nullptr, const QString &caption = QString(),
                    const QString &directory = QString());
    ~ImageSaveDialog();

protected slots:
    void saveAccepted();

protected:
    void saveJPEG(QString &filePath);
    void saveWebP(QString &filePath);
    void savePNG(QString &filePath);

private slots:
    void advancedOptionsToggled();
    void displayFilteredSaveOptions();

private:
    enum ImageType{JPEG = 0, PNG = 1, WEBP = 2};
    void appendImageOptionsWidget();
    void appendAdvancedOptionsButton();
    cv::Mat *image_m;
    QPushButton *buttonAdvancedOptions_m = nullptr;
    QStackedWidget *saveOptionsWidget_m = nullptr;
    ImageSaveJpegMenu *jpegMenu_m = nullptr;
    ImageSavePngMenu *pngMenu_m = nullptr;
    ImageSaveWebpMenu *webpMenu_m = nullptr;

};

#endif // IMAGESAVEDIALOG_H
