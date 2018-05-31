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
	property var config
	property real playbackFreq: 30.0

	ColumnLayout {
		id: topLayout
		anchors.horizontalCenter: parent.horizontalCenter
		anchors.top: parent.top
		anchors.topMargin: 8
		width: 0.95 * parent.width
		spacing: 16

		TabBar {
		    id: imageTabBar
			Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
			Layout.preferredWidth: 640
			Layout.topMargin: 8
		    TabButton {
		        text: qsTr("Video Topic")
		    }
		    TabButton {
		        text: qsTr("Map")
		    }
		}

		StackLayout {
			id: imageStack
			Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
			Layout.preferredWidth: 640
			Layout.maximumWidth: 640
			Layout.minimumWidth: 640
			Layout.preferredHeight: 480
			Layout.maximumHeight: 480
			Layout.minimumHeight: 480
			Layout.topMargin: -16
		    currentIndex: imageTabBar.currentIndex

			ImageItem {
				id: imageItem
				Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
				Layout.preferredWidth: 640
				Layout.maximumWidth: 640
				Layout.minimumWidth: 640
				Layout.preferredHeight: 480
				Layout.maximumHeight: 480
				Layout.minimumHeight: 480
			}
		    Canvas {
		        id: mapCanvas
				Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
				Layout.preferredWidth: 640
				Layout.maximumWidth: 640
				Layout.minimumWidth: 640
				Layout.preferredHeight: 480
				Layout.maximumHeight: 480
				Layout.minimumHeight: 480

				onImageLoaded: {
					draw()
				}

				onPaint: {
					if (config != undefined) {
						draw()
					}
				}

				function draw() {
					var ctx = mapCanvas.getContext('2d')

					ctx.save()

					ctx.clearRect(0, 0, mapCanvas.width, mapCanvas.height)

					if (mapCanvas.isImageLoaded(config.mapImageUrl)) {
						ctx.drawImage(config.mapImageUrl, 0, 0, 640, 480)
					}

					for (var i = 0; i < Object.keys(config.mapTopics).length; ++i) {
						var position = config.bagAnnotator.getCurrentValue(Object.keys(config.mapTopics)[i])
						if (position != null) {
							var ox = position[0] / config.mapWidth * mapCanvas.width
							var oy = position[1] / config.mapHeight * mapCanvas.height
							var r = mapCanvas.width / 10

							ctx.fillStyle = Qt.rgba(0.8, 0.1, 0.1, 1.0)
							ctx.beginPath()
							ctx.ellipse(ox - 0.5 * r, oy - 0.5 * r, r, r)
							ctx.fill()

							var text = String(i)
							ctx.font = "48px sans-serif"
							ctx.fillStyle = Qt.rgba(0.1, 0.1, 0.1, 1.0)
							ctx.beginPath()
							ctx.text(text, ox - 0.5 * ctx.measureText(text).width, oy + 18)
							ctx.fill()
						}
					}

					ctx.restore()
				}
		    }
		}

		Text {
			Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
			text: config != undefined ? "Current time: " + config.bagAnnotator.currentTime.toFixed(2) : ""
		}

		Rectangle {
			id: progressBar

			Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
			Layout.preferredWidth: 640
			Layout.maximumWidth: 640
			Layout.minimumWidth: 640
			Layout.preferredHeight: 30

			color: "lightGray"

			Rectangle {
				anchors.left: parent.left
				anchors.top: parent.top
				anchors.bottom: parent.bottom

				width: config != undefined ? parent.width * config.bagAnnotator.currentTime / config.bagAnnotator.length : 0

				color: "darkGray"
			}

			MouseArea {
				anchors.fill: parent
				onClicked: {
					seek(config.bagAnnotator.length * mouse.x / width);
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
				onClicked: previous(config.imageTopic)
			}

			Button {
				id: playPauseButton
				text: "Play"
				onClicked: {
					if (config.bagAnnotator.playing) {
						pause()
					}
					else {
						play()
					}
				}
			}

			Button {
				text: "Next frame"
				onClicked: next(config.imageTopic)
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

			ColumnLayout {
				anchors.horizontalCenter: parent.horizontalCenter
				anchors.top: parent.top
				anchors.topMargin: 8
				width: 0.95 * parent.width
				spacing: 8

				RowLayout {
					id: annotationComboRow
					Layout.fillWidth: true
					Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
					spacing: 8

					Text {
						text: "Topic:"
						font.bold: true
					}

					ComboBox {
						id: annotationTopicComboBox
						Layout.preferredWidth: 0.4 * annotationPopup.width
						editable: true
						model: config != undefined ? Object.keys(config.bagAnnotator.annotationTopics) : 0

					    onAccepted: {
					        if (config.bagAnnotator.annotationTopics[editText] === undefined) {
					        	var tmp = Object.keys(config.bagAnnotator.annotationTopics)
					            tmp.push(editText)
					            model = tmp
					        }
					    }

						validator: RegExpValidator {
							regExp: /^[a-zA-Z0-9_\/-]*$/
						}
					}

					Item {
						Layout.preferredWidth: 32
					}

					Text {
						text: "Type:"
						font.bold: true
					}

					ComboBox {
						id: annotationTypeComboBox
						model: ["Bool", "Int", "Double", "String", "IntArray", "DoubleArray"]
						onActivated: annotationValueInput.text = ""
					}
				}

				Rectangle {
					Layout.preferredWidth: 0.95 * annotationPopup.width
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

					TextField {
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

					Button {
						Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
						text: "Save"
						enabled: annotationValueInput.length > 0 && annotationTopicComboBox.currentText.length > 0
						onClicked: {
							if (annotationTypeComboBox.currentIndex == 0) {
								config.bagAnnotator.annotate(annotationTopicComboBox.currentText, Boolean(parseInt(annotationValueInput.text)), RosBagAnnotator.BOOL)
							}
							else if (annotationTypeComboBox.currentIndex == 1) {
								config.bagAnnotator.annotate(annotationTopicComboBox.currentText, parseInt(annotationValueInput.text), RosBagAnnotator.INT)
							}
							else if (annotationTypeComboBox.currentIndex == 2) {
								config.bagAnnotator.annotate(annotationTopicComboBox.currentText, parseFloat(annotationValueInput.text), RosBagAnnotator.DOUBLE)
							}
							else if (annotationTypeComboBox.currentIndex == 3) {
								config.bagAnnotator.annotate(annotationTopicComboBox.currentText, annotationValueInput.text, RosBagAnnotator.STRING)
							}
							else if (annotationTypeComboBox.currentIndex == 4) {
								config.bagAnnotator.annotate(annotationTopicComboBox.currentText, parseIntArray(annotationValueInput.text), RosBagAnnotator.INT_ARRAY)
							}
							else if (annotationTypeComboBox.currentIndex == 5) {
								config.bagAnnotator.annotate(annotationTopicComboBox.currentText, parseFloatArray(annotationValueInput.text), RosBagAnnotator.DOUBLE_ARRAY)
							}

							annotationPopup.close()
						}
					}
				}

				Text {
					Layout.margins: 8
					Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
					visible: annotationValueInput.length == 0 || annotationTopicComboBox.currentText.length == 0
					text: "Topic and value must both be defined in order to save the annotation"
					color: "#ff2222"
				}
			}
		}

		Rectangle {
			Layout.preferredWidth: 0.95 * root.width
			Layout.preferredHeight: 1
			Layout.bottomMargin: 16
			Layout.topMargin: 16
			color: "#111111"
		}

		Repeater {
			id: otherTopicsRepeater
			model: config != undefined ? Object.keys(config.otherTopics).length : 0

			RowLayout {
				Layout.fillWidth: true
				Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
				spacing: 8

				Text {
					Layout.preferredWidth: 0.3 * root.width
					text: Object.keys(config.otherTopics)[index]
				}

				Text {
					Layout.preferredWidth: 0.3 * root.width
					text: valueToString(
						config.bagAnnotator.getCurrentValue(Object.keys(config.otherTopics)[index]),
						config.otherTopics[Object.keys(config.otherTopics)[index]]
					)
				}

				Button {
					text: "Previous"
					onClicked: previous(Object.keys(config.otherTopics)[index])
				}

				Button {
					text: "Next"
					onClicked: next(Object.keys(config.otherTopics)[index])
				}
			}
		}

		Item {
			Layout.preferredHeight: 8
		}
	}

	function load(configuration) {
		config = configuration
		config.bagAnnotator.setUseSeparateBag(config.useSeparateBag)

		next(config.imageTopic)
		mapCanvas.loadImage(config.mapImageUrl)

		updateValues()

		config.bagAnnotator.onCurrentTimeChanged.connect(updateValues)
		config.bagAnnotator.onPlayingChanged.connect(updatePlayPauseButtonState)
	}

	function updateValues(time){
		imageItem.setImage(config.bagAnnotator.getCurrentValue(config.imageTopic))

		for (var i = 0; i < otherTopicsRepeater.count; ++i) {
			otherTopicsRepeater.itemAt(i).children[1].text = valueToString(
				config.bagAnnotator.getCurrentValue(Object.keys(config.otherTopics)[i]),
				config.otherTopics[Object.keys(config.otherTopics)[i]]
			)
		}
		
		mapCanvas.requestPaint()
	}

	function valueToString(value, type) {
		if (value === undefined) {
			return "undefined"
		}

		if (type === "IntArray" || type === "DoubleArray") {
			var str = "["
			for (var i = 0; i < value.length; ++i) {
				if (i > 0) {
					str += ", "
				}

			    if (type === "IntArray") {
			    	str += value[i]
			    }
				else {
					str += value[i].toFixed(4)
				}
			}

			return str + "]"
		}
		else if (type == "Double") {
			return value.toFixed(4)
		}
		else {
			return String(value)
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
		config.bagAnnotator.play(playbackFreq, config.audioTopic)
	}

	function pause() {
		config.bagAnnotator.stop()
	}

	function previous(topic) {
		var prevTime = config.bagAnnotator.findPreviousTime(topic);
		seek(prevTime)
	}

	function next(topic) {
		var nextTime = config.bagAnnotator.findNextTime(topic);
		seek(nextTime)
	}

	function seek(position) {
		if (position == config.bagAnnotator.currentTime) {
			return;
		}

		var playing = config.bagAnnotator.playing
		if (playing) {
			config.bagAnnotator.stop()
		}

		config.bagAnnotator.setCurrentTime(position)

		if (playing) {
			config.bagAnnotator.play(playbackFreq, config.audioTopic)
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
