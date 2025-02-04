#ifndef USERINFOWIDGET_H
#define USERINFOWIDGET_H

#include <QWidget>
#include <QDialog>
#include <QPushButton>
#include <QLabel>
#include <QCursor>
#include "model/data.h"

class UserInfoWidget : public QDialog
{
    Q_OBJECT
public:
    UserInfoWidget(const model::UserInfo& userInfo, QWidget* parent);
    void initSignalSlot();
    void clickDeleteFriendBtn();
    void clickApplyBtn();

private:
    const model::UserInfo& userInfo;

    QPushButton* avatarBtn;
    QLabel* idTag;
    QLabel* idLabel;
    QLabel* nameTag;
    QLabel* nameLabel;
    QLabel* phoneTag;
    QLabel* phoneLabel;
    QPushButton* applyBtn;
    QPushButton* sendMessageBtn;
    QPushButton* deleteFriendBtn;
};
#endif // USERINFOWIDGET_H
