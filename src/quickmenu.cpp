/***********************************************************************
* FILENAME :    quickmenu.cpp
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
*       It is important to note that when this object is destroyed it does
*       not destroy the QQuickView that was passed to it. That must be handled
*       by the application destructor if a parent is not tied to the
*       associated QQuickView. When instantiating and destroying the QQuickView
*       from this object, a segfault occurs during QQuickView destruction.
*
*       Ver0.8 was changed such that the QQuickMenu class is inside of this object
*       instead of being instantiated outside of the object and attached. Rendering
*       The QML surface using OpenGL and assigning the view's background color to
*       match the QuickMenu backend also helped mitigate gitter issues during
*       window resizing.
*
* AUTHOR :  Matthew R. Miller       START DATE :    December 19, 2017
*
* CHANGES : N/A - N/A
*
* VERSION       DATE            WHO                     DETAIL
* 0.1           12/19/2017      Matthew R. Miller       Initial Rev
* 0.8           01/25/2018      Matthew R. Miller       Enhancements
*
************************************************************************/

#include "quickmenu.h"
#include <QQuickView>
#include <QQmlEngine>
#include <QQmlContext>
#include <QHBoxLayout>
#include <QSurfaceFormat>

/* The QuickMenu constructor first creates a QQuickView to display and run
 * the QML code in the correct environment. The QML engine is then connected
 * to the QuickMenu backend class so that the QML has access to the C++ class
 * properties delegated in quickmenu.h. A QSurfaceFormat class is established
 * so that the QQuickMenu surface is rendered using OpenGL. The surface color
 * is then set to the QuickMenu background default. Finally the QQuickMenu is
 * encapsulated and set inside of a layout so that it fills the widget. */
QuickMenu::QuickMenu(QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_OpaquePaintEvent);

    QQuickView *view = new QQuickView(QUrl("qrc:///qml/menu.qml"));
    view->setResizeMode(QQuickView::SizeRootObjectToView);
    view->engine()->rootContext()->setContextProperty("backend", this);

    QSurfaceFormat format;
    format.setRenderableType(QSurfaceFormat::OpenGL);
    view->setFormat(format);
    view->setColor(this->palette().color(QWidget::backgroundRole()));
    view->setClearBeforeRendering(true);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setMargin(0);
    layout->addWidget(createWindowContainer(view, this));
    setLayout(layout);
    setFocusPolicy(Qt::ClickFocus);
}

/* Method used to access the most recently selected menu item by the QML backend.
 * This variable is initialized to zero */
int QuickMenu::menuItem()
{
    return m_menuItem;
}

/* This method sets the m_menuItem member passed (clicked on) from the QML main menu
 * through the Qt object properties. If the menu item is selected (clicked) twice,
 * the associated menu id number is reset to zero. */
void QuickMenu::setMenuItem(int m)
{
    if(m == m_menuItem)
        m_menuItem = 0;
    else
        m_menuItem = m;

    emit menuItemClicked(m_menuItem);
}

// Resets the m_menuItem member to its initial state of zero.
void QuickMenu::resetMenuIndex()
{
    setMenuItem(0);
}

/* This method returns the background color of the QuickMenu class. It is available
 * to the QML engine root context, once established, to achieve a transparent effect such
 * that the QML objects generated look like part of the QWidget if set as the QML item
 * background.*/
const QString QuickMenu::backgroundColor()
{
    return QString("#"+QString::number(this->palette().color(QWidget::backgroundRole()).rgb(), 16));
}
