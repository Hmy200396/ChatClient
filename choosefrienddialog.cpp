#include "choosefrienddialog.h"
#include <QScrollArea>
#include <QScrollBar>
#include <QPainter>
#include "debug.h"
#include "model/data.h"

///////////////////////////////////////
/// 选择好友窗口中的一个元素
///////////////////////////////////////
ChooseFriendItem::ChooseFriendItem(const QString& userId, const QIcon &avatar, const QString &name, bool checked)
    :userId(userId)
{
    // 1. 设置控件的基本属性
    this->setFixedHeight(50);
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    // 2. 设置布局管理器
    QHBoxLayout* layout = new QHBoxLayout();
    layout->setContentsMargins(20, 0, 20, 0);
    layout->setSpacing(10);
    this->setLayout(layout);

    // 3. 设置复选框
    checkBox = new QCheckBox();
    checkBox->setChecked(checked);
    checkBox->setFixedSize(25 ,25);
    checkBox->setStyleSheet("QCheckBox { background-color: transparent; }"
                            "QCheckBox::indicator { width: 20px; height: 20px; }"
                            "QCheckBox::indicator:unchecked { image: url(:resource/image/unchecked.png); }"
                            "QCheckBox::indicator:checked { image: url(:resource/image/checked.png); }");

    // 4. 创建头像
    avatarBtn = new QPushButton();
    avatarBtn->setFixedSize(40, 40);
    avatarBtn->setIconSize(QSize(40, 40));
    avatarBtn->setIcon(avatar);
    avatarBtn->setStyleSheet("QPushButton { background-color: transparent; }");

    // 5. 创建名字
    nameLabel = new QLabel();
    nameLabel->setText(name);
    nameLabel->setStyleSheet("QLabel { background-color: transparent; }");

    // 6. 添加到布局管理器
    layout->addWidget(checkBox);
    layout->addWidget(avatarBtn);
    layout->addWidget(nameLabel);

    // 7. 连接信号槽
    connect(checkBox, &QCheckBox::toggled, this, [=](bool checked){
        if(checked)
        {
            // 勾选了
            // 这四个parent分别是 totalContainer  qt_scrollarea_viewport  QScrollArea  ChooseFriendDialog
            ChooseFriendDialog* chooseFriendDialog = qobject_cast<ChooseFriendDialog*>(this->parent()->parent()->parent()->parent());
            if(chooseFriendDialog)
                chooseFriendDialog->addSelectedFriend(userId, avatar, name);
            else
                LOG()<<"父组件查询错误";

        }
        else
        {
            // 没有勾选
                ChooseFriendDialog* chooseFriendDialog = qobject_cast<ChooseFriendDialog*>(this->parent()->parent()->parent()->parent());
            if(chooseFriendDialog)
                chooseFriendDialog->deleteSelectedFriend(userId);
            else
                LOG()<<"父组件查询错误";
        }
    });
}

void ChooseFriendItem::paintEvent(QPaintEvent *event)
{
    // 根据鼠标的进入状态，决定绘制成不同的颜色
    QPainter painter(this);
    if(isHover)
    {
        painter.fillRect(this->rect(), QColor(230, 230 ,230));
    }
    else
    {
        painter.fillRect(this->rect(), QColor(255, 255 ,255));
    }
}

void ChooseFriendItem::enterEvent(QEnterEvent *event)
{
    isHover = true;
    this->update();
}

void ChooseFriendItem::leaveEvent(QEvent *event)
{
    isHover = false;
    this->update();
}

const QString& ChooseFriendItem::getUserId()
{
    return userId;
}

///////////////////////////////////////
/// 选择好友的窗口
///////////////////////////////////////
ChooseFriendDialog::ChooseFriendDialog(QWidget* parent)
    :QDialog(parent)
{
    // 1. 设置窗口基本属性
    this->setWindowTitle("选择好友");
    this->setWindowIcon(QIcon(":/resource/image/logo.png"));
    this->setFixedSize(750, 750);
    this->setStyleSheet("QDialog{ background-color: rgb(255, 255 ,255); }");
    this->setAttribute(Qt::WA_DeleteOnClose);

    // 2. 创建布局管理器
    QHBoxLayout* layout = new QHBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    this->setLayout(layout);

    // 3. 针对左侧窗口进行初始化
    initLeft(layout);

    // 4. 针对右侧窗口进行初始化
    initRight(layout);
}

