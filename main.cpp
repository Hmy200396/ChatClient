#include "mainwidget.h"
#include "model/datacenter.h"

#include <QApplication>
#include <QDebug>
#include "model/data.h"
#include "model/datacenter.h"
#include "loginwidget.h"
#include "debug.h"

FILE* output = nullptr;

void msgHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    QByteArray log = msg.toUtf8();
    fprintf(output, "%s\n", log.constData());
    fflush(output);
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

#if DEPOLY
    output = fopen("./log.txt", "a");
    qInstallMessageHandler(msgHandler);
#endif

    model::DataCenter* instance = model::DataCenter::getInstance();

#if TEST_SKIP_LOGIN
    MainWidget* w = MainWidget::getInstance();
    w->show();
#else
    LoginWidget* loginWidget = new LoginWidget();
    loginWidget->show();
#endif

#if TEST_NETWORK
    // network::NetClient netClient(nullptr);
    // netClient.ping();

    model::DataCenter* dataCenter = model::DataCenter::getInstance();
    dataCenter->initWebSocket();
    dataCenter->ping();
#endif
    return a.exec();
}
