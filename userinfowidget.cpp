#include "userinfowidget.h"
#include "model/datacenter.h"
#include "mainwidget.h"
#include <QGridLayout>
#include <QMessageBox>
UserInfoWidget::UserInfoWidget(const model::UserInfo& userInfo, QWidget* parent)
    :userInfo(userInfo),QDialog(parent)
{
    // 1. 设置整个窗口的属性
    this->setFixedSize(400, 200);
    this->setWindowTitle("用户详情");
    this->setWindowIcon(QIcon(":/resource/image/logo.png"));

    // 窗口被关闭时，自动销毁这个对话框对象
    this->setAttribute(Qt::WA_DeleteOnClose);
    // 把窗口移动到鼠标当前位置
    this->move(QCursor::pos());

    // 2. 创建布局管理器
    QGridLayout* layout = new QGridLayout();
    layout->setContentsMargins(20, 20, 0, 0);
    layout->setSpacing(10);
    layout->setAlignment(Qt::AlignTop);
    this->setLayout(layout);

    // 3. 添加头像
    avatarBtn = new QPushButton();
    avatarBtn->setFixedSize(75, 75);
    avatarBtn->setIconSize(QSize(75, 75));
    avatarBtn->setIcon(userInfo.avatar);

    QString labelStyle = "QLabel { font-weight: 800; }";
    QString redBtnStyle = "QPushButton { border: 1px solid rgb(89, 89, 89); border-radius: 5px;\
                        background-color: rgb(30, 30, 30); color: red;}\
                        QPushButton:pressed { background-color: rgb(64, 64, 64) }";
    QString GreenBtnStyle = "QPushButton { border: 1px solid rgb(89, 89, 89); border-radius: 5px;\
                          background-color: rgb(30, 30, 30); color: green;}\
                        QPushButton:pressed { background-color: rgb(64, 64, 64) }";
    QString GreyBtnStyle = "QPushButton { border: 1px solid rgb(89, 89, 89); border-radius: 5px;\
                            background-color: rgb(30, 30, 30); color: grey;}\
                        QPushButton:pressed { background-color: rgb(64, 64, 64) }";

    // 4. 添加用户序号
    idTag = new QLabel();
    idTag->setText("序号");
    idTag->setStyleSheet(labelStyle);
    idTag->setAlignment(Qt::AlignCenter);

    idLabel = new QLabel();
    idLabel->setText(userInfo.userId);
    idLabel->setAlignment(Qt::AlignLeft);
    idLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    // 5. 添加用户昵称
    nameTag = new QLabel();
    nameTag->setText("昵称");
    nameTag->setStyleSheet(labelStyle);
    nameTag->setAlignment(Qt::AlignCenter);

    nameLabel = new QLabel();
    nameLabel->setText(userInfo.nickname);
    nameLabel->setAlignment(Qt::AlignLeft);
    nameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    // 6. 设置电话
    phoneTag = new QLabel();
    phoneTag->setText("电话");
    phoneTag->setStyleSheet(labelStyle);
    phoneTag->setAlignment(Qt::AlignCenter);

    phoneLabel = new QLabel();
    phoneLabel->setText(userInfo.phone);
    phoneLabel->setAlignment(Qt::AlignLeft);
    phoneLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    // 7. 添加三个按钮
    applyBtn = new QPushButton();
    applyBtn->setText("申请好友");
    applyBtn->setFixedSize(80, 30);
    applyBtn->setStyleSheet(GreenBtnStyle);

    sendMessageBtn = new QPushButton();
    sendMessageBtn->setText("发送消息");
    sendMessageBtn->setFixedSize(80, 30);
    sendMessageBtn->setStyleSheet(GreenBtnStyle);

    deleteFriendBtn = new QPushButton();
    deleteFriendBtn->setText("删除好友");
    deleteFriendBtn->setFixedSize(80, 30);
    deleteFriendBtn->setStyleSheet(redBtnStyle);

    // 8. 添加到布局管理器
    layout->addWidget(avatarBtn, 0, 0, 3, 1);
    layout->addWidget(idTag, 0, 1, Qt::AlignLeft);
    layout->addWidget(idLabel, 0, 2, Qt::AlignLeft);
    layout->addWidget(nameTag, 1, 1, Qt::AlignLeft);
    layout->addWidget(nameLabel, 1, 2, Qt::AlignLeft);
    layout->addWidget(phoneTag, 2, 1, Qt::AlignLeft);
    layout->addWidget(phoneLabel, 2, 2, Qt::AlignLeft);
    layout->addWidget(applyBtn, 3, 0);
    layout->addWidget(sendMessageBtn, 3, 1);
    layout->addWidget(deleteFriendBtn, 3, 2);

    // 9. 初始化按钮的禁用关系
    // 拿着 userId 在 DataCenter 的好友列表中，查询即可
    model::DataCenter* dataCenter = model::DataCenter::getInstance();
    model::UserInfo* myFriend = dataCenter->findFriendById(this->userInfo.userId);
    if(myFriend == nullptr)
    {
        // 不是好友
        sendMessageBtn->setEnabled(false);
        deleteFriendBtn->setEnabled(false);
        sendMessageBtn->setStyleSheet(GreyBtnStyle);
        deleteFriendBtn->setStyleSheet(GreyBtnStyle);
    }
    else
    {
        // 是好友
        applyBtn->setEnabled(false);
        applyBtn->setStyleSheet(GreyBtnStyle);
    }

    // 10. 初始化信号槽
    initSignalSlot();
}

void UserInfoWidget::initSignalSlot()
{
    connect(sendMessageBtn, &QPushButton::clicked, this, [=](){
        // 拿到主窗口指针，通过主窗口中，前面实现的 切换到会话 这样的功能，直接调用即可
        MainWidget* mainWidget = MainWidget::getInstance();
        mainWidget->switchSession(userInfo.userId);

        this->close();
    });

    connect(deleteFriendBtn, &QPushButton::clicked, this, &UserInfoWidget::clickDeleteFriendBtn);
    connect(applyBtn, &QPushButton::clicked, this, &UserInfoWidget::clickApplyBtn);
}

void UserInfoWidget::clickDeleteFriendBtn()
{
    // 1. 弹出对话框，让用户确认是否要真的删除
    QMessageBox::StandardButton result = QMessageBox::warning(this, "确认删除", "确认删除当前好友？", QMessageBox::Ok | QMessageBox::Cancel);
    if(result != QMessageBox::Ok)
    {
        LOG()<<"删除好友取消";
        return;
    }

    // 2. 发送网络请求，实现删除好友功能
    model::DataCenter* dataCenter = model::DataCenter::getInstance();
    dataCenter->deleteFriendAsync(userInfo.userId);

    // 3. 关闭窗口
    this->close();
}

void UserInfoWidget::clickApplyBtn()
{
    // 1. 发送好友申请
    model::DataCenter* dataCenter = model::DataCenter::getInstance();
    dataCenter->addFriendApplyAsync(userInfo.userId);

    // 2. 关闭窗口
    this->close();
}
