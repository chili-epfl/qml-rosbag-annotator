import QtQuick 2.11
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
			x: 0.5 * (root.width - 640)
			width: 640
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
					id: annotationComboRow
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
						model: bagAnnotator != undefined ? Object.keys(bagAnnotator.annotationTopics) : 0

					    onAccepted: {
					        if (bagAnnotator.annotationTopics[editText] === undefined) {
					        	var tmp = Object.keys(bagAnnotator.annotationTopics)
					            tmp.push(editText)
					            model = tmp
					        }
					    }

						validator: RegExpValidator {
							regExp: /^[a-zA-Z0-9_\/-]*$/
						}
					}

					Text {
						text: "Type:"
						font.bold: true
					}

					ComboBox {
						id: annotationTypeComboBox
						model: ["Bool", "Int", "Real", "String", "Array of Int", "Array of Real"]
						onActivated: annotationValueInput.text = ""
					}
				}

				RowLayout {
					Layout.fillWidth: true
					Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
					spacing: 8

					Text {
						Layout.alignment: Qt.AlignRight | Qt.AlignVCenter

						text: {
							if (annotationTypeComboBox.currentIndex == 0) {
								return "Input either 0 or 1:"
							}
							else if (annotationTypeComboBox.currentIndex == 1) {
								return "Input an integer:"
							}
							else if (annotationTypeComboBox.currentIndex == 2) {
								return "Input a real number:"
							}
							else if (annotationTypeComboBox.currentIndex == 3) {
								return "Input a text string:"
							}
							else if (annotationTypeComboBox.currentIndex == 4) {
								return "Input a comma-separated list of integers:"
							}
							else if (annotationTypeComboBox.currentIndex == 5) {
								return "Input a comma-separated list of real numbers:"
							}
						}
						font.bold: true
					}

					TextInput {
						Layout.preferredWidth: 160
						Layout.margins: 8
						Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter

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
							id: stringValidator
							regExp: /.*$/
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
								return stringValidator
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

				Button {
					Layout.margins: 32
					Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
					text: "Save"
					enabled: annotationValueInput.length > 0 && annotationTopicComboBox.currentText.length > 0
					onClicked: {
						if (annotationTypeComboBox.currentIndex == 0) {
							bagAnnotator.annotate(annotationTopicComboBox.currentText, Boolean(parseInt(annotationValueInput.text)), RosBagAnnotator.BOOL)
						}
						else if (annotationTypeComboBox.currentIndex == 1) {
							bagAnnotator.annotate(annotationTopicComboBox.currentText, parseInt(annotationValueInput.text), RosBagAnnotator.INT)
						}
						else if (annotationTypeComboBox.currentIndex == 2) {
							bagAnnotator.annotate(annotationTopicComboBox.currentText, parseFloat(annotationValueInput.text), RosBagAnnotator.FLOAT)
						}
						else if (annotationTypeComboBox.currentIndex == 3) {
							bagAnnotator.annotate(annotationTopicComboBox.currentText, annotationValueInput.text, RosBagAnnotator.STRING)
						}
						else if (annotationTypeComboBox.currentIndex == 4) {
							bagAnnotator.annotate(annotationTopicComboBox.currentText, parseIntArray(annotationValueInput.text), RosBagAnnotator.INT_ARRAY)
						}
						else if (annotationTypeComboBox.currentIndex == 5) {
							bagAnnotator.annotate(annotationTopicComboBox.currentText, parseFloatArray(annotationValueInput.text), RosBagAnnotator.FLOAT_ARRAY)
						}

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

		next(imageTopic)
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
		if (position == bagAnnotator.currentTime) {
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

	function parseIntArray(str) {
		var tmp = str.split(",")
		var result = []
		for (var i = 0; i < tmp.length; ++i) {
			result.push(parseInt(tmp[i]))
		}
		return result;
	}

	function parseFloatArray(str) {
		var tmp = str.split(",")
		var result = []
		for (var i = 0; i < tmp.length; ++i) {
			result.push(parseFloat(tmp[i]))
		}
		return result;
	}
}
