#include "messageeditarea.h"
#include "historymessagewidget.h"
#include "model/datacenter.h"
#include "toast.h"
#include "mainwidget.h"
#include "soundrecorder.h"
#include <QVBoxLayout>
#include <QScrollBar>
#include <QFileDialog>
MessageEditArea::MessageEditArea(QWidget *parent)
    : QWidget(parent)
{
    // 1. 设置属性
    this->setFixedHeight(200);
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    // 2. 创建垂直方向的布局管理器
    QVBoxLayout* vlayout = new QVBoxLayout();
    vlayout->setContentsMargins(10, 0, 10, 10);
    vlayout->setSpacing(0);
    this->setLayout(vlayout);

    // 3. 创建水平方向布局管理器
    QHBoxLayout* hlayout = new QHBoxLayout();
    hlayout->setContentsMargins(10, 0, 0, 0);
    hlayout->setSpacing(0);
    hlayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    vlayout->addLayout(hlayout);


    // 4. 创建四个按钮，并添加到水平布局
    QString btnStyle = "QPushButton { background-color: rgb(245, 245, 245); border: none; }\
                        QPushButton:pressed { background-color: rgb(255, 255, 255); }";
    QSize btnSize(35, 35);
    QSize iconSize(25, 25);

    sendImageBtn = new QPushButton();
    sendImageBtn->setFixedSize(btnSize);
    sendImageBtn->setIconSize(iconSize);
    sendImageBtn->setIcon(QIcon(":/resource/image/image.png"));
    sendImageBtn->setStyleSheet(btnStyle);
    hlayout->addWidget(sendImageBtn);

    sendFileBtn = new QPushButton();
    sendFileBtn->setFixedSize(btnSize);
    sendFileBtn->setIconSize(iconSize);
    sendFileBtn->setIcon(QIcon(":/resource/image/file.png"));
    sendFileBtn->setStyleSheet(btnStyle);
    hlayout->addWidget(sendFileBtn);

    sendSpeechBtn = new QPushButton();
    sendSpeechBtn->setFixedSize(btnSize);
    sendSpeechBtn->setIconSize(iconSize);
    sendSpeechBtn->setIcon(QIcon(":/resource/image/sound.png"));
    sendSpeechBtn->setStyleSheet(btnStyle);
    hlayout->addWidget(sendSpeechBtn);

    showHistoryBtn = new QPushButton();
    showHistoryBtn->setFixedSize(btnSize);
    showHistoryBtn->setIconSize(iconSize);
    showHistoryBtn->setIcon(QIcon(":/resource/image/history.png"));
    showHistoryBtn->setStyleSheet(btnStyle);
    hlayout->addWidget(showHistoryBtn);

    // 5. 添加多行编辑框
    textEdit = new QPlainTextEdit();
    textEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    textEdit->setStyleSheet("QPlainTextEdit { border: none; color: black; background-color: transparent; font-size: 14px; padding: 10px; }");
    textEdit->verticalScrollBar()->setStyleSheet(
        "QScrollBar:vertical { "
        "width: 10px; "
        "background-color: rgb(220, 220, 220); "
        "margin: 0px 0px 0px 0px; "
        "}"
        "QScrollBar::handle:vertical { "
        "background-color: rgb(190, 190, 190); "
        "border-radius: 5px; "
        "min-height: 20px; "
        "}"
        "QScrollBar::sub-line:vertical, QScrollBar::add-line:vertical { "
        "height: 0px; "
        "width: 0px; "
        "}"
        "QScrollBar::up-arrow:vertical, QScrollBar::down-arrow:vertical { "
        "width: 0px; "
        "height: 0px; "
        "}"
        "QScrollBar::sub-page:vertical { "
        "background-color: transparent; "
        "}"
        "QScrollBar::add-page:vertical { "
        "background-color: transparent; "
        "position: absolute; "
        "top: 0px; "
        "}"
        );
    vlayout->addWidget(textEdit);

    // 5. 添加提示 “录制中” 这样的 QLabel
    tipLabel = new QLabel();
    tipLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    tipLabel->setText("录音中...");
    tipLabel->setAlignment(Qt::AlignCenter);
    tipLabel->setFont(QFont("微软雅黑", 24, 600));
    tipLabel->setStyleSheet("QLabel { color: black; }");
    tipLabel->hide();
    vlayout->addWidget(tipLabel);

    // 7. 添加发送按钮
    sendTextBtn = new QPushButton();
    sendTextBtn->setText("发送");
    sendTextBtn->setFixedSize(120, 40);
    sendTextBtn->setStyleSheet("QPushButton { font-size: 16px; color: rgb(7, 193, 96); border: none; background-color: rgb(233, 233, 233); border-radius: 10px;}"
                               "QPushButton:hover { background-color: rgb(210, 210, 210); }"
                               "QPushButton:pressed { background-color: rgb(200, 200, 200); }");
    vlayout->addWidget(sendTextBtn, 0, Qt::AlignRight | Qt::AlignVCenter);

    // 8. 初始化信号槽
    initSignalSlot();
}

