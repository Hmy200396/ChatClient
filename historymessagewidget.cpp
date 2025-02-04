#include "historymessagewidget.h"
#include "debug.h"
#include <QScrollArea>
#include <QScrollBar>
#include <QRadioButton>
#include <QPushButton>
#include <QLineEdit>
#include <QDateTimeEdit>
#include <QLabel>
#include <QFrame>
#include <QPainter>
#include <QStyleOption>

/////////////////////////////////////
/// 表示一个历史消息元素
/////////////////////////////////////
HistoryItem *HistoryItem::makeHistoryItem(const model::Message &message)
{
    // 1. 创建出对象
    HistoryItem* item = new HistoryItem();
    item->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    // 2. 创建布局
    QGridLayout* layout = new QGridLayout();
    layout->setVerticalSpacing(10);
    layout->setHorizontalSpacing(10);
    layout->setContentsMargins(0, 30 ,0, 0);
    item->setLayout(layout);

    // 3. 创建头像
    QPushButton* avatarbtn = new QPushButton();
    avatarbtn->setFixedSize(40, 40);
    avatarbtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    avatarbtn->setIconSize(QSize(40, 40));
    avatarbtn->setIcon(message.sender.avatar);
    avatarbtn->setStyleSheet("QPushButton { border: none; }");

    // 4. 创建昵称和时间
    QLabel* nameLabel = new QLabel();
    nameLabel->setText(message.sender.nickname + " | " + message.time);
    nameLabel->setFixedHeight(40);
    nameLabel->setAlignment(Qt::AlignTop);
    nameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    //nameLabel->setStyleSheet("QLabel { background-color: transparent; }");

    // 5. 创建消息内容
    QWidget* contentWidget = nullptr;
    if(message.messageType == model::TEXT_TYPE)
    {
        // 文本消息
        QFont font("微软雅黑", 12);
        QLabel* label = new QLabel();
        label->setFont(font);
        label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        label->setWordWrap(true); // 换行
        label->setText(QString(message.content));
        label->setAlignment(Qt::AlignTop);
        label->adjustSize(); // 设置让 label 能够自动调整大小
        contentWidget = label;
    }
    else if(message.messageType == model::IMAGE_TYPE)
    {
        // 图片消息
    }
    else if(message.messageType == model::FILE_TYPE)
    {
        // 文件消息
    }
    else if(message.messageType == model::SPEECH_TYPE)
    {
        // 语音消息
    }
    else
    {
        LOG() << "错误的消息类型！messageType = " << message.messageType;
    }

    // 6. 创建分隔线
    QFrame* line = new QFrame();
    line->setFrameShape(QFrame::HLine);  // 设置为水平线
    line->setFrameShadow(QFrame::Sunken); // 设置阴影效果
    line->setLineWidth(1);
    line->setStyleSheet("QFrame { border: none; background-color: rgb(246, 246, 246); }");

    // 7. 把上述控件添加到布局中
    layout->addWidget(avatarbtn, 0, 0, 2, 1);
    layout->addWidget(nameLabel, 0, 1, 2, 9);
    layout->addWidget(contentWidget, 2, 1, 10, 9);
    layout->addWidget(line, 11, 1, 1, 8);

    return item;
}

void HistoryItem::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void HistoryItem::enterEvent(QEnterEvent *event)
{
    this->setStyleSheet("background-color: rgb(240, 240 ,240);");
}

void HistoryItem::leaveEvent(QEvent *event)
{
    this->setStyleSheet("background-color: rgb(255, 255 ,255);");
}


