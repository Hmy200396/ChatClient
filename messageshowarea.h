#ifndef MESSAGESHOWAREA_H
#define MESSAGESHOWAREA_H

#include <QScrollArea>
#include <QWidget>
#include <QLabel>
#include "model/data.h"
#include "debug.h"

class MessageShowArea : public QScrollArea
{
    Q_OBJECT
public:
    MessageShowArea();

    // 尾插
    void addMessage(bool isLeft, const model::Message& message);
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
    static MessageItem* makeMessageItem(bool isLeft, const model::Message& message);

    // 添加工厂函数
    static QWidget* makeTextMessageItem(bool isLeft, const QString& text);
    static QWidget* makeImageMessageItem();
    static QWidget* makeFileMessageItem();
    static QWidget* makeSpeechMessageItem();
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

#endif // MESSAGESHOWAREA_H
