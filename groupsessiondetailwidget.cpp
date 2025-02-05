#include "groupsessiondetailwidget.h"
#include <QVBoxLayout>
#include <QScrollArea>
#include <QScrollBar>
#include <QPushButton>
#include "sessiondetailwidget.h"
#include "debug.h"

GroupSessionDetailWidget::GroupSessionDetailWidget(QWidget* parent)
    :QDialog(parent)
{
    // 1. 设置窗口基本属性
    this->setFixedSize(410, 600);
    this->setWindowTitle("群组详情");
    this->setWindowIcon(QIcon(":/resource/image/logo.png"));
    this->setStyleSheet("QDialog { background-color: rgb(255, 255, 255); }");
    this->setAttribute(Qt::WA_DeleteOnClose);

    // 2. 创建布局管理器
    QVBoxLayout* vlayout = new QVBoxLayout();
    vlayout->setContentsMargins(50, 20, 50, 50);
    vlayout->setSpacing(10);
    vlayout->setAlignment(Qt::AlignTop);
    this->setLayout(vlayout);

    // 3. 创建滚动区域
    // 3.1 创建 QScrollArea 对象
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
    scrollArea->setStyleSheet("QWidget { border: none; background-color: transparent; }");
    scrollArea->setFixedSize(310, 350);
    scrollArea->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    // 3.2 创建一个 QScrollArea 内部的 QWidget
    QWidget* container = new QWidget();
    scrollArea->setWidget(container);

    // 3.3 给 container 里面添加一个网格布局
    glayout = new QGridLayout();
    glayout->setSpacing(0);
    glayout->setContentsMargins(0, 0, 0, 0);
    glayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    container->setLayout(glayout);

    // 3.4 把滚动区域添加到布局管理器中
    vlayout->addWidget(scrollArea);

    // 4. 添加 “添加”按钮
    AvatarItem* addBtn = new AvatarItem(QIcon(":/resource/image/cross.png"), "添加");
    glayout->addWidget(addBtn, 0, 0);

    // 5. 添加 群聊名称
    QLabel* groupNameTag = new QLabel();
    groupNameTag->setText("群聊名称");
    groupNameTag->setFixedHeight(50);
    groupNameTag->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    groupNameTag->setStyleSheet("QLabel { font-weight: 700; font-size: 16px; color: black;}");
    groupNameTag->setAlignment(Qt::AlignBottom);
    vlayout->addWidget(groupNameTag);

    // 6. 添加 真实的群聊名字 和 修改按钮
    // 6.1 创建水平布局
    QHBoxLayout* hlayout = new QHBoxLayout();
    hlayout->setSpacing(0);
    hlayout->setContentsMargins(0, 0, 0, 0);
    vlayout->addLayout(hlayout);

    // 6.2 创建真实群聊名字的 label
    QLabel* groupNameLabel = new QLabel();
    groupNameLabel->setFixedHeight(50);
    groupNameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    groupNameLabel->setStyleSheet("QLabel { font-size: 18px; }");
    hlayout->addWidget(groupNameLabel);

    // 6.3 创建 “修改按钮”
    QPushButton* modifyBtn = new QPushButton();
    modifyBtn->setFixedSize(30, 30);
    modifyBtn->setIconSize(QSize(20, 20));
    modifyBtn->setIcon(QIcon(":/resource/image/modify.png"));
    modifyBtn->setStyleSheet("QPushButton { border: none; background-color: transparent;}"
                             "QPushButton:pressed { background-color: rgb(230, 230 ,230); }");
    hlayout->addWidget(modifyBtn);

    // 7 退出群聊按钮
    QPushButton* exitGroupBtn = new QPushButton();
    exitGroupBtn->setText("退出群聊");
    exitGroupBtn->setFixedHeight(50);
    exitGroupBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    QString btnStyle = "QPushButton { border: 2px solid rgb(90, 90, 90); border-radius: 5px; background-color: transparent; color: red; font-size: 20px;}";
    btnStyle += "QPushButton:pressed { background-color: rgb(230, 230 ,230);}";
    exitGroupBtn->setStyleSheet(btnStyle);
    vlayout->addWidget(exitGroupBtn);

    // 此处构造假数据测试页面
#if TEST_UI
    groupNameLabel->setText("米琅混合群");
    for(int i = 0; i< 20 ; ++i)
    {
        AvatarItem *avatarItem = new AvatarItem(QIcon(":/resource/image/defaultAvatar.jpg"), "琅琅"+QString::number(i));
        this->addMember(avatarItem);
    }
#endif

}

void GroupSessionDetailWidget::addMember(AvatarItem* avatarItem)
{
    // 拿到滚动区域的布局管理器
    const int MAX_COL = 4;
    if(curCol >= MAX_COL)
    {
        curCol = 0;
        ++curRow;
    }
    glayout->addWidget(avatarItem, curRow, curCol);
    ++curCol;
}
