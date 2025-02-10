#ifndef MESSAGEEDITAREA_H
#define MESSAGEEDITAREA_H

#include <QWidget>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QLabel>
#include "model/data.h"
class MessageEditArea : public QWidget
{
    Q_OBJECT
public:
    explicit MessageEditArea(QWidget *parent = nullptr);

private:
    QPushButton* sendImageBtn;
    QPushButton* sendFileBtn;
    QPushButton* sendSpeechBtn;
    QPushButton* showHistoryBtn;
    QPlainTextEdit* textEdit;
    QPushButton* sendTextBtn;
    QLabel* tipLabel;
private:
    void initSignalSlot();
    void sendTextMessage();
    void addSelfMessage(model::MessageType messageType, const QByteArray& content, const QString& extraInfo, const QString& path = "");
    void addOtherMessage(const model::Message& message);

    void clickSendImageBtn();
    void clickSendFileBtn();

    void soundRecordPressed();
    void soundRecordReleased();

    void sendSpeech(const QString& path);
signals:
};

#endif // MESSAGEEDITAREA_H
