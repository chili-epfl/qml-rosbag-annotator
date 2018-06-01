### Description
QML plugin that helps annotating a previously recorded rosbag.
Features:
 - retrieve the list of topics present in the rosbag
 - seek inside the rosbag and retreive the last published message of a topic
 - retrieve messages of type `sensor_msgs/CompressedImage` as a `QImage` object
 - playback a rosbag in real-time, continously updating topic messages while outputting audio of any topic of type `audio_common_msgs/AudioData`
 - create annotation topics of different types and insert messages into them (either directly into the original rosbag, or into a separate bag)

### Requirements
 - `Qt 5.11`
 - `ros-kinetic-rosbag-storage`
 - `ros-kinetic-std-msgs`
 - `ros-kinetic-audio-common`
 - `ros-kinetic-sensor-msgs`
 - `ros-chili-msgs`

### Build instructions
```
. /opt/ros/kinetic/setup.bash
mkdir build
cd build
make -j 8 && sudo make install
```
