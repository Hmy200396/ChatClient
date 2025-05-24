#include "groupsessiondetailwidget.h"
#include <QVBoxLayout>
#include <QScrollArea>
#include <QScrollBar>
#include <QPushButton>
#include "sessiondetailwidget.h"
#include "debug.h"
#include "model/datacenter.h"
#include "invitefriendjoingroupdialog.h"

GroupSessionDetailWidget::GroupSessionDetailWidget(QWidget* parent, model::ChatSessionInfo* chatSessionInfo)
    :QDialog(parent)
    ,chatSessionInfo(chatSessionInfo)
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
    vlayout->setSpacing(20);
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
    groupNameTag->setStyleSheet("QLabel { font-weight: 700; font-size: 16px; color: black; }");
    groupNameTag->setAlignment(Qt::AlignBottom);
    vlayout->addWidget(groupNameTag);

    // 6. 添加 真实的群聊名字 和 修改按钮
    // 6.1 创建水平布局
    hlayout = new QHBoxLayout();
    hlayout->setSpacing(0);
    hlayout->setContentsMargins(0, 0, 0, 0);
    vlayout->addLayout(hlayout);

    // 6.2 创建真实群聊名字的 label
    groupNameLabel = new QLabel();
    groupNameLabel->setFixedHeight(40);
    groupNameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    groupNameLabel->setStyleSheet("QLabel { font-size: 18px; color: black; }");
    hlayout->addWidget(groupNameLabel);

    // 6.3 创建 “修改按钮”
    modifyBtn = new QPushButton();
    modifyBtn->setFixedSize(30, 30);
    modifyBtn->setIconSize(QSize(20, 20));
    modifyBtn->setIcon(QIcon(":/resource/image/modify.png"));
    modifyBtn->setStyleSheet("QPushButton { border: none; background-color: transparent;}"
                             "QPushButton:pressed { background-color: rgb(230, 230 ,230); }");
    hlayout->addWidget(modifyBtn);

    // 6.4 创建 群聊名称编辑 edit
    groupNameEdit = new QLineEdit();
    groupNameEdit->setFixedHeight(40);
    groupNameEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    groupNameEdit->setStyleSheet("QLineEdit { border: none; border-radius: 10px; padding-left: 5px; color: black; font-size: 18px; background-color: rgb(230, 230 ,230);}");
    groupNameEdit->hide();

    // 6.5 创建 “提交按钮”
    submitBtn = new QPushButton();
    submitBtn->setFixedSize(30, 30);
    submitBtn->setIconSize(QSize(20, 20));
    submitBtn->setIcon(QIcon(":/resource/image/submit.png"));
    submitBtn->setStyleSheet("QPushButton { border: none; background-color: transparent;}"
                             "QPushButton:pressed { background-color: rgb(230, 230 ,230); }");
    submitBtn->hide();

    // 7 退出群聊按钮
    exitGroupBtn = new QPushButton();
    exitGroupBtn->setText("退出群聊");
    exitGroupBtn->setFixedHeight(50);
    exitGroupBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    QString btnStyle = "QPushButton { border: 2px solid rgb(90, 90, 90); border-radius: 5px; background-color: transparent; color: red; font-size: 20px;}";
    btnStyle += "QPushButton:pressed { background-color: rgb(230, 230 ,230);}";
    exitGroupBtn->setStyleSheet(btnStyle);
    vlayout->addWidget(exitGroupBtn);

    // 8. 绑定信号槽
    connect(addBtn->getAvatar(), &QPushButton::clicked, this, [=](){
        InviteFriendJoinGroupDialog* inviteFriendJoinGroupDialog = new InviteFriendJoinGroupDialog(this, this->userIdList);
        inviteFriendJoinGroupDialog->exec();
    });
    connect(exitGroupBtn, &QPushButton::clicked, this, &GroupSessionDetailWidget::exitGroup);

    // 此处构造假数据测试页面
