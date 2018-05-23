#ifndef ROSBAGANNOTATOR_H
#define ROSBAGANNOTATOR_H

#include <QQuickItem>
#include <QBuffer>
#include <QImage>
#include <QMediaPlayer>
#include <QVector2D>
#include <QVector3D>

namespace rosbag {
	class Bag;
}

#include <rosbag/message_instance.h>

#include <memory>

class RosBagAnnotator : public QQuickItem
{
	Q_OBJECT
	Q_DISABLE_COPY(RosBagAnnotator)

	Q_PROPERTY(QString bagPath READ bagPath WRITE setBagPath NOTIFY bagPathChanged)
	Q_PROPERTY(bool useRosTime READ useRosTime WRITE setUseRosTime NOTIFY useRosTimeChanged)
	Q_PROPERTY(Status status READ status NOTIFY statusChanged)
	Q_PROPERTY(double length READ length NOTIFY lengthChanged)
	Q_PROPERTY(double currentTime READ currentTime WRITE setCurrentTime NOTIFY currentTimeChanged)
	Q_PROPERTY(QVariantMap topics READ topics NOTIFY topicsChanged)
	Q_PROPERTY(QVariantMap topicsByType READ topicsByType NOTIFY topicsByTypeChanged)
	Q_PROPERTY(bool audioPlaying READ audioPlaying NOTIFY audioPlayingChanged)

public:
	enum Status {
		EMPTY,
		PARSING,
		READY
	};
	Q_ENUM(Status)

	RosBagAnnotator(QQuickItem *parent = nullptr);
	~RosBagAnnotator();

	Status status() const { return mStatus; }
	const QString &bagPath() const { return mBagPath; }
	bool useRosTime() const { return mUseRosTime; }
	double length() const { return 1e-9 * (mEndTime - mStartTime); }
	double currentTime() const { return 1e-9 * (mCurrentTime - mStartTime); }
	const QVariantMap &topics() const { return mTopics; }
	const QVariantMap &topicsByType() const { return mTopicsByType; }
	bool audioPlaying() const { return mMediaPlayer.state() == QMediaPlayer::PlayingState; }

public slots:
	void setBagPath(QString path);
	void setUseRosTime(bool use) {
		mUseRosTime = use;
		emit useRosTimeChanged(use);
	}

	void setCurrentTime(double time);
	void advance(double time);
	void rewind(double time);

	double findPreviousTime(const QString &topic);
	double findNextTime(const QString &topic);

	QVariant getCurrentValue(const QString &topic);

	void playAudio(const QString &topic);
	void stopAudio();

signals:
	void statusChanged(Status status);
	void bagPathChanged(const QString &path);
	void useRosTimeChanged(bool use);
	void lengthChanged(double length);
	void currentTimeChanged(double time);
	void topicsChanged(const QVariantMap &topics);
	void topicsByTypeChanged(const QVariantMap &topicsByType);
	void audioPlayingChanged(bool playing);

private slots:
	void playerStateChanged(QMediaPlayer::State state);

private:
	void reset();
	void parseBag(std::unique_ptr<rosbag::Bag> &&bag);
	void extractMessage(const rosbag::MessageInstance &msg);
	void updateCurrentItems();

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

	Status mStatus;
	QString mBagPath;
	bool mUseRosTime;

	uint64_t mStartTime;
	uint64_t mEndTime;
	uint64_t mCurrentTime;

	QVariantMap mTopics;
	QVariantMap mTopicsByType;

	QMap<QString, QList<QPair<uint64_t, bool>>::const_iterator> mCurrentBool;
	QMap<QString, QList<QPair<uint64_t, float>>::const_iterator> mCurrentFloat;
	QMap<QString, QList<QPair<uint64_t, int>>::const_iterator> mCurrentInt;
	QMap<QString, QList<QPair<uint64_t, QString>>::const_iterator> mCurrentString;
	QMap<QString, QList<QPair<uint64_t, QVector2D>>::const_iterator> mCurrentVector2;
	QMap<QString, QList<QPair<uint64_t, QVector3D>>::const_iterator> mCurrentVector3;
	QMap<QString, QList<QPair<uint64_t, int>>::const_iterator> mCurrentAudio;
	QMap<QString, QList<QPair<uint64_t, QImage>>::const_iterator> mCurrentImage;

	QMap<QString, QList<QPair<uint64_t, bool>>> mBoolMsgs;
	QMap<QString, QList<QPair<uint64_t, float>>> mFloatMsgs;
	QMap<QString, QList<QPair<uint64_t, int>>> mIntMsgs;
	QMap<QString, QList<QPair<uint64_t, QString>>> mStringMsgs;
	QMap<QString, QList<QPair<uint64_t, QVector2D>>> mVector2Msgs;
	QMap<QString, QList<QPair<uint64_t, QVector3D>>> mVector3Msgs;
	QMap<QString, QList<QPair<uint64_t, int>>> mAudioMsgs;
	QMap<QString, QList<QPair<uint64_t, QImage>>> mImageMsgs;

	QMap<QString, QByteArray> mAudioByteArrays;
	QBuffer mAudioBuffer;
	QMediaPlayer mMediaPlayer;
};

#endif // ROSBAGANNOTATOR_H
