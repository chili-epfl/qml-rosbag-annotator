#include "rosbagannotator.h"

#include <rosbag/bag.h>
#include <rosbag/view.h>

#include <std_msgs/Bool.h>
#include <std_msgs/Int32.h>
#include <std_msgs/Float64.h>
#include <std_msgs/String.h>
#include <std_msgs/Int32MultiArray.h>
#include <std_msgs/Float64MultiArray.h>
#include <audio_common_msgs/AudioData.h>

#include <chili_msgs/Bool.h>
#include <chili_msgs/Float32.h>
#include <chili_msgs/Int32.h>
#include <chili_msgs/String.h>
#include <chili_msgs/Vector2Float32.h>
#include <chili_msgs/Vector2Int32.h>
#include <chili_msgs/Vector3Float32.h>

#include <algorithm>
#include <limits>

RosBagAnnotator::RosBagAnnotator(QQuickItem *parent):
	QQuickItem(parent),
	mStatus(EMPTY),
	mUseRosTime(false),
	mStartTime(0),
	mEndTime(0),
	mCurrentTime(0)
{
	// By default, QQuickItem does not draw anything. If you subclass
	// QQuickItem to create a visual item, you will need to uncomment the
	// following line and re-implement updatePaintNode()

	// setFlag(ItemHasContents, true);

	connect(&mPlaybackTimer, &QTimer::timeout, this, &RosBagAnnotator::updatePlayback);
}

RosBagAnnotator::~RosBagAnnotator()
{
}

void RosBagAnnotator::setBagPath(QString path) {
	mBagPath = path;

	reset();

	if (!mBagPath.isEmpty()) {
		try {
			mBag.reset(new rosbag::Bag(mBagPath.toStdString()));
		}
		catch (const rosbag::BagException &e) {
			qDebug() << "An exception has occured while opening bag " << mBagPath << ": " << e.what();
			return;
		}

		parseBag();
		
		mBag->close();
	}

	emit bagPathChanged(mBagPath);
}

void RosBagAnnotator::setCurrentTime(double time) {
	mCurrentTime = mStartTime + static_cast<uint64_t>(1e9 * time);

	if (mCurrentTime < mStartTime) {
		mCurrentTime = mStartTime;
	}
	else if (mCurrentTime > mEndTime) {
		mCurrentTime = mEndTime;
	}

	seekCurrentMessageIndices(mBoolMsgs, mCurrentBool);
	seekCurrentMessageIndices(mFloatMsgs, mCurrentFloat);
	seekCurrentMessageIndices(mIntMsgs, mCurrentInt);
	seekCurrentMessageIndices(mStringMsgs, mCurrentString);
	seekCurrentMessageIndices(mIntArrayMsgs, mCurrentIntArray);
	seekCurrentMessageIndices(mDoubleArrayMsgs, mCurrentDoubleArray);
	seekCurrentMessageIndices(mAudioMsgs, mCurrentAudio);
	seekCurrentMessageIndices(mImageMsgs, mCurrentImage);

	emit currentTimeChanged(time);
}

void RosBagAnnotator::advance(double time) {
	setCurrentTime(currentTime() + time);
}

void RosBagAnnotator::rewind(double time) {
	setCurrentTime(currentTime() - time);
}

double RosBagAnnotator::findPreviousTime(const QString &topic) {
	assert(mTopics.find(topic) != mTopics.end());

	uint64_t prevTime = mCurrentTime;
	const QString &type = mTopics[topic].toString();

	if (type == "Bool") {
		prevTime = previousMessageTime(mBoolMsgs[topic], mCurrentBool[topic]);
	}
	else if (type == "Float") {
		prevTime = previousMessageTime(mFloatMsgs[topic], mCurrentFloat[topic]);
	}
	else if (type == "Int") {
		prevTime = previousMessageTime(mIntMsgs[topic], mCurrentInt[topic]);
	}
	else if (type == "String") {
		prevTime = previousMessageTime(mStringMsgs[topic], mCurrentString[topic]);
	}
	else if (type == "IntArray") {
		prevTime = previousMessageTime(mIntArrayMsgs[topic], mCurrentIntArray[topic]);
	}
	else if (type == "DoubleArray") {
		prevTime = previousMessageTime(mDoubleArrayMsgs[topic], mCurrentDoubleArray[topic]);
	}
	else if (type == "Audio") {
		prevTime = previousMessageTime(mAudioMsgs[topic], mCurrentAudio[topic]);
	}
	else if (type == "Image") {
		prevTime = previousMessageTime(mImageMsgs[topic], mCurrentImage[topic]);
	}

	return std::max(1e-9 * (prevTime - mStartTime), 0.0);
}

