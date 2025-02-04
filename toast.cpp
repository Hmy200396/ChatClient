#include "toast.h"
#include <QApplication>
#include <QScreen>
#include <QVBoxLayout>
#include <QLabel>
#include <QTimer>

Toast::Toast(const QString& text)
{
    qDebug()<<text;
    // 1. 设置窗口基本属性
    this->setFixedSize(500, 150);
    this->setWindowTitle("消息通知");
    this->setWindowIcon(QIcon(":/resource/image/logo.png"));
    this->setStyleSheet("QDialog { background-color: rgb(255, 255, 255); }");
    this->setAttribute(Qt::WA_DeleteOnClose);
    // 去掉窗口的标题栏
    this->setWindowFlags(Qt::FramelessWindowHint);

    // 2. 窗口位置
    // a) 获取到整个屏幕的尺寸
    QScreen* screen = QApplication::primaryScreen();
    int width = screen->size().width();
    int height = screen->size().height();
    int x = (width - this->width()) / 2;
    int y = height - this->height() - 100; // 100 是底部编剧
    this->move(x, y);

    // 3. 添加布局管理器
    QVBoxLayout* layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    this->setLayout(layout);

    // 4. 创建显示文本的Label
    QLabel* label = new QLabel();
    label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    label->setAlignment(Qt::AlignCenter);
    label->setStyleSheet("QLabel { font-size: 30px; color: black; }");
    label->setText(text);
    layout->addWidget(label);

    // 5. 实现两秒后自动关闭
    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [=](){
        timer->stop();
        this->close();
    });
    timer->start(2000);
}

void Toast::showMessage(const QString &text)
{
    Toast* toast = new Toast(text);
    toast->show();
}
