#include "messageshowarea.h"
#include "userinfowidget.h"
#include "mainwidget.h"
#include "model/datacenter.h"
#include <QSizePolicy>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QFontMetrics>
#include <QPainter>
#include <QPainterPath>
#include <QDebug>
#include <QTimer>
///////////////////////////////////////
/// 表示一个消息展示区
///////////////////////////////////////
MessageShowArea::MessageShowArea()
{
    // 1. 初始化基本属性
    this->setWidgetResizable(true);
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
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // 2. 创建 container 这样的 widget，作为包含内部元素的容器
    container = new QWidget();
    this->setWidget(container);

    // 3. 给 container 添加布局管理器
    QVBoxLayout* layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    container->setLayout(layout);

    // 添加构造测试数据
#if TEST_UI
    for(int i = 0; i < 30; ++i)
    {
        model::UserInfo userInfo;
        userInfo.nickname = "丢丢丢" + QString::number(i);
        userInfo.userId = "2000400616";
        userInfo.phone = "17302083024";
        userInfo.description = "刻晴，四叶激推";
        userInfo.avatar = QIcon(":/resource/image/defaultAvatar.jpg");
        model::Message message = model::Message::makeMessage(model::TEXT_TYPE, "", userInfo, ("这是一条测试消息这是一条测试消息这是一条测试消息这是一条测试消息这是一条测试消息"+QString::number(i)).toUtf8(), "");
        this->addMessage(i % 2 == 0 ? true : false, message);
    }
#endif
}

// 尾插
void MessageShowArea::addMessage(bool isLeft, const model::Message &message)
{
    // 构造 MessageItem，添加到布局管理器
    MessageItem* messageItem = MessageItem::makeMessageItem(isLeft, message);
    container->layout()->addWidget(messageItem);
}

// 头插
void MessageShowArea::addFrontMessage(bool isLeft, const model::Message &message)
{
    MessageItem* messageItem = MessageItem::makeMessageItem(isLeft, message);
    QVBoxLayout* layout = dynamic_cast<QVBoxLayout*>(container->layout());
    layout->insertWidget(0, messageItem);
}

// 清空消息
void MessageShowArea::clear()
{
    QVBoxLayout* layout = dynamic_cast<QVBoxLayout*>(container->layout());
    for(int i = layout->count() - 1; i >= 0; --i)
    {
        QLayoutItem* item = layout->takeAt(i);
        // if (item)
        // {
        //     if (item->widget())
        //     {
        //         delete item->widget();
        //     }
        //     delete item;
        // }
        if(item != nullptr && item->widget() != nullptr)
            delete item->widget();
    }
}

void MessageShowArea::scrollToEnd()
{
    // 拿到垂直滚动条，获取到滚动条的最大值，根据最大值设置滚动条位置
    // 为了解决界面绘制的延时问题，加个延时
    QTimer* timer = new QTimer();
    connect(timer, &QTimer::timeout, this, [=](){
        int maxValue = this->verticalScrollBar()->maximum();
        this->verticalScrollBar()->setValue(maxValue);
        timer->stop();
        timer->deleteLater();
    });
    timer->start(500);
}

///////////////////////////////////////
/// 表示一个消息元素
///////////////////////////////////////
MessageItem::MessageItem(bool isLeft)
    :isLeft(isLeft)
{

}

