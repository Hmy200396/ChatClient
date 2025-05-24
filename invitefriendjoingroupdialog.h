#ifndef INVITEFRIENDJOINGROUPDIALOG_H
#define INVITEFRIENDJOINGROUPDIALOG_H

#include <QDialog>
#include <QWidget>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include "choosefrienddialog.h"

class InviteFriendJoinGroupDialog : public QDialog
{
    Q_OBJECT
public:
    InviteFriendJoinGroupDialog(QWidget* parent, const QList<QString>& userIdList);
    void initLeft(QHBoxLayout* layout);
    void initRight(QHBoxLayout* layout);
    void initData();
    void addFriend(const QString& userId, const QIcon &avatar, const QString &name, bool checked, bool enable = true);
    void addSelectedFriend(const QString& userId, const QIcon &avatar, const QString &name);
    void deleteSelectedFriend(const QString& userId);
    void clickOkBtn();
    QList<QString> generateMemberList();

private:
    QPushButton* okBtn;
    QPushButton* cancelBtn;
    // 保存左侧全部好友列表的 QWidget
    QWidget* totalContainer;

    // 保存右侧选中好友列表的 QWidget
    QWidget* selectedContainer;

    // 当前窗口是点击哪个用户来弹出的
    const QList<QString> userIdList;

    // 保存当前选中了几个群聊成员
    int  count = 0;
};

#endif // INVITEFRIENDJOINGROUPDIALOG_H
