import QtQuick 2.7
import QtQuick.Window 2.2
import QtQuick.Controls 2.3
import QtQml.Models 2.2
import QtQuick.Dialogs 1.3
import QtQuick.Extras 1.4
import QtQuick.Layouts 1.3

import ch.epfl.chili 1.0

import "./pages"

ApplicationWindow {
    id: root
    visible: true

    property bool mobile: Qt.platform.os === "android"
    width: mobile ? Screen.width : 1280
    height: mobile ? Screen.height : 720

    Config {
        id: config
    }

    Annotate {
        id: annotate
        visible: false
    }
    
    header: ToolBar {
        contentHeight: toolButton.implicitHeight

        ToolButton {
            id: toolButton
            text: "\u2630"
            font.pixelSize: Qt.application.font.pixelSize * 1.6
            onClicked: {
                stackView.pop()
                drawer.open()
            }
        }

        Label {
            id: headerLabel
            text: stackView.currentItem.title
            anchors.centerIn: parent
        }
    }

    Popup {
        id: popup
        modal: true
        focus: true

        ColumnLayout {
            anchors.fill: parent
            Text {
                id: popupText
                font.bold: true
            }
        }
    }

    Drawer {
        id: drawer
        width: root.width * 0.4
        height: root.height

        Column {
            id: mainColumn
            anchors.fill: parent
            padding: 5

            ItemDelegate {
                id: startItem
                text: qsTr("Configure")
                width: parent.width
                onClicked: {
                    annotate.visible = false
                    config.visible = true
                    stackView.push(config)
                    drawer.close()
                }
            }

            ItemDelegate {
                text: qsTr("Annotate")
                width: parent.width
                onClicked: {
                    if (config.bagAnnotator.status != RosBagAnnotator.READY) {
                        popupText.text = "Please configure before annotating!"
                        popup.open()
                        return
                    }

                    if (String(config.imageTopic) === "") {
                        popupText.text = "Please choose an image topic to use for annotating!"
                        popup.open()
                        return
                    }

                    config.save()
                    config.visible = false
                    
                    annotate.visible = true
                    annotate.bagAnnotator = config.bagAnnotator
                    annotate.imageTopic = config.imageTopic
                    annotate.audioTopic = config.audioTopic
                    annotate.otherTopics = config.otherTopics
                    annotate.load()

                    stackView.push(annotate)
                    drawer.close()
                }
            }
        }
    }

    StackView {
        id: stackView
        initialItem: config
        anchors.fill: parent
        onCurrentItemChanged: console.log(currentItem)
    }
}
