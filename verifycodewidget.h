#ifndef VERIFYCODEWIDGET_H
#define VERIFYCODEWIDGET_H

#include <QWidget>
#include <QRandomGenerator>
#include <QPaintEvent>

class VerifyCodeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit VerifyCodeWidget(QWidget *parent = nullptr);

    // 生成随机验证码字符串
    QString generateVerifyCode();

    // 检验验证码是否匹配
    bool checkVerifyCode(const QString& verifyCode);

    // 重新生成验证码显示到界面上
    void refreshVerifyCode();

    void paintEvent(QPaintEvent* event) override;

    // 用户点击的时候，刷新验证码并重新显示
    void mousePressEvent(QMouseEvent* event) override;

private:
    // 随机数生成器
    QRandomGenerator randomGenerator;

    // 保存验证码的值
    QString verifyCode = "";

signals:
};

#endif // VERIFYCODEWIDGET_H
