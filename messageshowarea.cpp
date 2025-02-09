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
#include <QDesktopServices>
#include <QDir>
#include <QMessageBox>
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
void MessageShowArea::addMessage(bool isLeft, const model::Message &message, const QString& path)
{
    // 构造 MessageItem，添加到布局管理器
    MessageItem* messageItem = MessageItem::makeMessageItem(isLeft, message, path);
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
    timer->start(200);

}

///////////////////////////////////////
/// 表示一个消息元素
///////////////////////////////////////
MessageItem::MessageItem(bool isLeft)
    :isLeft(isLeft)
{

}

MessageItem *MessageItem::makeMessageItem(bool isLeft, const model::Message &message, const QString& path)
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
        contentWidget = makeImageMessageItem(isLeft, message.fileId, message.content);
        break;
    case model::FILE_TYPE:
        contentWidget = makeFileMessageItem(isLeft, message.fileId, message.fileName, path);
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

QWidget *MessageItem::makeImageMessageItem(bool isLeft, const QString& fileId, const QByteArray& content)
{
    MessageImageLabel* messageImageLabel = new MessageImageLabel(fileId, content, isLeft);
    return messageImageLabel;
}

QWidget *MessageItem::makeFileMessageItem(bool isLeft, const QString &fileId, const QString &fileName, const QString& filePath)
{
    MessageFileLabel* messageFileLabel = new MessageFileLabel(fileId, fileName, filePath, isLeft);
    return messageFileLabel;
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
    const int maxWidth = parent->width() * 0.6;

    // 2. 计算当前文本，如果是一行放置需要多宽
    QFontMetrics metrics(this->label->font());
    int totalWidth = metrics.horizontalAdvance(this->label->text());

    // // 3. 计算行数
    // int width = maxWidth;
    // int rows = (totalWidth / (maxWidth - 40)) + 1;
    // if(rows == 1)
    // {
    //     // 只有一行 40为左右各20边距
    //     width = totalWidth + 40;
    // }

    // // 4. 根据行数，计算得到的高度  20为上下各10边距
    // int height = rows * (this->label->font().pixelSize() * 1.2) + 20;

    // 计算宽度：短文本不换行，长文本换行
    bool needWrap = totalWidth > (maxWidth - 40);
    int width = needWrap ? (maxWidth) : (totalWidth + 40);

    // 设置 QLabel 的宽度和换行
    this->label->setWordWrap(needWrap);
    this->label->setFixedWidth(width); // 预留内边距
    this->label->adjustSize(); // 计算 QLabel 高度
    int height = this->label->height() + 20;

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

///////////////////////////////////////
/// 创建类表示 “图片消息” 正文部分
///////////////////////////////////////
MessageImageLabel::MessageImageLabel(const QString &fileId, const QByteArray &content, bool isLeft)
    :fileId(fileId),content(content),isLeft(isLeft)
{
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    imageBtn = new QPushButton(this);
    imageBtn->setStyleSheet("QPushButton { border: none; }");

    if(content.isEmpty())
    {
        // 此处这个控件，是针对 “从服务器拿到图片消息” 这种情况
        // 拿着 fileId，去服务器获取图片
        model::DataCenter* dataCenter = model::DataCenter::getInstance();
        connect(dataCenter, &model::DataCenter::getSingleFileDone, this, &MessageImageLabel::updateUI);
        dataCenter->getSingleFileAsync(fileId);
    }
}

void MessageImageLabel::updateUI(const QString &fileId, const QByteArray &content)
{
    if(this->fileId != fileId)
        return;

    // 保存图片内容
    this->content = content;

    // 绘制图片到界面上
    this->update();
}

void MessageImageLabel::paintEvent(QPaintEvent *event)
{
    // 1. 先拿到该元素的父元素，看父元素的宽度是多少
    //    显示的图片宽度的上限 父元素宽度的30%
    QObject* object = this->parent();
    if(!object->isWidgetType())
        return;

    QWidget* parent = dynamic_cast<QWidget*>(object);
    int width = parent->width() * 0.3;

    // 2. 加载二进制数据为图片对象
    QImage image;
    if(content.isEmpty())
    {
        // 此时图片响应数据还没回来
        // 先拿默认图片显示
        QByteArray tmpContent = model::loadFileToByteArray(":/resource/image/loading.png");
        image.loadFromData(tmpContent);
    }
    else
    {
        image.loadFromData(content);
    }

    // 3. 针对图片进行缩放
    int height = 0;
    if(image.width() > width)
    {
        // 图片太宽
        // 等比例缩放
        height = ((double)image.height() / image.width()) * width;
    }
    else
    {
        width = image.width();
        height = image.height();
    }

    QPixmap pixmap = QPixmap::fromImage(image);
    imageBtn->setFixedSize(width, height);
    imageBtn->setIconSize(QSize(width, height));
    imageBtn->setIcon(QIcon(pixmap));

    // 4. 重新设置父元素高度，确保父元素足够高，能够容纳下上述绘制区域
    parent->setFixedHeight(height + 50);

    // 5. 确定按钮所在的位置
    if(isLeft)
    {
        // 左侧消息
        this->imageBtn->setGeometry(10, 0, width, height);
    }
    else
    {
        // 右侧消息
        int leftPos = this->width() - width - 10;
        this->imageBtn->setGeometry(leftPos, 0, width, height);
    }
}

MessageFileLabel::MessageFileLabel(const QString &fileId, const QString &fileName, const QString& filePath, bool isLeft)
    :isLeft(isLeft), fileId(fileId), fileName(fileName), filePath(filePath)
{
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QFont font;
    font.setFamily("微软雅黑");
    font.setPixelSize(16);

    this->label = new QLabel(this);
    this->label->setFont(font);
    this->label->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    this->label->setWordWrap(true); // 文本自动换行
    this->label->setStyleSheet("QLabel { padding: 0 10px; line-height: 1.2; color: rgb(35, 35, 35); background-color: transparent}");
    if(isLeft)
    {
        this->label->setText("[文件][未下载]"+fileName);
    }
    else
    {
        this->label->setText("[文件][已发送]"+fileName);
    }
    model::DataCenter* dataCenter = model::DataCenter::getInstance();
    connect(dataCenter, &model::DataCenter::getSingleFileDone, this, &MessageFileLabel::saveFile);
    connect(dataCenter, &model::DataCenter::getSingleFileFail, this, [=](const QString& fileId, const QString& reason){
        if(this->fileId != fileId)
            return;
        this->label->setText("[文件][下载失败]"+reason);
    });
}

void MessageFileLabel::mousePressEvent(QMouseEvent *event)
{
    if(this->filePath.isEmpty())
    {
        model::DataCenter* dataCenter = model::DataCenter::getInstance();
        this->label->setText("[文件][正在下载]"+fileName);
        dataCenter->getSingleFileAsync(this->fileId);
    }
    else
    {
        // 获取文件所在文件夹的路径
        QFileInfo fileInfo(this->filePath);
        if(!fileInfo.exists())
        {
            LOG()<<"文件不存在："<<filePath;
            filePath.clear();

            QMessageBox msgBox;
            msgBox.setWindowIcon(QIcon(":/resource/image/logo.png"));
            msgBox.setWindowTitle("文件不存在");
            msgBox.setText("文件不存在,是否重新下载？");
            msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
            int result = msgBox.exec();
            if(result == QMessageBox::Ok)
            {
                LOG()<<"重新下载"<<fileName;
                this->label->setText("[文件][正在下载]"+fileName);
                model::DataCenter* dataCenter = model::DataCenter::getInstance();
                dataCenter->getSingleFileAsync(this->fileId);
            }
            else
            {
                this->label->setText("[文件][已失效]"+fileName);
            }
            return;
        }

        QString folderPath = fileInfo.dir().absolutePath();

        // 使用QDesktopServices打开文件夹
        QUrl folderUrl = QUrl::fromLocalFile(folderPath);
        if (!QDesktopServices::openUrl(folderUrl))
        {
            LOG() << "Failed to open folder:" << folderPath;
        }
    }
}

void MessageFileLabel::paintEvent(QPaintEvent *event)
{
    // 1. 获取父元素的宽度
    QObject* object = this->parent();
    if(!object->isWidgetType())
        return;
    QWidget* parent = dynamic_cast<QWidget*>(object);
    const int maxWidth = parent->width() * 0.6;

    // 2. 计算当前文本，如果是一行放置需要多宽
    QFontMetrics metrics(this->label->font());
    int totalWidth = metrics.horizontalAdvance(this->label->text());

    // // 3. 计算行数
    // int width = maxWidth;
    // int rows = (totalWidth / (maxWidth - 40)) + 1;
    // if(rows == 1)
    // {
    //     // 只有一行 40为左右各20边距
    //     width = totalWidth + 40;
    // }

    // // 4. 根据行数，计算得到的高度  20为上下各10边距
    // int height = rows * (this->label->font().pixelSize() * 1.2) + 20;

    // 计算宽度：短文本不换行，长文本换行
    bool needWrap = totalWidth > (maxWidth - 40);
    int width = needWrap ? (maxWidth) : (totalWidth + 40);

    // 设置 QLabel 的宽度和换行
    this->label->setWordWrap(needWrap);
    this->label->setFixedWidth(width); // 预留内边距
    this->label->adjustSize(); // 计算 QLabel 高度
    int height = this->label->height() + 20;

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

void MessageFileLabel::saveFile(const QString &fileId, const QByteArray &content)
{
    if(this->fileId != fileId)
        return;

    QString path = model::getDownLoadPath() + fileName;


    if(model::writeByteArrayToFile(path, content))
    {
        this->filePath = path;
        this->label->setText("[文件][已下载]"+fileName);
        this->update();
    }
    else
    {
        this->label->setText("[文件][保存失败]"+path);
    }
}
