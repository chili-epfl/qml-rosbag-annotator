rosbag-annotator-interface
===========================

QML interface for annotating data inside a rosbag.
A `sensor_msgs/CompressedImage` topic and a `audio_common_msgs/AudioData` topic from the bag serve as visual and audio during annotation.
Data of other topics is live-updated. Play in real-time or seek a point of time in the rosbag. Create new topics for your annotations and insert them into the rosbag.
Follow the instructions in [the qml-rosbag-annotation README](../../README.md) before trying to run this sample.

Tested with Qt 5.10.1 on desktop.


Prerequisites
-------------

 - `qml-rosbag-annotation` QML plugin

build
-----

```
    $ mkdir build && cd build
    $ qmake ..
    $ make
```
