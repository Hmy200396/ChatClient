#ifndef SESSIONFRIENDAREA_H
#define SESSIONFRIENDAREA_H

#include <QWidget>
#include <QScrollArea>
#include <QLabel>

/////////////////////////////////////////
/// 滚动区域中 Item 的类型
/////////////////////////////////////////
enum ItemType
{
    SessionItemType,
    FreiendItemType,
    ApplyItemType,
};

/////////////////////////////////////////
/// 整个滚动区域的实现
/////////////////////////////////////////
class SessionFriendArea : public QScrollArea
{
    Q_OBJECT
public:
    explicit SessionFriendArea(QWidget *parent = nullptr);

    // 清空该区域中所有的 item
    void clear();

    // 添加一个 item 到该区域中，itemType 表示添加哪种 item
    // 如果是SessionItem，id 就是 chatSessionId
    // 如果是FreiendItem，id 就是 userId
    // 如果是ApplyItem，id 就是 userId
    void addItem(ItemType itemType, const QString& id, const QIcon &avatar, const QString &name, const QString &text = "");

    // 选中某个指定的 item
    void clickItem(int index);
private:
    // 后续往 container 内部的 layout 中添加元素，就能够触发 QScrollArea 的滚动效果
    QWidget* container;

signals:
};

/////////////////////////////////////////
/// 滚动区域中 Item 基类的实现
/////////////////////////////////////////
class SessionFriendItem : public QWidget
{
    Q_OBJECT
public:
    // 此处的oner就是对应着 SessionFriendArea 对象的指针
    SessionFriendItem(QWidget* owner, const QIcon& avatar, const QString& name, const QString& text);
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;

    void select();
    // active 函数期望实现 Item 被点击之后的逻辑
    virtual void active();
private:
    QWidget* owner;

    // 标记Item是否为选中状态
    bool selected = false;
protected:
    QLabel* messageLabel;
};

/////////////////////////////////////////
/// 会话 Item 的实现
/////////////////////////////////////////
class SessionItem : public SessionFriendItem
{
    Q_OBJECT
public:
    SessionItem(QWidget* owner, const QString& chatSessionId, const QIcon& avatar, const QString& name, const QString& lastMessage);
    void active() override;

private:
    // 当前会话 id
    QString chatSessionId;

private:
    void updateLastMessage(const QString& chatSessionId);
    // 最后一条消息的文本预览
    QString text;

};

/////////////////////////////////////////
/// 好友 Item 的实现
/////////////////////////////////////////
class FriendItem : public SessionFriendItem
{
    Q_OBJECT
public:
    FriendItem(QWidget* owner, const QString& userId, const QIcon& avatar, const QString& name, const QString& description);
    void active() override;

private:
    // 好友的用户 id
    QString userId;

};

/////////////////////////////////////////
/// 好友申请 Item 的实现
/////////////////////////////////////////
class ApplyItem : public SessionFriendItem
{
    Q_OBJECT
public:
    ApplyItem(QWidget* owner, const QString& userId, const QIcon& avatar, const QString& name);
    void active() override;
private:
    // 申请人的 userid
    QString userId;

};
#endif // SESSIONFRIENDAREA_H
