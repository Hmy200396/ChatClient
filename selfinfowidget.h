#ifndef SELFINFOWIDGET_H
#define SELFINFOWIDGET_H

#include <QWidget>
#include <QDialog>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QGridLayout>

class SelfInfoWidget : public QDialog
{
    Q_OBJECT
public:
    SelfInfoWidget(QWidget* parent);

    void initSignalSlot();
    void clickNameSubmitBtn();
    void clickNameSubmitBtnDone();
    void clickDescSubmitBtn();
    void clickDescSubmitBtnDone();
    void clickGetVerifyCodeBtn();
    void clickPhoneSubmitBtn();
    void clickPhoneSubmitBtnDone();
    void clickAvatarBtn();
    void clickAvatarBtnDone();

private:
    QGridLayout* layout;

    QPushButton* avatarBtn;         // 头像
    QLabel* idTag;                  // 显示“序号”
    QLabel* idLabel;                // id

    QLabel* nameTag;                // 显示“昵称”
    QLabel* nameLabel;              // 显示“名字”
    QLineEdit* nameEdit;            // 编辑昵称
    QPushButton* nameModifyBtn;     // 修改名字
    QPushButton* nameSubmitBtn;     // 提交修改

    QLabel* descTag;                // 显示“签名”
    QLabel* descLabel;              // 显示签名内容
    QLineEdit* descEdit;            // 编辑签名
    QPushButton* descModifyBtn;     // 修改签名
    QPushButton* descSubmitBtn;     // 提交签名

    QLabel* phoneTag;                // 显示“电话”
    QLabel* phoneLabel;              // 显示电话号码
    QLineEdit* phoneEdit;            // 编辑电话
    QPushButton* phoneModifyBtn;     // 修改电话
    QPushButton* phoneSubmitBtn;     // 提交电话

    QLabel* verifyCodeTag;           // 显示“验证码”
    QLineEdit* verifyCodeEdit;       // 输入验证码
    QPushButton* getVerifyCodeBtn;   // 获取验证码按钮

    QString phoneToChange;           // 要修改的新的手机号码

    int leftTime = 30;               // 倒计时的时间

};

#endif // SELFINFOWIDGET_H
