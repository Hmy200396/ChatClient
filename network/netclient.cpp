#include "netclient.h"
#include <QNetworkReply>
#include <QUuid>
#include "../model/data.h"
#include "../model//datacenter.h"
#include "../toast.h"
#include "../mainwidget.h"

namespace network{

NetClient::NetClient(model::DataCenter* dataCenter)
    :dataCenter(dataCenter)
{
    //initWebSocket();
}

void NetClient::ping()
{
    QNetworkRequest httpReq;
    httpReq.setUrl(QUrl(HTTP_URL + "/ping"));
    QNetworkReply* httpResp = httpClient.get(httpReq);
    connect(httpResp, &QNetworkReply::finished, this, [=](){
       // 说明响应回来了
        if(httpResp->error() != QNetworkReply::NoError)
        {
            // 请求失败
            LOG() << "HTTP 请求失败！" << httpResp->errorString();
            httpResp->deleteLater();
            return;
        }
        // 获取到响应的 body
        QByteArray body = httpResp->readAll();
        LOG() << "响应内容" << body;
        httpResp->deleteLater();
    });
}

////////////////////////////////////////////////////////
/// WebSocket
////////////////////////////////////////////////////////
void NetClient::initWebSocket()
{
    // 1. 准备好所有需要的信号槽
    connect(&webSocketClient, &QWebSocket::connected, this, [=](){
        LOG() << "webSocket 连接成功！";
        // 发送认证消息
        sendAuth();
    });
    connect(&webSocketClient, &QWebSocket::disconnected, this, [=](){
        LOG() << "webSocket 连接断开！";
    });
    connect(&webSocketClient, &QWebSocket::errorOccurred, this, [=](QAbstractSocket::SocketError error){
        LOG() << "webSocket 连接出错！" << error;
    });
    connect(&webSocketClient, &QWebSocket::textMessageReceived, this, [=](const QString&  message){
        LOG() << "webSocket 收到文本消息：" << message;
    });
    connect(&webSocketClient, &QWebSocket::binaryMessageReceived, this, [=](const QByteArray& byteArray){
        LOG() << "webSocket 收到二进制消息：" << byteArray.length();
        proto::NotifyMessage notifyMessage;
        notifyMessage.deserialize(&serializer, byteArray);
        handleWsResponse(notifyMessage);
    });

    // 2. 和服务器建立连接
    webSocketClient.open(WEBSOCKET_URL);
}

void NetClient::handleWsResponse(const proto::NotifyMessage &notifyMessage)
{
    if(notifyMessage.notifyType() == proto::NotifyTypeGadget::NotifyType::CHAT_MESSAGE_NOTIFY)
    {
        // 收到消息
        // 1. 把 pb 中的 MessageInfo 转成客户端自己的 Message
        model::Message message;
        message.load(notifyMessage.newMessageInfo().messageInfo());
        // 2. 针对自己的 message 做进一步的处理
        handleWsMessage(message);
    }
    else if(notifyMessage.notifyType() == proto::NotifyTypeGadget::NotifyType::CHAT_SESSION_CREATE_NOTIFY)
    {
        // 创建新的会话通知
        model::ChatSessionInfo chatSessionInfo;
        chatSessionInfo.load(notifyMessage.newChatSessionInfo().chatSessionInfo());
        handleWsSessionCreate(chatSessionInfo);
    }
    else if(notifyMessage.notifyType() == proto::NotifyTypeGadget::NotifyType::FRIEND_ADD_APPLY_NOTIFY)
    {
        // 添加好友申请通知
        model::UserInfo userInfo;
        userInfo.load(notifyMessage.friendAddApply().userInfo());
        handleWsAddFriendApply(userInfo);
    }
    else if(notifyMessage.notifyType() == proto::NotifyTypeGadget::NotifyType::FRIEND_ADD_PROCESS_NOTIFY)
    {
        // 添加好友申请的处理结果通知
        model::UserInfo userInfo;
        userInfo.load(notifyMessage.friendProcessResult().userInfo());
        bool agree = notifyMessage.friendProcessResult().agree();
        handleWsAddFriendProcess(userInfo, agree);
    }
    else if(notifyMessage.notifyType() == proto::NotifyTypeGadget::NotifyType::FRIEND_REMOVE_NOTIFY)
    {
        // 删除好友通知
        const QString& userId = notifyMessage.friendRemove().userId();
        handleWsRemoveFriend(userId);
    }
}

void NetClient::handleWsMessage(const model::Message &message)
{
    // 两个情况
    QList<model::Message>* messageList = dataCenter->getRecentMessageList(message.chatSessionId);
    if(messageList == nullptr)
    {
        // 1. 如果当前这个消息所属的会话，里面的消息列表，没有在本地加载，此时就需要通过网络先加载整个消息列表
        connect(dataCenter, &model::DataCenter::getRecentMessageListDoneNoUI, this, &NetClient::receiveMessage, Qt::UniqueConnection);
        dataCenter->getRecentMessageListAsync(message.chatSessionId, false);
    }
    else
    {
        // 2. 如果当前这个消息所属的会话，里面的消息已经在本地加载了，直接把这个消息尾插到消息列表中即可
        messageList->push_back(message);
        this->receiveMessage(message.chatSessionId);
    }
}

void NetClient::handleWsRemoveFriend(const QString &userId)
{
    const model::UserInfo* userInfo = dataCenter->findFriendById(userId);
    if(userInfo == nullptr)
        return;
    const QString nickname = userInfo->nickname;

    // 1. 删除数据 DataCenter 好友列表的数据
    dataCenter->removeFriend(userId);

    // 2. 通知界面变化 更新 好友列表/会话列表
    emit dataCenter->deleteFriendDone();

    Toast::showMessage(nickname+" 解除了与您的好友关系");
}

void NetClient::handleWsAddFriendApply(const model::UserInfo &userInfo)
{
    // 1. DataCenter 中有一个 好友申请列表，需要把这个数据添加到好友申请列表中
    QList<model::UserInfo>* applyList = dataCenter->getApplyList();
    if(applyList == nullptr)
    {
        LOG()<<"客户端没有加载到好友申请列表！";
        return;
    }
    // 如果该用户重复申请，则将最近一次申请的内容删除
    dataCenter->removeFromApplyList(userInfo.userId);
    // 把新元素放到列表前面
    applyList->push_front(userInfo);

    // 2. 通知界面更新
    emit dataCenter->receiveFriendApplyDone();
}

void NetClient::handleWsSessionCreate(const model::ChatSessionInfo &chatSessionInfo)
{
    // 把这个 ChatSessionInfo 添加到会话列表
    QList<model::ChatSessionInfo>* chatSessionList = dataCenter->getSessionList();
    if(chatSessionList == nullptr)
    {
        LOG()<<"客户端没有加载会话列表";
        return;
    }
    // 新的元素添加到列表头部
    chatSessionList->push_front(chatSessionInfo);
    // 发送一个信号，通知界面更新
    emit dataCenter->receiveSessionCreateDone();
}

void NetClient::handleWsAddFriendProcess(const model::UserInfo &userInfo, bool agree)
{
    if(agree)
    {
        // 对方同意了
        QList<model::UserInfo>* friendList = dataCenter->getFriendList();
        if(friendList == nullptr)
        {
            LOG()<<"客户端没有加载好友列表";
            return;
        }

        // 防止重复添加好友，所以先判断一下
        if(dataCenter->isInFriendList(userInfo.userId))
            return;
        friendList->push_front(userInfo);

        // 更新界面
        emit dataCenter->receiveFriendProcessDone(userInfo.nickname, agree);
    }
    else
    {
        // 对方未同意
        emit dataCenter->receiveFriendProcessDone(userInfo.nickname, agree);
    }
}

void NetClient::sendAuth()
{
    proto::ClientAuthenticationReq req;
    req.setRequestId(makeRequestId());
    req.setSessionId(dataCenter->getLoginSessionId());
    QByteArray body = req.serialize(&serializer);
    webSocketClient.sendBinaryMessage(body);
    LOG() << "[WS身份认证] requestId = " << req.requestId() << ", loginSessionId = " << req.sessionId();
}

////////////////////////////////////////////////////////
/// HTTP
////////////////////////////////////////////////////////
QString NetClient::makeRequestId()
{
    // 确保每个请求的 id 都是不重复的
    // 通过 UUID 实现
    return "R" + QUuid::createUuid().toString().sliced(25,12);
}

QNetworkReply *NetClient::sendHttpRequest(const QString &apiPath, const QByteArray &body)
{
    QNetworkRequest httpReq;
    httpReq.setUrl(QUrl(HTTP_URL + apiPath));
    httpReq.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-protobuf");
    QNetworkReply* httpResp = httpClient.post(httpReq, body);
    return httpResp;
}

void NetClient::getMyself(const QString &loginSessionId)
{
    // 1. 构造 HTPP 请求的 body
    proto::GetUserInfoReq req;
    req.setRequestId(makeRequestId());
    req.setSessionId(loginSessionId);
    QByteArray body = req.serialize(&serializer);
    LOG()<<"[获取个人信息] 发送请求 requestId = "<<req.requestId()<<", loginSessionId = "<<req.sessionId();

    // 2. 构造HTTP请求，并发送
    QNetworkReply* httpResp = sendHttpRequest("/service/user/get_user_info", body);

    // 3. 通过信号槽，获取到当前响应
    connect(httpResp, &QNetworkReply::finished, this, [=](){
        // a) 处理响应对象
        bool ok = false;
        QString reason;
        std::shared_ptr<proto::GetUserInfoRsp> resp = handleHttpResponse<proto::GetUserInfoRsp>(httpResp, &ok, &reason);

        // b) 判断响应是否正确
        if(!ok)
        {
            LOG() << "[获取个人信息] 出错！requestId = " << req.requestId() << ", reason = " << reason;
            return;
        }

        // c) 把响应的数据保存到 DataCenter 中
        dataCenter->resetMyself(resp);

        // d) 通知调用逻辑，响应已经处理完了，通过信号槽通知
        emit dataCenter->getMyselfDone();

        //e) 打印日志
        LOG() << "[获取个人信息] 处理响应 requestId = " << req.requestId();
    });
}

void NetClient::getFriendList(const QString &loginSessionId)
{
    // 1. 通过 protobuf 构造 body
    proto::GetFriendListReq req;
    req.setRequestId(makeRequestId());
    req.setSessionId(loginSessionId);
    QByteArray body = req.serialize(&serializer);
    LOG()<<"[获取好友列表] 发送请求 requestId = "<<req.requestId()<<", loginSessionId = "<<req.sessionId();

    // 2. 发送 HTTP 请求
    QNetworkReply* httpResp = sendHttpRequest("/service/friend/get_friend_list", body);

    // 3. 处理响应
    connect(httpResp, &QNetworkReply::finished, this, [=](){
        // a) 处理响应对象
        bool ok = false;
        QString reason;
        std::shared_ptr<proto::GetFriendListRsp> friendListResp = this->handleHttpResponse<proto::GetFriendListRsp>(httpResp, &ok, &reason);

        // b) 判断响应是否正确
        if(!ok)
        {
            LOG() << "[获取好友列表] 失败！requestId = " << req.requestId() << ", reason = " << reason;
            return;
        }

        // c) 把响应的数据保存到 DataCenter 中
        dataCenter->resetFriendList(friendListResp);

        // d) 通知调用逻辑，响应已经处理完了，通过信号槽通知
        emit dataCenter->getFriendListDone();

        //e) 打印日志
        LOG() << "[获取好友列表] 处理响应 requestId = " << req.requestId();
    });
}

void NetClient::getSessionList(const QString &loginSessionId)
{
    // 1. 通过 protobuf 构造 body
    proto::GetChatSessionListReq req;
    req.setRequestId(makeRequestId());
    req.setSessionId(loginSessionId);
    QByteArray body = req.serialize(&serializer);
    LOG()<<"[获取会话列表] 发送请求 requestId = "<<req.requestId()<<", loginSessionId = "<<req.sessionId();

    // 2. 发送 HTTP 请求
    QNetworkReply* httpResp = sendHttpRequest("/service/friend/get_chat_session_list", body);

    // 3. 处理响应
    connect(httpResp, &QNetworkReply::finished, this, [=](){
        // a) 处理响应对象
        bool ok = false;
        QString reason;
        std::shared_ptr<proto::GetChatSessionListRsp> chatSessionListResp = this->handleHttpResponse<proto::GetChatSessionListRsp>(httpResp, &ok, &reason);

        // b) 判断响应是否正确
        if(!ok)
        {
            LOG() << "[获取会话列表] 失败！requestId = " << req.requestId() << ", reason = " << reason;
            return;
        }

        // c) 把响应的数据保存到 DataCenter 中
        dataCenter->resetChatSessionList(chatSessionListResp);

        // d) 通知调用逻辑，响应已经处理完了，通过信号槽通知
        emit dataCenter->getChatSessionListDone();

        //e) 打印日志
        LOG() << "[获取会话列表] 处理响应 requestId = " << req.requestId();
    });

}

void NetClient::getApplyList(const QString &loginSessionId)
{
    // 1. 通过 protobuf 构造 body
    proto::GetPendingFriendEventListReq req;
    req.setRequestId(makeRequestId());
    req.setSessionId(loginSessionId);
    QByteArray body = req.serialize(&serializer);
    LOG()<<"[获取好友申请列表] 发送请求 requestId = "<<req.requestId()<<", loginSessionId = "<<req.sessionId();

    // 2. 发送 HTTP 请求
    QNetworkReply* httpResp = sendHttpRequest("/service/friend/get_pending_friend_events", body);

    // 3. 处理响应
    connect(httpResp, &QNetworkReply::finished, this, [=](){
        // a) 处理响应对象
        bool ok = false;
        QString reason;
        std::shared_ptr<proto::GetPendingFriendEventListRsp> pendingFriendEventListResp = this->handleHttpResponse<proto::GetPendingFriendEventListRsp>(httpResp, &ok, &reason);

        // b) 判断响应是否正确
        if(!ok)
        {
            LOG() << "[获取好友申请列表] 失败！requestId = " << req.requestId() << ", reason = " << reason;
            return;
        }

        // c) 把响应的数据保存到 DataCenter 中
        dataCenter->resetApplyList(pendingFriendEventListResp);

        // d) 通知调用逻辑，响应已经处理完了，通过信号槽通知
        emit dataCenter->getApplyListDone();

        //e) 打印日志
        LOG() << "[获取好友申请列表] 处理响应 requestId = " << req.requestId();
    });
}

void NetClient::getRecentMessageList(const QString &loginSessionId, const QString &chatSessionId, bool updateUI)
{
    // 1. 通过 protobuf 构造 body
    proto::GetRecentMsgReq req;
    req.setRequestId(makeRequestId());
    req.setChatSessionId(chatSessionId);
    req.setMsgCount(50);
    req.setSessionId(loginSessionId);
    QByteArray body = req.serialize(&serializer);
    LOG()<<"[获取最近消息] 发送请求 requestId = "<<req.requestId()<<", loginSessionId = "<<req.sessionId()<<", chatSessionId = "<<req.chatSessionId();

    // 2. 发送 HTTP 请求
    QNetworkReply* httpResp = sendHttpRequest("/service/message_storage/get_recent", body);

    // 3. 处理响应
    connect(httpResp, &QNetworkReply::finished, this, [=](){
        // a) 处理响应对象
        bool ok = false;
        QString reason;
        std::shared_ptr<proto::GetRecentMsgRsp> recentMsgResp = this->handleHttpResponse<proto::GetRecentMsgRsp>(httpResp, &ok, &reason);

        // b) 判断响应是否正确
        if(!ok)
        {
            LOG() << "[获取最近消息] 失败！requestId = " << req.requestId() << ", reason = " << reason;
            return;
        }

        // c) 把响应的数据保存到 DataCenter 中
        dataCenter->resetRecentMessageList(chatSessionId, recentMsgResp);

        // d) 通知调用逻辑，响应已经处理完了，通过信号槽通知
        if(updateUI)
            emit dataCenter->getRecentMessageListDone(chatSessionId);
        else
            emit dataCenter->getRecentMessageListDoneNoUI(chatSessionId);

        // e) 打印日志
        LOG() << "[获取好友申请列表] 处理响应 requestId = " << req.requestId();
    });
}

void NetClient::sendMessage(const QString &loginSessionId, const QString &chatSessionId, model::MessageType messageType, const QByteArray &content, const QString& extraInfo)
{
    // 1. 通过 protobuf 构造 body
    proto::NewMessageReq req;
    req.setRequestId(makeRequestId());
    req.setSessionId(loginSessionId);
    req.setChatSessionId(chatSessionId);

    proto::MessageContent messageContent;
    if(messageType == model::MessageType::TEXT_TYPE)
    {
        messageContent.setMessageType(proto::MessageTypeGadget::MessageType::STRING);

        proto::StringMessageInfo stringMessageInfo;
        stringMessageInfo.setContent(content);

        messageContent.setStringMessage(stringMessageInfo);
    }
    else if(messageType == model::MessageType::IMAGE_TYPE)
    {
        messageContent.setMessageType(proto::MessageTypeGadget::MessageType::IMAGE);

        proto::ImageMessageInfo imageMessageInfo;
        imageMessageInfo.setFileId("");
        imageMessageInfo.setImageContent(content);

        messageContent.setImageMessage(imageMessageInfo);
    }
    else if(messageType == model::MessageType::FILE_TYPE)
    {
        messageContent.setMessageType(proto::MessageTypeGadget::MessageType::FILE);

        proto::FileMessageInfo fileMessageInfo;
        fileMessageInfo.setFileId("");
        fileMessageInfo.setFileSize(content.size());
        fileMessageInfo.setFileName(extraInfo);
        fileMessageInfo.setFileContents(content);

        messageContent.setFileMessage(fileMessageInfo);
    }
    else if(messageType == model::MessageType::SPEECH_TYPE)
    {
        messageContent.setMessageType(proto::MessageTypeGadget::MessageType::SPEECH);

        proto::SpeechMessageInfo speechMessageInfo;
        speechMessageInfo.setFileId("");
        speechMessageInfo.setFileContents(content);

        messageContent.setSpeechMessage(speechMessageInfo);
    }
    else
    {
        LOG()<<"错误的消息类型！ MessageType = " << messageType;
    }
    req.setMessage(messageContent);

    QByteArray body = req.serialize(&serializer);
    LOG()<<"[发送消息] 发送请求 requestId = "<<req.requestId()<<", loginSessionId = "<<req.sessionId()<<", chatSessionId = "<<req.chatSessionId()<<", messageType = "<<req.message().messageType();

    // 2. 发送 HTTP 请求
    QNetworkReply* httpResp = sendHttpRequest("/service/message_transmit/new_message", body);

    // 3. 处理响应
    connect(httpResp, &QNetworkReply::finished, this, [=](){
        // a) 处理响应对象
        bool ok = false;
        QString reason;
        std::shared_ptr<proto::NewMessageRsp> newMessageResp = this->handleHttpResponse<proto::NewMessageRsp>(httpResp, &ok, &reason);

        // b) 判断响应是否正确
        if(!ok)
        {
            LOG() << "[发送消息] 失败！requestId = " << newMessageResp->requestId() << ", reason = " << reason;
            return;
        }

        // c) 不需要把响应的数据保存到 DataCenter 中

        // d) 通知调用逻辑，响应已经处理完了，通过信号槽通知
        emit dataCenter->sendMessageDone(messageType, content, extraInfo);

        //e) 打印日志
        LOG() << "[发送消息] 处理响应 requestId = " << newMessageResp->requestId();
    });
}

void NetClient::receiveMessage(const QString &chatSessionId)
{
    // 先需要判定一下，当前这个收到的消息对应的会话，是否是正在被用户选中的 “当前会话”
    // 当前会话，就需要把消息，显示到消息展示区，也需要更新会话列表的消息预览
    // 不是当前会话，只需要更新会话列表中的消息预览，并且更新 “未读消息数目”
    if(chatSessionId == dataCenter->getCurrentChatSessionId())
    {
        // 收到的消息，是选中的会话
        // 在消息展示区新增一个消息
        const model::Message lastMessage = dataCenter->getRecentMessageList(chatSessionId)->back();
        // 通过信号，让 netClient 模块，能够通知界面（消息展示区）
        emit dataCenter->receiveMessageDone(lastMessage);
    }
    else
    {
        // 收到的消息，不是选中的会话
        // 更新未读消息数目
        dataCenter->addUnread(chatSessionId);
    }
    // 统一更新会话列表的消息预览
    emit dataCenter->updateLastMessage(chatSessionId);
}

void NetClient::changeNickname(const QString &loginSessionId, const QString &nickname)
{
    // 1. 通过 protobuf 构造 body
    proto::SetUserNicknameReq req;
    req.setRequestId(makeRequestId());
    req.setSessionId(loginSessionId);
    req.setNickname(nickname);
    QByteArray body = req.serialize(&serializer);
    LOG() << "[修改用户昵称] 发送请求 requestId = " << req.requestId()<<", loginSessionId = "<<req.sessionId()<<", nickname = "<<req.nickname();

    // 2. 发送 http 请求
    QNetworkReply* httpResp = this->sendHttpRequest("/service/user/set_nickname", body);

    // 3. 处理响应
    connect(httpResp, &QNetworkReply::finished, this, [=](){
        // a) 处理响应对象
        bool ok = false;
        QString reason;
        std::shared_ptr<proto::SetUserNicknameRsp> pbResp = this->handleHttpResponse<proto::SetUserNicknameRsp>(httpResp, &ok, &reason);

        // b) 判断响应是否正确
        if(!ok)
        {
            LOG() << "[发送消息] 失败！requestId = " << pbResp->requestId() << ", reason = " << reason;
            return;
        }

        // c) 把响应的数据保存到 DataCenter 中
        dataCenter->resetNickname(nickname);

        // d) 通知调用逻辑，响应已经处理完了，通过信号槽通知
        emit dataCenter->changeNicknameDone();

        //e) 打印日志
        LOG() << "[修改用户昵称] 处理响应完毕！ requestId = " << pbResp->requestId();
    });
}

void NetClient::changeDescription(const QString &loginSessionId, const QString &desc)
{
    // 1. 通过 protobuf 构造 body
    proto::SetUserDescriptionReq pbReq;
    pbReq.setRequestId(makeRequestId());
    pbReq.setSessionId(loginSessionId);
    pbReq.setDescription(desc);
    QByteArray body = pbReq.serialize(&serializer);
    LOG() << "[修改签名] 发送请求 requestId = " << pbReq.requestId()<<", loginSessionId = "<<pbReq.sessionId()<<", description = "<<pbReq.description();

    // 2. 发送 http 请求
    QNetworkReply* httpResp = this->sendHttpRequest("/service/user/set_description", body);

    // 3. 处理响应
    connect(httpResp, &QNetworkReply::finished, this, [=](){
        // a) 处理响应对象
        bool ok = false;
        QString reason;
        std::shared_ptr<proto::SetUserDescriptionRsp> pbResp = this->handleHttpResponse<proto::SetUserDescriptionRsp>(httpResp, &ok, &reason);

        // b) 判断响应是否正确
        if(!ok)
        {
            LOG() << "[发送消息] 失败！requestId = " << pbResp->requestId() << ", reason = " << reason;
            return;
        }

        // c) 把响应的数据保存到 DataCenter 中
        dataCenter->resetDescription(desc);

        // d) 通知调用逻辑，响应已经处理完了，通过信号槽通知
        emit dataCenter->changeDescriptionDone();

        //e) 打印日志
        LOG() << "[修改签名] 处理响应完毕！ requestId = " << pbResp->requestId();
    });
}

void NetClient::getVerifyCode(const QString &phone)
{
    // 获取短信验证码                  /service/user/get_phone_verify_code
    // 1. 构造请求 body
    proto::PhoneVerifyCodeReq pbReq;
    pbReq.setRequestId(makeRequestId());
    pbReq.setPhoneNumber(phone);
    QByteArray body = pbReq.serialize(&serializer);
    LOG() << "[获取手机验证码] 发送请求 requestId = " << pbReq.requestId() << ", phone = " << phone;

    // 2. 发送 HTTP 请求
    QNetworkReply* resp = this->sendHttpRequest("/service/user/get_phone_verify_code", body);

    // 3. 处理响应
    connect(resp, &QNetworkReply::finished, this, [=](){
        // a) 处理响应对象
        bool ok = false;
        QString reason;
        std::shared_ptr<proto::PhoneVerifyCodeRsp> pbResp = this->handleHttpResponse<proto::PhoneVerifyCodeRsp>(resp, &ok, &reason);

        // b) 判断响应是否正确
        if(!ok)
        {
            LOG() << "[获取手机验证码] 响应失败！reason = " << reason;
            return;
        }

        // c) 把响应的数据保存到 DataCenter 中
        dataCenter->resetVerifyCodeId(pbResp->verifyCodeId());

        // d) 通知调用逻辑，响应已经处理完了，通过信号槽通知
        emit dataCenter->getVerifyCodeDone();

        //e) 打印日志
        LOG() << "[获取手机验证码] 响应完毕！ requestId = " << pbResp->requestId();
    });
}

void NetClient::changePhone(const QString &loginSessionId, const QString &phone, const QString &verifyCodeId, const QString &verifyCode)
{
    // 修改绑定手机                    /service/user/set_phone
    // 1. 构造请求 body
    proto::SetUserPhoneNumberReq pbReq;
    pbReq.setRequestId(makeRequestId());
    pbReq.setSessionId(loginSessionId);
    pbReq.setPhoneNumber(phone);
    pbReq.setPhoneVerifyCodeId(verifyCodeId);
    pbReq.setPhoneVerifyCode(verifyCode);
    QByteArray body = pbReq.serialize(&serializer);
    LOG() << "[修改手机号] 发送请求 requestId = " << pbReq.requestId() << ", loginSessionId = " << pbReq.sessionId() << ", phone = " << pbReq.phoneNumber() << ", verifyCodeId = " << pbReq.phoneVerifyCodeId() << ", verifyCode = " << pbReq.phoneVerifyCode();

    // 2. 发送 HTTP 请求
    QNetworkReply* resp = this->sendHttpRequest("/service/user/set_phone", body);

    // 3. 处理响应
    connect(resp, &QNetworkReply::finished, this, [=](){
        // a) 处理响应对象
        bool ok = false;
        QString reason;
        std::shared_ptr<proto::SetUserPhoneNumberRsp> pbResp = this->handleHttpResponse<proto::SetUserPhoneNumberRsp>(resp, &ok, &reason);

        // b) 判断响应是否正确
        if(!ok)
        {
            LOG() << "[修改手机号] 响应失败！reason = " << reason;
            return;
        }

        // c) 把响应的数据保存到 DataCenter 中
        dataCenter->resetPhone(phone);

        // d) 通知调用逻辑，响应已经处理完了，通过信号槽通知
        emit dataCenter->changePhoneDone();

        //e) 打印日志
        LOG() << "[修改手机号] 响应完毕！ requestId = " << pbResp->requestId();
    });
}

void NetClient::changeAvatar(const QString &loginSessionId, const QByteArray &avatar)
{
    // 修改头像                        /service/user/set_avatar
    // 1. 构造请求 body
    proto::SetUserAvatarReq pbReq;
    pbReq.setRequestId(makeRequestId());
    pbReq.setSessionId(loginSessionId);
    pbReq.setAvatar(avatar);
    QByteArray body = pbReq.serialize(&serializer);
    LOG()<<"[修改头像] 发送请求 requestId = " << pbReq.requestId() << ", loginSessionId = " << pbReq.sessionId();

    // 2. 发送 HTTP 请求
    QNetworkReply* resp = this->sendHttpRequest("/service/user/set_avatar", body);

    // 3. 处理响应
    connect(resp, &QNetworkReply::finished, this, [=](){
        // a) 处理响应对象
        bool ok = false;
        QString reason;
        std::shared_ptr<proto::SetUserAvatarRsp> pbResp = this->handleHttpResponse<proto::SetUserAvatarRsp>(resp, &ok, &reason);

        // b) 判断响应是否正确
        if(!ok)
        {
            LOG() << "[修改头像] 响应出错！reason = " << reason;
            return;
        }

        // c) 把数据保存到 DataCenter 中
        dataCenter->resetAvatar(avatar);

        // d) 通知调用逻辑，响应已经处理完了，通过信号槽通知
        emit dataCenter->changeAvatarDone();

        //e) 打印日志
        LOG() << "[修改头像] 响应完毕！ requestId = " << pbResp->requestId();
    });
}

void NetClient::deleteFriend(const QString &loginSessionId, const QString &userId)
{
    // 删除好友                        /service/friend/remove_friend
    // 1. 构造请求 body
    proto::FriendRemoveReq pbReq;
    pbReq.setRequestId(makeRequestId());
    pbReq.setSessionId(loginSessionId);
    pbReq.setPeerId(userId);
    QByteArray body = pbReq.serialize(&serializer);
    LOG()<<"[删除好友] 发送请求 requestId = " << pbReq.requestId() << ", loginSessionId = " << pbReq.sessionId() << ", peerId = " << pbReq.peerId();

    // 2. 发送 HTTP 请求
    QNetworkReply* resp = this->sendHttpRequest("/service/friend/remove_friend", body);

    // 3. 处理响应
    connect(resp, &QNetworkReply::finished, this, [=](){
        // a) 处理响应对象
        bool ok = false;
        QString reason;
        std::shared_ptr<proto::FriendRemoveRsp> pbResp = this->handleHttpResponse<proto::FriendRemoveRsp>(resp, &ok, &reason);

        // b) 判断响应是否正确
        if(!ok)
        {
            LOG() << "[删除好友] 响应出错！reason = " << reason;
            return;
        }

        // c) 把结果写入到 DataCenter 中
        dataCenter->removeFriend(userId);

        // d) 通知调用逻辑，响应已经处理完了，通过信号槽通知
        emit dataCenter->deleteFriendDone();

        //e) 打印日志
        LOG() << "[删除好友] 响应完毕！ requestId = " << pbResp->requestId();
    });
}

void NetClient::addFriendApply(const QString &loginSessionId, const QString &userId)
{
    // 发送好友申请                    /service/friend/add_friend_apply
    // 1. 构造请求 body
    proto::FriendAddReq pbReq;
    pbReq.setRequestId(makeRequestId());
    pbReq.setSessionId(loginSessionId);
    pbReq.setRespondentId(userId);
    QByteArray body = pbReq.serialize(&serializer);
    LOG()<<"[添加好友申请] 发送请求 requestId = " << pbReq.requestId() << ", loginSessionId = " << pbReq.sessionId() << ", userId = " << pbReq.respondentId();

    // 2. 发送 HTTP 请求
    QNetworkReply* resp = this->sendHttpRequest("/service/friend/add_friend_apply", body);

    // 3. 处理响应
    connect(resp, &QNetworkReply::finished, this, [=](){
        // a) 处理响应对象
        bool ok = false;
        QString reason;
        std::shared_ptr<proto::FriendAddRsp> pbResp = this->handleHttpResponse<proto::FriendAddRsp>(resp, &ok, &reason);

        // b) 判断响应是否正确
        if(!ok)
        {
            LOG() << "[添加好友申请] 响应出错！reason = " << reason;
            return;
        }

        // c) 把结果写入到 DataCenter 中，此处不需要记录任何数据

        // d) 通知调用逻辑，响应已经处理完了，通过信号槽通知
        emit dataCenter->addFriendApplyDone();

        //e) 打印日志
        LOG() << "[添加好友申请] 响应完毕！ requestId = " << pbResp->requestId();
    });
}

void NetClient::acceptFriendApply(const QString &loginSessionId, const QString &userId)
{
    // 好友申请处理                    /service/friend/add_friend_process
    // 1. 构造请求 body
    proto::FriendAddProcessReq pbReq;
    pbReq.setRequestId(makeRequestId());
    pbReq.setSessionId(loginSessionId);
    pbReq.setAgree(true);
    pbReq.setApplyUserId(userId);
    QByteArray body = pbReq.serialize(&serializer);
    LOG()<<"[同意好友申请] 发送请求 requestId = " << pbReq.requestId() << ", loginSessionId = " << pbReq.sessionId() << ", userId = " << pbReq.applyUserId();

    // 2. 发送 HTTP 请求
    QNetworkReply* resp = this->sendHttpRequest("/service/friend/add_friend_process", body);

    // 3. 处理响应
    connect(resp, &QNetworkReply::finished, this, [=](){
        // a) 处理响应对象
        bool ok = false;
        QString reason;
        std::shared_ptr<proto::FriendAddProcessRsp> pbResp = this->handleHttpResponse<proto::FriendAddProcessRsp>(resp, &ok, &reason);

        // b) 判断响应是否正确
        if(!ok)
        {
            LOG() << "[同意好友申请] 响应出错！reason = " << reason;
            return;
        }

        // c) 把结果写入到 DataCenter 中，此处做一个好友列表的更新
        // 一个是把数据从好友申请列表中删除掉
        // 另一个是把好友申请列表中的这个数据添加到好友列表中
        model::UserInfo applyUser = dataCenter->removeFromApplyList(userId);
        // 防止重复添加好友，所以先判断一下
        if(dataCenter->isInFriendList(userId))
        {
            emit dataCenter->updateApplyListUI();
            Toast::showMessage("你们已经是好友了，不能重复添加");
            return;
        }
        QList<model::UserInfo>* friendList = dataCenter->getFriendList();
        friendList->push_front(applyUser);

        // d) 通知调用逻辑，响应已经处理完了，通过信号槽通知
        emit dataCenter->acceptFriendApplyDone(applyUser.nickname);

        //e) 打印日志
        LOG() << "[同意好友申请] 响应完毕！ requestId = " << pbResp->requestId();
    });
}

void NetClient::rejectFriendApply(const QString &loginSessionId, const QString &userId)
{
    // 好友申请处理                    /service/friend/add_friend_process
    // 1. 构造请求 body
    proto::FriendAddProcessReq pbReq;
    pbReq.setRequestId(makeRequestId());
    pbReq.setSessionId(loginSessionId);
    pbReq.setAgree(false);
    pbReq.setApplyUserId(userId);
    QByteArray body = pbReq.serialize(&serializer);
    LOG()<<"[拒绝好友申请] 发送请求 requestId = " << pbReq.requestId() << ", loginSessionId = " << pbReq.sessionId() << ", userId = " << pbReq.applyUserId();

    // 2. 发送 HTTP 请求
    QNetworkReply* resp = this->sendHttpRequest("/service/friend/add_friend_process", body);

    // 3. 处理响应
    connect(resp, &QNetworkReply::finished, this, [=](){
        // a) 处理响应对象
        bool ok = false;
        QString reason;
        std::shared_ptr<proto::FriendAddProcessRsp> pbResp = this->handleHttpResponse<proto::FriendAddProcessRsp>(resp, &ok, &reason);

        // b) 判断响应是否正确
        if(!ok)
        {
            LOG() << "[拒绝好友申请] 响应出错！reason = " << reason;
            return;
        }

        // c) 删除申请列表元素，不需要更新好友列表
        dataCenter->removeFromApplyList(userId);

        // d) 通知调用逻辑，响应已经处理完了，通过信号槽通知
        emit dataCenter->rejectFriendApplyDone();

        //e) 打印日志
        LOG() << "[拒绝好友申请] 响应完毕！ requestId = " << pbResp->requestId();
    });
}

void NetClient::createGroupChatSession(const QString &loginSessionId, const QList<QString> &userIdList)
{
    // 创建消息会话                    /service/friend/create_chat_session
    // 1. 构造请求 body
    proto::ChatSessionCreateReq pbReq;
    pbReq.setRequestId(makeRequestId());
    pbReq.setSessionId(loginSessionId);
    pbReq.setChatSessionName("新的群聊");
    pbReq.setMemberIdList(userIdList);
    QByteArray body = pbReq.serialize(&serializer);
    LOG()<<"[创建群聊会话] 发送请求 requestId = " << pbReq.requestId() << ", loginSessionId = " << pbReq.sessionId()<< ", userIdList = " << userIdList;

    // 2. 发送 HTTP 请求
    QNetworkReply* resp = this->sendHttpRequest("/service/friend/create_chat_session", body);

    // 3. 处理响应
    connect(resp, &QNetworkReply::finished, this, [=](){
        // a) 处理响应对象
        bool ok = false;
        QString reason;
        std::shared_ptr<proto::ChatSessionCreateRsp> pbResp = this->handleHttpResponse<proto::ChatSessionCreateRsp>(resp, &ok, &reason);

        // b) 判断响应是否正确
        if(!ok)
        {
            LOG() << "[创建群聊会话] 响应出错！reason = " << reason;
            return;
        }

        // c) 由于此处创建好的会话，是 websocket 推送过来的
        // 在这里无需更新 DataCenter，后续通过 websocket 的逻辑来更新

        // d) 通知调用逻辑，响应已经处理完了，通过信号槽通知
        emit dataCenter->createGroupChatSessionDone();

        //e) 打印日志
        LOG() << "[创建群聊会话] 响应完毕！ requestId = " << pbResp->requestId();
    });
}

} // end network
