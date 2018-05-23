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
	property var otherTopics

	ColumnLayout {
		anchors.fill: parent
		spacing: 4

		ImageItem {
			id: imageItem
			Layout.alignment: Qt.AlignCenter

			width: 640
			height: 480
		}

		Text {
			Layout.alignment: Qt.AlignCenter
			text: bagAnnotator != undefined ? "Current time: " + bagAnnotator.currentTime.toFixed(2) : ""
		}

		Rectangle {
			id: progressBar

			anchors.left: parent.left
			anchors.right: parent.right
			anchors.margins: 100

			height: 30

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
			spacing: 4
			Layout.margins: 8
			Layout.preferredWidth: 1280
			Layout.fillWidth: true

			Button {
				text: "Reset"
				onClicked: seek(0)
			}

			Button {
				id: playPauseButton
				text: "Play"
				onClicked: {
					if (text === "Play") {
						play()
					}
					else {
						pause()
					}
				}
			}

			Button {
				text: "Advance 1s"
				onClicked: advance(1.0)
			}

			Button {
				text: "Add annotation"
			}
		}

		GridLayout {
			Layout.preferredWidth: 1280
			Layout.fillWidth: true

			columns: 4
			columnSpacing: 4
			rowSpacing: 4

			Repeater {
				id: otherTopicsRepeater
				model: otherTopics != undefined ? Object.keys(otherTopics).length : 0

				RowLayout {
					Layout.preferredWidth: 1280
					Layout.fillWidth: true
					Layout.columnSpan: 4

					Text {
						Layout.preferredWidth: 300
						Layout.margins: 4
						text: Object.keys(otherTopics)[index]
					}

					Text {
						Layout.preferredWidth: 300
						Layout.margins: 4
						text: String(bagAnnotator.getCurrentValue(Object.keys(otherTopics)[index]))
					}

					Button {
						Layout.preferredWidth: 120
						Layout.margins: 4

						text: "Previous change"
						onClicked: {
							var prevTime = bagAnnotator.findPreviousTime(Object.keys(otherTopics)[index]);
							console.log("time: " + bagAnnotator.currentTime + ", prev: " + prevTime)
							seek(prevTime)
						}
					}

					Button {
						Layout.preferredWidth: 120
						Layout.margins: 4

						text: "Next change"
						onClicked: {
							var nextTime = bagAnnotator.findNextTime(Object.keys(otherTopics)[index]);
							console.log("time: " + bagAnnotator.currentTime + ", next: " + nextTime)
							seek(nextTime)
						}
					}
				}
			}
		}
	}

	Timer {
		id: playTimer
		interval: 10;
		running: false;
		repeat: true
		onTriggered: {
			advance(interval * 1e-3)

			if (!bagAnnotator.audioPlaying) {
				bagAnnotator.playAudio(audioTopic)
			}
		}
	}

	function load() {
		update()
		bagAnnotator.onCurrentTimeChanged.connect(update)
	}

	function update(time){
		imageItem.setImage(bagAnnotator.getCurrentValue(imageTopic))
		for (var i = 0; i < otherTopicsRepeater.count; ++i) {
			otherTopicsRepeater.itemAt(i).children[1].text = String(bagAnnotator.getCurrentValue(Object.keys(otherTopics)[i]))
		}
	}

	function reset() {
		pause()
		bagAnnotator.setCurrentTime(0)
	}

	function play() {
		if (playPauseButton.text === "Play") {
			playPauseButton.text = "Pause"
		}

		playTimer.start()
	}

	function pause() {
		if (playPauseButton.text === "Pause") {
			playPauseButton.text = "Play"
		}

		playTimer.stop()
		bagAnnotator.stopAudio()
	}

	function advance(time) {
		bagAnnotator.advance(time)
	}

	function seek(position) {
		bagAnnotator.setCurrentTime(position)
		bagAnnotator.stopAudio()
	}
}