double RosBagAnnotator::findNextTime(const QString &topic) {
	assert(mTopics.find(topic) != mTopics.end());

	uint64_t nextTime = mCurrentTime;
	const QString &type = mTopics[topic].toString();

	if (type == "Bool") {
		nextTime = nextMessageTime(mBoolMsgs[topic], mCurrentBool[topic]);
	}
	else if (type == "Float") {
		nextTime = nextMessageTime(mFloatMsgs[topic], mCurrentFloat[topic]);
	}
	else if (type == "Int") {
		nextTime = nextMessageTime(mIntMsgs[topic], mCurrentInt[topic]);
	}
	else if (type == "String") {
		nextTime = nextMessageTime(mStringMsgs[topic], mCurrentString[topic]);
	}
	else if (type == "IntArray") {
		nextTime = nextMessageTime(mIntArrayMsgs[topic], mCurrentIntArray[topic]);
	}
	else if (type == "DoubleArray") {
		nextTime = nextMessageTime(mDoubleArrayMsgs[topic], mCurrentDoubleArray[topic]);
	}
	else if (type == "Audio") {
		nextTime = nextMessageTime(mAudioMsgs[topic], mCurrentAudio[topic]);
	}
	else if (type == "Image") {
		nextTime = nextMessageTime(mImageMsgs[topic], mCurrentImage[topic]);
	}

	return std::min(1e-9 * (nextTime - mStartTime), 1e-9 * (mEndTime - mStartTime));
}

QVariant RosBagAnnotator::getCurrentValue(const QString &topic) {
	static sensor_msgs::CompressedImage::ConstPtr lastImagePtr = nullptr;
	static QImage lastImage;

	QVariant value;

	auto it = mTopics.find(topic);
	if (it == mTopics.end()) {
		return value;
	}

	const QString type = it.value().toString();

	if (type == "Bool") {
		auto it = mCurrentBool[topic];
		if (it >= mBoolMsgs[topic].begin()) {
			value = it->second;
		}
	}
	else if (type == "Float") {
		auto it = mCurrentFloat[topic];
		if (it >= mFloatMsgs[topic].begin()) {
			value = it->second;
		}
	}
	else if (type == "Int") {
		auto it = mCurrentInt[topic];
		if (it >= mIntMsgs[topic].begin()) {
			value = it->second;
		}
	}
	else if (type == "String") {
		auto it = mCurrentString[topic];
		if (it >= mStringMsgs[topic].begin()) {
			value = it->second;
		}
	}
	else if (type == "IntArray") {
		auto it = mCurrentIntArray[topic];
		if (it >= mIntArrayMsgs[topic].begin()) {
			value = it->second;
		}
	}
	else if (type == "DoubleArray") {
		auto it = mCurrentDoubleArray[topic];
		if (it >= mDoubleArrayMsgs[topic].begin()) {
			value = it->second;
		}
	}
	else if (type == "Audio") {
		auto it = mCurrentAudio[topic];
		if (it >= mAudioMsgs[topic].begin()) {
			value = it->second;
		}
	}
	else if (type == "Image") {
		auto it = mCurrentImage[topic];
		if (it >= mImageMsgs[topic].begin()) {
			if (it->second != lastImagePtr) {
				lastImagePtr = it->second;
				lastImage.loadFromData(lastImagePtr->data.data(), lastImagePtr->data.size(), lastImagePtr->format.c_str());
			}

			value = lastImage;
		}
	}

	return value;
}

