#ifndef ADDFRIENDDIALOG_H
#define ADDFRIENDDIALOG_H

#include <QDialog>
#include <QWidget>
#include <QGridLayout>
#include <QLineEdit>
#include "model/data.h"

////////////////////////////////
/// 表示一个好友搜索的结果
////////////////////////////////
class FriendResultItem : public QWidget
{
    Q_OBJECT
public:
    FriendResultItem(const model::UserInfo& userInfo);
private:
    const model::UserInfo& userInfo;
};

////////////////////////////////
/// 整个搜索好友的窗口
////////////////////////////////
class AddFriendDialog : public QDialog
{
    Q_OBJECT
public:
    AddFriendDialog(QWidget* parent);

    // 初始化结果显示区
    void initResultArea();
    // 往窗口中新增一个好友搜索结果
    void addResult(const model::UserInfo& userInfo);
    // 清楚界面上所有的好友结果
    void clear();

    void setSearchKey(const QString& searchKey);
private:
    QGridLayout* layout;
    QWidget* resultContainer;
    QLineEdit* searchEdit;
};

#endif // ADDFRIENDDIALOG_H
