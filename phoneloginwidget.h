#ifndef PHONELOGINWIDGET_H
#define PHONELOGINWIDGET_H

#include <QWidget>

class PhoneLoginWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PhoneLoginWidget(bool isLoginMode = true, QWidget *parent = nullptr);

private:
    bool isLoginMode;

signals:
};

#endif // PHONELOGINWIDGET_H
