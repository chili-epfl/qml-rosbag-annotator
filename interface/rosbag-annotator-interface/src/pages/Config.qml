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
	property var title: qsTr("Configure your annotation session")

	property var bagAnnotator: null
	property var imageTopic: null
	property var audioTopic: null
	property var otherTopics: {}

	ColumnLayout {
		anchors.fill: parent
		spacing: 4

		RosBagAnnotator {
			id: annotator

			Component.onCompleted: bagAnnotator = annotator
		}

		RowLayout {
			spacing: 4
			Layout.margins: 4
			Layout.preferredWidth: 1280
			Layout.fillWidth: true

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
					
					annotator.setUseRosTime(useRosTimeCheckBox.checked)
					annotator.setBagPath(path);

					imageTopic = imageTopicComboBox.currentText
					audioTopic = audioTopicComboBox.currentText
				}
			}

			CheckBox {
				id: useRosTimeCheckBox
				Layout.preferredWidth: 220

				text: qsTr("Use ROS master time")
				checked: false
			}

			Button {
				Layout.preferredWidth: 80

				text: "Open..."
				onClicked: bagFileDialog.open();
			}

			TextField {
				id: bagFilePath
				Layout.preferredWidth: 640

				placeholderText: qsTr("Select a *.bag file")
			}
		}

		RowLayout {
			Layout.preferredWidth: 1280
			Layout.fillWidth: true

			Text {
				id: topicsFoundText
				Layout.preferredWidth: 400
				Layout.margins: 4
				text: "Length of bag in [s]: " + annotator.length + ", found " + Object.keys(annotator.topics).length + " topics in bag."
			}

			Button {
				Layout.preferredWidth: 160
				Layout.margins: 4
				text: "Hide topics"
				onClicked: {
					if (text === "Hide topics") {
						text = "Show topics"
						topicGrid.visible = false
					}
					else {
						text = "Hide topics"
						topicGrid.visible = true
					}
				}
			}
		}

		RowLayout {
			Layout.preferredWidth: 1280
			Layout.fillWidth: true

			Text {
				id: imageTopicText
				Layout.preferredWidth: 400
				Layout.margins: 4
				text: "Image topic to use for annotation: "
			}

			ComboBox {
				id: imageTopicComboBox
				Layout.preferredWidth: 400
				Layout.margins: 4
				anchors.left: imageTopicText.right

				model: annotator.topicsByType["Image"]
				onActivated: imageTopic = imageTopicComboBox.currentText
			}
		}

		RowLayout {
			Layout.preferredWidth: 1280
			Layout.fillWidth: true

			Text {
				id: audioTopicText
				Layout.preferredWidth: 400
				Layout.margins: 4
				text: "Audio topic to use for annotation: "
			}

			ComboBox {
				id: audioTopicComboBox
				Layout.preferredWidth: 400
				Layout.margins: 4
				anchors.left: audioTopicText.right

				model: annotator.topicsByType["Audio"]
				onActivated: audioTopic = audioTopicComboBox.currentText
			}
		}

		GridLayout {
			id: topicGrid
			Layout.preferredWidth: 1280
			Layout.fillWidth: true

			columns: 3
			columnSpacing: 4
			rowSpacing: 4

			Text {
				Layout.preferredWidth: 48
				Layout.margins: 4
				text: "Use"
				font.bold: true
			}

			Text {
				Layout.preferredWidth: 320
				Layout.margins: 4
				text: "Name"
				font.bold: true
			}
			Text {
				Layout.preferredWidth: 320
				Layout.margins: 4
				text: "Type"
				font.bold: true
			}

			Repeater {
				id: topicRepeater
				model: Object.keys(annotator.topics).length

				RowLayout {
					Layout.preferredWidth: 1280
					Layout.fillWidth: true
					Layout.columnSpan: 3

					CheckBox {
						Layout.preferredWidth: 48
					}
					Text {
						Layout.preferredWidth: 320
						Layout.margins: 4
						text: Object.keys(annotator.topics)[index]
					}
					Text {
						Layout.preferredWidth: 320
						Layout.margins: 4
						text: annotator.topics[Object.keys(annotator.topics)[index]]
					}
				}
			}
		}
	}

	function save() {
		otherTopics = {}
		for (var i = 0; i < topicRepeater.count; ++i) {
			if (topicRepeater.itemAt(i).children[0].checked &&
				Object.keys(annotator.topics)[i] != audioTopic &&
				Object.keys(annotator.topics)[i] != imageTopic) {
				otherTopics[Object.keys(annotator.topics)[i]] = annotator.topics[Object.keys(annotator.topics)[i]]
			}
		}
	}
}
