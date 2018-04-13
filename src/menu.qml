/***********************************************************************
* FILENAME :    menu.qml
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

Item {
    id: rootMenu
    anchors.fill: parent
    property color backgroundColor: backend.backgroundColor

    /* rootBackground fetches the default window color from quickmenubackend.cpp which was added as
       its rootContext in mainwindow.cpp - this is how it blends into the window background.*/
//    Rectangle {
//        id: rootBackground
//        color: "transparent" // backend.backgroundColor
//        anchors.fill: parent

        //contains the main menu buttons and graphics - lays them out horizontally from left to right
        RowLayout {
            anchors.fill: parent
            layoutDirection: Qt.LeftToRight

//            Image {
//                id: leftImage
//                source: "qrc:/img/filigree.png"
//                Layout.alignment: Qt.AlignLeft
//                Layout.fillHeight: true
//                Layout.preferredWidth: 100
//                cache: false
//                mirror: true
//                fillMode: Image.PreserveAspectFit
//            }

            //This ListView, listMenu, dynamically generates the buttons from the ListModel items.
            ListView {
                id: listMenu
                Layout.alignment: Qt.AlignCenter
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                cacheBuffer: rootButton.width * 3
                spacing: 9
                model: items
                orientation: ListView.Horizontal
                delegate: Item {
                    id: rootButton
                    width: (rootButton.height - 20) * 2
                    height: parent.height

                    Rectangle {
                        id: button
                        anchors.centerIn: parent
                        width: button.height * 1.75
                        height: rootButton.height - 20
                        border.width: 1
                        border.color: "#501873"
                        radius: 2

                        //Ties the Text color to a Rectangle property so it can be accessed by the MouseArea state machine
                        property color textColor
                        Text { id: itemText; color: button.textColor; text: menuname; anchors.centerIn: parent }

                        //The graphical buttons handle mouse interactions as a state machine in the MouseArea.
                        states: [
                            State {
                                name: "Hovering"
                                PropertyChanges {
                                    target: button
                                    color: "#c7eb5e"
                                    //textColor: "black"
                                    scale: 1.5
                                }
                            },
                            State {
                                name: "Pressed"
                                PropertyChanges{
                                    target: button
                                    color: "#c7eb5e"
                                    //textColor: "black"
                                    scale: 1.3
                                }
                            },
                            State {
                                name: "Default"
                                PropertyChanges {
                                    target: button
                                    color: backgroundColor
                                    //textColor: backend.backgroundColor
                                    scale: 1

                                }
                            }

                        ]

                        //Transitions are used to define the animations between state transitions
                        transitions: [
                            Transition {
                                from: "Default"; to: "Hovering"
                                ColorAnimation { duration: 200 }
                            },
                            Transition {
                                from: "*"; to: "Pressed"
                                ColorAnimation { duration: 50 }
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
                preferredHighlightBegin: (width / 2) - (listMenu.spacing)
                preferredHighlightEnd: (width / 2) + (listMenu.spacing)
                highlightRangeMode: ListView.StrictlyEnforceRange
                Component.onCompleted: currentIndex = count / 2

            } //listMenu

//            Image {
//                id: rightImage
//                source: "qrc:/img/filigree.png"
//                Layout.alignment: Qt.AlignRight
//                Layout.fillHeight: true
//                Layout.preferredWidth: 100
//                cache: false
//                mirror: false
//                fillMode: Image.PreserveAspectFit
//            }

        } //RowLayout


        /*menuOverlay sits on the layer above the graphical menu items. It provides the fading effect when menu
          items reach the end of the menu.*/
        RowLayout {
            id: menuOverlay
            anchors.fill: parent
            layoutDirection: Qt.LeftToRight

            Rectangle {
                Layout.alignment: Qt.AlignLeft
                Layout.leftMargin: leftImage.width
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
                Layout.rightMargin: rightImage.width
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

    //} //rootBackground

    /*The items ListModel contains the menu name and menuid. When a button is clicked in the graphical
      menu, its corresponding menuid is sent as a signal to the quickmenubackend.cpp container that
      instantiated toolmenu.qml in mainwindow.cpp. A menuid represents a corresponding wiget in the
      mainwindow application. When no subwidget menus are opened from this ListModel, the container's
      menuid will be set to zero. All menuids in this list shall have a unique number, the same as its
      associated wiget, larger than zero. The titles should also match the window titles in those wigets.*/
    ListModel { //* = start with
        id: items

        ListElement {
            menuname: "Adjust" //*
            menuid: 1
        }

        ListElement {
            menuname: "Filter" //*
            menuid: 2
        }

        ListElement {
            menuname: "Transform" //*
            menuid: 3
        }

        ListElement {
            menuname: "Equalize" //*
            menuid: 4
        }

        ListElement {
            menuname: "Presets"
            menuid: 5
        }

        ListElement {
            menuname: "SelectiveColor"
            menuid: 6
        }

        ListElement {
            menuname: "Distort"
            menuid: 7
        }

        ListElement {
            menuname: "AutoAdjust"
            menuid: 8
        }

//        {FAIL, LEVELS, FILTER, TRANSFORM, HISTOGRAM,
//                           PRESET, SELECTIVECOLOR, DISTORT, AUTOADJUST}

    } //ListModel
} //rootMenu
