/***********************************************************************
* FILENAME :    quickmenubackend.h
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
*       This is the backend C++ class for the toolmenu.qml front end menu.
*       The QuickMenu class acts as an intermediate data structure
*       between the QML engine and main QWidget application. Methods are
*       shared between the front and back end through the Qt property system.
*       This class must be set as the associated QQuickView's root context
*       property for the QML and C++ to pass data between the QML engine and
*       Widget application.
*
* NOTES :
*       The QML is able to blend in with the application as the Widget
*       background color is passed as read-only to the QML front end. A signal
*       is sent notifying the main application by the setMenuItem method which
*       is called when the QML writes its associated button (menuid) id to the
*       backend's menuItem variable when a button is clicked.
*
* AUTHOR :  Matthew R. Miller       START DATE :    December 19, 2017
*
* CHANGES : N/A - N/A
*
* VERSION       DATE            WHO                     DETAIL
* 0.1           12/19/2017      Matthew R. Miller       Initial Rev
*
************************************************************************/

#ifndef QUICKMENU_H
#define QUICKMENU_H

#include <QWidget>
class QString;
class QQuickView;

//potential fix for qml metatype pointer warning: http://www.qtcentre.org/threads/14835-How-to-use-Q_DECLARE_METATYPE?p=76295#post76295
class QuickMenu : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString backgroundColor READ backgroundColor)
    Q_PROPERTY(int menuItem READ menuItem WRITE setMenuItem NOTIFY menuItemClicked)

public:
    explicit QuickMenu(QWidget *parent = nullptr);
    int menuItem();
    void setMenuItem(int m);

signals:
    void menuItemClicked(int menuIndex);

public slots:
    void resetMenuIndex();

private:
    const QString backgroundColor();
    int menuItem_m = 0;
};

#endif // QUICKMENU_H
