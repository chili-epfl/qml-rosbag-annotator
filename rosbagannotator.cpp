#include "rosbagannotator.h"

#include <rosbag/bag.h>
#include <rosbag/view.h>

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

    seekCurrentMessageIndices(mBoolMsgs);
    seekCurrentMessageIndices(mFloatMsgs);
    seekCurrentMessageIndices(mIntMsgs);
    seekCurrentMessageIndices(mStringMsgs);
    seekCurrentMessageIndices(mVector2Msgs);
    seekCurrentMessageIndices(mVector3Msgs);
    seekCurrentMessageIndices(mAudioMsgs);
    seekCurrentMessageIndices(mImageMsgs);

    updateCurrentItems("Bool", mCurrentBool, mBoolMsgs);
    updateCurrentItems("Float", mCurrentFloat, mFloatMsgs);
    updateCurrentItems("Int", mCurrentInt, mIntMsgs);
    updateCurrentItems("String", mCurrentString, mStringMsgs);
    updateCurrentItems("Vector2", mCurrentVector2, mVector2Msgs);
    updateCurrentItems("Vector3", mCurrentVector3, mVector3Msgs);
    updateCurrentItems("Audio", mCurrentAudio, mAudioMsgs);
    updateCurrentItems("Image", mCurrentImage, mImageMsgs);

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
		assert(mBoolMsgs.find(topic) != mBoolMsgs.end());
		prevTime = previous(mBoolMsgs[topic]);
	}
	else if (type == "Float") {
		assert(mFloatMsgs.find(topic) != mFloatMsgs.end());
		prevTime = previous(mFloatMsgs[topic]);
	}
	else if (type == "Int") {
		assert(mIntMsgs.find(topic) != mIntMsgs.end());
		prevTime = previous(mIntMsgs[topic]);
	}
	else if (type == "String") {
		assert(mStringMsgs.find(topic) != mStringMsgs.end());
		prevTime = previous(mStringMsgs[topic]);
	}
	else if (type == "Vector2") {
		assert(mVector2Msgs.find(topic) != mVector2Msgs.end());
		prevTime = previous(mVector2Msgs[topic]);
	}
	else if (type == "Vector3") {
		assert(mVector3Msgs.find(topic) != mVector3Msgs.end());
		prevTime = previous(mVector3Msgs[topic]);
	}
	else if (type == "Audio") {
		assert(mAudioMsgs.find(topic) != mAudioMsgs.end());
		prevTime = previous(mAudioMsgs[topic]);
	}
	else if (type == "Image") {
		assert(mImageMsgs.find(topic) != mImageMsgs.end());
		prevTime = previous(mImageMsgs[topic]);
	}

	return std::max(1e-9 * (prevTime - mStartTime) - 1e-6, 0.0);
}

double RosBagAnnotator::findNextTime(const QString &topic) {
	assert(mTopics.find(topic) != mTopics.end());

	uint64_t nextTime = mCurrentTime;
	const QString &type = mTopics[topic].toString();

	if (type == "Bool") {
		assert(mBoolMsgs.find(topic) != mBoolMsgs.end());
		nextTime = next(mBoolMsgs[topic]);
	}
	else if (type == "Float") {
		assert(mFloatMsgs.find(topic) != mFloatMsgs.end());
		nextTime = next(mFloatMsgs[topic]);
	}
	else if (type == "Int") {
		assert(mIntMsgs.find(topic) != mIntMsgs.end());
		nextTime = next(mIntMsgs[topic]);
	}
	else if (type == "String") {
		assert(mStringMsgs.find(topic) != mStringMsgs.end());
		nextTime = next(mStringMsgs[topic]);
	}
	else if (type == "Vector2") {
		assert(mVector2Msgs.find(topic) != mVector2Msgs.end());
		nextTime = next(mVector2Msgs[topic]);
	}
	else if (type == "Vector3") {
		assert(mVector3Msgs.find(topic) != mVector3Msgs.end());
		nextTime = next(mVector3Msgs[topic]);
	}
	else if (type == "Audio") {
		assert(mAudioMsgs.find(topic) != mAudioMsgs.end());
		nextTime = next(mAudioMsgs[topic]);
	}
	else if (type == "Image") {
		assert(mImageMsgs.find(topic) != mImageMsgs.end());
		nextTime = next(mImageMsgs[topic]);
	}

	return std::min(1e-9 * (nextTime - mStartTime) + 1e-6, 1e-9 * (mEndTime - mStartTime));
}

QVariant RosBagAnnotator::getCurrentValue(const QString &topic) {
	assert(mTopics.find(topic) != mTopics.end());

	const QString &type = mTopics[topic].toString();
	QVariant value;

	if (type == "Bool") {
		auto it = mCurrentBool.find(topic);
		if (it != mCurrentBool.end()) {
			value = *it;
		}
	}
	else if (type == "Float") {
		auto it = mCurrentFloat.find(topic);
		if (it != mCurrentFloat.end()) {
			value = *it;
		}
	}
	else if (type == "Int") {
		auto it = mCurrentInt.find(topic);
		if (it != mCurrentInt.end()) {
			value = *it;
		}
	}
	else if (type == "String") {
		auto it = mCurrentString.find(topic);
		if (it != mCurrentString.end()) {
			value = *it;
		}
	}
	else if (type == "Vector2") {
		auto it = mCurrentVector2.find(topic);
		if (it != mCurrentVector2.end()) {
			value = *it;
		}
	}
	else if (type == "Vector3") {
		auto it = mCurrentVector3.find(topic);
		if (it != mCurrentVector3.end()) {
			value = *it;
		}
	}
	else if (type == "Audio") {
		auto it = mCurrentAudio.find(topic);
		if (it != mCurrentAudio.end()) {
			value = it->first;
		}
	}
	else if (type == "Image") {
		auto it = mCurrentImage.find(topic);
		if (it != mCurrentImage.end()) {
			value = *it;
		}
	}

	return value;
}

void RosBagAnnotator::reset() {
	mStartTime = mEndTime = mCurrentTime = 0;

	mTopics.clear();
	mTopicsByType.clear();
	mCurrentMessageIndices.clear();

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

	mStatus = EMPTY;
	emit statusChanged(mStatus);

    emit currentTimeChanged(0.0f);
    emit lengthChanged(length());
    emit topicsChanged(mTopics);
    emit topicsByTypeChanged(mTopicsByType);
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

	mStatus = READY;

	emit statusChanged(mStatus);
    emit currentTimeChanged(0.0f);
    emit lengthChanged(length());
    emit topicsChanged(mTopics);
    emit topicsByTypeChanged(mTopicsByType);
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
			mAudioByteArrays[topic].append(QByteArray(reinterpret_cast<const char *>(m->data.data()), m->data.size()));
			mAudioMsgs[topic].append(QPair<uint64_t, QPair<int, int>>(time, QPair<int, int>(mAudioByteArrays[topic].size(), m->data.size())));
		}
		else if (type == "sensor_msgs/CompressedImage") {
			type = "Image";
			sensor_msgs::CompressedImage::ConstPtr m = msg.instantiate<sensor_msgs::CompressedImage>();
			QImage img;
			img.loadFromData(m->data.data(), m->data.size(), m->format.c_str());
			mImageMsgs[topic].append(QPair<uint64_t, QImage>(time, img));
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

		mCurrentMessageIndices[topic] = 0;
	}

	if (time < mStartTime) {
		mStartTime = time;
	}

	if (time > mEndTime) {
		mEndTime = time;
	}
}
