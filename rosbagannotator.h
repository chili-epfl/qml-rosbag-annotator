#ifndef ROSBAGANNOTATOR_H
#define ROSBAGANNOTATOR_H

#include <QQuickItem>
#include <QBuffer>
#include <QFileInfo>
#include <QImage>
#include <QMediaPlayer>
#include <QTimer>
#include <QElapsedTimer>
#include <QVector2D>
#include <QVector3D>

namespace rosbag {
	class Bag;
}

#include <rosbag/message_instance.h>
#include <sensor_msgs/CompressedImage.h>

#include <memory>

class RosBagAnnotator : public QQuickItem
{
	Q_OBJECT
	Q_DISABLE_COPY(RosBagAnnotator)

	Q_PROPERTY(QString bagPath READ bagPath WRITE setBagPath NOTIFY bagPathChanged)
	Q_PROPERTY(bool useRosTime READ useRosTime WRITE setUseRosTime NOTIFY useRosTimeChanged)
	Q_PROPERTY(bool useSeparateBag READ useSeparateBag WRITE setUseSeparateBag NOTIFY useSeparateBagChanged)
	Q_PROPERTY(Status status READ status NOTIFY statusChanged)
	Q_PROPERTY(double length READ length NOTIFY lengthChanged)
	Q_PROPERTY(double currentTime READ currentTime WRITE setCurrentTime NOTIFY currentTimeChanged)
	Q_PROPERTY(QVariantMap topics READ topics NOTIFY topicsChanged)
	Q_PROPERTY(QVariantMap topicsByType READ topicsByType NOTIFY topicsByTypeChanged)
	Q_PROPERTY(QVariantMap annotationTopics READ annotationTopics NOTIFY annotationTopicsChanged)
	Q_PROPERTY(bool playing READ playing NOTIFY playingChanged)

public:
	enum Status {
		EMPTY,
		PARSING,
		READY
	};
	Q_ENUM(Status)

	enum AnnotationType {
		BOOL,
		INT,
		FLOAT,
		STRING,
		INT_ARRAY,
		DOUBLE_ARRAY
	};
	Q_ENUM(AnnotationType)

	RosBagAnnotator(QQuickItem *parent = nullptr);
	~RosBagAnnotator();

	Status status() const { return mStatus; }
	const QString &bagPath() const { return mBagPath; }
	bool useRosTime() const { return mUseRosTime; }
	bool useSeparateBag() const { return mUseSeparateBag; }
	double length() const { return 1e-9 * (mEndTime - mStartTime); }
	double currentTime() const { return 1e-9 * (mCurrentTime - mStartTime); }
	const QVariantMap &topics() const { return mTopics; }
	const QVariantMap &topicsByType() const { return mTopicsByType; }
	bool playing() const { return mMediaPlayer.state() == QMediaPlayer::PlayingState; }
	const QVariantMap &annotationTopics() const { return mAnnotationTopics; }

public slots:
	void setBagPath(QString path);
	void setUseRosTime(bool use) {
		mUseRosTime = use;
		emit useRosTimeChanged(use);
	}
	void setUseSeparateBag(bool use) {
		mUseSeparateBag = use;
		emit useSeparateBagChanged(use);
	}

	void setCurrentTime(double time);
	void advance(double time);
	void rewind(double time);

	double findPreviousTime(const QString &topic);
	double findNextTime(const QString &topic);

	QVariant getCurrentValue(const QString &topic);

	void play(double frequency, const QString &audioTopic);
	void stop();

	void annotate(const QString &topic, const QVariant &value, AnnotationType type);

signals:
	void statusChanged(Status status);
	void bagPathChanged(const QString &path);
	void useRosTimeChanged(bool use);
	void useSeparateBagChanged(bool use);
	void lengthChanged(double length);
	void currentTimeChanged(double time);
	void topicsChanged(const QVariantMap &topics);
	void topicsByTypeChanged(const QVariantMap &topicsByType);
	void playingChanged(bool playing);
	void annotationTopicsChanged(const QVariantMap &annotationTopics);

private slots:
	void updatePlayback();

private:
	void reset();
	void parseBag();
	void extractMessage(const rosbag::MessageInstance &msg);
	void playAudio(const QString &audioTopic);


	template<class T>
	uint64_t extractChiliMessageTime(const T msg) {
		return msg->header.stamp;
	}

	template<class T>
	void sortMessages(QMap<QString, QList<QPair<uint64_t, T>>> &typedMessages) {
		for (auto it = typedMessages.begin(); it != typedMessages.end(); ++it) {
			std::sort(it->begin(), it->end(), 
				[&](const QPair<uint64_t, T> &a, const QPair<uint64_t, T> &b) {
					return a.first < b.first;
				}
			);
		}
	}

	template<class T>
	void seekCurrentMessageIndices(const QMap<QString, QList<QPair<uint64_t, T>>> &typedMessages,
								   QMap<QString, typename QList<QPair<uint64_t, T>>::const_iterator> &currentMessages) {
		for (auto topicIt = typedMessages.begin(); topicIt != typedMessages.end(); ++topicIt) {
			const QString &topic = topicIt.key();
			const auto &messages = topicIt.value();
			
			auto currentIt = messages.begin() - 1;
			if (currentMessages.find(topicIt.key()) != currentMessages.end()) {
				currentIt = currentMessages[topic];
			}

			uint64_t prevTime = mStartTime;
			if (currentIt >= messages.begin()) {
				prevTime = currentIt->first;
			}

			if (prevTime < mCurrentTime) {
				for (auto peekIt = currentIt + 1; peekIt != messages.end() && peekIt->first <= mCurrentTime; ++currentIt, ++peekIt);
			}
			else if (prevTime > mCurrentTime) {
				for (; currentIt >= messages.begin() && currentIt->first > mCurrentTime; --currentIt);
			}

			currentMessages[topic] = currentIt;
		}
	}

