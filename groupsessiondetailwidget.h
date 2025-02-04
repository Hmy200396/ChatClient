#ifndef GROUPSESSIONDETAILWIDGET_H
#define GROUPSESSIONDETAILWIDGET_H

#include <QDialog>
#include <QWidget>
#include <QGridLayout>

class AvatarItem;
//////////////////////////////////////////////
/// 表示 “群聊会话详情” 窗口
//////////////////////////////////////////////
class GroupSessionDetailWidget : public QDialog
{
    Q_OBJECT
public:
    GroupSessionDetailWidget(QWidget* parent);

    void addMember(AvatarItem* avatarItem);
private:
    QGridLayout* glayout;

    int curRow = 0;
    int curCol = 1;
};

#endif // GROUPSESSIONDETAILWIDGET_H
