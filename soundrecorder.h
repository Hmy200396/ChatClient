#ifndef SOUNDRECORDER_H
#define SOUNDRECORDER_H

#include <QObject>
#include <QStandardPaths>
#include <QFile>
#include <QDir>
#include <QAudioSource>
#include <QAudioSink>
#include <QMediaDevices>
///////////////////////////////////////////
/// 音频录制播放
///////////////////////////////////////////
class SoundRecorder : public QObject
{
    Q_OBJECT
public:
    const QString RECORD_PATH = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/sound/tmpRecord.pcm";
    const QString PLAY_PATH = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/sound/tmpPlay.pcm";

public:
    ~SoundRecorder();
    static SoundRecorder* getInstance();

    ///////////////////////////////////////////
    /// 录制语言
    ///////////////////////////////////////////
    // 开始录制
    void startRecord();
    // 停止录制
    void stopRecord();

private:
    explicit SoundRecorder(QObject *parent = nullptr);

private:
    static SoundRecorder* instance;
    QFile soundFile;
    QAudioSource* audioSource;

signals:
    // 录制完毕后发送这个信号
    void soundRecordDone(const QString& path);


    ///////////////////////////////////////////
    /// 播放语言
    ///////////////////////////////////////////
public:
    // 开始播放
    void startPlay(const QByteArray& content);
    // 停止播放
    void stopPlay();

private:
    QAudioSink *audioSink;
    QMediaDevices *outputDevices;
    QAudioDevice outputDevice;
    QFile inputFile;

signals:
    // 播放完毕发送这个信号
    void soundPlayDone();
};

#endif // SOUNDRECORDER_H
