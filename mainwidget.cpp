#include "mainwidget.h"
#include "./ui_mainwidget.h"
#include "sessionfriendarea.h"
#include "selfinfowidget.h"
#include "sessiondetailwidget.h"
#include "groupsessiondetailwidget.h"
#include "addfrienddialog.h"
#include "debug.h"
#include "model/datacenter.h"
#include "toast.h"

#include <QHBoxLayout>
#include <QVBoxLayout>


MainWidget* MainWidget::instance = nullptr;
MainWidget* MainWidget::getInstance()
{
    if (instance == nullptr)
    {
        // 以桌面为父窗口
        instance = new MainWidget();
    }
    return instance;
}

MainWidget::MainWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainWidget)
{
    ui->setupUi(this);
    this->setWindowTitle("我的聊天室");
    this->setWindowIcon(QIcon(":/resource/image/logo.png"));

    // 1. 初始化整体窗⼝布局
    initMainWindow();
    // 2. 初始化左侧
    initLeftWindow();
    // 3. 初始化中间
    initMidWindow();
    // 4. 初始化右侧
    initRightWindow();
    // 5. 初始化信号槽
    initSignalSlot();
}

MainWidget::~MainWidget()
{
    delete ui;
}
void MainWidget::initMainWindow()
{
    QHBoxLayout* layout = new QHBoxLayout();
    // 设置 layout 内部元素距离
    layout->setSpacing(0);
    // 设置 layout 内部元素距离四个边界的距离
    layout->setContentsMargins(0, 0, 0, 0);
    this->setLayout(layout);

    windowLeft = new QWidget();
    windowMid = new QWidget();
    windowRight = new QWidget();

    windowLeft->setFixedWidth(70);
    windowMid->setFixedWidth(230);
    windowRight->setMinimumWidth(800);

    windowLeft->setStyleSheet("QWidget { background-color: rgb(46, 46, 46); }");
    windowMid->setStyleSheet("QWidget { background-color: rgb(247, 247, 247); }");
    windowRight->setStyleSheet("QWidget { background-color: rgb(245, 245, 245); }");

    layout->addWidget(windowLeft);
    layout->addWidget(windowMid);
    layout->addWidget(windowRight);
}

void MainWidget::initLeftWindow()
{
    QVBoxLayout* layout = new QVBoxLayout();
    layout->setSpacing(20);
    layout->setContentsMargins(0, 50, 0, 0);
    windowLeft->setLayout(layout);

    // 添加 用户头像
    userAvatar = new QPushButton();
    userAvatar->setFixedSize(45, 45);
    userAvatar->setIconSize(QSize(38, 38));
    //userAvatar->setIcon(QIcon(":/resource/image/defaultAvatar.jpg"));
    userAvatar->setStyleSheet("QPushButton { background-color: transparent; }");
    layout->addWidget(userAvatar, 1, Qt::AlignTop | Qt::AlignHCenter);

    // 添加 会话标签按钮
    sessionTabBtn = new QPushButton();
    sessionTabBtn->setFixedSize(45, 45);
    sessionTabBtn->setIconSize(QSize(30, 30));
    sessionTabBtn->setIcon(QIcon(":/resource/image/session_active.png"));
    sessionTabBtn->setStyleSheet("QPushButton { background-color: transparent; }");
    layout->addWidget(sessionTabBtn, 1, Qt::AlignTop | Qt::AlignHCenter);

    // 添加 好友标签页按钮
    friendTabBtn = new QPushButton();
    friendTabBtn->setFixedSize(45, 45);
    friendTabBtn->setIconSize(QSize(30, 30));
    friendTabBtn->setIcon(QIcon(":/resource/image/friend_inactive.png"));
    friendTabBtn->setStyleSheet("QPushButton { background-color: transparent; }");
    layout->addWidget(friendTabBtn, 1, Qt::AlignTop | Qt::AlignHCenter);

    // 添加 好友申请标签页按钮
    applyTabBtn = new QPushButton();
    applyTabBtn->setFixedSize(45, 45);
    applyTabBtn->setIconSize(QSize(30, 30));
    applyTabBtn->setIcon(QIcon(":/resource/image/apply_inactive.png"));
    applyTabBtn->setStyleSheet("QPushButton { background-color: transparent; }");
    layout->addWidget(applyTabBtn, 1, Qt::AlignTop | Qt::AlignHCenter);

    layout->addStretch(20);
}

