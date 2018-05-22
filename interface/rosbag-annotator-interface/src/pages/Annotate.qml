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
    		text: "Current time: " + bagAnnotator.currentTime.toFixed(2)
    	}

	    RowLayout {
			spacing: 4
			Layout.margins: 8
			Layout.preferredWidth: 1280
			Layout.fillWidth: true

		    Button {
		    	text: "Reset"

		    	onClicked: reset()
		    }

		    Button {
		    	text: "Play"

		    	onClicked: {
		    		if (text === "Play") {
		    			text = "Pause"
		    			play()
		    		}
		    		else {
		    			text = "Play"
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

			columns: 2
			columnSpacing: 4
			rowSpacing: 4

			Repeater {
				id: otherTopicsRepeater
				model: Object.keys(otherTopics).length

				RowLayout {
					Layout.preferredWidth: 1280
					Layout.fillWidth: true
					Layout.columnSpan: 2

					Text {
						Layout.preferredWidth: 320
						Layout.margins: 4
						text: Object.keys(otherTopics)[index]
					}
					Text {
						Layout.preferredWidth: 320
						Layout.margins: 4
						text: String(bagAnnotator.getCurrentValue(Object.keys(otherTopics)[index]))
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
		playTimer.stop()
	    bagAnnotator.setCurrentTime(0)
	}

	function play() {
	    playTimer.start()
	}

	function pause() {
	    playTimer.stop()
	}

    function advance(time) {
    	bagAnnotator.advance(time)
    }
}
