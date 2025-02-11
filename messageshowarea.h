#ifndef MESSAGESHOWAREA_H
#define MESSAGESHOWAREA_H

#include <QScrollArea>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include "model/data.h"
#include "debug.h"


class MessageShowArea : public QScrollArea
{
    Q_OBJECT
public:
    MessageShowArea();

    // 尾插
    void addMessage(bool isLeft, const model::Message& message, const QString& path = "");
    // 头插
    void addFrontMessage(bool isLeft, const model::Message& message);
    // 清空消息
    void clear();
    // 滚动到末尾
    void scrollToEnd();

private:
    QWidget* container;
};

///////////////////////////////////////
/// 表示一个消息元素
/// 同时支持 文本消息，图片消息，文件消息，语音消息
///////////////////////////////////////
class MessageItem : public QWidget
{
    Q_OBJECT
public:
    // isLeft 表示是否是 左侧消息
    MessageItem(bool isLeft);

    // 通过 工厂方法 创建 MessageItem 实例
    static MessageItem* makeMessageItem(bool isLeft, const model::Message& message, const QString& path = "");

    // 添加工厂函数
    static QWidget* makeTextMessageItem(bool isLeft, const QString& text);
    static QWidget* makeImageMessageItem(bool isLeft, const QString& fileId, const QByteArray& content);
    static QWidget* makeFileMessageItem(bool isLeft, const QString &fileId, const QString &fileName, const QString& filePath);
    static QWidget* makeSpeechMessageItem(bool isLeft, const model::Message& message);
private:
    bool isLeft;
};

///////////////////////////////////////
/// 创建类表示 “文本消息” 正文部分
///////////////////////////////////////
class MessageContentLabel : public QWidget
{
    Q_OBJECT
public:
    MessageContentLabel(const QString& text, bool isLeft);

    void paintEvent(QPaintEvent* event) override;
private:
    QLabel* label;
    bool isLeft;  
};

///////////////////////////////////////
/// 创建类表示 “图片消息” 正文部分
///////////////////////////////////////
class MessageImageLabel : public QWidget
{
    Q_OBJECT
public:
    MessageImageLabel(const QString& fileId, const QByteArray& content, bool isLeft);
    void updateUI(const QString& fileId, const QByteArray& content);
    void paintEvent(QPaintEvent* event) override;

private:
    QPushButton* imageBtn;
    QString fileId; // 该图片在服务器对应的文件 id
    QByteArray content; // 图片的二进制数据
    bool isLeft;
};

///////////////////////////////////////
/// 创建类表示 “文件消息” 正文部分
///////////////////////////////////////
class MessageFileLabel : public QWidget
{
    Q_OBJECT
public:
    MessageFileLabel(const QString& fileId, const QString& fileName, const QString& filePath, bool isLeft);

    void mousePressEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent* event) override;

    void saveFile(const QString& fileId, const QByteArray& content);

private:
    QLabel* label;
    bool isLeft;
    QString fileId;
    QString fileName;
    QString filePath;
    //QByteArray content;
};

class MessageSpeechLabel : public QWidget
{
    Q_OBJECT
public:
    MessageSpeechLabel(bool isLeft, const model::Message& message);

    void mousePressEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent* event) override;
    void contextMenuEvent(QContextMenuEvent * event) override;

    void speechConvertTextDone(const QString& fileId, const QString& text);

    void playDone();
    void playStop();
    void updateUI(const QString& fileId, const QByteArray& fileContent);

private:
    bool isLeft;
    QString fileId;
    QLabel* label;
    QByteArray content;
};

#endif // MESSAGESHOWAREA_H
