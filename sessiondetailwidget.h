#ifndef SESSIONDETAILWIDGET_H
#define SESSIONDETAILWIDGET_H

#include <QDialog>
#include <QWidget>
#include <QPushButton>
#include <QLabel>
//////////////////////////////////////////////
/// 表示 一个头像 + 一个名字组合控件
//////////////////////////////////////////////
class AvatarItem : public QWidget
{
    Q_OBJECT
public:
    AvatarItem(const QIcon& avatar, const QString& name);

    QPushButton* getAvatar();
private:
    QPushButton* avatarBtn;
    QLabel* nameLabel;
};

//////////////////////////////////////////////
/// 表示 “单聊会话详情” 窗口
//////////////////////////////////////////////
class SessionDetailWidget : public QDialog
{
    Q_OBJECT
public:
    SessionDetailWidget(QWidget* parent);

private:
    QPushButton* deleteFriendBtn;
};

#endif // SESSIONDETAILWIDGET_H
