#pragma once

#include <QString>
#include <QIcon>
#include <QUuid>
#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include "base.qpb.h"
#include "gateway.qpb.h"
#include "user.qpb.h"
#include "friend.qpb.h"
#include "file.qpb.h"
#include "notify.qpb.h"
#include "speech_recognition.qpb.h"
#include "message_storage.qpb.h"
#include "message_transmit.qpb.h"

namespace model
{
/////////////////////////////////////////
/// 工具函数
////////////////////////////////////////
static inline QString getFileName(const QString& path)
{
    QFileInfo fileInfo(path);
    return fileInfo.fileName();
}
// 封装一个 “宏” 作为打印日志的方式
#define TAG QString("[%1:%2]").arg(QString(model::getFileName(__FILE__)), QString::number(__LINE__))
#define LOG() qDebug().noquote() << TAG

// 函数的定义如果写在.h中，必须加 static 或者 inline
// 格式化时间
static inline QString formatTime(int64_t timestamp)
{
    QDateTime dataTime = QDateTime::fromSecsSinceEpoch(timestamp);
    return dataTime.toString("MM-dd HH:mm::ss");
}
// 获得 秒级 时间戳
static inline int64_t getTime()
{
    return QDateTime::currentSecsSinceEpoch();
}
// 根据QByteArray, 转成 QIcon
static inline QIcon makeIcon(const QByteArray& byteArray)
{
    QPixmap pixmap;
    pixmap.loadFromData(byteArray);
    QIcon icon(pixmap);
    return icon;
}
// 读写文件操作
// 从指定文件中，读取所有的二进制内容，得到一个 QByteArray
static inline QByteArray loadFileToByteArray(const QString& path)
{
    QFile file(path);
    bool ok = file.open(QFile::ReadOnly);
    if(!ok)
    {
        LOG()<< "文件打开失败";
        return QByteArray();
    }
    QByteArray content = file.readAll();
    file.close();
    return content;
}
// 把 QByteArray 中的内容，写入到某个指定文件里
static inline void writeByteArrayToFile(const QString& path, const QByteArray& content)
{
    QFile file(path);
    bool ok = file.open(QFile::WriteOnly);
    if(!ok)
    {
        LOG()<< "文件打开失败";
        return;
    }
    file.write(content);
    file.flush();
    file.close();
}

/////////////////////////////////////////
/// 用户信息
////////////////////////////////////////
class UserInfo
{
public:
    QString userId = "";          // 用户编号
    QString nickname = "";        // 用户昵称
    QString description = "";     // 用户签名
    QString phone = "";           // 手机号码
    QIcon avatar;                 // 用户头像

    // 从 protobuff 的 UserInfo 对象，转化成当前代码的 UserInfo 对象
    void load(const proto::UserInfo& userInfo)
    {
        this->userId = userInfo.userId();
        this->nickname = userInfo.nickname();
        this->description = userInfo.description();
        this->phone = userInfo.phone();
        if(userInfo.avatar().isEmpty())
        {
            // 使用默认头像
            this->avatar = QIcon(":/resource/image/defaultAvatar.jpg");
        }
        else
        {
            this->avatar = makeIcon(userInfo.avatar());
        }
    }
};

/////////////////////////////////////////
/// 消息类型
////////////////////////////////////////
enum MessageType
{
    TEXT_TYPE,   // 文本消息
    IMAGE_TYPE,  // 图片消息
    FILE_TYPE,   // 文件消息
    SPEECH_TYPE, // 语音消息
};

/////////////////////////////////////////
/// 消息信息
////////////////////////////////////////
class Message
{
public:
    QString messageId = "";                   // 消息编号
    QString chatSessionId = "";               // 消息所属会话的编号
    QString time = "";                        // 消息的时间，通过“格式化”时间的方式来表示，形如 06-07 12:00:00
    MessageType messageType = TEXT_TYPE;      // 消息类型
    UserInfo sender;                          // 发送者的信息
    QByteArray content;                       // 消息的正文内容
    // “获取消息列表”，只是拿到消息的 fileId，等到客户端得到消息列表之后，再根据拿到的fileId，给服务器发送额外的请求，获取文件内容
    // 提高客户端访问服务器的并发程度
    QString fileId = "";                      // 文件的身份标识，当消息类型为 文件，图片，语音 的时候，才有效，当消息类型为 文本，则为 ""
    QString fileName = "";                    // 文件名称，当消息是 文件 消息，才有效，其他消息均为 ""

    // 此处 extraInfo 目前只是在消息类型为文件消息时，作为 文件名 补充
    static Message makeMessage(MessageType messageType, const QString& chatSessionId, const UserInfo& sender,
                               const QByteArray& content, const QString& extraInfo)
    {
        if (messageType == TEXT_TYPE)
            return makeTextMessage(chatSessionId, sender, content);
        else if (messageType == IMAGE_TYPE)
            return makeImageMessage(chatSessionId, sender, content);
        else if (messageType == FILE_TYPE)
            return makeFileMessage(chatSessionId, sender, content, extraInfo);
        else if (messageType == SPEECH_TYPE)
            return makeSpeechMessage(chatSessionId, sender, content);
        else
            return Message();
    }

    bool isEmpty()
    {
        return  messageId.isEmpty() && chatSessionId.isEmpty() && time.isEmpty() && \
                content.isEmpty() && fileId.isEmpty() && fileName.isEmpty();
    }