void MainWidget::initMidWindow()
{
    QGridLayout* layout = new QGridLayout();
    layout->setContentsMargins(0, 20, 0, 0);
    layout->setHorizontalSpacing(0);
    layout->setVerticalSpacing(10);
    windowMid->setLayout(layout);

    searchEdit = new QLineEdit();
    searchEdit->setFixedHeight(30);
    searchEdit->setPlaceholderText("搜索");
    searchEdit->setStyleSheet("QLineEdit { border-radius: 5px; background-color: rgb(226, 226, 226);\
                              padding-left: 5px;}");

    addFriendBtn = new QPushButton();
    addFriendBtn->setFixedSize(30, 30);
    addFriendBtn->setIconSize(QSize(30, 30));
    addFriendBtn->setIcon(QIcon(":/resource/image/cross.png"));
    addFriendBtn->setStyleSheet("QPushButton { border-radius: 5px; background-color: rgb(226, 226, 226);}\
    QPushButton::hover { background-color: rgb(200, 200, 200); }");

    sessionFriendArea = new SessionFriendArea();

    // 为了更灵活的控制边距，只影响搜索框按钮这一行，不影响下方列表这一行
    // 创建空白 widget 填充到布局管理器

    QWidget* spacer1 = new QWidget();
    spacer1->setFixedSize(10, 30);
    QWidget* spacer2 = new QWidget();
    spacer2->setFixedSize(10, 30);
    QWidget* spacer3 = new QWidget();
    spacer3->setFixedSize(10, 30);

    layout->addWidget(spacer1, 0, 0);
    layout->addWidget(searchEdit, 0, 1);
    layout->addWidget(spacer2, 0, 2);
    layout->addWidget(addFriendBtn, 0, 3);
    layout->addWidget(spacer3, 0, 4);
    layout->addWidget(sessionFriendArea, 1, 0, 1, 5);
}

void MainWidget::initRightWindow()
{
    // 1. 创建右侧窗口布局管理器
    QVBoxLayout* vlayout = new QVBoxLayout();
    vlayout->setContentsMargins(0, 0, 0, 0);
    vlayout->setSpacing(0);
    vlayout->setAlignment(Qt::AlignTop);
    windowRight->setLayout(vlayout);

    // 2. 创建上方标题栏
    QWidget* titleWidget = new QWidget();
    titleWidget->setFixedHeight(62);
    titleWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    titleWidget->setObjectName("titleWidget");
    titleWidget->setStyleSheet("#titleWidget { border-bottom: 1px solid rgb(230, 230, 230); border-left: 1px solid rgb(230, 230, 230) }");
    vlayout->addWidget(titleWidget);

    // 3. 给标题栏，添加标题 label 和 一个按钮
    QHBoxLayout* hlayout = new QHBoxLayout();
    hlayout->setContentsMargins(15, 0, 15, 0);
    hlayout->setSpacing(0);
    titleWidget->setLayout(hlayout);

    sessionTitleLabel = new QLabel();
    sessionTitleLabel->setStyleSheet("QLabel { font-size: 22px; font-weight: 600; font-family: 'SimSun'; border-bottom: 1px solid rgb(230, 230, 230);}");

    // 临时测试
#if TEST_UI
    sessionTitleLabel->setText("这是会话标题");
#endif

    extraBtn = new QPushButton();
    extraBtn->setFixedSize(30, 30);
    extraBtn->setIconSize(QSize(30, 30));
    extraBtn->setIcon(QIcon(":/resource/image/more.png"));
    extraBtn->setStyleSheet("QPushButton { border: none; }"
                            "QPushButton:pressed { background-color: rgb(245, 245, 245) }");
    hlayout->addWidget(sessionTitleLabel);
    hlayout->addWidget(extraBtn);

    // 4. 添加消息展示区
    messageShowArea = new MessageShowArea();
    vlayout->addWidget(messageShowArea);

    // 5. 添加消息编辑区
    messageEditArea = new MessageEditArea();
    // 确保消息编辑区处于窗口下方
    vlayout->addWidget(messageEditArea, 0, Qt::AlignBottom);
}

