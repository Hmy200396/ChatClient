
#include "addfrienddialog.h"
#include <QGridLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QLabel>
#include "debug.h"
#include "model/datacenter.h"

////////////////////////////////
/// 表示一个好友搜索的结果
////////////////////////////////
FriendResultItem::FriendResultItem(const model::UserInfo &userInfo)
    :userInfo(userInfo)
{
    // 1. 设置基本属性
    this->setFixedHeight(70);
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    // 2. 创建布局管理器
    QGridLayout* layout = new QGridLayout();
    layout->setContentsMargins(0, 0, 20, 0);
    layout->setHorizontalSpacing(10);
    layout->setVerticalSpacing(0);
    this->setLayout(layout);

    // 3. 创建头像
    QPushButton* avatarBtn = new QPushButton();
    avatarBtn->setFixedSize(50, 50);
    avatarBtn->setIconSize(QSize(50, 50));
    avatarBtn->setIcon(QIcon(userInfo.avatar));
    //avatarBtn->setIcon(QIcon(":/resource/image/defaultAvatar.jpg"));

    // 4. 创建用户昵称
    QLabel* nameLabel = new QLabel();
    nameLabel->setFixedHeight(35);
    nameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    nameLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    nameLabel->setStyleSheet("QLabel { font-size: 16px; font-weight: 700; color: black; }");
    nameLabel->setText(userInfo.nickname);

    // 5. 创建个性签名
    QLabel* descLabel = new QLabel();
    descLabel->setFixedHeight(35);
    descLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    descLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    descLabel->setStyleSheet("QLabel { font-size: 14px; color: black; }");
    descLabel->setText(userInfo.description);

    // 6. 创建添加好友按钮
    addBtn = new QPushButton();
    addBtn->setFixedSize(100, 40);
    addBtn->setText("添加好友");
    QString btnStyle = "QPushButton { border:none; background-color: rgb(137, 217, 97); color: rgb(255, 255, 255); border-radius: 10px; }";
    btnStyle += "QPushButton:pressed { background-color: rgb(200, 200, 200); }";
    addBtn->setStyleSheet(btnStyle);

    // 7. 上述内容添加到布局管理器
    layout->addWidget(avatarBtn, 0, 0, 2, 1);
    layout->addWidget(nameLabel, 0, 1);
    layout->addWidget(descLabel, 1, 1);
    layout->addWidget(addBtn, 0, 2, 2, 1);

    // 8. 连接信号槽
    connect(addBtn, &QPushButton::clicked, this, &FriendResultItem::clickAddBtn);
}

void FriendResultItem::clickAddBtn()
{
    // 1. 发送好友申请
    model::DataCenter* dataCenter = model::DataCenter::getInstance();
    dataCenter->addFriendApplyAsync(userInfo.userId);

    // 2. 设置按钮为禁用状态
    addBtn->setEnabled(false);
    addBtn->setText("已申请");
    addBtn->setStyleSheet("QPushButton{ border:none; background-color: rgb(89, 180, 180); color: rgb(255, 255, 255); border-radius: 10px; }");
}



