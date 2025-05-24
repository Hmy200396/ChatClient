#ifndef GROUPSESSIONDETAILWIDGET_H
#define GROUPSESSIONDETAILWIDGET_H

#include <QDialog>
#include <QLabel>
#include <QWidget>
#include <QGridLayout>
#include <QPushButton>
#include <QLineEdit>
#include "model/data.h"

class AvatarItem;
//////////////////////////////////////////////
/// 表示 “群聊会话详情” 窗口
//////////////////////////////////////////////
class GroupSessionDetailWidget : public QDialog
{
    Q_OBJECT
public:
    GroupSessionDetailWidget(QWidget* parent, model::ChatSessionInfo* chatSessionInfo);

    void addMember(AvatarItem* avatarItem);
    void initData();
    void initMembers(const QString& chatSessionId);
private:
    void initSignalSlot();
    void exitGroup();
    void exitGroupChatSessionDone();
    void clickSubmitBtn();
    void clickSubmitBtnDone(const QString& chatSessionId, const QString& groupname);
private:
    model::ChatSessionInfo* chatSessionInfo;
    QGridLayout* glayout;
    QHBoxLayout* hlayout;

    QLabel* groupNameLabel;
    QLineEdit* groupNameEdit;
    QPushButton* modifyBtn;
    QPushButton* submitBtn;

    QPushButton* exitGroupBtn;

    QList<QString> userIdList;

    int curRow = 0;
    int curCol = 1;
};

#endif // GROUPSESSIONDETAILWIDGET_H
