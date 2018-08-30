/***********************************************************************
* FILENAME :    menu.qml
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
*       This file contains the QML for the main graphical menu of the
*       qvcTouchUp application. The buttons in this menu are used to launch
*       sub-wigets which contain groupings of image configuration settings
*       for the user to adjust.
*
* NOTES :
*       The menu spacing is not perfect. If the qWiget window container size
*       is changed from its default height of 60px, values within the graphical
*       menu may need adjusted. The menu was designed to rely on the window
*       container, but not to the extent of large dynamic resizing of the menu.
*
*       Ver0.8
*
* AUTHOR :  Matthew R. Miller       START DATE :    December 19, 2017
*
* CHANGES : N/A - N/A
*
* VERSION       DATE            WHO                     DETAIL
* 0.1           12/19/2017      Matthew R. Miller       Initial Rev
* 0.8                           Matthew R. Miller       Styling & Efficiency
************************************************************************/

import QtQuick 2.6
import QtQuick.Layouts 1.1
import QtGraphicalEffects 1.0

/* rootMenu fetches the default window color from quickmenubackend.cpp which was added as
   its rootContext in mainwindow.cpp - this is how it blends into the window background.*/
Item {
    id: rootMenu
    anchors.fill: parent
    property color backgroundColor: backend.backgroundColor

        //contains the main menu buttons and graphics - lays them out horizontally from left to right
        RowLayout {
            anchors.fill: parent
            layoutDirection: Qt.LeftToRight

            //This ListView, listMenu, dynamically generates the buttons from the ListModel items.
            ListView {
                id: listMenu
                Layout.alignment: Qt.AlignCenter
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                spacing: rootMenu.height / 3
                model: items
                orientation: ListView.Horizontal

                delegate: Item {
                    id: rootButton
                    width: itemText.width
                    height: parent.height

                    /* Rectangle is used to correct fade overlay slipping behind rootButton Item during QQuickView resize
                      and also contains properties for child object animations*/
                    Rectangle {
                        id: button
                        anchors.centerIn: parent
                        color: "transparent" //backgroundColor
                        width: rootButton.width
                        height: rootButton.height - (rootButton.height * 0.3) //room to expand on button transition
                        property color textColor
                        property color glowColor

                        RadialGradient {
                            id: textGlow
                            anchors.fill: parent
                            gradient: Gradient {
                                GradientStop { position: 0.0; color: button.glowColor}
                                GradientStop { position: 0.5; color: "transparent" }
                            }
                        }

                        Text {
                            id: itemText
                            color: button.textColor
                            text: menuname
                            font.pixelSize: rootMenu.height / 3
                            anchors.centerIn: parent
                        }

                        //The graphical buttons handle mouse interactions as a state machine in the MouseArea.
                        states: [
                            State {
                                name: "Hovering"
                                PropertyChanges {
                                    target: button
                                    glowColor: "#c7eb5e"
                                    scale: 1.25
                                }
                            },
                            State {
                                name: "Pressed"
                                PropertyChanges{
                                    target: button
                                    glowColor: "#c7eb5e"
                                    scale: 1.1
                                }
                            },
                            State {
                                name: "Default"
                                PropertyChanges {
                                    target: button
                                    glowColor: backgroundColor
                                    scale: 1
                                }
                            }
                        ]

                        //Transitions are used to define the animations between state transitions
                        transitions: [
                            Transition {
                                from: "Default"; to: "Hovering"
                                ColorAnimation { duration: 110 }
                            },
                            Transition {
                                from: "*"; to: "Pressed"
                                ColorAnimation { duration: 60 }
                            }
                        ]

                        /*Using a MouseArea keeps the menu selection separate from the ListView, or listMenu, selection to
                          maintain the list's center position on the initial load of the menu. The MouseArea assigns the
                          states of the state machine to mouse actions as well as initiates the "menuItemsChanged" signal
                          in the quickmenubackend.cpp class.*/
                        MouseArea {
                            anchors.fill: parent
                            hoverEnabled: true
                            onEntered: button.state = "Hovering"
                            onExited: button.state = "Default"
                            onPressed: button.state = "Pressed"
                            onReleased: containsMouse ? button.state = "Hovering" : button.state = "Default"
                            onClicked: backend.menuItem = menuid;
                        }

                        Behavior on scale { NumberAnimation { duration: 100 } }
                        Component.onCompleted: button.state = "Default"

                    } //button

                } //rootButton

                /* Centers the list by setting the boundary area (spacing width x2) for the highlighted item
                   to the center of the view (as listMenu fills its parent), and setting the initial index to
                   the center of the items list in listMenu.*/
                preferredHighlightBegin: (width / 2) - (listMenu.spacing) //ADJUST THESE AND TURN OFF IF ALL FIT ON SCREEN!
                preferredHighlightEnd: (width / 2) + (listMenu.spacing)   //DO THE SAME WITH TRANSPARENT EDGES?
                highlightRangeMode: ListView.StrictlyEnforceRange
                Component.onCompleted: currentIndex = count / 2

            } //listMenu

        } //RowLayout


        /*menuOverlay sits on the layer above the graphical menu items. It provides the fading effect when menu
          items reach the end of the menu.*/
        RowLayout {
            id: menuOverlay
            anchors.fill: parent
            layoutDirection: Qt.LeftToRight

            Rectangle {
                Layout.alignment: Qt.AlignLeft
                Layout.preferredHeight: parent.height
                Layout.preferredWidth: parent.height
                antialiasing: false
                rotation: 90
                gradient: Gradient {
                    GradientStop { position: 0.0; color: "transparent" }
                    GradientStop { position: 0.8; color: backgroundColor }
                }
            }

            Rectangle {
                Layout.alignment: Qt.AlignRight
                Layout.preferredHeight: parent.height
                Layout.preferredWidth: parent.height
                antialiasing: false
                rotation: -90
                gradient: Gradient {
                    GradientStop { position: 0.0; color: "transparent" }
                    GradientStop { position: 0.8; color: backgroundColor }
                }
            }
        } //menuOverlay

    /*The items ListModel contains the menu name and menuid. When a button is clicked in the graphical
      menu, its corresponding menuid int is sent as a signal to the quickmenubackend.cpp container that
      instantiated toolmenu.qml in mainwindow.cpp. A menuid represents a corresponding wiget in the
      mainwindow application. When no subwidget menus are opened from this ListModel, the container's
      menuid will be set to zero. All menuids in this list shall have a unique number, the same as its
      associated wiget, larger than zero. The titles should also match the window titles in those wigets.
      Enumerations were not used as they would need to be maintained between C++ and QML anyway*/
    ListModel { //* = start with
        id: items

        ListElement {
            menuname: "Adjust"
            menuid: 1
        }

        ListElement {
            menuname: "Filter"
            menuid: 2
        }

        ListElement {
            menuname: "Temperature"
            menuid: 3
        }

        ListElement {
            menuname: "Transform"
            menuid: 4
        }

        ListElement {
            menuname: "Slice"
            menuid: 5
        }

        ListElement {
            menuname: "Select"
            menuid: 6
        }

    } //ListModel
} //rootMenu
