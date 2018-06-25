import QtQuick 2.11
import QtQuick.Window 2.2
import QtQuick.Controls 2.3
import QtQml.Models 2.2
import QtQuick.Dialogs 1.3
import QtQuick.Extras 1.4
import QtQuick.Layouts 1.3

import ch.epfl.chili.ros.annotation 1.0

import "./pages"

ApplicationWindow {
	id: root
	visible: true

	property bool mobile: Qt.platform.os === "android"
	width: mobile ? Screen.width : 1280
	height: mobile ? Screen.height : 960

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
		x: 0.5 * (root.width - (popupText.implicitWidth + 64))
		width: popupText.implicitWidth + 64
		height: popupText.implicitHeight + 64
		modal: true
		focus: true

		ColumnLayout {
			id: popupLayout
			anchors.fill: parent

			Text {
				id: popupText
				Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
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
					if (config.visible) {
						drawer.close()
						return;
					}

					annotate.visible = false
					config.visible = true

					stackView.pop()
					drawer.close()
				}
			}

			ItemDelegate {
				text: qsTr("Annotate")
				width: parent.width
				onClicked: {
					if (annotate.visible) {
						drawer.close()
						return;
					}

					config.save()

					if (config.bagAnnotator.status != RosBagAnnotator.READY) {
						popupText.text = "Please configure before annotating!"
						popup.open()
						return
					}

					if (String(config.imageTopic) === "") {
						popupText.text = "Please choose an image topic to use during annotation!"
						popup.open()
						return
					}

					annotate.load(config)
					
					config.visible = false
					annotate.visible = true
					
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