void MainWidget::initSignalSlot()
{
    model::DataCenter* dataCenter = model::DataCenter::getInstance();
    dataCenter->initWebSocket();
    //dataCenter->ping();

    //////////////////////////////////////////////////
    // 连接信号槽，处理标签页按钮切换问题
    //////////////////////////////////////////////////
    connect(sessionTabBtn, &QPushButton::clicked, this, &MainWidget::switchTabToSession);
    connect(friendTabBtn, &QPushButton::clicked, this, &MainWidget::switchTabToFriend);
    connect(applyTabBtn, &QPushButton::clicked, this, &MainWidget::switchTabToApply);

    //////////////////////////////////////////////////
    // 点击头像，显示个人主页
    //////////////////////////////////////////////////
    connect(userAvatar, &QPushButton::clicked, this, [=](){
        SelfInfoWidget* selfInfoWidget = new SelfInfoWidget(this);
        selfInfoWidget->exec(); // 弹出模态对话框
        //selfInfoWidget->show(); // 弹出非模态对话框
    });

    //////////////////////////////////////////////////
    // 点击会话详情按钮，弹出会话详情窗口
    //////////////////////////////////////////////////
    connect(extraBtn, &QPushButton::clicked, this, [=](){
        // 判断当前会话是单聊还是群聊

// #if TEST_GROUP_SESSION_DETAIL
//         bool isSingleChat = false;
// #else
//         bool isSingleChat = true;
// #endif

        // 获取到当前会话的详细信息，通过会话中的 userId 属性
        model::ChatSessionInfo* chatSessionInfo = dataCenter->findChatSessionById(dataCenter->getCurrentChatSessionId());
        if(chatSessionInfo == nullptr)
        {
            LOG()<<"当前会话不存在，无法弹出会话详情对话框";
            return;
        }
        bool isSingleChat = chatSessionInfo->userId != "";
        if(isSingleChat)
        {
            // 单聊
            model::UserInfo* userInfo = dataCenter->findFriendById(chatSessionInfo->userId);
            if(userInfo == nullptr)
            {
                LOG()<<"单聊会话对应的用户不存在，无法弹出会话详情窗口";
                Toast::showMessage("该用户已经不是你的好友了");
            }

            SessionDetailWidget* sessionDetailWidget = new SessionDetailWidget(this, *userInfo);
            sessionDetailWidget->exec();
        }
        else
        {
            // 群聊
            GroupSessionDetailWidget* groupSessionDetailWidget = new GroupSessionDetailWidget(this);
            groupSessionDetailWidget->exec();
        }

    });

    //////////////////////////////////////////////////
    // 点击添加好友按钮，弹出添加好友窗口
    //////////////////////////////////////////////////
    connect(addFriendBtn, &QPushButton::clicked, this, [=](){
        AddFriendDialog* addFriendDialog = new AddFriendDialog(this);
        addFriendDialog->exec();
    });

    //////////////////////////////////////////////////
    // 修改搜索框内容，弹出添加好友窗口
    //////////////////////////////////////////////////
    connect(searchEdit, &QLineEdit::textEdited, this, [=](){
       // 取出当前输入框的内容，设置到新弹框的输入框里面
        const QString& searchKey = searchEdit->text();
        AddFriendDialog* addFriendDialog = new AddFriendDialog(this);
        addFriendDialog->setSearchKey(searchKey);
        searchEdit->setText("");
        addFriendDialog->exec();
    });

    //////////////////////////////////////////////////
    // 获取个人信息
    //////////////////////////////////////////////////
    connect(dataCenter, &model::DataCenter::getMyselfDone, this, [=](){
        // 从 DataCenter 中拿到响应结果的 myself，把里面的头像取出来，显示到界面上
        model::UserInfo* myself = dataCenter->getMyself();
        userAvatar->setIcon(myself->avatar);
    });
    dataCenter->getMyselfAsync();

    //////////////////////////////////////////////////
    // 获取好友列表
    //////////////////////////////////////////////////
    connect(dataCenter, &model::DataCenter::getFriendListDone, this, &MainWidget::updateFriendList);
    loadFriendList();

    //////////////////////////////////////////////////
    // 获取会话列表
    //////////////////////////////////////////////////
    connect(dataCenter, &model::DataCenter::getChatSessionListDone, this, &MainWidget::updateChatSessionList);
    loadSessionList();

    //////////////////////////////////////////////////
    // 获取好友申请列表
    //////////////////////////////////////////////////
    connect(dataCenter, &model::DataCenter::getApplyListDone, this, &MainWidget::updateApplyList);
    loadApplyList();

    //////////////////////////////////////////////////
    // 处理修改头像
    //////////////////////////////////////////////////
    connect(dataCenter, &model::DataCenter::changeAvatarDone, this, [=](){
        model::UserInfo* myself = dataCenter->getMyself();
        userAvatar->setIcon(myself->avatar);
    });

    //////////////////////////////////////////////////
    // 处理删除好友
    //////////////////////////////////////////////////
    connect(dataCenter, &model::DataCenter::deleteFriendDone, this, [=](){
        this->updateFriendList();
        this->updateChatSessionList();
        LOG()<<"删除好友完成";
    });
    connect(dataCenter, &model::DataCenter::clearCurrentSession, this, [=](){
        sessionTitleLabel->setText("");
        messageShowArea->clear();
        dataCenter->setCurrentChatSessionId("");
        LOG()<<"清空当前会话完成";
    });

    //////////////////////////////////////////////////
    // 处理添加好友申请
    //////////////////////////////////////////////////
    connect(dataCenter, &model::DataCenter::addFriendApplyDone, this, [=](){
        Toast::showMessage("好友申请已发送");
    });

    //////////////////////////////////////////////////
    // 处理添加好友申请推送数据
    //////////////////////////////////////////////////
    connect(dataCenter, &model::DataCenter::receiveFriendApplyDone, this, [=](){
        this->updateApplyList();
        Toast::showMessage("收到新的好友申请");
    });

    //////////////////////////////////////////////////
    // 处理同意好友申请
    //////////////////////////////////////////////////
    connect(dataCenter, &model::DataCenter::acceptFriendApplyDone, this, [=](const QString& nickName){
        this->updateApplyList();
        this->updateFriendList();
        Toast::showMessage("您已与"+nickName+"成为好友");
    });
    connect(dataCenter, &model::DataCenter::updateApplyListUI, this, [=](){
        this->updateApplyList();
    });

    //////////////////////////////////////////////////
    // 处理拒绝好友申请
    //////////////////////////////////////////////////
    connect(dataCenter, &model::DataCenter::rejectFriendApplyDone, this, [=](){
        this->updateApplyList();
    });

    //////////////////////////////////////////////////
    // 处理好友申请结果的推送数据
    //////////////////////////////////////////////////
    connect(dataCenter, &model::DataCenter::receiveFriendProcessDone, this, [=](const QString& nickName, bool agree){
        if(agree)
        {
            // 同意
            this->updateFriendList();
            Toast::showMessage(nickName+"同意了你的好友申请");
        }
        else
        {
            // 拒绝
            Toast::showMessage(nickName+"拒绝了你的好友申请");
        }
    });

    //////////////////////////////////////////////////
    // 处理创建群聊的响应信号
    //////////////////////////////////////////////////
    connect(dataCenter, &model::DataCenter::createGroupChatSessionDone, this, [=](){
        Toast::showMessage("创建群聊会话请求已经发送");
    });

    //////////////////////////////////////////////////
    // 处理创建会话的推送数据
    //////////////////////////////////////////////////
    connect(dataCenter, &model::DataCenter::receiveSessionCreateDone, this, [=](){
        this->updateChatSessionList();
        Toast::showMessage("您被拉入到新的群聊中");
    });
}