void RosBagAnnotator::play(double frequency, const QString &audioTopic) {
	stop();

	mPlaybackStartTime = mCurrentTime;
	mAudioTopic = audioTopic;

	mPlaybackTimer.setTimerType(Qt::PreciseTimer);
	mPlaybackTimer.setInterval(1e3 / frequency);

	mPlaybackElapsedTimer.start();
	mPlaybackTimer.start();

	emit playingChanged(true);
}

void RosBagAnnotator::stop() {
	mPlaybackTimer.stop();

	mMediaPlayer.stop();
	mMediaPlayer.setMedia(QMediaContent());
	if (mAudioBuffer.isOpen()) {
		mAudioBuffer.close();
	}

	emit playingChanged(false);
}

void RosBagAnnotator::annotate(const QString &topic, const QVariant &value, const AnnotationType type) {
	// Input validation should occur before passing a value to this function.
	// Invalid inputs will result in default-constructed values being written to the bag.
	if (type == BOOL) {
		std_msgs::Bool msg;
		msg.data = value.toBool();
		publishAnnotation(topic, type, msg);
	}
	else if (type == INT) {
		std_msgs::Int32 msg;
		msg.data = value.toInt();
		publishAnnotation(topic, type, msg);
	}
	else if (type == FLOAT) {
		std_msgs::Float64 msg;
		msg.data = value.toDouble();
		publishAnnotation(topic, type, msg);
	}
	else if (type == STRING) {
		std_msgs::String msg;
		msg.data = value.toString().toStdString();
		publishAnnotation(topic, type, msg);
	}
	else if (type == INT_ARRAY || type == DOUBLE_ARRAY) {
		QJSValue jsValue = value.value<QJSValue>();
		int length = jsValue.property("length").toInt();

		if (type == INT_ARRAY) {
			std_msgs::Int32MultiArray msg;
			for (int i = 0; i < length; ++i) {
				msg.data.push_back(jsValue.property(i).toInt());
			}
			publishAnnotation(topic, type, msg);
		}
		else {
			std_msgs::Float64MultiArray msg;
			for (int i = 0; i < length; ++i) {
				msg.data.push_back(jsValue.property(i).toInt());
			}
			publishAnnotation(topic, type, msg);
		}
	}
}

void RosBagAnnotator::updatePlayback() {
	uint64_t currentTime = mPlaybackStartTime + mPlaybackElapsedTimer.nsecsElapsed();

	if (currentTime > mEndTime) {
		stop();
	}
	else {
		setCurrentTime(1e-9 * (currentTime - mStartTime));

		if (mMediaPlayer.state() != QMediaPlayer::PlayingState) {
			playAudio(mAudioTopic);
		}
	}
}

void RosBagAnnotator::reset() {
	stop();

	mStartTime = mEndTime = mCurrentTime = 0;

	mTopics.clear();
	mTopicsByType.clear();

	mCurrentBool.clear();
	mCurrentFloat.clear();
	mCurrentInt.clear();
	mCurrentString.clear();
	mCurrentIntArray.clear();
	mCurrentDoubleArray.clear();
	mCurrentAudio.clear();
	mCurrentImage.clear();

	mBoolMsgs.clear();
	mFloatMsgs.clear();
	mIntMsgs.clear();
	mStringMsgs.clear();
	mIntArrayMsgs.clear();
	mDoubleArrayMsgs.clear();
	mAudioMsgs.clear();
	mImageMsgs.clear();

	mAudioByteArrays.clear();

	mStatus = EMPTY;
	emit statusChanged(mStatus);
	emit lengthChanged(length());
	emit topicsChanged(mTopics);
	emit topicsByTypeChanged(mTopicsByType);
	emit currentTimeChanged(0.0);
}

