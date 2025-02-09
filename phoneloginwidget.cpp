#include "phoneloginwidget.h"
#include "loginwidget.h"
#include <QGridLayout>
#include <QLabel>
#include <QRegularExpression>
#include <QLineEdit>
#include <QPushButton>
#include "model/datacenter.h"
#include "toast.h"
#include "mainwidget.h"


PhoneLoginWidget::PhoneLoginWidget(bool isLoginMode, QWidget *parent)
    : isLoginMode(!isLoginMode),QWidget{parent}
{
    // 1. 设置窗口基本属性
    this->setFixedSize(400, 350);
    this->setWindowTitle("登录");
    this->setWindowIcon(QIcon(":/resource/image/logo.png"));
    this->setStyleSheet("QWidget { background-color: rgb(255, 255, 255); }");
    this->setAttribute(Qt::WA_DeleteOnClose);

    // 2. 创建布局管理器
    QGridLayout* layout = new QGridLayout();
    layout->setSpacing(0);
    layout->setContentsMargins(50, 0, 50, 0);
    this->setLayout(layout);

    // 3. 创建标题
    titleLabel = new QLabel();
    //titleLabel->setText("登录");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setFixedHeight(50);
    titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    titleLabel->setStyleSheet("QLabel { font-size: 40px; font-weight: 600; color: black; }");

    // 4. 创建手机号码输入框
    phoneEdit = new QLineEdit();
    phoneEdit->setPlaceholderText("输入手机号码");
    phoneEdit->setFixedHeight(40);
    phoneEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    QString editStyle = "QLineEdit { border: none; border-radius: 10px; font-size: 15px; background-color: rgb(240, 240, 240); padding-left: 5px; color: black; }"
                        "QLineEdit::placeholder { color: gray; }";
    phoneEdit->setStyleSheet(editStyle);

    // 5. 创建验证码输入框
    verifyCodeEdit = new QLineEdit();
    verifyCodeEdit->setFixedHeight(40);
    verifyCodeEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    verifyCodeEdit->setPlaceholderText("输入短信验证码");
    verifyCodeEdit->setStyleSheet(editStyle);

    // 6. 创建发送验证码按钮
    sendVerifyCodeBtn = new QPushButton();
    sendVerifyCodeBtn->setFixedSize(100, 40);
    sendVerifyCodeBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    sendVerifyCodeBtn->setText("发送验证码");
    QString btnWhiteStyle = "QPushButton { border: none; border-radius: 10px; background-color: transparent; color: black; }";
    btnWhiteStyle += "QPushButton:pressed { background-color: rgb(240, 240, 240); }";
    sendVerifyCodeBtn->setStyleSheet(btnWhiteStyle);

    // 7. 创建提交按钮
    submitBtn = new QPushButton();
    submitBtn->setFixedHeight(40);
    submitBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    //submitBtn->setText("登录");
    QString btnGreenStyle = "QPushButton { border: none; border-radius: 10px; background-color: rgb(44, 182, 61); color: rgb(255, 255, 255); }";
    btnGreenStyle += "QPushButton:pressed { background-color: rgb(240, 240, 240); }";
    submitBtn->setStyleSheet(btnGreenStyle);

    // 8. 创建 “切换到用户名” 模式按钮
    QPushButton* userModeBtn = new QPushButton();
    userModeBtn->setFixedSize(100, 40);
    userModeBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    userModeBtn->setText("切换到用户名");
    userModeBtn->setStyleSheet(btnWhiteStyle);

    // 9. 创建切换模式（登录和注册）按钮
    switchModeBtn = new QPushButton();
    switchModeBtn->setFixedSize(100, 40);
    switchModeBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    //switchModeBtn->setText("注册");
    switchModeBtn->setStyleSheet(btnWhiteStyle);

    // 10. 添加到布局管理器中
    layout->addWidget(titleLabel, 0, 0, 1, 5);
    layout->addWidget(phoneEdit, 1, 0, 1, 5);
    layout->addWidget(verifyCodeEdit, 2, 0, 1, 4);
    layout->addWidget(sendVerifyCodeBtn, 2, 4, 1, 1);
    layout->addWidget(submitBtn, 3, 0, 1, 5);
    layout->addWidget(userModeBtn, 4, 0, 1, 1);
    layout->addWidget(switchModeBtn, 4, 4, 1, 1);

    // 11. 添加信号槽
    connect(switchModeBtn, &QPushButton::clicked, this, &PhoneLoginWidget::switchMode);

    connect(userModeBtn, &QPushButton::clicked, this, [=](){
        LoginWidget* loginWidget = new LoginWidget();
        loginWidget->show();

        // 关闭当前窗口
        this->close();
    });

    connect(sendVerifyCodeBtn, &QPushButton::clicked, this, &PhoneLoginWidget::sendVerifyCode);

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &PhoneLoginWidget::countDown);

    connect(submitBtn, &QPushButton::clicked, this, &PhoneLoginWidget::clickSubmitBtn);

    switchModeBtn->click();
}