    void load(const proto::MessageInfo& messageInfo)
    {
        this->messageId = messageInfo.messageId();
        this->chatSessionId = messageInfo.chatSessionId();
        this->time = formatTime(messageInfo.timestamp());
        this->sender.load(messageInfo.sender());

        // 设置消息类型
        auto type = messageInfo.message().messageType();
        if(type == proto::MessageTypeGadget::MessageType::STRING)
        {
            this->messageType = TEXT_TYPE;
            this->content = messageInfo.message().stringMessage().content().toUtf8();
        }
        else if (type == proto::MessageTypeGadget::MessageType::IMAGE)
        {
            this->messageType = IMAGE_TYPE;
            if(!messageInfo.message().imageMessage().imageContent().isEmpty())
            {
                this->content = messageInfo.message().imageMessage().imageContent();
            }
            if(!messageInfo.message().imageMessage().fileId().isEmpty())
            {
                this->fileId = messageInfo.message().imageMessage().fileId();
            }
        }
        else if (type == proto::MessageTypeGadget::MessageType::FILE)
        {
            this->messageType = FILE_TYPE;
            if(!messageInfo.message().fileMessage().fileContents().isEmpty())
            {
                this->content = messageInfo.message().fileMessage().fileContents();
            }
            if(!messageInfo.message().fileMessage().fileId().isEmpty())
            {
                this->fileId = messageInfo.message().fileMessage().fileId();
            }
            this->fileName = messageInfo.message().fileMessage().fileName();
        }
        else if (type == proto::MessageTypeGadget::MessageType::SPEECH)
        {
            this->messageType = SPEECH_TYPE;
            if(!messageInfo.message().speechMessage().fileContents().isEmpty())
            {
                this->content = messageInfo.message().speechMessage().fileContents();
            }
            if(!messageInfo.message().speechMessage().fileId().isEmpty())
            {
                this->fileId = messageInfo.message().speechMessage().fileId();
            }
        }
        else
        {
            // 错误类型，打印日志
            LOG() << "非法的消息类型 type = " << type;
        }

    }

private:
    // 生成唯一 messageId
    static inline QString makeId()
    {
        return "M" + QUuid::createUuid().toString().sliced(25,12);
    }
    static Message makeTextMessage(const QString& chatSessionId, const UserInfo& sender, const QByteArray& content)
    {
        Message message;
        // 确保 messageId 是唯一的
        message.messageId = makeId();
        message.chatSessionId = chatSessionId;
        message.sender = sender;
        message.time = formatTime(getTime());
        message.content = content;
        message.messageType = TEXT_TYPE;
        message.fileId = "";
        message.fileName = "";
        return message;
    }
    static Message makeImageMessage(const QString& chatSessionId, const UserInfo& sender, const QByteArray& content)
    {
        Message message;
        message.messageId = makeId();
        message.chatSessionId = chatSessionId;
        message.sender = sender;
        message.time = formatTime(getTime());
        message.content = content;
        message.messageType = IMAGE_TYPE;
        message.fileId = "";
        message.fileName = "";
        return message;
    }
    static Message makeFileMessage(const QString& chatSessionId, const UserInfo& sender, const QByteArray& content, const QString& fileName)
    {
        Message message;
        message.messageId = makeId();
        message.chatSessionId = chatSessionId;
        message.sender = sender;
        message.time = formatTime(getTime());
        message.content = content;
        message.messageType = FILE_TYPE;
        message.fileId = "";
        message.fileName = fileName;
        return message;
    }
    static Message makeSpeechMessage(const QString& chatSessionId, const UserInfo& sender, const QByteArray& content)
    {
        Message message;
        message.messageId = makeId();
        message.chatSessionId = chatSessionId;
        message.sender = sender;
        message.time = formatTime(getTime());
        message.content = content;
        message.messageType = SPEECH_TYPE;
        message.fileId = "";
        message.fileName = "";
        return message;
    }
};

/////////////////////////////////////////
/// 会话信息
////////////////////////////////////////
class ChatSessionInfo
{
public:
    QString chatSessionId = "";   // 会话编号
    QString chatSessionName = ""; // 会话名字，如果会话是单聊，名字就是对方的昵称；如果是群聊，名字就是群聊的名称
    Message lastMessage;          // 表示最新的消息
    QIcon avatar;                 // 会话头像，如果会话是单聊，头像就是对方的头像；如果是群聊，头像就是群聊的头像
    QString userId = "";          // 如果会话是单聊，此时userId表示对方的用户id，如果会话是群聊，userId设为 "" ，后续通过其他方式拿到完整的用户id列表

    void load(const proto::ChatSessionInfo& chatSessionInfo)
    {
        this->chatSessionId = chatSessionInfo.chatSessionId();
        this->chatSessionName = chatSessionInfo.chatSessionName();
        if(!chatSessionInfo.singleChatFriendId().isEmpty())
        {
            this->userId = chatSessionInfo.singleChatFriendId();
        }
        Message message;
        message.load(chatSessionInfo.prevMessage());
        if(!message.isEmpty())
        {
            this->lastMessage.load(chatSessionInfo.prevMessage());
        }
        if(!chatSessionInfo.avatar().isEmpty())
        {
            // 有头像设置头像
            this->avatar = makeIcon(chatSessionInfo.avatar());
        }
        else
        {
            // 没有头像设置默认头像
            if(!userId.isEmpty())
            {
                // 单聊
                this->avatar = QIcon(":/resource/image/defaultAvatar.jpg");
            }
            else
            {
                // 群聊
                this->avatar = QIcon(":/resource/image/groupAvatar_active.png");
            }
        }
    }
};
} // end model
