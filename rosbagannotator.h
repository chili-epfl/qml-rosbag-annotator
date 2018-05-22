#ifndef ROSBAGANNOTATOR_H
#define ROSBAGANNOTATOR_H

#include <QQuickItem>
#include <QImage>
#include <QVector2D>
#include <QVector3D>

namespace rosbag {
    class Bag;
}

#include <rosbag/message_instance.h>

#include <audio_common_msgs/AudioData.h>
#include <sensor_msgs/CompressedImage.h>

#include <chili_msgs/Bool.h>
#include <chili_msgs/Float32.h>
#include <chili_msgs/Int32.h>
#include <chili_msgs/String.h>
#include <chili_msgs/Vector2Float32.h>
#include <chili_msgs/Vector3Float32.h>
#include <chili_msgs/Vector2Int32.h>

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

public slots:
    void setBagPath(QString path);

    void setUseRosTime(bool use) {
        mUseRosTime = use;
        emit useRosTimeChanged(use);
    }

    void setCurrentTime(double time);
    void advance(double time);
    void rewind(double time);

    QVariant getCurrentValue(const QString &topic);

signals:
    void statusChanged(Status status);
    void bagPathChanged(const QString &path);
    void useRosTimeChanged(bool use);
    void lengthChanged(double length);
    void currentTimeChanged(double time);
    void topicsChanged(const QVariantMap &topics);
    void topicsByTypeChanged(const QVariantMap &topicsByType);

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
    void sortMessages(QMap<QString, QList<QPair<uint64_t, T>>> &topicMessagesMap) {
        for (auto it = topicMessagesMap.begin(); it != topicMessagesMap.end(); ++it) {
            std::sort(it->begin(), it->end(), 
                [&](const QPair<uint64_t, T> &a, const QPair<uint64_t, T> &b) {
                    return a.first < b.first;
                }
            );
        }
    }

    template<class T>
    void seekCurrentMessageIndices(QMap<QString, QList<QPair<uint64_t, T>>> &topicMessagesMap) {
        for (auto topicIt = topicMessagesMap.begin(); topicIt != topicMessagesMap.end(); ++topicIt) {
            assert(topicIt.value().size());

            int index = -1;
            for (auto msgIt = topicIt->begin(); msgIt != topicIt->end() && msgIt->first <= mCurrentTime; ++msgIt) {
                index = msgIt - topicIt->begin();
            }

            mCurrentMessageIndices[topicIt.key()] = index;
        }
    }

    template<class T, class U>
    void updateCurrentItems(const QString &type, T &current, const U &messages) {
        const auto &topics = mTopicsByType[type].toList();
        for (auto it = topics.begin(); it != topics.end(); ++it) {
            const QString &topic = it->toString();

            assert(mCurrentMessageIndices.find(topic) != mCurrentMessageIndices.end());
            int index = mCurrentMessageIndices[topic];

            if (index < 0) {
                current.remove(topic);
            }
            else {
                assert(messages.find(topic) != messages.end());
                const auto &list = messages[topic];

                current[topic] = list.at(index).second;
            }
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

    QMap<QString, bool> mCurrentBool;
    QMap<QString, float> mCurrentFloat;
    QMap<QString, int> mCurrentInt;
    QMap<QString, QString> mCurrentString;
    QMap<QString, QVector2D> mCurrentVector2;
    QMap<QString, QVector3D> mCurrentVector3;
    QMap<QString, QPair<int,int>> mCurrentAudio;
    QMap<QString, QImage> mCurrentImage;

    QMap<QString, int> mCurrentMessageIndices; // index of most recent message for each topic

    QMap<QString, QList<QPair<uint64_t, bool>>> mBoolMsgs;
    QMap<QString, QList<QPair<uint64_t, float>>> mFloatMsgs;
    QMap<QString, QList<QPair<uint64_t, int>>> mIntMsgs;
    QMap<QString, QList<QPair<uint64_t, QString>>> mStringMsgs;
    QMap<QString, QList<QPair<uint64_t, QVector2D>>> mVector2Msgs;
    QMap<QString, QList<QPair<uint64_t, QVector3D>>> mVector3Msgs;
    QMap<QString, QList<QPair<uint64_t, QPair<int, int>>>> mAudioMsgs;
    QMap<QString, QList<QPair<uint64_t, QImage>>> mImageMsgs;

    QMap<QString, QByteArray> mAudioByteArrays;
};

#endif // ROSBAGANNOTATOR_H
