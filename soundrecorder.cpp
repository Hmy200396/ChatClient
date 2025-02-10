#include "soundrecorder.h"
#include "model/data.h"
#include "toast.h"
#include <QMediaDevices>

SoundRecorder* SoundRecorder::instance = nullptr;
SoundRecorder::~SoundRecorder()
{

}

SoundRecorder *SoundRecorder::getInstance()
{
    if (instance == nullptr) {
        instance = new SoundRecorder();
    }
    return instance;
}

SoundRecorder::SoundRecorder(QObject *parent)
    : QObject{parent}
{
    // 1. 创建目录
    QDir soundRootPath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    soundRootPath.mkdir("sound");

    // 2. 初始化录制模块
    soundFile.setFileName(RECORD_PATH);

    // 设置录制文件格式
    QAudioFormat inputFormat;
    inputFormat.setSampleRate(16000); // 设置采样率
    inputFormat.setChannelCount(1); // 单声道
    inputFormat.setSampleFormat(QAudioFormat::Int16); // 16bits 小端序

    QAudioDevice info = QMediaDevices::defaultAudioInput();
    if (!info.isFormatSupported(inputFormat))
    {
        LOG() << "录制设备, 格式不⽀持!";
        return;
    }
    audioSource = new QAudioSource(inputFormat, this);
    connect(audioSource, &QAudioSource::stateChanged, this, [=](QtAudio::State state) {
        if (state == QtAudio::StoppedState)
        {
            // 录制完毕
            if (audioSource->error() != QAudio::NoError)
            {
                LOG() << audioSource->error();
            }
        }
    });

    // 3. 初始化播放模块
    outputDevices = new QMediaDevices(this);
    outputDevice = outputDevices->defaultAudioOutput();
    QAudioFormat outputFormat;
    outputFormat.setSampleRate(16000);
    outputFormat.setChannelCount(1);
    outputFormat.setSampleFormat(QAudioFormat::Int16);
    if (!outputDevice.isFormatSupported(outputFormat))
    {
        LOG() << "播放设备, 格式不⽀持";
        return;
    }
    audioSink = new QAudioSink(outputDevice, outputFormat);

    connect(audioSink, &QAudioSink::stateChanged, this, [=](QtAudio::State state) {
        if (state == QtAudio::IdleState)
        {
            LOG() << "IdleState";
            this->stopPlay();
            emit this->soundPlayDone();
        }
        else if (state == QAudio::ActiveState)
        {
            LOG() << "ActiveState";
        }
        else if (state == QAudio::StoppedState)
        {
            LOG() << "StoppedState";
            if (audioSink->error() != QtAudio::NoError)
            {
                LOG() << audioSink->error();
            }
        }
    });
}

///////////////////////////////////////////
/// 录制语言
///////////////////////////////////////////
void SoundRecorder::startRecord() {
    soundFile.open( QIODevice::WriteOnly | QIODevice::Truncate );
    audioSource->start(&soundFile);
}

void SoundRecorder::stopRecord() {
    audioSource->stop();
    soundFile.close();
    emit this->soundRecordDone(RECORD_PATH);
}

///////////////////////////////////////////
/// 播放语言
///////////////////////////////////////////
void SoundRecorder::startPlay(const QByteArray& content)
{
    if (content.isEmpty())
    {
        Toast::showMessage("数据加载中, 请稍后播放");
        return;
    }
    // 1. 把数据写入到临时文件
    model::writeByteArrayToFile(PLAY_PATH, content);
    // 2. 播放语音
    inputFile.setFileName(PLAY_PATH);
    inputFile.open(QIODevice::ReadOnly);
    audioSink->start(&inputFile);
    // LOG() << "startPlay! n=" << n << " error: " << device->errorString();
}
void SoundRecorder::stopPlay()
{
    audioSink->stop();
    inputFile.close();
}
