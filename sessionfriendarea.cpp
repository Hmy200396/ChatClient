#include "sessionfriendarea.h"
#include "debug.h"
#include "model/data.h"
#include "model/datacenter.h"
#include "mainwidget.h"
#include <QScrollBar>
#include <QVBoxLayout>
#include <QPushButton>
#include <QStyleOption>
#include <QPainter>
SessionFriendArea::SessionFriendArea(QWidget *parent)
    : QScrollArea{parent}
{
    // 1. 设置必要属性
    // 设置这个属性，才能开启滚动效果
    this->setWidgetResizable(true);
    // 设置滚动条相关样式
    //this->verticalScrollBar()->setStyleSheet("QScrollBar:vertical { width: 10px; background-color: rgb(46, 46, 46) }");
    this->verticalScrollBar()->setStyleSheet(
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
    this->horizontalScrollBar()->setStyleSheet("QScrollBar:horizontal { height: 0px; }");
    this->setStyleSheet("QScrollArea { border: none }");

    // 2. 把 widget 创建出来
    container = new QWidget();
    container->setFixedWidth(230);
    this->setWidget(container);

    // 3. 给这个 widget 指定布局管理器，以便后续添加元素进去
    QVBoxLayout* layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->setAlignment(Qt::AlignTop);
    container->setLayout(layout);

#if TEST_UI
    for(int i = 0; i < 30; ++i)
    {
        addItem(ApplyItemType, QString::number(i), QIcon(":/resource/image/defaultAvatar.jpg"), "琅琅", "我是琅琅"+QString::number(i+1)+"号");
    }
#endif
}

void SessionFriendArea::clear()
{
    QLayout* layout = container->layout();
    for(int i = layout->count() - 1; i >= 0; --i)
    {
        // takeAt 移除对应下标元素
        QLayoutItem* item = layout->takeAt(i);
        // 还要对对象进行释放
        if(item && item->widget())
            delete item->widget();
    }
}

void SessionFriendArea::addItem(ItemType itemType, const QString& id, const QIcon &avatar, const QString &name, const QString &text)
{
    SessionFriendItem* item = nullptr;
    if (itemType == SessionItemType)
    {
        item = new SessionItem(this, id, avatar, name, text);
    }
    else if (itemType == FreiendItemType)
    {
        item = new FriendItem(this, id, avatar, name, text);
    }
    else if (itemType == ApplyItemType)
    {
        item = new ApplyItem(this, id, avatar, name);
    }
    else
    {
        LOG() << "错误的 ItemType！itemType = " << itemType;
        return;
    }
    if(item)
        container->layout()->addWidget(item);
}

void SessionFriendArea::clickItem(int index)
{
    if(index < 0 || index >= container->layout()->count())
    {
        LOG() << "点击元素下标超出范围！ index = " << index;
        return;
    }
    QLayoutItem* layoutItem = container->layout()->itemAt(index);
    if(layoutItem == nullptr || layoutItem->widget() == nullptr)
    {
        LOG() << "指定元素不存在！ index = " << index;
        return;
    }
    SessionFriendItem* item = dynamic_cast<SessionFriendItem*>(layoutItem->widget());
    item->select();
}



SessionFriendItem::SessionFriendItem(QWidget *owner, const QIcon &avatar, const QString &name, const QString &text)
    :owner(owner)
{
    this->setFixedHeight(70);
    this->setStyleSheet("QWidget {background-color: rgb(220, 220, 220);}");

    QGridLayout* layout = new QGridLayout();
    layout->setContentsMargins(10, 0, 0, 0);
    layout->setHorizontalSpacing(10);
    layout->setVerticalSpacing(0);
    this->setLayout(layout);

    // 创建头像
    QPushButton* avatarBtn = new QPushButton();
    avatarBtn->setFixedSize(50, 50);
    avatarBtn->setIconSize(QSize(50, 50));
    avatarBtn->setIcon(avatar);
    avatarBtn->setStyleSheet("QPushButton { border: none }");
    // 固定尺寸
    avatarBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    // 创建名字
    QLabel* nameLabel = new QLabel();
    nameLabel->setText(name);
    nameLabel->setStyleSheet("QLabel { font-size: 18px; font-weight: 600; font-family: 'SimSun'; }");
    nameLabel->setFixedHeight(35);
    nameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    // 消息预览
    messageLabel = new QLabel();
    messageLabel->setText(text);
    //messageLabel->setStyleSheet("QLabel { font-size: 12px; font-weight: 550; font-family: 'SimSun'; }");
    messageLabel->setFixedHeight(35);
    messageLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    layout->addWidget(avatarBtn, 0, 0, 2, 2);
    layout->addWidget(nameLabel, 0, 2, 1, 8, Qt::AlignTop);
    layout->addWidget(messageLabel, 1, 2, 1, 8);
}

void SessionFriendItem::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

// 鼠标点击时会触发这个函数
void SessionFriendItem::mousePressEvent(QMouseEvent *event)
{
    select();
}

// 鼠标悬停时会触发这个函数
void SessionFriendItem::enterEvent(QEnterEvent *event)
{
    if(!selected)
        this->setStyleSheet("QWidget {background-color: rgb(210, 210, 210);}");
}

// 鼠标离开时会触发这个函数
void SessionFriendItem::leaveEvent(QEvent *event)
{
    // 还原背景色
    if(!selected)
        this->setStyleSheet("QWidget {background-color: rgb(220, 220, 220);}");
}

void SessionFriendItem::select()
{
    // 遍历其他元素
    const QObjectList children = this->parentWidget()->children();
    for(QObject* child : children)
    {
        if(!child->isWidgetType())
            continue;
        SessionFriendItem* item = dynamic_cast<SessionFriendItem*>(child);
        if(item->selected)
        {
            // 置为默认色
            item->selected = false;
            item->setStyleSheet("QWidget {background-color: rgb(220, 220, 220);}");
        }
    }

    // 点击时，修改背景色
    this->setStyleSheet("QWidget {background-color: rgb(200, 200, 200);}");
    this->selected = true;

    active();
}

void SessionFriendItem::active()
{
    // 父类的 active
    // 不需要实现逻辑，逻辑在子类重写
}

SessionItem::SessionItem(QWidget *owner, const QString &chatSessionId, const QIcon &avatar, const QString &name, const QString &lastMessage)
    :SessionFriendItem(owner, avatar, name, lastMessage)
    ,chatSessionId(chatSessionId)
    ,text(lastMessage)
{
    // 处理更新最后一个消息的信号
    model::DataCenter* dataCenter = model::DataCenter::getInstance();
    connect(dataCenter, &model::DataCenter::updateLastMessage, this, &SessionItem::updateLastMessage);

    // 需要显示出未读消息的数目
    int unread = dataCenter->getUnread(chatSessionId);
    if(unread > 0)
    {
        this->messageLabel->setText("[未读"+QString::number(unread)+"条]"+text);
    }
}

void SessionItem::active()
{
    // TODO 点击之后，要加载会话的历史消息列表
    LOG() << "点击 SessionItem 触发的逻辑！ chatSessionId = " << chatSessionId;

    MainWidget* mainWidget = MainWidget::getInstance();
    mainWidget->loadRecentMessage(chatSessionId);

    // 清空未读消息的数目，并且更新显示
    model::DataCenter* dataCenter = model::DataCenter::getInstance();
    dataCenter->clearUnread(chatSessionId);

    // 更新界面的显示
    this->messageLabel->setText(text);
}

void SessionItem::updateLastMessage(const QString &chatSessionId)
{
    model::DataCenter* dataCenter = model::DataCenter::getInstance();
    // 1. 判定 chatSessionId 是否匹配
    if(this->chatSessionId != chatSessionId)
        return;

    // chatSessionId 匹配，更新最后一条消息
    // 2. 获取到最后一条消息
    QList<model::Message>* messageList = dataCenter->getRecentMessageList(chatSessionId);
    if(messageList == nullptr || messageList->size() == 0)
        return;
    const model::Message& lastMessage = messageList->back();

    // 3. 明确显示的文本内容
    if(lastMessage.messageType == model::TEXT_TYPE)
        text = lastMessage.content;
    else if(lastMessage.messageType == model::IMAGE_TYPE)
        text = "[图片]";
    else if(lastMessage.messageType == model::FILE_TYPE)
        text = "[文件]";
    else if(lastMessage.messageType == model::SPEECH_TYPE)
        text = "[语音 ]";
    else
    {
        LOG()<<"错误的消息类型！messageType = "<<lastMessage.messageType;
        return;
    }

    // 4. 把内容显示到界面上
    // 先判定，当前会话是否是正在选中的会话，如果是，不会更新任何未读消息
    // 如果不是，看未读消息是否 > 0，并作出前缀拼装
    if(chatSessionId == dataCenter->getCurrentChatSessionId())
    {
        this->messageLabel->setText(text);
    }
    else
    {
        int unread = dataCenter->getUnread(chatSessionId);
        if(unread > 0)
        {
            this->messageLabel->setText("[未读"+QString::number(unread)+"条]"+text);
            dataCenter->saveDataFile();
        }
    }
}

FriendItem::FriendItem(QWidget *owner, const QString &userId, const QIcon &avatar, const QString &name, const QString &description)
    :SessionFriendItem(owner, avatar, name, description)
    ,userId(userId)
{

}

void FriendItem::active()
{
    // TODO 点击之后，要激活对应的会话列表元素
    LOG() << "点击 FriendItem 触发的逻辑！ usernId = " << userId;

    MainWidget* mainWidget = MainWidget::getInstance();
    mainWidget->switchSession(userId);
}

ApplyItem::ApplyItem(QWidget *owner, const QString &userId, const QIcon &avatar, const QString &name)
    :SessionFriendItem(owner, avatar, name, "")
    ,userId(userId)
{
    // 1. 移除父类的 messageLabel
    QGridLayout* layout = dynamic_cast<QGridLayout*>(this->layout());
    layout->removeWidget(messageLabel);
    delete messageLabel;

    // 2. 创建两个按钮
    QPushButton* acceptBtn = new QPushButton("同意");
    QPushButton* rejectBtn = new QPushButton("拒绝");
    acceptBtn->setStyleSheet("QPushButton { border: none }");
    rejectBtn->setStyleSheet("QPushButton { border: none }");
    layout->addWidget(acceptBtn, 1, 2, 1, 1, Qt::AlignCenter);
    layout->addWidget(rejectBtn, 1, 3, 1, 1, Qt::AlignCenter);
}

void ApplyItem::active()
{
    // TODO 什么也不做
    LOG() << "点击 ApplyItem 触发的逻辑！ usernId = " << userId;
}