void MainWidget::switchTabToSession()
{
    // 1. 记录当前切换到的标签页
    activeTab = SESSION_LIST;
    // 2. 调整图标显示情况，把会话按钮图标设为 active ，另外两个图标设为 inactive
    sessionTabBtn->setIcon(QIcon(":/resource/image/session_active.png"));
    friendTabBtn->setIcon(QIcon(":/resource/image/friend_inactive.png"));
    applyTabBtn->setIcon(QIcon(":/resource/image/apply_inactive.png"));
    // 3. 再主窗口的中间部分，加载出会话列表数据
    loadSessionList();
}

void MainWidget::switchTabToFriend()
{
    // 1. 记录当前切换到的标签页
    activeTab = FRIEND_LIST;
    // 2. 调整图标显示情况，把会话按钮图标设为 active ，另外两个图标设为 inactive
    sessionTabBtn->setIcon(QIcon(":/resource/image/session_inactive.png"));
    friendTabBtn->setIcon(QIcon(":/resource/image/friend_active.png"));
    applyTabBtn->setIcon(QIcon(":/resource/image/apply_inactive.png"));
    // 3. 再主窗口的中间部分，加载出好友列表数据
    loadFriendList();
}

void MainWidget::switchTabToApply()
{
    // 1. 记录当前切换到的标签页
    activeTab = APPLY_LIST;
    // 2. 调整图标显示情况，把会话按钮图标设为 active ，另外两个图标设为 inactive
    sessionTabBtn->setIcon(QIcon(":/resource/image/session_inactive.png"));
    friendTabBtn->setIcon(QIcon(":/resource/image/friend_inactive.png"));
    applyTabBtn->setIcon(QIcon(":/resource/image/apply_active.png"));
    // 3. 再主窗口的中间部分，加载出好友申请列表数据
    loadApplyList();
}

