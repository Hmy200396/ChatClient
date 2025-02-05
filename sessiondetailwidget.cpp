#include "sessiondetailwidget.h"
#include <QVBoxLayout>
#include <QFontMetrics>
#include <QMessageBox>
#include <QIcon>
#include"model/datacenter.h"
#include "debug.h"
#include "choosefrienddialog.h"

//////////////////////////////////////////////
/// 表示 一个头像 + 一个名字组合控件
//////////////////////////////////////////////
AvatarItem::AvatarItem(const QIcon &avatar, const QString &name)
{
    // 1. 设置自身基本属性
    this->setFixedSize(70, 80);

    // 2. 创建布局管理器
    QVBoxLayout* layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->setAlignment(Qt::AlignHCenter);
    this->setLayout(layout);

    // 3. 创建头像
    avatarBtn = new QPushButton();
    avatarBtn->setFixedSize(45, 45);
    avatarBtn->setIconSize(QSize(45, 45));
    avatarBtn->setIcon(avatar);
    avatarBtn->setStyleSheet("QPushButton { border: none; }");

    // 4. 创建名字
    nameLabel = new QLabel();
    nameLabel->setText(name);
    QFont font("微软雅黑", 12);
    nameLabel->setFont(font);
    nameLabel->setStyleSheet("QLabel { color: black; }");
    nameLabel->setAlignment(Qt::AlignCenter);

    // 5. 对名字做截断操作
    const int MAX_WIDTH = 65;
    QFontMetrics metrics(font);
    int totalWidth = metrics.horizontalAdvance(name);
    if(totalWidth >= MAX_WIDTH)
    {
        QString tail = "...";
        int tailWidth = metrics.horizontalAdvance(tail);
        int availableWidth = MAX_WIDTH - tailWidth;
        int availableSize = name.size() * ((double)availableWidth / totalWidth);
        QString newName = name.left(availableSize);
        nameLabel->setText(newName + tail);
    }

    // 6. 添加到布局管理器
    layout->addWidget(avatarBtn);
    layout->addWidget(nameLabel);
}

QPushButton *AvatarItem::getAvatar()
{
    return avatarBtn;
}

//////////////////////////////////////////////
/// 表示 “单聊会话详情” 窗口
//////////////////////////////////////////////
SessionDetailWidget::SessionDetailWidget(QWidget* parent, const model::UserInfo& userInfo)
    :QDialog(parent),userInfo(userInfo)
{
    // 1. 设计基本属性
    this->setWindowTitle("会话详情");
    this->setWindowIcon(QIcon(":/resource/image/logo.png"));
    this->setFixedSize(300, 300);
    this->setStyleSheet("QDialog{ background-color: rgb(255, 255 ,255); }");
    this->setAttribute(Qt::WA_DeleteOnClose);

    // 2. 创建布局管理器
    QGridLayout* layout = new QGridLayout();
    layout->setContentsMargins(50, 0, 50, 0);
    layout->setSpacing(10);
    //layout->setAlignment(Qt::AlignHCenter);
    this->setLayout(layout);

    // 3. 添加 “创建群聊” 按钮
    AvatarItem* createGroupBtn = new AvatarItem(QIcon(":/resource/image/cross.png"), "添加");
    layout->addWidget(createGroupBtn, 0, 0);

    // 4. 添加当前用户的信息
#if TEST_UI
    AvatarItem* currentUser = new AvatarItem(QIcon(":/resource/image/defaultAvatar.jpg"), "琅琅琅琅琅琅琅琅琅琅琅琅琅琅");
    layout->addWidget(currentUser, 0, 1);
#endif
    AvatarItem* currentUser = new AvatarItem(userInfo.avatar,userInfo.nickname);
    layout->addWidget(currentUser, 0, 1);

    // 5. 添加 “删除好友” 按钮
    deleteFriendBtn = new QPushButton();
    deleteFriendBtn->setFixedHeight(50);
    deleteFriendBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    deleteFriendBtn->setText("删除好友");
    deleteFriendBtn->setStyleSheet("QPushButton { border: 1px solid rgb(90, 90 ,90); border-radius: 5px; color: red;}"
                                   "QPushButton:pressed { background-color: rgb(235, 235 ,235); }");
    layout->addWidget(deleteFriendBtn, 1, 0, 1, 3);

    // 6. 添加信号槽，处理点击 “创建群聊” 按钮
    connect(createGroupBtn->getAvatar(), &QPushButton::clicked, this, [=](){
        ChooseFriendDialog* chooseFriendDialog = new ChooseFriendDialog(this, userInfo.userId);
        chooseFriendDialog->exec();
    });
    connect(deleteFriendBtn, &QPushButton::clicked, this, &SessionDetailWidget::clickDeleteFriendBtn);
}

void SessionDetailWidget::clickDeleteFriendBtn()
{
    // 1. 弹出一个对话框让用户确认是否是要真的删除
    // int result = QMessageBox::warning(this, "确认删除", "确认删除当前好友？", QMessageBox::Ok, QMessageBox::Cancel);
    QMessageBox msgBox;
    msgBox.setWindowIcon(QIcon(":/resource/image/logo.png"));
    msgBox.setWindowTitle("确认删除");
    msgBox.setText("确认删除当前好友？");
    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    int result = msgBox.exec();

    if(result != QMessageBox::Ok)
    {
        LOG()<<"用户取消了好友删除";
        return;
    }

    // 2. 发送删除好友请求
    model::DataCenter* dataCenter = model::DataCenter::getInstance();
    dataCenter->deleteFriendAsync(this->userInfo.userId);

    // 3. 关闭当前窗口
    this->close();
}
