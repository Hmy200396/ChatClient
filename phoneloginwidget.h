#ifndef PHONELOGINWIDGET_H
#define PHONELOGINWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QTimer>
#include <QLabel>
#include <QPushButton>

class PhoneLoginWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PhoneLoginWidget(bool isLoginMode = true, QWidget *parent = nullptr);

    void sendVerifyCode();
    void sendVerifyCodeDone();
    void clickSubmitBtn();
    void phoneLoginDone(bool ok, const QString& reason);
    void phoneRegisterDone(bool ok, const QString& reason);
    void countDown();
    void switchMode();
    bool isPhoneNumberValid(const QString &phoneNumber);

private:
    bool isLoginMode;
    QString currentPhone = "";

    QLabel* titleLabel;
    QLineEdit* phoneEdit;
    QPushButton* sendVerifyCodeBtn;
    QLineEdit* verifyCodeEdit;
    QPushButton* submitBtn;
    QPushButton* switchModeBtn;

    QTimer* timer;
    int leftTime = 30;

signals:
};

#endif // PHONELOGINWIDGET_H
