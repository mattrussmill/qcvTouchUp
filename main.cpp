/***********************************************************************
* FILENAME :    main.cpp
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
*       This file creates the entry point for the program, instantiates
*       the GUI interface and launches the QApplication.
*
* NOTES :
*       None.
*
* AUTHOR :  Matthew R. Miller       START DATE :    November 11, 2017
*
* CHANGES : N/A - N/A
*
* VERSION       DATE            WHO                     DETAIL
* 0.1           11/11/2017      Matthew R. Miller       Initial Rev
*
************************************************************************/

#include "mainwindow.h"
#include "signalsuppressor.h"
#include <QApplication>
#include <QMetaType>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //load stylesheet
    QFile styleFile(":/css/stylesheet.css");
    styleFile.open(QFile::ReadOnly);
    QString style(styleFile.readAll());
    styleFile.close();
    a.setStyleSheet(style);

    //types registered for queued signal/slot connections
    qRegisterMetaType<QVector<float>>("QVector<float>");
    qRegisterMetaType<QVector<int>>("QVector<int>");
    qRegisterMetaType<SignalSuppressor*>("SignalSuppressor*");
    qRegisterMetaType<cv::Mat*>("cv::Mat*");

    MainWindow w;
    w.show();

    return a.exec();
}
