#ifndef HISTORYMESSAGEWIDGET_H
#define HISTORYMESSAGEWIDGET_H

#include <QDialog>
#include <QWidget>
#include <QGridLayout>
#include <QRadioButton>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include "model/data.h"

/////////////////////////////////////
/// 表示一个历史消息元素
/////////////////////////////////////
class HistoryItem : public QWidget
{
    Q_OBJECT
public:
    HistoryItem(){}

    static HistoryItem* makeHistoryItem(const model::Message& message);
    void paintEvent(QPaintEvent* event);
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
private:
    bool isHover = false;
};

/////////////////////////////////////
/// 展示历史消息窗口
/////////////////////////////////////
class HistoryMessageWidget : public QDialog
{
    Q_OBJECT
public:
    HistoryMessageWidget(QWidget* parent);
    void addHistoryMessage(const model::Message& message);
    void clear();
    void clickSearchbtn();
    void clickSearchbtnDone();
private:
    void initScrollArea(QGridLayout* layout);

    QRadioButton* keyRadioBtn;
    QRadioButton* timeRadioBtn;
    QLineEdit* searchEdit;
    QDateTimeEdit* begTimeEdit;
    QDateTimeEdit* endTimeEdit;

    QWidget* container;
};

/////////////////////////////////////
/// 展示图片历史消息
/////////////////////////////////////
class ImageButton : public QPushButton
{
public:
    ImageButton(const QString& fileId, const QByteArray& content);
    void updateUI(const QString& fileId, const QByteArray& content);
private:
    QString fileId;
};

/////////////////////////////////////
/// 展示文件历史消息
/////////////////////////////////////
class FileLabel : public QLabel
{
public:
    FileLabel(const QString& fileId, const QString& fileName);
    void saveFile(const QString &fileId, const QByteArray &content);
    void mousePressEvent(QMouseEvent *event) override;
private:
    QString fileId;
    QString filePath;
    QString fileName;
};

/////////////////////////////////////
/// 展示语音历史消息
/////////////////////////////////////
class SpeechLabel : public QLabel
{
public:
    SpeechLabel(const QString& fileId, const QByteArray& content);

    void getContentDone(const QString& fileId, const QByteArray& content);
    void mousePressEvent(QMouseEvent *event) override;

private:
    QString fileId;
    QByteArray content;
};

#endif // HISTORYMESSAGEWIDGET_H
