/***********************************************************************
* FILENAME :    quickmenubackend.h
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
#include <QString>
#include <QQuickView>

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
    int m_menuItem = 0;
};

#endif // QUICKMENU_H