void RosBagAnnotator::parseBag() {
	mStatus = PARSING;
	emit statusChanged(mStatus);

	mStartTime = std::numeric_limits<uint64_t>::max();
	mEndTime = 0;

	rosbag::View view(*mBag);

	for (auto it = view.begin(); it != view.end(); ++it)
	{
		extractMessage(*it);
	}

	mCurrentTime = mStartTime;

	sortMessages(mBoolMsgs);
	sortMessages(mFloatMsgs);
	sortMessages(mIntMsgs);
	sortMessages(mStringMsgs);
	sortMessages(mIntArrayMsgs);
	sortMessages(mDoubleArrayMsgs);

	emit lengthChanged(length());
	emit topicsChanged(mTopics);
	emit topicsByTypeChanged(mTopicsByType);

	setCurrentTime(0.0);

	mStatus = READY;
	emit statusChanged(mStatus);
}

void RosBagAnnotator::extractMessage(const rosbag::MessageInstance &msg) {
	const QString topic(msg.getTopic().c_str());
	QString type(msg.getDataType().c_str());
	uint64_t time = msg.getTime().toNSec();

	if (type == "chili_msgs/Bool") {
		type = "Bool";
		chili_msgs::Bool::ConstPtr m = msg.instantiate<chili_msgs::Bool>();
		if (!mUseRosTime) {
			time = extractChiliMessageTime(m);
		}

		mBoolMsgs[topic].append(QPair<uint64_t, bool>(time, m->value));
	}
	else if (type == "chili_msgs/Float32") {
		type = "Float";
		chili_msgs::Float32::ConstPtr m = msg.instantiate<chili_msgs::Float32>();
		if (!mUseRosTime) {
			time = extractChiliMessageTime(m);
		}

		mFloatMsgs[topic].append(QPair<uint64_t, float>(time, m->value));
	}
	else if (type == "chili_msgs/Int32"){
		type = "Int";
		chili_msgs::Int32::ConstPtr m = msg.instantiate<chili_msgs::Int32>();
		if (!mUseRosTime) {
			time = extractChiliMessageTime(m);
		}

		mIntMsgs[topic].append(QPair<uint64_t, int>(time, m->value));
	}
	else if (type == "chili_msgs/String"){
		type = "String";
		chili_msgs::String::ConstPtr m = msg.instantiate<chili_msgs::String>();
		if (!mUseRosTime) {
			time = extractChiliMessageTime(m);
		}

		mStringMsgs[topic].append(QPair<uint64_t, QString>(time, m->value.c_str()));
	}
	else if (type == "chili_msgs/Vector2Float32"){
		type = "DoubleArray";
		chili_msgs::Vector2Float32::ConstPtr m = msg.instantiate<chili_msgs::Vector2Float32>();
		if (!mUseRosTime) {
			time = extractChiliMessageTime(m);
		}

		QList<QVariant> data{m->x, m->y};
		mDoubleArrayMsgs[topic].append(QPair<uint64_t, QList<QVariant>>(time, data));
	}
	else if (type == "chili_msgs/Vector2Int32"){
		type = "IntArray";
		chili_msgs::Vector2Int32::ConstPtr m = msg.instantiate<chili_msgs::Vector2Int32>();
		if (!mUseRosTime) {
			time = extractChiliMessageTime(m);
		}


		QList<QVariant> data{m->x, m->y};
		mIntArrayMsgs[topic].append(QPair<uint64_t, QList<QVariant>>(time, data));
	}
	else if (type == "chili_msgs/Vector3Float32"){
		type = "DoubleArray";
		chili_msgs::Vector3Float32::ConstPtr m = msg.instantiate<chili_msgs::Vector3Float32>();
		if (!mUseRosTime) {
			time = extractChiliMessageTime(m);
		}

		QList<QVariant> data{m->x, m->y, m->z};
		mDoubleArrayMsgs[topic].append(QPair<uint64_t, QList<QVariant>>(time, data));
	}
	else if (type == "audio_common_msgs/AudioData") {
		type = "Audio";
		audio_common_msgs::AudioData::ConstPtr m = msg.instantiate<audio_common_msgs::AudioData>();

		if (mAudioByteArrays.find(topic) == mAudioByteArrays.end()) {
			mAudioByteArrays.insert(topic, QByteArray());
		}

		mAudioMsgs[topic].append(QPair<uint64_t, int>(time, mAudioByteArrays[topic].size()));
		mAudioByteArrays[topic].append(QByteArray(reinterpret_cast<const char *>(m->data.data()), m->data.size()));
	}
	else if (type == "sensor_msgs/CompressedImage") {
		type = "Image";
		sensor_msgs::CompressedImage::ConstPtr m = msg.instantiate<sensor_msgs::CompressedImage>();
		mImageMsgs[topic].append(QPair<uint64_t, sensor_msgs::CompressedImage::ConstPtr>(time, m));
	}
	else if (type == "std_msgs/Bool") {
		type = "Bool";
		std_msgs::Bool::ConstPtr m = msg.instantiate<std_msgs::Bool>();
		mBoolMsgs[topic].append(QPair<uint64_t, bool>(time, m->data));
	}
	else if (type == "std_msgs/Int32") {
		type = "Int";
		std_msgs::Int32::ConstPtr m = msg.instantiate<std_msgs::Int32>();
		mIntMsgs[topic].append(QPair<uint64_t, int>(time, m->data));
	}
	else if (type == "std_msgs/Float64") {
		type = "Int";
		std_msgs::Float64::ConstPtr m = msg.instantiate<std_msgs::Float64>();
		mFloatMsgs[topic].append(QPair<uint64_t, float>(time, m->data));
	}
	else if (type == "std_msgs/String") {
		type = "String";
		std_msgs::String::ConstPtr m = msg.instantiate<std_msgs::String>();
		mStringMsgs[topic].append(QPair<uint64_t, QString>(time, m->data.c_str()));
	}
	else if (type == "std_msgs/Int32MultiArray") {
		type = "IntArray";
		std_msgs::Int32MultiArray::ConstPtr m = msg.instantiate<std_msgs::Int32MultiArray>();
		QList<QVariant> data;
		for (auto value : m->data) {
			data.append(value);
		}
		mIntArrayMsgs[topic].append(QPair<uint64_t, QList<QVariant>>(time, data));
	}
	else if (type == "std_msgs/Float64MultiArray") {
		type = "DoubleArray";
		std_msgs::Float64MultiArray::ConstPtr m = msg.instantiate<std_msgs::Float64MultiArray>();
		QList<QVariant> data;
		for (auto value : m->data) {
			data.append(value);
		}
		mDoubleArrayMsgs[topic].append(QPair<uint64_t, QList<QVariant>>(time, data));
	}

	if (mTopics.find(topic) == mTopics.end()) {
		mTopics.insert(topic, QVariant(type));

		if (mTopicsByType.find(type) == mTopicsByType.end()) {
			mTopicsByType.insert(type, QVariantList({topic}));
		}
		else {
			QVariantList tmp = mTopicsByType[type].toList();
			tmp.append(topic);
			mTopicsByType.insert(type, tmp);
		}
	}

	if (time < mStartTime) {
		mStartTime = time;
	}

	if (time > mEndTime) {
		mEndTime = time;
	}
}

void RosBagAnnotator::playAudio(const QString &audioTopic) {
	// check for existence of topic
	auto it = mAudioMsgs.find(audioTopic);
	if (it == mAudioMsgs.end()) {
		return;
	}

	// check if audio has ended
	if (it->rbegin()->first < mCurrentTime) {
		return;
	}

	// check if audio has started
	auto currentIt = mCurrentAudio[audioTopic];
	if (currentIt < it->begin()) {
		return;
	}

	// seek to correct position when setting up the buffer
	mAudioBuffer.setData(
		mAudioByteArrays[audioTopic].constData() + currentIt->second, 
		mAudioByteArrays[audioTopic].size() - currentIt->second
	);

	mAudioBuffer.open(QIODevice::ReadOnly);
	mMediaPlayer.setMedia(QMediaContent(), &mAudioBuffer);
	mMediaPlayer.play();
}
