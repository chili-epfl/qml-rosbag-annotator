#include "rosbagannotator.h"

#include <rosbag/bag.h>
#include <rosbag/view.h>

#include <audio_common_msgs/AudioData.h>
#include <sensor_msgs/CompressedImage.h>

#include <chili_msgs/Bool.h>
#include <chili_msgs/Float32.h>
#include <chili_msgs/Int32.h>
#include <chili_msgs/String.h>
#include <chili_msgs/Vector2Float32.h>
#include <chili_msgs/Vector3Float32.h>
#include <chili_msgs/Vector2Int32.h>

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
	emit bagPathChanged(mBagPath);

	reset();

	if (!mBagPath.isEmpty()) {
		std::unique_ptr<rosbag::Bag> bag;
		try {
			bag.reset(new rosbag::Bag(mBagPath.toStdString()));
		}
		catch (const rosbag::BagException &e) {
			qDebug() << "An exception has occured while opening bag " << mBagPath << ": " << e.what();
			return;
		}

		parseBag(std::move(bag));
	}
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
	seekCurrentMessageIndices(mVector2Msgs, mCurrentVector2);
	seekCurrentMessageIndices(mVector3Msgs, mCurrentVector3);
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
	else if (type == "Vector2") {
		prevTime = previousMessageTime(mVector2Msgs[topic], mCurrentVector2[topic]);
	}
	else if (type == "Vector3") {
		prevTime = previousMessageTime(mVector3Msgs[topic], mCurrentVector3[topic]);
	}
	else if (type == "Audio") {
		prevTime = previousMessageTime(mAudioMsgs[topic], mCurrentAudio[topic]);
	}
	else if (type == "Image") {
		prevTime = previousMessageTime(mImageMsgs[topic], mCurrentImage[topic]);
	}

	return std::max(1e-9 * (prevTime - mStartTime) - 1e-6, 0.0);
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
	else if (type == "Vector2") {
		nextTime = nextMessageTime(mVector2Msgs[topic], mCurrentVector2[topic]);
	}
	else if (type == "Vector3") {
		nextTime = nextMessageTime(mVector3Msgs[topic], mCurrentVector3[topic]);
	}
	else if (type == "Audio") {
		nextTime = nextMessageTime(mAudioMsgs[topic], mCurrentAudio[topic]);
	}
	else if (type == "Image") {
		nextTime = nextMessageTime(mImageMsgs[topic], mCurrentImage[topic]);
	}

	return std::min(1e-9 * (nextTime - mStartTime) + 1e-6, 1e-9 * (mEndTime - mStartTime));
}

QVariant RosBagAnnotator::getCurrentValue(const QString &topic) {
	assert(mTopics.find(topic) != mTopics.end());

	const QString &type = mTopics[topic].toString();
	QVariant value;

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
	else if (type == "Vector2") {
		auto it = mCurrentVector2[topic];
		if (it >= mVector2Msgs[topic].begin()) {
			value = it->second;
		}
	}
	else if (type == "Vector3") {
		auto it = mCurrentVector3[topic];
		if (it >= mVector3Msgs[topic].begin()) {
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
			value = it->second;
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
	mAudioBuffer.close();

	emit playingChanged(false);
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
	mStartTime = mEndTime = mCurrentTime = 0;

	mTopics.clear();
	mTopicsByType.clear();

	mCurrentBool.clear();
	mCurrentFloat.clear();
	mCurrentInt.clear();
	mCurrentString.clear();
	mCurrentVector2.clear();
	mCurrentVector3.clear();
	mCurrentAudio.clear();
	mCurrentImage.clear();

	mBoolMsgs.clear();
	mFloatMsgs.clear();
	mIntMsgs.clear();
	mStringMsgs.clear();
	mVector2Msgs.clear();
	mVector3Msgs.clear();
	mAudioMsgs.clear();
	mImageMsgs.clear();

	mAudioByteArrays.clear();

	emit lengthChanged(length());
	emit topicsChanged(mTopics);
	emit topicsByTypeChanged(mTopicsByType);
	emit currentTimeChanged(0.0);

	mStatus = EMPTY;
	emit statusChanged(mStatus);
}

void RosBagAnnotator::parseBag(std::unique_ptr<rosbag::Bag> &&bag) {
	mStatus = PARSING;
	emit statusChanged(mStatus);

	mStartTime = std::numeric_limits<uint64_t>::max();
	mEndTime = 0;

	rosbag::View view(*bag);

	for (auto it = view.begin(); it != view.end(); ++it)
	{
		extractMessage(*it);
	}

	mCurrentTime = mStartTime;

	sortMessages(mBoolMsgs);
	sortMessages(mFloatMsgs);
	sortMessages(mIntMsgs);
	sortMessages(mStringMsgs);
	sortMessages(mVector2Msgs);
	sortMessages(mVector3Msgs);

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

	if (type.startsWith("chili_msgs")) {
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
			type = "Vector2";
			chili_msgs::Vector2Float32::ConstPtr m = msg.instantiate<chili_msgs::Vector2Float32>();
			if (!mUseRosTime) {
				time = extractChiliMessageTime(m);
			}

			mVector2Msgs[topic].append(QPair<uint64_t, QVector2D>(time, QVector2D(m->x, m->y)));
		}
		else if (type == "chili_msgs/Vector2Int32"){
			type = "Vector2";
			chili_msgs::Vector2Int32::ConstPtr m = msg.instantiate<chili_msgs::Vector2Int32>();
			if (!mUseRosTime) {
				time = extractChiliMessageTime(m);
			}

			mVector2Msgs[topic].append(QPair<uint64_t, QVector2D>(time, QVector2D(m->x, m->y)));
		}
		else if (type == "chili_msgs/Vector3Float32"){
			type = "Vector3";
			chili_msgs::Vector3Float32::ConstPtr m = msg.instantiate<chili_msgs::Vector3Float32>();
			if (!mUseRosTime) {
				time = extractChiliMessageTime(m);
			}

			mVector3Msgs[topic].append(QPair<uint64_t, QVector3D>(time, QVector3D(m->x, m->y, m->z)));
		}
	}
	else {
		if (type == "audio_common_msgs/AudioData") {
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
			mImageMsgs[topic].append(QPair<uint64_t, QImage>(time, QImage::fromData(m->data.data(), m->data.size(), m->format.c_str())));
		}
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

	qDebug() << "Playing audio from topic" << audioTopic;

	// seek to correct position when setting up the buffer
	// (QMediaPlayer can only seek asynchronously)
	mAudioBuffer.setData(
		mAudioByteArrays[audioTopic].constData() + currentIt->second, 
		mAudioByteArrays[audioTopic].size() - currentIt->second
	);

	mAudioBuffer.open(QIODevice::ReadOnly);
	mMediaPlayer.setMedia(QMediaContent(), &mAudioBuffer);
	mMediaPlayer.play();
}