////////////////////////////////
/// 整个搜索好友的窗口
////////////////////////////////
AddFriendDialog::AddFriendDialog(QWidget* parent)
    :QDialog(parent)
{
    // 1. 设置基本属性
    this->setFixedSize(500, 500);
    this->setWindowTitle("添加好友");
    this->setWindowIcon(QIcon(":/resource/image/logo.png"));
    this->setStyleSheet("QDialog { background-color: rgb(255, 255, 255); }");
    this->setAttribute(Qt::WA_DeleteOnClose);

    // 2. 添加布局管理器
    layout = new QGridLayout();
    layout->setContentsMargins(20, 20, 20, 0);
    layout->setSpacing(10);
    this->setLayout(layout);

    // 3. 创建搜索框
    searchEdit = new QLineEdit();
    searchEdit->setFixedHeight(50);
    searchEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    QString editStyle = "QLineEdit { border:none; border-radius: 10px; font-size: 16px; background-color: rgb(240, 240 ,240); padding-left: 5px; color: black}"
                        "QLineEdit::placeholder { color: gray; }";
    searchEdit->setStyleSheet(editStyle);
    searchEdit->setPlaceholderText("按手机号/用户序号/昵称搜索");
    layout->addWidget(searchEdit, 0, 0, 1, 8);

    // 4. 创建搜索按钮
    QPushButton* searchbtn = new QPushButton();
    searchbtn->setFixedSize(50, 50);
    searchbtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    searchbtn->setIconSize(QSize(30, 30));
    searchbtn->setIcon(QIcon(":/resource/image/search.png"));
    QString btnStyle = "QPushButton { border: none; background-color: rgb(240, 240 ,240); border-radius: 10px;}";
    btnStyle += "QPushButton:hover { background-color: rgb(220, 220 ,220); }";
    btnStyle += "QPushButton:pressed { background-color: rgb(200, 200 ,200); }";
    searchbtn->setStyleSheet(btnStyle);
    layout->addWidget(searchbtn, 0, 8, 1, 1);

    // 5. 添加滚动区域
    initResultArea();

    // 6构造数据逻辑
#if TEST_UI
    for(int i = 0; i < 20; ++i)
    {
        model::UserInfo* userInfo = new model::UserInfo();
        userInfo->userId = QString::number(1000+i);
        userInfo->nickname = "琅琅" + QString::number(i);
        userInfo->description = "小蛋糕大王";
        userInfo->avatar = QIcon(":/resource/image/defaultAvatar.jpg");
        addResult(*userInfo);
    }
#endif

    // 6. 连接信号槽
    connect(searchbtn, &QPushButton::clicked, this, &AddFriendDialog::clickSearchBtn);
}

void AddFriendDialog::initResultArea()
{
    // 1. 创建滚动区域对象
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
    layout->addWidget(scrollArea, 1, 0, 1, 9);

    // 2. 创建 QWidget
    resultContainer = new QWidget();
    resultContainer->setObjectName("resultContainer");
    resultContainer->setStyleSheet("#resultContainer { background-color: rgb(255, 255, 255); }");
    scrollArea->setWidget(resultContainer);

    // 3. 给 QWidget 里面添加元素
    QVBoxLayout* vlayout = new QVBoxLayout();
    vlayout->setContentsMargins(0, 0, 0, 0);
    vlayout->setSpacing(0);
    resultContainer->setLayout(vlayout);
}

void AddFriendDialog::addResult(const model::UserInfo &userInfo)
{
    FriendResultItem* item = new FriendResultItem(userInfo);
    resultContainer->layout()->addWidget(item);
}

void AddFriendDialog::clear()
{
    QVBoxLayout* layout = dynamic_cast<QVBoxLayout*>(resultContainer->layout());
    for(int i = layout->count() - 1; i>=0; --i)
    {
        QLayoutItem* item = layout->takeAt(i);
        if(item && item->widget())
            delete item->widget();
        // if (item)
        // {
        //     if (item->widget())
        //     {
        //         delete item->widget();
        //     }
        //     delete item;
        // }
    }
}

void AddFriendDialog::setSearchKey(const QString &searchKey)
{
    searchEdit->setText(searchKey);
}

void AddFriendDialog::clickSearchBtn()
{
    // 1. 拿到输入框的内容
    const QString& text = searchEdit->text();
    if(text.isEmpty())
        return;

    // 2. 给服务器发起请求
    model::DataCenter* dataCenter = model::DataCenter::getInstance();
    connect(dataCenter, &model::DataCenter::searchUserDone, this, &AddFriendDialog::clickSearchBtnDone, Qt::UniqueConnection);
    dataCenter->searchUserAsync(text);
}

void AddFriendDialog::clickSearchBtnDone()
{
    // 1. 拿到 DataCenter 中的搜索结果
    model::DataCenter* dataCenter = model::DataCenter::getInstance();
    QList<model::UserInfo>* searchResult = dataCenter->getSearchUserResult();
    if(searchResult == nullptr)
        return;

    this->clear();

    for(const auto& u : *searchResult)
    {
        this->addResult(u);
    }
}


