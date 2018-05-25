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
				text: "Previous frame"
				onClicked: previous(imageTopic)
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
				text: "Next frame"
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

			ColumnLayout {
				anchors.horizontalCenter: parent.horizontalCenter
				anchors.top: parent.top
				anchors.topMargin: 8
				width: 0.9 * parent.width
				spacing: 16

				RowLayout {
					Layout.fillWidth: true
					Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
					spacing: 32
					Text {
						text: "Topic:"
						font.bold: true
					}

					ComboBox {
						id: annotationTopicComboBox
						editable: true
						model: Object.keys(bagAnnotator.annotationTopics)

					    onAccepted: {
					        if (bagAnnotator.annotationTopics.editText === undefined) {
					        	var tmp = Object.keys(bagAnnotator.annotationTopics)
					            tmp.push(editText)
					            model = tmp
					        }
					    }
					}

					Text {
						text: "Type:"
						font.bold: true
					}

					ComboBox {
						id: annotationTypeComboBox
						model: ["Bool", "Int", "Real", "String", "Array of Int", "Array of Real"]
					}
				}

				Rectangle {
					Layout.fillWidth: true
					Layout.preferredHeight: 1
					Layout.bottomMargin: 16
					Layout.topMargin: 16
					color: "#111111"
				}

				RowLayout {
					Layout.fillWidth: true
					Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
					spacing: 8

					Text {
						text: "Value(s): "
						font.bold: true
					}

					TextInput {
						Layout.preferredWidth: 320
						id: annotationValueInput

						IntValidator {
							id: boolValidator
							bottom: 0
							top: 1
						}
						IntValidator {
							id: intValidator
						}
						DoubleValidator {
							id: realValidator
						}
						RegExpValidator {
							id: intArrayValidator
							regExp: /\d{1,12}(?: *, *\d{1,12})+ *$/
						}
						RegExpValidator {
							id: realArrayValidator
							regExp: /(\d{1,12}(?:\.\d{1,12})?)(?: *, *(\d{1,12}(?:\.\d{1,12})?))+ *$/
						}

						validator: {
							if (annotationTypeComboBox.currentIndex == 0) {
								return boolValidator
							}
							else if (annotationTypeComboBox.currentIndex == 1) {
								return intValidator
							}
							else if (annotationTypeComboBox.currentIndex == 2) {
								return realValidator
							}
							else if (annotationTypeComboBox.currentIndex == 3) {
								return realValidator
							}
							else if (annotationTypeComboBox.currentIndex == 4) {
								return intArrayValidator
							}
							else if (annotationTypeComboBox.currentIndex == 5) {
								return realArrayValidator
							}
						}
					}
				}

				// Button {
				// 	text: "Test int"
				// 	onClicked: bagAnnotator.annotate(annotationTopicComboBox.currentText, 1)
				// }
				// Button {
				// 	text: "Test float"
				// 	onClicked: bagAnnotator.annotate("Test", 1.1)
				// }
				// Button {
				// 	text: "Test string"
				// 	onClicked: bagAnnotator.annotate("Test", "bla")
				// }
				Button {
					text: "Test string list"
					onClicked: bagAnnotator.annotate("Test", ["bla", "la"])
				}
				Button {
					text: "Test int list"
					onClicked: bagAnnotator.annotate("Test", [1, 2, 3])
				}
				Button {
					text: "Test real list"
					onClicked: bagAnnotator.annotate("Test", [1.1, 2.2, 3.3])
				}
				// Button {
				// 	text: "Test map"
				// 	onClicked: bagAnnotator.annotate("Test", {"first": 1, "second": 2})
				// }
				// Button {
				// 	text: "Test qvector"
				// 	onClicked: bagAnnotator.annotate("Test", Qt.vector3d(0.0, 1.0, 2.0))
				// }
				Button {
					Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
					text: "Save"
					onClicked: {
						bagAnnotator.annotate(annotationTopicComboBox.currentText, 1)
						annotationPopup.close()
					}
				}
			}
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
		if (position == bagAnnotator.currentTime()) {
			return;
		}

		var playing = bagAnnotator.playing
		if (playing) {
			bagAnnotator.stop()
		}

		bagAnnotator.setCurrentTime(position)

		if (playing) {
			bagAnnotator.play(playbackFreq, audioTopic)
		}
	}
}