void MainWidget::loadSessionList()
{
    // 会话列表在 DataCenter 中存储
    // 判定 DataCenter 中是否已经有数据，如果有，直接从本地加载
    // 如果没有数据，从服务器获取
    model::DataCenter* dataCenter = model::DataCenter::getInstance();
    if(dataCenter->getSessionList() != nullptr)
    {
        // 从内存加载
        updateChatSessionList();
    }
    else
    {
        // 通过网络加载
        dataCenter->getSessionListAsync();
    }

}

void MainWidget::loadFriendList()
{
    // 好友列表在 DataCenter 中存储
    // 判定 DataCenter 中是否已经有数据，如果有，直接从本地加载
    // 如果没有数据，从服务器获取
    model::DataCenter* dataCenter = model::DataCenter::getInstance();
    if(dataCenter->getFriendList() != nullptr)
    {
        // 从内存加载
        updateFriendList();
    }
    else
    {
        // 通过网络加载
        // connect(dataCenter, &model::DataCenter::getFriendListDone, this, &MainWidget::updateFriendList, Qt::UniqueConnection);
        dataCenter->getFriendListAsync();
    }
}

void MainWidget::loadApplyList()
{
    // 好友申请列表在 DataCenter 中存储
    // 判定 DataCenter 中是否已经有数据，如果有，直接从本地加载
    // 如果没有数据，从服务器获取
    model::DataCenter* dataCenter = model::DataCenter::getInstance();
    if(dataCenter->getApplyList() != nullptr)
    {
        // 从内存加载
        updateApplyList();
    }
    else
    {
        // 通过网络加载
        // connect(dataCenter, &model::DataCenter::getFriendListDone, this, &MainWidget::updateFriendList, Qt::UniqueConnection);
        dataCenter->getApplyListAsync();
    }
}

void MainWidget::updateFriendList()
{
    // 判断当前标签页是否是好友列表
    if(activeTab != FRIEND_LIST)
        return;

    model::DataCenter* dataCenter = model::DataCenter::getInstance();
    QList<model::UserInfo>* friendList = dataCenter->getFriendList();

    // 清空之前界面的数据
    sessionFriendArea->clear();

    // 遍历好友列表，添加到界面上
    for(const model::UserInfo& f : *friendList)
    {
        sessionFriendArea->addItem(FreiendItemType, f.userId, f.avatar, f.nickname, f.description);
    }
}