MessageItem *MessageItem::makeMessageItem(bool isLeft, const model::Message &message)
{
    // 1. 创建对象和布局管理器
    MessageItem* messageItem = new MessageItem(isLeft);
    QGridLayout* layout = new QGridLayout();
    layout->setContentsMargins(30, 10, 30, 0);
    layout->setSpacing(10);
    messageItem->setMinimumHeight(100);
    messageItem->setLayout(layout);

    // 2. 创建头像
    QPushButton* avatarBtn = new QPushButton();
    avatarBtn->setFixedSize(40, 40);
    avatarBtn->setIconSize(QSize(40, 40));
    avatarBtn->setIcon(message.sender.avatar);
    avatarBtn->setStyleSheet("QPushButton { border: none; }");
    if(isLeft)
    {
        layout->addWidget(avatarBtn, 0, 0, 2, 1, Qt::AlignTop | Qt::AlignLeft);
    }
    else
    {
        layout->addWidget(avatarBtn, 0, 1, 2, 1, Qt::AlignTop | Qt::AlignRight);
    }

    // 3. 创建名字和时间
    QLabel* nameLabel = new QLabel();
    nameLabel->setText(message.sender.nickname + " | " + message.time);
    nameLabel->setAlignment(Qt::AlignBottom);
    nameLabel->setStyleSheet("QLabel { font-size: 12px; color: rgb(18, 18, 18); }");
    if(isLeft)
    {
        layout->addWidget(nameLabel, 0, 1);
    }
    else
    {
        layout->addWidget(nameLabel, 0, 0, Qt::AlignTop | Qt::AlignRight);
    }

    // 4. 创建消息体
    QWidget* contentWidget = nullptr;
    switch (message.messageType)
    {
    case model::TEXT_TYPE:
        contentWidget = makeTextMessageItem(isLeft, message.content);
        break;
    case model::IMAGE_TYPE:
        contentWidget = makeImageMessageItem();
        break;
    case model::FILE_TYPE:
        contentWidget = makeFileMessageItem();
        break;
    case model::SPEECH_TYPE:
        contentWidget = makeSpeechMessageItem();
        break;
    default:
        LOG() << "错误的消息类型! messageType = " << message.messageType;
    }
    if(isLeft)
    {
        layout->addWidget(contentWidget, 1, 1);
    }
    else
    {
        layout->addWidget(contentWidget, 1, 0);
    }

    // 5. 连接信号槽，处理用户点击头像操作
    connect(avatarBtn, &QPushButton::clicked, messageItem, [=](){
        MainWidget* mainWidget = MainWidget::getInstance();
        UserInfoWidget* userInfoWidget = new UserInfoWidget(message.sender, mainWidget);
        userInfoWidget->exec();
    });

    // 6. 当用户修改了昵称的时候，同步修改此处的用户昵称
    if(!isLeft)
    {
        model::DataCenter* dataCenter = model::DataCenter::getInstance();
        connect(dataCenter, &model::DataCenter::changeNicknameDone, messageItem, [=](){
            nameLabel->setText(dataCenter->getMyself()->nickname + " | " + message.time);
        });

        connect(dataCenter, &model::DataCenter::changeAvatarDone, messageItem, [=](){
            model::UserInfo* myself = dataCenter->getMyself();
            avatarBtn->setIcon(myself->avatar);
        });
    }

    return messageItem;
}

QWidget *MessageItem::makeTextMessageItem(bool isLeft, const QString& text)
{
    MessageContentLabel* messageContentLabel = new MessageContentLabel(text, isLeft);
    return messageContentLabel;
}

QWidget *MessageItem::makeImageMessageItem()
{
    return nullptr;
}

QWidget *MessageItem::makeFileMessageItem()
{
    return nullptr;
}

QWidget *MessageItem::makeSpeechMessageItem()
{
    return nullptr;
}

MessageContentLabel::MessageContentLabel(const QString &text, bool isLeft)
    :isLeft(isLeft)
{
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QFont font;
    font.setFamily("微软雅黑");
    font.setPixelSize(16);

    this->label = new QLabel(this);
    this->label->setText(text);
    this->label->setFont(font);
    this->label->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    this->label->setWordWrap(true); // 文本自动换行
    this->label->setStyleSheet("QLabel { padding: 0 10px; line-height: 1.2; color: rgb(35, 35, 35); background-color: transparent}");
}

void MessageContentLabel::paintEvent(QPaintEvent *event)
{
    // 1. 获取父元素的宽度
    QObject* object = this->parent();
    if(!object->isWidgetType())
        return;
    QWidget* parent = dynamic_cast<QWidget*>(object);
    int width = parent->width() * 0.6;

    // 2. 计算当前文本，如果是一行放置需要多宽
    QFontMetrics metrics(this->label->font());
    int totalWidth = metrics.horizontalAdvance(this->label->text());

    // 3. 计算行数
    int rows = (totalWidth / (width - 40)) + 1;
    if(rows == 1)
    {
        // 只有一行 40为左右各20边距
        width = totalWidth + 40;
    }

    // 4. 根据行数，计算得到的高度  20为上下各10边距
    int height = rows * (this->label->font().pixelSize() * 1.2) + 20;

    // 5. 绘制圆角矩形和箭头
    QPainter painter(this);
    QPainterPath path;
    // 设置 抗锯齿
    painter.setRenderHint(QPainter::Antialiasing);
    if(isLeft)
    {
        painter.setPen((QPen(QColor(255, 255 ,255))));
        painter.setBrush((QColor(255, 255 ,255)));

        painter.drawRoundedRect(10, 0, width, height, 10, 10);
        path.moveTo(10, 15);
        path.lineTo(0, 20);
        path.lineTo(10, 25);
        path.closeSubpath();
        painter.drawPath(path);

        this->label->setGeometry(10, 0, width, height);
    }
    else
    {
        painter.setPen((QPen(QColor(137, 217 ,97))));
        painter.setBrush((QColor(137, 217 ,97)));

        int leftPos = this->width() - width - 10;
        int rightPos = this->width() - 10;
        painter.drawRoundedRect(leftPos, 0, width, height, 10, 10);
        path.moveTo(rightPos, 15);
        path.lineTo(rightPos + 10, 20);
        path.lineTo(rightPos, 25);
        path.closeSubpath();
        painter.drawPath(path);

        this->label->setGeometry(leftPos, 0, width, height);
    }

    // 6. 重新设置父元素高度，确保父元素足够高，能够容纳下上述绘制区域
    parent->setFixedHeight(height + 50);
}
