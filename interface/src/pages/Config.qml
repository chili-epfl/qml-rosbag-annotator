import QtQuick 2.11
import QtQuick.Window 2.2
import QtQuick.Controls 2.3
import QtQml.Models 2.2
import QtQuick.Dialogs 1.3
import QtQuick.Layouts 1.3

import ch.epfl.chili 1.0

ScrollView {
	id: root
	property var title: qsTr("Configure your annotation session")
	property var bagAnnotator
	property var useSeparateBag: true
	property var imageTopic
	property var audioTopic
	property var otherTopics: new Object({})
	property var mapTopics: new Object({})
	property var selectableTopics: new Object({})
	property var mapImageUrl
	property var mapWidth
	property var mapHeight

	ColumnLayout {
		anchors.horizontalCenter: parent.horizontalCenter
		anchors.top: parent.top
		anchors.topMargin: 8
		width: 0.95 * parent.width
		spacing: 4

		RosBagAnnotator {
			id: annotator
			visible: false
			Component.onCompleted: bagAnnotator = annotator
		}

		RowLayout {
			Layout.fillWidth: true
			Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

			spacing: 8

			FileDialog {
				id: bagFileDialog

				title: "Please choose a rosbag to annotate"
				modality: Qt.WindowModal
				selectMultiple: false

				nameFilters: [ "rosbag files (*.bag)" ]

				onAccepted: {
					var path = bagFileDialog.fileUrl.toString();
					path = path.replace(/^(file:\/{2})/,"");
					path = decodeURIComponent(path);
					bagFilePath.text = path;
					load()
				}
			}

			TextField {
				id: bagFilePath
				Layout.preferredWidth: 0.7 * root.width

				placeholderText: qsTr("Path to *.bag file")

				onAccepted: load()
			}

			Button {
				text: "Choose file"
				onClicked: bagFileDialog.open()
			}
		}

		GridLayout {
			Layout.fillWidth: true
			Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

			columns: 2
			columnSpacing: 8
			rowSpacing: 4

			Rectangle {
				Layout.preferredWidth: 0.95 * root.width
				Layout.preferredHeight: 1
				Layout.columnSpan: 2
				Layout.bottomMargin: 8
				Layout.topMargin: 8
				color: "#111111"
			}

			Text {
				Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
				text: "Always use message arrival time to sort topics?"
			}

			CheckBox {
				id: useRosTimeCheckBox
				checked: true
			}

			Text {
				Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
				text: "Save annotations to separate rosbag?"
			}

			CheckBox {
				id: useSeparateBagCheckBox
				checked: false
			}

			Rectangle {
				Layout.preferredWidth: 0.95 * root.width
				Layout.preferredHeight: 1
				Layout.columnSpan: 2
				Layout.bottomMargin: 8
				Layout.topMargin: 8
				color: "#111111"
			}

			Text {
				Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
				text: "Length of bag in seconds:"
			}
			
			Text {
				text: String(annotator.length.toFixed(2))
				font.bold: true
			}

			Text {
				Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
				text: "Number of topics found:"
			}

			Text {
				text: String(Object.keys(annotator.topics).length)
				font.bold: true
			}

			Rectangle {
				Layout.preferredWidth: 0.95 * root.width
				Layout.preferredHeight: 1
				Layout.columnSpan: 2
				Layout.bottomMargin: 8
				Layout.topMargin: 8
				color: "#111111"
			}

			Text {
				id: imageTopicText
				Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
				text: "Image topic to use during annotation:"
			}

			ComboBox {
				id: imageTopicComboBox
				Layout.preferredWidth: 0.4 * root.width

				model: annotator.topicsByType["Image"]
			}

			Text {
				id: audioTopicText
				Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
				text: "Audio topic to use during annotation (optional):"
			}

			ComboBox {
				id: audioTopicComboBox
				Layout.preferredWidth: 0.4 * root.width

				model: annotator.topicsByType["Audio"]
			}

			Rectangle {
				Layout.preferredWidth: 0.95 * root.width
				Layout.preferredHeight: 1
				Layout.columnSpan: 2
				Layout.bottomMargin: 8
				Layout.topMargin: 8
				color: "#111111"
			}

			FileDialog {
				id: mapFileDialog

				title: "Choose a map image file"
				modality: Qt.WindowModal
				selectMultiple: false

				nameFilters: [ "Image files (*.jpg *.png *.svg)" ]

				onAccepted: mapImage.source = mapFileDialog.fileUrl
			}

			Text {
				Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
				text: "Map image file (optional):"
			}

			Button {
				text: "Choose file"
				onClicked: mapFileDialog.open()
			}

			Text {
				Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
				text: "Map width (in mm):"
			}

			TextField {
				id: mapWidthInput
				text: "420"
				validator: IntValidator{bottom: 0}
			}

			Text {
				Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
				text: "Map height (in mm):"
			}

			TextField {
				id: mapHeightInput
				text: "420"
				validator: IntValidator{bottom: 0}
			}

			Image {
				id: mapImage
				Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
				Layout.maximumWidth: 0.4 * root.width
				Layout.maximumHeight: 0.4 * root.width
				Layout.columnSpan: 2
				fillMode: Image.PreserveAspectFit
			}

			Rectangle {
				Layout.preferredWidth: 0.95 * root.width
				Layout.preferredHeight: 1
				Layout.columnSpan: 2
				Layout.bottomMargin: 8
				Layout.topMargin: 8
				color: "#111111"
			}
		}

		RowLayout {
			Layout.fillWidth: true
			Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
			Layout.topMargin: 8
			Layout.bottomMargin: 8

			Button {
				enabled: bagFilePath.length > 0
				text: "Reload topics"
				onClicked: {
					load()
				}
			}
		}

		RowLayout {
			id: topicHeader
			Layout.fillWidth: true
			Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
			spacing: 4

			visible: annotator.status == RosBagAnnotator.READY

			Text {
				id: nameText
				Layout.preferredWidth: 0.35 * root.width
				text: "Name"
				font.bold: true
			}

			Text {
				id: typeText
				Layout.preferredWidth: 0.15 * root.width
				text: "Type"
				font.bold: true
			}

			Text {
				Layout.preferredWidth: 0.15 * root.width
				Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
				id: displayText
				text: "Show during annotation?"
				font.bold: true
			}

			Text {
				Layout.preferredWidth: 0.15 * root.width
				Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
				id: mapDisplayText
				text: "Show on map?"
				font.bold: true
			}
		}

		Repeater {
			id: topicRepeater
			model: Object.keys(selectableTopics).length

			RowLayout {
				Layout.fillWidth: true
				Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
				spacing: 4

				Text {
					Layout.preferredWidth: 0.35 * root.width
					text: Object.keys(selectableTopics)[index]
				}

				Text {
					Layout.preferredWidth: 0.15 * root.width
					text: selectableTopics[Object.keys(selectableTopics)[index]]
				}

				CheckBox {
					Layout.preferredWidth: 0.15 * root.width
					Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
				}

				CheckBox {
					Layout.preferredWidth: 0.15 * root.width
					Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
					enabled: selectableTopics[Object.keys(selectableTopics)[index]] == "DoubleArray"
				}
			}
		}

		Item {
			Layout.preferredHeight: 8
		}
	}

	function load() {
		annotator.setUseRosTime(useRosTimeCheckBox.checked)
		annotator.setBagPath(bagFilePath.text)

		var temp = {}
		for (var i = 0; i < Object.keys(annotator.topics).length; ++i) {
			if (annotator.topics[Object.keys(annotator.topics)[i]] != "Audio" && 
				annotator.topics[Object.keys(annotator.topics)[i]] != "Image") {
				temp[Object.keys(annotator.topics)[i]] = annotator.topics[Object.keys(annotator.topics)[i]]
			}
		}

		selectableTopics = new Object(temp)
	}

	function save() {
		useSeparateBag = useSeparateBagCheckBox.checked
		imageTopic = imageTopicComboBox.currentText
		audioTopic = audioTopicComboBox.currentText
		mapImageUrl = mapFileDialog.fileUrl
		mapWidth = parseFloat(mapWidthInput.text)
		mapHeight = parseFloat(mapHeightInput.text)

		otherTopics = {}
		for (var i = 0; i < topicRepeater.count; ++i) {
			if (topicRepeater.itemAt(i).children[2].checked) {
				otherTopics[Object.keys(selectableTopics)[i]] = annotator.topics[Object.keys(selectableTopics)[i]]
			}
		}

		mapTopics = {}
		for (var i = 0; i < topicRepeater.count; ++i) {
			if (topicRepeater.itemAt(i).children[3].checked) {
				mapTopics[Object.keys(selectableTopics)[i]] = annotator.topics[Object.keys(selectableTopics)[i]]
			}
		}
	}
}