	template<class T>
	uint64_t previousMessageTime(const QList<QPair<uint64_t, T>>& messages, typename QList<QPair<uint64_t, T>>::const_iterator current) {
		if (current <= messages.begin()) {
			return mCurrentTime;
		}

		auto previous = current - 1;
		return previous->first;
	}

	template<class T>
	uint64_t nextMessageTime(const QList<QPair<uint64_t, T>>& messages, typename QList<QPair<uint64_t, T>>::const_iterator current) {
		auto next = current + 1;
		if (next == messages.end()) {
			return mCurrentTime;
		}
		else {
			return next->first;
		}
	}

	template<class T>
	void publishAnnotation(const QString &topic, const AnnotationType type, const T& msg) {
		if (mStatus != READY) {
			qDebug() << "Cannot publish annotation because bag isn't ready!";
			return;
		}

		auto it = mAnnotationTopics.find(topic);
		if (it != mAnnotationTopics.end()) {
			if (it->value<AnnotationType>() != type) {
				qDebug() << "Cannot publish different type to existing topic!";
				return;
			}
		}
		else {
			mAnnotationTopics.insert(topic, type);
			emit annotationTopicsChanged(mAnnotationTopics);
		}

		ros::Time time;
		time.fromNSec(mCurrentTime);
		std::string annotationTopic = ("/annotation/" + topic).toStdString();
		qDebug() << "Writing annotation of type" << type << "to topic" << annotationTopic.c_str();

		QString annotationBagPath = mBagPath;
		rosbag::bagmode::BagMode mode = rosbag::bagmode::Append;
		if (mUseSeparateBag) {
			annotationBagPath.replace(".bag", "-annotations.bag");

			if (!QFileInfo::exists(annotationBagPath)) {
				mode = rosbag::bagmode::Write;
			}
		}

		try {
			mAnnotationBag.reset(new rosbag::Bag());
			mAnnotationBag->open(annotationBagPath.toStdString(), mode);
			mAnnotationBag->write(annotationTopic, time, msg);
			mAnnotationBag->close();
		}
		catch (const rosbag::BagException &e) {
			qDebug() << "An exception ocurred while writing annotation to bag:" << e.what();
		}
	}

	Status mStatus;
	QString mBagPath;
	std::unique_ptr<rosbag::Bag> mBag;
	std::unique_ptr<rosbag::Bag> mAnnotationBag;
	bool mUseRosTime;
	bool mUseSeparateBag;

	uint64_t mStartTime;
	uint64_t mEndTime;
	uint64_t mCurrentTime;
	uint64_t mPlaybackStartTime;

	QTimer mPlaybackTimer;
	QElapsedTimer mPlaybackElapsedTimer;

	QVariantMap mTopics;
	QVariantMap mTopicsByType;

	typedef sensor_msgs::CompressedImage::ConstPtr ImagePtr; 

	QMap<QString, QList<QPair<uint64_t, bool>>::const_iterator> mCurrentBool;
	QMap<QString, QList<QPair<uint64_t, float>>::const_iterator> mCurrentFloat;
	QMap<QString, QList<QPair<uint64_t, int>>::const_iterator> mCurrentInt;
	QMap<QString, QList<QPair<uint64_t, QString>>::const_iterator> mCurrentString;
	QMap<QString, QList<QPair<uint64_t, QList<QVariant>>>::const_iterator> mCurrentIntArray;
	QMap<QString, QList<QPair<uint64_t, QList<QVariant>>>::const_iterator> mCurrentDoubleArray;
	QMap<QString, QList<QPair<uint64_t, int>>::const_iterator> mCurrentAudio;
	QMap<QString, QList<QPair<uint64_t, ImagePtr>>::const_iterator> mCurrentImage;

	QMap<QString, QList<QPair<uint64_t, bool>>> mBoolMsgs;
	QMap<QString, QList<QPair<uint64_t, float>>> mFloatMsgs;
	QMap<QString, QList<QPair<uint64_t, int>>> mIntMsgs;
	QMap<QString, QList<QPair<uint64_t, QString>>> mStringMsgs;
	QMap<QString, QList<QPair<uint64_t, QList<QVariant>>>> mIntArrayMsgs;
	QMap<QString, QList<QPair<uint64_t, QList<QVariant>>>> mDoubleArrayMsgs;
	QMap<QString, QList<QPair<uint64_t, int>>> mAudioMsgs;
	QMap<QString, QList<QPair<uint64_t, ImagePtr>>> mImageMsgs;

	QMap<QString, QByteArray> mAudioByteArrays;

	QString mAudioTopic;
	QBuffer mAudioBuffer;
	QMediaPlayer mMediaPlayer;

	QVariantMap mAnnotationTopics;
};

#endif // ROSBAGANNOTATOR_H
