#ifndef CHOOSEFRIENDDIALOG_H
#define CHOOSEFRIENDDIALOG_H

#include <QDialog>
#include <QWidget>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>

///////////////////////////////////////
/// 选择好友窗口中的一个元素
///////////////////////////////////////
class ChooseFriendItem : public QWidget
{
public:
    ChooseFriendItem(const QString& userId, const QIcon& avatar, const QString& name, bool checked);
    void paintEvent(QPaintEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;

    const QString& getUserId();
    QCheckBox* getCheckBox(){
        return checkBox;
    }
private:
    QCheckBox* checkBox;
    QPushButton* avatarBtn;
    QLabel* nameLabel;

    bool isHover = false;

    QString userId;
};


///////////////////////////////////////
/// 选择好友的窗口
///////////////////////////////////////
class ChooseFriendDialog : public QDialog
{
    Q_OBJECT
public:
    ChooseFriendDialog(QWidget* parent);
    void initLeft(QHBoxLayout* layout);
    void initRight(QHBoxLayout* layout);
    void addFriend(const QString& userId, const QIcon &avatar, const QString &name, bool checked);
    void addSelectedFriend(const QString& userId, const QIcon &avatar, const QString &name);
    void deleteSelectedFriend(const QString& userId);
private:
    // 保存左侧全部好友列表的 QWidget
    QWidget* totalContainer;

    // 保存右侧选中好友列表的 QWidget
    QWidget* selectedContainer;
};

#endif // CHOOSEFRIENDDIALOG_H