void MainWidget::updateChatSessionList()
{
    // 判断当前标签页是否是会话列表
    if(activeTab != SESSION_LIST)
        return;

    model::DataCenter* dataCenter = model::DataCenter::getInstance();
    QList<model::ChatSessionInfo>* chatSessionList = dataCenter->getSessionList();

    // 清空之前界面的数据
    sessionFriendArea->clear();

    // 遍历会话列表，添加到界面上
    for(const model::ChatSessionInfo& c : *chatSessionList)
    {
        QString text;
        if(c.lastMessage.messageType == model::TEXT_TYPE)
            text = c.lastMessage.content;
        else if(c.lastMessage.messageType == model::IMAGE_TYPE)
            text = "[图片]";
        else if(c.lastMessage.messageType == model::FILE_TYPE)
            text = "[文件]";
        else if(c.lastMessage.messageType == model::SPEECH_TYPE)
            text = "[语音]";
        else
        {
            LOG() << "错误的消息类型！messageType = " << c.lastMessage.messageType;
            continue;
        }
        sessionFriendArea->addItem(SessionItemType, c.chatSessionId, c.avatar, c.chatSessionName, text);
    }
}

void MainWidget::updateApplyList()
{
    // 判断当前标签页是否是好友申请列表
    if(activeTab != APPLY_LIST)
        return;

    model::DataCenter* dataCenter = model::DataCenter::getInstance();
    QList<model::UserInfo>* applyList = dataCenter->getApplyList();

    // 清空之前界面的数据
    sessionFriendArea->clear();

    // 遍历好友列表，添加到界面上
    for(const model::UserInfo& u : *applyList)
    {
        sessionFriendArea->addItem(ApplyItemType, u.userId, u.avatar, u.nickname);
    }
}

void MainWidget::updateRecentMessage(const QString &chatSessionId)
{
    // 1. 拿到该会话的最近消息列表
    model::DataCenter* dataCenter = model::DataCenter::getInstance();
    QList<model::Message>* recentMessageList = dataCenter->getRecentMessageList(chatSessionId);

    // 2. 清空之前界面的数据
    messageShowArea->clear();

    // 3. 在界面上显示，头插
    for(int i = recentMessageList->size() - 1; i >= 0; --i)
    {
        const model::Message& message = recentMessageList->at(i);
        bool isLeft = message.sender.userId != dataCenter->getMyself()->userId;
        messageShowArea->addFrontMessage(isLeft, message);
    }

    // 4. 设置会话标题
    model::ChatSessionInfo* chatSessionInfo = dataCenter->findChatSessionById(chatSessionId);
    if(chatSessionInfo != nullptr)
    {
        sessionTitleLabel->setText(chatSessionInfo->chatSessionName);
    }

    // 5. 保存当前选中的会话是哪个
    dataCenter->setCurrentChatSessionId(chatSessionId);

    // 6. 自动把滚动条滚动到末尾
    messageShowArea->scrollToEnd();
}

void MainWidget::loadRecentMessage(const QString &chatSessionId)
{
    model::DataCenter* dataCenter = model::DataCenter::getInstance();
    if( dataCenter->getRecentMessageList(chatSessionId) != nullptr)
    {
        // 用本地数据更新
        updateRecentMessage(chatSessionId);
    }
    else
    {
        // 从网络上加载
        connect(dataCenter, &model::DataCenter::getRecentMessageListDone, this, &MainWidget::updateRecentMessage, Qt::UniqueConnection);
        dataCenter->getRecentMessageListAsync(chatSessionId, true);
    }
}

void MainWidget::switchSession(const QString &userId)
{
    // 1. 在会话列表中找到对应的会话元素
    model::DataCenter* dataCenter = model::DataCenter::getInstance();
    model::ChatSessionInfo* chatSessionInfo = dataCenter->findChatSessionByUserId(userId);
    if(chatSessionInfo == nullptr)
    {
        LOG()<<"[严重错误] 当前选中的好友，对应的会话不存在";
        return;
    }

    // 2. 把选中的会话置顶
    dataCenter->topChatSessionInfo(*chatSessionInfo);

    // 3. 切换到会话列表标签页
    switchTabToSession();

    // 4. 加载这个会话对应的历史消息
    sessionFriendArea->clickItem(0);
}

MessageShowArea *MainWidget::getMessageShowArea()
{
    return messageShowArea;
}