void ChooseFriendDialog::initLeft(QHBoxLayout *layout)
{
    // 1. 创建滚动区域
    QScrollArea* scrollArea = new QScrollArea();
    scrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    scrollArea->setWidgetResizable(true);
    scrollArea->verticalScrollBar()->setStyleSheet(
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
    scrollArea->horizontalScrollBar()->setStyleSheet("QScrollBar:horizontal { height: 0px; }");
    scrollArea->setStyleSheet("QScrollArea { border: none }");
    layout->addWidget(scrollArea, 1);

    // 2. 创建 QWidget 设置到滚动区域中
    totalContainer = new QWidget();
    totalContainer->setStyleSheet("QWidget { background-color: rgb(255, 255 ,255); }");
    scrollArea->setWidget(totalContainer);

    // 3. 创建左侧子窗口内部的 垂直布局管理器
    QVBoxLayout* vlayout = new QVBoxLayout();
    vlayout->setContentsMargins(0, 0, 0, 0);
    vlayout->setSpacing(0);
    vlayout->setAlignment(Qt::AlignTop);
    totalContainer->setLayout(vlayout);

#if TEST_UI
    QIcon icon(":/resource/image/defaultAvatar.jpg");
    for(int i=0;i<50;++i)
    {
        this->addFriend(QString::number(1000 + i), icon, "琅琅" + QString::number(i), false);
    }
#endif
}

void ChooseFriendDialog::initRight(QHBoxLayout *layout)
{
    // 1. 创建右侧布局管理器
    QGridLayout* gridLayout = new QGridLayout();
    gridLayout->setContentsMargins(20, 0, 20, 20);
    gridLayout->setSpacing(10);
    layout->addLayout(gridLayout, 1);

    // 2. 创建 “标题” label
    QLabel* tipLabel = new QLabel();
    tipLabel->setText("选择联系人");
    tipLabel->setFixedHeight(30);
    tipLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    tipLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    tipLabel->setStyleSheet("QLabel { font-size: 16px; font-weight: 700; }");

    // 3.创建滚动区域
    QScrollArea* scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->verticalScrollBar()->setStyleSheet(
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
    scrollArea->horizontalScrollBar()->setStyleSheet("QScrollBar:horizontal { height: 0px; }");
    scrollArea->setStyleSheet("QScrollArea { border: none }");
    scrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // 4. 创建滚动区域的 QWidget
    selectedContainer = new QWidget();
    selectedContainer->setObjectName("selectedContainer");
    selectedContainer->setStyleSheet("#selectedContainer { background-color: rgb(255, 255 ,255); }");
    scrollArea->setWidget(selectedContainer);

    // 5. 创建 selectedContainer 中的垂直布局
    QVBoxLayout* vlayout = new QVBoxLayout();
    vlayout->setSpacing(0);
    vlayout->setContentsMargins(0, 0, 0, 0);
    vlayout->setAlignment(Qt::AlignTop);
    selectedContainer->setLayout(vlayout);

    // 6. 创建底部按钮
    QString style = "QPushButton { color: rgb(7, 191, 96); background-color: rgb(240, 240, 240); border: none; border-radius: 5px; }";
    style += "QPushButton:hover {background-color: rgb(220, 220, 220);}";
    style += "QPushButton:pressed {background-color: rgb(200, 200, 200);}";

    QPushButton* okBtn = new QPushButton();
    okBtn->setFixedHeight(40);
    okBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    okBtn->setText("完成");
    okBtn->setStyleSheet(style);

    QPushButton* cancelBtn = new QPushButton();
    cancelBtn->setFixedHeight(40);
    cancelBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    cancelBtn->setText("取消");
    cancelBtn->setStyleSheet(style);

    // 7. 把上述控件添加到布局
    gridLayout->addWidget(tipLabel, 0, 0, 1, 9);
    gridLayout->addWidget(scrollArea, 1, 0, 1, 9);
    gridLayout->addWidget(okBtn, 2, 1, 1, 3);
    gridLayout->addWidget(cancelBtn, 2, 5, 1, 3);
}

void ChooseFriendDialog::addFriend(const QString& userId, const QIcon &avatar, const QString &name, bool checked)
{
    ChooseFriendItem* item = new ChooseFriendItem(userId, avatar, name, checked);
    totalContainer->layout()->addWidget(item);
}

void ChooseFriendDialog::addSelectedFriend(const QString& userId, const QIcon &avatar, const QString &name)
{
    ChooseFriendItem* item = new ChooseFriendItem(userId, avatar, name, true);
    selectedContainer->layout()->addWidget(item);
}

void ChooseFriendDialog::deleteSelectedFriend(const QString &userId)
{
    // 遍历 selectedContainer 中的每个 Item ，对比userId
    QVBoxLayout* vlayout = dynamic_cast<QVBoxLayout*>(selectedContainer->layout());
    for(int i = vlayout->count(); i >= 0; --i)
    {
        QLayoutItem* item = vlayout->itemAt(i);
        if(item == nullptr || item->widget() == nullptr)
            continue;
        ChooseFriendItem* chooseFriendItem = dynamic_cast<ChooseFriendItem*>(item->widget());
        if(chooseFriendItem->getUserId() != userId)
            continue;
        vlayout->removeWidget(chooseFriendItem);
        // 此处直接使用 delete 可能导致程序直接崩溃，因为 delete 该对象的时候，该对象内部的 QCheckBox 还在使用中（触发着信号槽呢）
        // 改成 deleteLater，就相当于把 delete 操作委托给Qt自身来完成了
        // Qt会确保在用完之后去真正删除
        // delete chooseFriendItem;
        chooseFriendItem->deleteLater();
    }

    // 再遍历一下左侧列表，把左侧列表中对应 item 的 checkBox 勾选状态取消掉
    QVBoxLayout* vlayoutLeft = dynamic_cast<QVBoxLayout*>(totalContainer->layout());
    for(int i = 0; i < vlayoutLeft->count(); ++i)
    {
        QLayoutItem* item = vlayoutLeft->itemAt(i);
        if(item == nullptr || item->widget() == nullptr)
            continue;
        ChooseFriendItem* chooseFriendItem = dynamic_cast<ChooseFriendItem*>(item->widget());
        if(chooseFriendItem->getUserId() != userId)
            continue;
        // 取消 checkBox的选中状态
        chooseFriendItem->getCheckBox()->setChecked(false);
    }
}
