/***********************************************************************
* FILENAME :    imagesavejpegmenu.h
*
* LICENSE:
*       qcvTouchUp provides an image processing toolset for editing
*       photographs, purposed and packaged for use in a desktop application
*       user environment. Copyright (C) 2018,  Matthew R. Miller
*
*       This program is free software: you can redistribute it and/or modify
*       it under the terms of the GNU General Public License as published by
*       the Free Software Foundation (version 3 of the License).
*
*       The framework and libraries used to create this software are licenced
*       under the  GNU Lesser General Public License (LGPL) version 3 and the
*       3-clause BSD License as agreed upon through the use of the Qt toolkit
*       and OpenCV libraries respectively. Copies of the appropriate licenses
*       for qcvTouchup, and its source code, can be found in LICENSE.txt,
*       LICENSE.Qt.txt, and LICENSE.CV.txt. If not, please see
*       <http://www.gnu.org/licenses/> and <https://opencv.org/license.html>
*       for further information on licensing.
*
*       This program is distributed in the hope that it will be useful,
*       but WITHOUT ANY WARRANTY; without even the implied warranty of
*       MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*       GNU General Public License for more details.
*
*       If you wish to contact the developer about this project, please do so
*       through their account at <https://github.com/mattrussmill>
*
* DESCRIPTION :
*       This widget is used to collect user input parameters for saving an
*       image in the JPEG file format.
*
* NOTES :
*       The values captured are associated with cv::imwrite(), an OpenCV
*       function.
*
*
* AUTHOR :  Matthew R. Miller       START DATE :    March 03/04/2019
*
* CHANGES : N/A - N/A
*
* VERSION       DATE            WHO                     DETAIL
* 0.1           07/07/2019      Matthew R. Miller       Initial Rev
* 0.2           11/05/2019      Matthew R. Miller       Remove chroma/luma quality
*
************************************************************************/
#ifndef IMAGESAVEJPEGMENU_H
#define IMAGESAVEJPEGMENU_H

#include <QWidget>

namespace Ui {
class ImageSaveJpegMenu;
}

class ImageSaveJpegMenu : public QWidget
{
    Q_OBJECT

public:
    explicit ImageSaveJpegMenu(QWidget *parent = nullptr);
    ~ImageSaveJpegMenu();
    int getQuality();
    int getRestartInterval();
    int getProgressiveScan();
    int getBaselineOptimized();

private:
    Ui::ImageSaveJpegMenu *ui;
};

#endif // IMAGESAVEJPEGMENU_H
