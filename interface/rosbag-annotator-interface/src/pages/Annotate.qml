import QtQuick 2.7
import QtQuick.Window 2.2
import QtQuick.Controls 2.3
import QtQml.Models 2.2
import QtQuick.Dialogs 1.3
import QtQuick.Layouts 1.3

import ch.epfl.chili 1.0

ScrollView {
	id: root
	anchors.fill: parent

	property var title: qsTr("Annotate")
	property var bagAnnotator
	property var imageTopic
	property var audioTopic
	property var otherTopics: new Object({})
	property real playbackFreq: 30.0

	ColumnLayout {
		id: topLayout
		anchors.horizontalCenter: parent.horizontalCenter
		anchors.top: parent.top
		anchors.topMargin: 8
		width: 0.9 * parent.width
		spacing: 16

		ImageItem {
			id: imageItem
			Layout.preferredWidth: imageItem.dims.x ? imageItem.dims.x : 640
			Layout.preferredHeight: imageItem.dims.y ? imageItem.dims.y : 480
			Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
		}

		Text {
			Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
			text: bagAnnotator != undefined ? "Current time: " + bagAnnotator.currentTime.toFixed(2) : ""
		}

		Rectangle {
			id: progressBar

			Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
			Layout.preferredWidth: 0.8 * root.width
			Layout.preferredHeight: 30


			color: "lightGray"

			Rectangle {
				anchors.left: parent.left
				anchors.top: parent.top
				anchors.bottom: parent.bottom

				width: bagAnnotator != undefined ? parent.width * bagAnnotator.currentTime / bagAnnotator.length : 0

				color: "darkGray"
			}

			MouseArea {
				anchors.fill: parent
				onClicked: {
					seek(bagAnnotator.length * mouse.x / width);
				}
			}
		}

		RowLayout {
			Layout.fillWidth: true
			Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
			spacing: 32

			Button {
				text: "Reset"
				onClicked: seek(0)
			}

			Button {
				id: playPauseButton
				text: "Play"
				onClicked: {
					if (bagAnnotator.playing) {
						pause()
					}
					else {
						play()
					}
				}
			}

			Button {
				text: "Previous"
				onClicked: previous(imageTopic)
			}

			Button {
				text: "Next"
				onClicked: next(imageTopic)
			}

			Button {
				text: "Add annotation"
				onClicked: annotationPopup.open()
			}
		}

		Popup {
	        id: annotationPopup
	        width: 640
	        height: 640
	        modal: true
	        focus: true
		    closePolicy: Popup.NoAutoClose
	    }

		Rectangle {
			Layout.preferredWidth: 0.9 * root.width
			Layout.preferredHeight: 1
			Layout.bottomMargin: 16
			Layout.topMargin: 16
			color: "#111111"
		}

		Repeater {
			id: otherTopicsRepeater
			model: Object.keys(otherTopics).length

			RowLayout {
				Layout.fillWidth: true
				Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
				spacing: 8

				Text {
					Layout.preferredWidth: 0.4 * root.width
					text: Object.keys(otherTopics)[index]
				}

				Text {
					Layout.preferredWidth: 0.2 * root.width
					text: String(bagAnnotator.getCurrentValue(Object.keys(otherTopics)[index]))
				}

				Button {
					text: "Previous"
					onClicked: previous(Object.keys(otherTopics)[index])
				}

				Button {
					text: "Next"
					onClicked: next(Object.keys(otherTopics)[index])
				}
			}
		}
	}

	function load(config) {
        bagAnnotator = config.bagAnnotator
        imageTopic = config.imageTopic
        audioTopic = config.audioTopic
        otherTopics = config.otherTopics

		updateValues()

		bagAnnotator.onCurrentTimeChanged.connect(updateValues)
		bagAnnotator.onPlayingChanged.connect(updatePlayPauseButtonState)
	}

	function updateValues(time){
		imageItem.setImage(bagAnnotator.getCurrentValue(imageTopic))

		for (var i = 0; i < otherTopicsRepeater.count; ++i) {
			otherTopicsRepeater.itemAt(i).children[1].text = String(bagAnnotator.getCurrentValue(Object.keys(otherTopics)[i]))
		}
	}

	function updatePlayPauseButtonState(playing){
		if (playing) {
			playPauseButton.text = "Pause"
		}
		else {
			playPauseButton.text = "Play"			
		}
	}

	function play() {
		bagAnnotator.play(playbackFreq, audioTopic)
	}

	function pause() {
		bagAnnotator.stop()
	}

	function previous(topic) {
		var prevTime = bagAnnotator.findPreviousTime(topic);
		seek(prevTime)
	}

	function next(topic) {
		var nextTime = bagAnnotator.findNextTime(topic);
		seek(nextTime)
	}

	function seek(position) {
		var playing = bagAnnotator.playing

		bagAnnotator.stop()
		bagAnnotator.setCurrentTime(position)

		if (playing) {
			bagAnnotator.play(playbackFreq, audioTopic)
		}
	}
}