#if TEST_UI
    groupNameLabel->setText("米琅混合群");
    for(int i = 0; i< 20 ; ++i)
    {
        AvatarItem *avatarItem = new AvatarItem(QIcon(":/resource/image/defaultAvatar.jpg"), "琅琅"+QString::number(i));
        this->addMember(avatarItem);
    }
#endif

    // 8. 初始化信号槽
    initSignalSlot();

    // 9. 从服务器加载数据
    initData();

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

void GroupSessionDetailWidget::initData()
{
    model::DataCenter* dataCenter = model::DataCenter::getInstance();
    connect(dataCenter, &model::DataCenter::getMemberListDone, this, &GroupSessionDetailWidget::initMembers);
    dataCenter->getMemberListAsync(dataCenter->getCurrentChatSessionId());
}

void GroupSessionDetailWidget::initMembers(const QString& chatSessionId)
{
    // 根据刚才拿到的成员列表，把成员列表渲染到界面上
    model::DataCenter* dataCenter = model::DataCenter::getInstance();
    QList<model::UserInfo>* memberList = dataCenter->getMemberList(chatSessionId);
    if(memberList == nullptr)
    {
        LOG()<<"获取成员列表为空 chatSessionId = "<<chatSessionId;
        return;
    }

    // 遍历成员列表
    for(const auto& u : *memberList)
    {
        userIdList.push_back(u.userId);
        AvatarItem* avatarItem = new AvatarItem(u.avatar, u.nickname);
        this->addMember(avatarItem);
    }

    // 群聊名称
    groupNameLabel->setText(chatSessionInfo->chatSessionName);
}

void GroupSessionDetailWidget::initSignalSlot()
{
    connect(modifyBtn, &QPushButton::clicked, this, [=](){
        // 把当前的 groupNameLabel 和 modifyBtn 隐藏起来
        groupNameLabel->hide();
        modifyBtn->hide();
        hlayout->removeWidget(groupNameLabel);
        hlayout->removeWidget(modifyBtn);
        // 把 nameEdit 和 nameSubmitBtn 显示出来
        groupNameEdit->show();
        submitBtn->show();
        hlayout->addWidget(groupNameEdit);
        hlayout->addWidget(submitBtn);
        // 把输入框内容进行设置
        groupNameEdit->setText(groupNameLabel->text());
    });

    connect(submitBtn, &QPushButton::clicked, this, &GroupSessionDetailWidget::clickSubmitBtn);
}

void GroupSessionDetailWidget::exitGroup()
{
    model::DataCenter* dataCenter = model::DataCenter::getInstance();
    connect(dataCenter, &model::DataCenter::exitGroupChatSessionDone, this, &GroupSessionDetailWidget::exitGroupChatSessionDone, Qt::UniqueConnection);
    dataCenter->exitGroupChatSessionAsync(dataCenter->getCurrentChatSessionId());
}

void GroupSessionDetailWidget::exitGroupChatSessionDone()
{
    this->close();
}

void GroupSessionDetailWidget::clickSubmitBtn()
{
    // 1. 输入框中拿到修改后的名称
    const QString& groupname = groupNameEdit->text();
    if(groupname.isEmpty())
        return;

    // 2. 发送网络请求
    model::DataCenter* dataCenter = model::DataCenter::getInstance();
    connect(dataCenter, &model::DataCenter::changeGroupnameDone, this, &GroupSessionDetailWidget::clickSubmitBtnDone, Qt::UniqueConnection);
    dataCenter->changeGroupnameAsync(chatSessionInfo->chatSessionId, groupname);
}

void GroupSessionDetailWidget::clickSubmitBtnDone(const QString& chatSessionId, const QString& groupname)
{
    hlayout->removeWidget(groupNameEdit);
    groupNameEdit->hide();
    hlayout->addWidget(groupNameLabel);
    groupNameLabel->show();
    groupNameLabel->setText(groupname);

    hlayout->removeWidget(submitBtn);
    submitBtn->hide();
    hlayout->addWidget(modifyBtn);
    modifyBtn->show();
}
