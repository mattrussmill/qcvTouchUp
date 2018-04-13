/***********************************************************************
* FILENAME :    main.cpp
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
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow w;
    w.show();

    return a.exec();
}
