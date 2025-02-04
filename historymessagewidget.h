#ifndef HISTORYMESSAGEWIDGET_H
#define HISTORYMESSAGEWIDGET_H

#include <QDialog>
#include <QWidget>
#include <QGridLayout>
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
private:
    void initScrollArea(QGridLayout* layout);

    QWidget* container;
};

#endif // HISTORYMESSAGEWIDGET_H