void MessageEditArea::initSignalSlot()
{
    model::DataCenter* dataCenter = model::DataCenter::getInstance();
    // 1. 关联 “显示历史信息” 信号槽
    connect(showHistoryBtn, &QPushButton::clicked, this, [=](){
        if(dataCenter->getCurrentChatSessionId().isEmpty())
            return;
        HistoryMessageWidget* historyMessageWidget = new HistoryMessageWidget(this);
        historyMessageWidget->exec();
    });

    // 2. 关联 “发送文本信息” 信号槽
    connect(sendTextBtn, &QPushButton::clicked, this, &MessageEditArea::sendTextMessage);
    connect(dataCenter, &model::DataCenter::sendMessageDone, this, &MessageEditArea::addSelfMessage);

    // 3. 关联 “收到信息” 信号槽
    connect(dataCenter, &model::DataCenter::receiveMessageDone, this, &MessageEditArea::addOtherMessage);

    // 4. 关联 “发送图片” 信号槽
    connect(sendImageBtn, &QPushButton::clicked, this, &MessageEditArea::clickSendImageBtn);

    // 5. 关联 “发送文件” 信号槽
    connect(sendFileBtn, &QPushButton::clicked, this, &MessageEditArea::clickSendFileBtn);

    // 6. 关联 “发送语音” 信号槽
    connect(sendSpeechBtn, &QPushButton::pressed, this, &MessageEditArea::soundRecordPressed);
    connect(sendSpeechBtn, &QPushButton::released, this, &MessageEditArea::soundRecordReleased);
    SoundRecorder* soundRecorder = SoundRecorder::getInstance();
    connect(soundRecorder, &SoundRecorder::soundRecordDone, this, &MessageEditArea::sendSpeech);
}

void MessageEditArea::sendTextMessage()
{
    // 1. 先确认是否有会话选中
    model::DataCenter* dataCenter = model::DataCenter::getInstance();
    if(dataCenter->getCurrentChatSessionId().isEmpty())
    {
        LOG() << "当前未选择任何会话，不会发送消息";
        Toast::showMessage("当前未选择任何会话，不会发送消息!");
        return;
    }

    // 2. 获取到输入框的内容，看输入框里是否有内容
    const QString& content = textEdit->toPlainText().trimmed();
    if(content.isEmpty())
    {
        LOG()<<"输入框为空";
        return;
    }

    // 3. 清空输入框已有内容
    textEdit->setPlainText("");

    // 4. 通过网络发送数据给服务器
    dataCenter->sendTextMessageAsync(dataCenter->getCurrentChatSessionId(), content);
}

void MessageEditArea::addSelfMessage(model::MessageType messageType, const QByteArray &content, const QString &extraInfo, const QString& path)
{
    model::DataCenter* dataCenter = model::DataCenter::getInstance();
    const QString& currentChatSessionId = dataCenter->getCurrentChatSessionId();
    // 1. 构造消息对象，更新最近消息列表
    model::Message message = model::Message::makeMessage(messageType, currentChatSessionId, *dataCenter->getMyself(), content, extraInfo);
    dataCenter->addMessage(message);

    // 2. 新增消息到消息展示区
    MainWidget* mainWidget = MainWidget::getInstance();
    MessageShowArea* messageShowArea = mainWidget->getMessageShowArea();
    messageShowArea->addMessage(false, message, path);

    // 3. 滚动条滚动到末尾
    messageShowArea->scrollToEnd();

    // 4. 发送信号，通知会话列表，更新最后一条消息
    emit dataCenter->updateLastMessage(currentChatSessionId);
}