void PhoneLoginWidget::sendVerifyCode()
{
    // 1. 获取到手机号码
    const QString& phone = this->phoneEdit->text();
    if(phone.isEmpty())
        return;
    if(!isPhoneNumberValid(phone))
    {
        Toast::showMessage("请输入合法的手机号");
        return;
    }
    this->currentPhone = phone;

    // 2. 开启定时器
    timer->start(1000);

    // 3. 发送网络请求，获取到验证码
    model::DataCenter* dataCenter = model::DataCenter::getInstance();
    connect(dataCenter, &model::DataCenter::getVerifyCodeDone, this, &PhoneLoginWidget::sendVerifyCodeDone, Qt::UniqueConnection);
    dataCenter->getVerifyCodeAsync(phone);
}

void PhoneLoginWidget::sendVerifyCodeDone()
{
    Toast::showMessage("验证码已经发送");
}

void PhoneLoginWidget::clickSubmitBtn()
{
    // 1. 从输入框中拿到必要的内容
    const QString& phone = this->currentPhone;
    const QString& verifyCode = verifyCodeEdit->text();
    if(phone.isEmpty())
    {
        Toast::showMessage("请输入手机号并点击发送验证码");
        return;
    }
    if(verifyCode.isEmpty())
    {
        Toast::showMessage("电话或者验证码不应该为空!");
        return;
    }

    // 2. 发送请求
    model::DataCenter* dataCenter = model::DataCenter::getInstance();
    if(isLoginMode)
    {
        // 登录
        connect(dataCenter, &model::DataCenter::phoneLoginDone, this, &PhoneLoginWidget::phoneLoginDone, Qt::UniqueConnection);
        dataCenter->phoneLoginAsync(phone, verifyCode);
    }
    else
    {
        // 注册
        connect(dataCenter, &model::DataCenter::phoneRegisterDone, this, &PhoneLoginWidget::phoneRegisterDone, Qt::UniqueConnection);
        dataCenter->phoneRegisterAsync(phone, verifyCode);
    }
}

void PhoneLoginWidget::phoneLoginDone(bool ok, const QString &reason)
{
    // 登录失败
    if(!ok)
    {
        Toast::showMessage("登录失败!" + reason);
        return;
    }
    // 登录成功
    MainWidget* mainWidget = MainWidget::getInstance();
    mainWidget->show();

    this->close();
}

void PhoneLoginWidget::phoneRegisterDone(bool ok, const QString &reason)
{
    // 注册失败
    if(!ok)
    {
        Toast::showMessage("注册失败!" + reason);
        return;
    }
    // 注册成功
    Toast::showMessage("注册成功!");

    // 切换到登录界面
    this->switchMode();

    // 验证码输入框清空一下
    verifyCodeEdit->clear();

    // 处理一下倒计时的按钮
    leftTime = 1;
}

void PhoneLoginWidget::countDown()
{
    if(leftTime <= 1)
    {
        // 时间到，发送按钮设置为可用状态，并停止定时器
        sendVerifyCodeBtn->setEnabled(true);
        sendVerifyCodeBtn->setText("发送验证码");
        timer->stop();
        leftTime = 30;
        return;
    }

    --leftTime;
    sendVerifyCodeBtn->setText(QString::number(leftTime) + "s");
    sendVerifyCodeBtn->setEnabled(false);
}

void PhoneLoginWidget::switchMode()
{
    if(this->isLoginMode)
    {
        // 当前是登录模式，切换到注册模式
        this->setWindowTitle("注册");
        titleLabel->setText("注册");
        submitBtn->setText("注册");
        switchModeBtn->setText("登录");
    }
    else
    {
        // 当前是注册模式，切换到登录模式
        this->setWindowTitle("登录");
        titleLabel->setText("登录");
        submitBtn->setText("登录");
        switchModeBtn->setText("注册");
    }
    this->isLoginMode = !this->isLoginMode;
}

bool PhoneLoginWidget::isPhoneNumberValid(const QString &phoneNumber)
{
    // 中国大陆手机号规则：以1开头，第二位是3/4/5/6/7/8/9，后面跟9位数字
    QRegularExpression regExp("1[3-9]\\d{9}");
    QRegularExpressionMatch match = regExp.match(phoneNumber);
    return match.hasMatch();
}