/////////////////////////////////////
/// 展示历史消息窗口
/////////////////////////////////////
HistoryMessageWidget::HistoryMessageWidget(QWidget* parent)
    :QDialog(parent)
{
    // 1. 设置窗口属性
    this->setFixedSize(600, 600);
    this->setWindowTitle("历史消息");
    this->setWindowIcon(QIcon(":/resource/image/logo.png"));
    this->setStyleSheet("QWidget { background-color: rgb(255, 255, 255); }");
    this->setAttribute(Qt::WA_DeleteOnClose);

    // 2. 创建布局管理器
    QGridLayout* layout = new QGridLayout();
    layout->setContentsMargins(30, 30, 30, 0);
    layout->setSpacing(10);
    this->setLayout(layout);

    // 3. 创建单选按钮
    QRadioButton* keyRadioBtn = new QRadioButton();
    QRadioButton* timeRadioBtn = new QRadioButton();
    keyRadioBtn->setText("按关键词查询");
    timeRadioBtn->setText("按时间查询");
    // 默认按照关键词查询
    keyRadioBtn->setChecked(true);
    layout->addWidget(keyRadioBtn, 0, 0, 1, 2);
    layout->addWidget(timeRadioBtn, 0, 2, 1, 2);

    // 4. 创建搜索框
    QLineEdit* searchEdit = new QLineEdit();
    searchEdit->setFixedHeight(50);
    searchEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    searchEdit->setPlaceholderText("要搜索的关键词");
    searchEdit->setStyleSheet("QLineEdit { border: none; border-radius: 10px; background-color: rgb(240, 240, 240); font-size: 16px; padding-left:10px; }");
    layout->addWidget(searchEdit, 1, 0, 1, 8);

    // 5. 创建搜索按钮
    QPushButton* searchBtn = new QPushButton();
    searchBtn->setFixedSize(50, 50);
    searchBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    searchBtn->setIconSize(QSize(30, 30));
    searchBtn->setIcon(QIcon(":/resource/image/search.png"));
    QString btnStyle = "QPushButton { border: none; border-radius: 10px; background-color: rgb(240, 240, 240); }";
    btnStyle += "QPushButton:pressed { background-color: rgb(220, 220, 220); }";
    searchBtn->setStyleSheet(btnStyle);
    layout->addWidget(searchBtn, 1, 8, 1, 1);

    // 6. 创建时间相关的部分控件
    QLabel* begTag = new QLabel();
    begTag->setText("开始时间");
    QLabel* endTag = new QLabel();
    endTag->setText("结束时间");
    QDateTimeEdit* begTimeEdit = new QDateTimeEdit();
    QDateTimeEdit* endTimeEdit = new QDateTimeEdit();
    begTimeEdit->setFixedHeight(40);
    endTimeEdit->setFixedHeight(40);
    begTag->hide();
    endTag->hide();
    begTimeEdit->hide();
    endTimeEdit->hide();

    // 7. 创建滚动区域
    initScrollArea(layout);

    // 8. 设置槽函数
    connect(keyRadioBtn, &QRadioButton::clicked, this ,[=](){
        // 把时间相关控件隐藏
        layout->removeWidget(begTag);
        layout->removeWidget(begTimeEdit);
        layout->removeWidget(endTag);
        layout->removeWidget(endTimeEdit);
        begTag->hide();
        begTimeEdit->hide();
        endTag->hide();
        endTimeEdit->hide();

        // 关键词搜索框显示并加入布局
        layout->addWidget(searchEdit, 1, 0, 1, 8);
        searchEdit->show();
    });
    connect(timeRadioBtn, &QRadioButton::clicked, this ,[=](){
        // 关键词搜索框隐藏并从布局中删除掉
        layout->removeWidget(searchEdit);
        searchEdit->hide();

        // 把时间相关的控件，添加到布局中，并显示
        layout->addWidget(begTag, 1, 0, 1, 1);
        layout->addWidget(begTimeEdit, 1, 1, 1, 3);
        layout->addWidget(endTag, 1, 4, 1, 1);
        layout->addWidget(endTimeEdit, 1, 5, 1, 3);
        begTag->show();
        begTimeEdit->show();
        endTag->show();
        endTimeEdit->show();
    });

    // 构造测试数据
#ifdef TEST_UI
    QString str;
    for(int i = 0; i < 30; ++i)
    {
        model::UserInfo sender;
        sender.userId = "";
        sender.nickname = "琅琅" + QString::number(i);
        sender.avatar = QIcon(":/resource/image/defaultAvatar.jpg");
        sender.description = "";
        sender.phone = "17302083024";
        str += "消息内容消息内容";
        model::Message message = model::Message::makeMessage(model::TEXT_TYPE, "", sender, QString( str + QString::number(i)).toUtf8(), "");
        this->addHistoryMessage(message);
    }
#endif
}

void HistoryMessageWidget::addHistoryMessage(const model::Message& message)
{
    HistoryItem* item = HistoryItem::makeHistoryItem(message);
    container->layout()->addWidget(item);
}

void HistoryMessageWidget::clear()
{
    QVBoxLayout* layout = dynamic_cast<QVBoxLayout*>(container->layout());
    for(int i = layout->count() - 1; i >= 0; --i)
    {
        // QLayoutItem* item = layout->takeAt(i);
        // if(item && item->widget())
        //     delete item->widget();
        QWidget* w = layout->itemAt(i)->widget();
        if(w == nullptr)
            continue;
        layout->removeWidget(w);
        w->deleteLater();
    }
}

void HistoryMessageWidget::initScrollArea(QGridLayout *layout)
{
    // 1. 创建滚动区域
    QScrollArea* scrollArea = new QScrollArea();
    scrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    scrollArea->setWidgetResizable(true);
    scrollArea->horizontalScrollBar()->setStyleSheet("QScrollBar:horizontal { height: 0px; }");
    scrollArea->verticalScrollBar()->setStyleSheet(
        "QScrollBar:vertical { "
        "width: 10px; "
        "background-color: rgb(255, 255, 255); "
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
    scrollArea->setStyleSheet("QScrollArea { border: none;}");
    layout->addWidget(scrollArea, 2, 0, 1, 9);

    // 2. 创建 QWidget
    container = new QWidget();
    scrollArea->setWidget(container);

    // 3. 创建 container 中的布局管理器
    QVBoxLayout* vlayout = new QVBoxLayout();
    vlayout->setContentsMargins(0, 0, 0, 0);
    vlayout->setSpacing(0);
    vlayout->setAlignment(Qt::AlignTop);
    container->setLayout(vlayout);
}