void MessageEditArea::addOtherMessage(const model::Message &message)
{
    // 1. 通过主界面，拿到消息展示区
    MainWidget* mainWidget = MainWidget::getInstance();
    MessageShowArea* messageShowArea = mainWidget->getMessageShowArea();

    // 2. 新增消息到消息展示区
    messageShowArea->addMessage(true, message);

    // 3. 滚动条滚动到末尾
    messageShowArea->scrollToEnd();

    // 4. 提示一个收到消息
    Toast::showMessage("收到"+message.sender.nickname+"的新消息！");
}

void MessageEditArea::clickSendImageBtn()
{
    model::DataCenter* dataCenter = model::DataCenter::getInstance();
    // 1. 先判定当前是否有选中的会话
    if(dataCenter->getCurrentChatSessionId().isEmpty())
        return;

    // 2. 弹出文件对话框
    QString filter = "Image Files (*.png *.jpg *.jpeg)";
    QList<QString> imagePath = QFileDialog::getOpenFileNames(this, "选择图片", QDir::homePath(), filter);
    if(imagePath.isEmpty())
        return;

    // 3. 读取图片的内容
    QList<QByteArray> imageContent;
    for(auto& path : imagePath)
    {
        imageContent.push_back(model::loadFileToByteArray(path));
    }

    // 4. 发送请求
    for(auto& content : imageContent)
    {
        dataCenter->sendImageMessageAsync(dataCenter->getCurrentChatSessionId(), content);
    }
}

void MessageEditArea::clickSendFileBtn()
{
    model::DataCenter* dataCenter = model::DataCenter::getInstance();
    // 1. 先判定当前是否有选中的会话
    if(dataCenter->getCurrentChatSessionId().isEmpty())
        return;

    // 2. 弹出文件对话框
    QString filter = "*";
    QList<QString> filePath = QFileDialog::getOpenFileNames(this, "选择文件", QDir::homePath(), filter);
    if(filePath.isEmpty())
        return;

    // 3. 读取文件的内容
    QList<QByteArray> fileContent;
    QList<QString> fileNameList;
    for(auto& path : filePath)
    {
        if(model::isFileLargerThanMB(path, 200))
        {
            Toast::showMessage("不支持传输大小超过200MB的文件");
            continue;
        }
        // fileContent.push_back(model::loadFileToByteArray(path));
        // fileNameList.push_back(model::getFileName(path));
        // 发送请求
        dataCenter->sendFileMessageAsync(dataCenter->getCurrentChatSessionId(), path, model::getFileName(path), model::loadFileToByteArray(path));
    }

    // // 4. 发送请求
    // for(int i = 0; i < fileContent.size() && i < fileNameList.size(); ++i)
    // {
    //     dataCenter->sendFileMessageAsync(dataCenter->getCurrentChatSessionId(), fileNameList[i], fileContent[i]);
    // }
}

void MessageEditArea::soundRecordPressed()
{
    // 判定当前是否选择会话
    model::DataCenter* dataCenter = model::DataCenter::getInstance();
    if(dataCenter->getCurrentChatSessionId().isEmpty())
        return;

    sendSpeechBtn->setIcon(QIcon(":/resource/image/sound_active.png"));

    // 开始录音
    SoundRecorder* soundRecorder = SoundRecorder::getInstance();
    soundRecorder->startRecord();

    tipLabel->show();
    textEdit->hide();
}

void MessageEditArea::soundRecordReleased()
{
    // 判定当前是否选择会话
    model::DataCenter* dataCenter = model::DataCenter::getInstance();
    if(dataCenter->getCurrentChatSessionId().isEmpty())
        return;

    sendSpeechBtn->setIcon(QIcon(":/resource/image/sound.png"));

    // 停止录音
    SoundRecorder* soundRecorder = SoundRecorder::getInstance();
    soundRecorder->stopRecord();

    tipLabel->hide();
    textEdit->show();
}

void MessageEditArea::sendSpeech(const QString &path)
{
    model::DataCenter* dataCenter = model::DataCenter::getInstance();
    // 1. 读取到语音文件的内容
    QByteArray content = model::loadFileToByteArray(path);
    if(content.isEmpty())
        return;
    dataCenter->sendSpeechMessageAsync(dataCenter->getCurrentChatSessionId(), content);
}
