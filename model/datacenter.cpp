#include "datacenter.h"
#include <QStandardPaths>
#include <QFile>
#include <QDir>
#include <QJsonObject>
#include <QJsonDocument>

namespace model{

DataCenter* DataCenter::instance = nullptr;
DataCenter *DataCenter::getInstance()
{
    if(instance == nullptr)
    {
        instance = new DataCenter();
    }
    return instance;
}

DataCenter::~DataCenter()
{
    delete myself;
    delete friendList;
    delete chatSessionList;
    delete memberList;
    delete applyList;
    delete recentMessages;
    delete unreadMessageCount;
    delete searchUserResult;
    delete searchMessageResult;
}

DataCenter::DataCenter()
    :netClient(this)
{
    recentMessages = new QHash<QString ,QList<Message>>();
    memberList = new QHash<QString, QList<UserInfo>>();
    unreadMessageCount = new QHash<QString, int>();

    // 加载数据
    initDataFile();
    loadDataFile();
}

void DataCenter::initDataFile()
{
    // 构造出文件路径，使用 appData 存储文件
    QString basePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString filePath = basePath + "/ChatClient.json";
    downLoadPath = basePath + "/downLoad/";
    LOG() << "filePath = " << filePath;
    LOG() << "downLoadPath = " << downLoadPath;

    QDir dir;
    if(!dir.exists(basePath))
    {
        dir.mkpath(basePath);
    }
    if(!dir.exists(downLoadPath))
    {
        dir.mkpath(downLoadPath);
    }

    // 创建文件
    // 写方式打开，并且写入初始内容
    QFile file(filePath);
    if(!file.exists())
    {
        if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            LOG() << "打开文件失败!" << file.errorString();
            file.close();
            return;
        }
    }

    // // 打开成功，写入初始内容
    // QString data = "{\n\n}";
    // file.write(data.toUtf8());
    file.close();
}

void DataCenter::saveDataFile()
{
    QString filePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/ChatClient.json";
    QFile file(filePath);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        LOG() << "打开文件失败!" << file.errorString();
        file.close();
        return;
    }

    // 按照 json 格式来写入数据
    QJsonObject jsonObj;
    jsonObj["loginSessionId"] = loginSessionId;

    QJsonObject jsonUnread;
    for(auto it = unreadMessageCount->begin(); it != unreadMessageCount->end(); ++it)
    {
        jsonUnread[it.key()] = it.value();
    }
    jsonObj["unread"] = jsonUnread;

    // 把json写入文件
    QJsonDocument jsonDoc(jsonObj);
    QString s = jsonDoc.toJson();
    file.write(s.toUtf8());

    // 关闭文件
    file.close();
}

// 加载文件，是在 DataCenter 被实例化的时候，调用执行的
void DataCenter::loadDataFile()
{
    // 确保在加载之前，现针对文件进行初始化
    QString filePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/ChatClient.json";

    // 判定文件是否存在，不存在则初始化，并创建出新的 json 文件
    QFileInfo fileInfo(filePath);
    if(!fileInfo.exists())
    {
        initDataFile();
    }

    // 读方式打开文件
    QFile file(filePath);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        LOG() << "打开文件失败!" << file.errorString();
        file.close();
        return;
    }

    // 读取文件内容，解析为 Json 对象
    QJsonDocument jsonDoc = QJsonDocument::fromJson(file.readAll());
    if(jsonDoc.isNull())
    {
        LOG() << "解析 Json 文件失败！Json 文件格式有误";
        file.close();
        return;
    }

    QJsonObject jsonObj = jsonDoc.object();
    this->loginSessionId = jsonObj["loginSessionId"].toString();
    LOG() << "loginSessionId = " << loginSessionId;

    this->unreadMessageCount->clear();
    QJsonObject jsonUnread = jsonObj["unread"].toObject();

    for(auto it = jsonUnread.begin(); it != jsonUnread.end(); ++it)
    {
        this->unreadMessageCount->insert(it.key(), it.value().toInt());
    }

    file.close();
}
QString getDownLoadPath()
{
    return downLoadPath;
}

void DataCenter::clearUnread(const QString &chatSessionId)
{
    (*unreadMessageCount)[chatSessionId] = 0;

    // 保存结果到文件中
    saveDataFile();
}

void DataCenter::addUnread(const QString &chatSessionId)
{
    ++(*unreadMessageCount)[chatSessionId];
}

int DataCenter::getUnread(const QString &chatSessionId)
{
    return (*unreadMessageCount)[chatSessionId];
}

void DataCenter::getMyselfAsync()
{
    netClient.getMyself(loginSessionId);
}

UserInfo *DataCenter::getMyself()
{
    return myself;
}

void DataCenter::resetMyself(std::shared_ptr<proto::GetUserInfoRsp> resp)
{
    if(myself == nullptr)
        myself = new UserInfo();
    const proto::UserInfo& userInfo = resp->userInfo();
    myself->load(userInfo);
}

void DataCenter::getFriendListAsync()
{
    netClient.getFriendList(loginSessionId);
}



QList<UserInfo> *DataCenter::getFriendList()
{
    return friendList;
}

void DataCenter::resetFriendList(std::shared_ptr<proto::GetFriendListRsp> resp)
{
    if(friendList == nullptr)
        friendList = new QList<UserInfo>();
    friendList->clear();

    const QList<proto::UserInfo>& friendListPB = resp->friendList();
    for(auto& f : friendListPB)
    {
        UserInfo userInfo;
        userInfo.load(f);
        friendList->push_back(userInfo);
    }
}

void DataCenter::getSessionListAsync()
{
    netClient.getSessionList(loginSessionId);
}

QList<ChatSessionInfo> *DataCenter::getSessionList()
{
    return chatSessionList;
}

void DataCenter::resetChatSessionList(std::shared_ptr<proto::GetChatSessionListRsp> resp)
{
    if(chatSessionList == nullptr)
        chatSessionList = new QList<ChatSessionInfo>();
    chatSessionList->clear();

    const QList<proto::ChatSessionInfo>& chatSessionListPB = resp->chatSessionInfoList();
    for(auto& c : chatSessionListPB)
    {
        ChatSessionInfo chatSessionInfo;
        chatSessionInfo.load(c);
        chatSessionList->push_back(chatSessionInfo);
    }
}

void DataCenter::getApplyListAsync()
{
    netClient.getApplyList(loginSessionId);
}

QList<UserInfo> *DataCenter::getApplyList()
{
    return applyList;
}

void DataCenter::resetApplyList(std::shared_ptr<proto::GetPendingFriendEventListRsp> resp)
{
    if(applyList == nullptr)
        applyList = new QList<UserInfo>();
    applyList->clear();

    const QList<proto::FriendEvent>& eventList = resp->event();
    for(auto& event : eventList)
    {
        UserInfo userInfo;
        userInfo.load(event.sender());
        applyList->push_back(userInfo);
    }
}

void DataCenter::getRecentMessageListAsync(const QString &chatSessionId, bool updateUI)
{
    netClient.getRecentMessageList(loginSessionId, chatSessionId, updateUI);
}

QList<Message> *DataCenter::getRecentMessageList(const QString &chatSessionId)
{
    if(!recentMessages->contains(chatSessionId))
        return nullptr;
    return &(*recentMessages)[chatSessionId];
}

void DataCenter::resetRecentMessageList(const QString &chatSessionId, std::shared_ptr<proto::GetRecentMsgRsp> resp)
{
    // 拿到 chatSessionId 对应的消息列表并清空
    QList<Message>& messageList = (*recentMessages)[chatSessionId];
    messageList.clear();

    // 遍历响应结果的列表
    QList<proto::MessageInfo> messageInfoList =  resp->msgList();
    for(const proto::MessageInfo& m : messageInfoList)
    {
        Message message;
        message.load(m);

        messageList.push_back(message);
    }
}

void DataCenter::sendTextMessageAsync(const QString &chatSessionId, const QString &content)
{
    netClient.sendMessage(loginSessionId, chatSessionId, MessageType::TEXT_TYPE, content.toUtf8(), "");
}

void DataCenter::sendImageMessageAsync(const QString &chatSessionId, const QByteArray &content)
{
    netClient.sendMessage(loginSessionId, chatSessionId, IMAGE_TYPE, content, "");
}

void DataCenter::sendFileMessageAsync(const QString &chatSessionId, const QString& path, const QString &fileName, const QByteArray &content)
{
    netClient.sendMessage(loginSessionId, chatSessionId, FILE_TYPE, content, fileName, path);
}

void DataCenter::sendSpeechMessageAsync(const QString &chatSessionId, const QByteArray &content)
{
    netClient.sendMessage(loginSessionId, chatSessionId, SPEECH_TYPE, content, "");
}

void DataCenter::changeNicknameAsync(const QString &nickname)
{
    netClient.changeNickname(loginSessionId, nickname);
}

void DataCenter::resetNickname(const QString &nickname)
{
    if(myself == nullptr)
        return;
    myself->nickname = nickname;
}

void DataCenter::changeDescriptionAsync(const QString &desc)
{
    netClient.changeDescription(loginSessionId, desc);
}

void DataCenter::resetDescription(const QString &desc)
{
    if(myself == nullptr)
        return;
    myself->description = desc;
}

void DataCenter::getVerifyCodeAsync(const QString &phone)
{
    netClient.getVerifyCode(phone);
}

void DataCenter::resetVerifyCodeId(const QString &verifyCodeId)
{
    this->currentVerifyCodeId = verifyCodeId;
}

const QString &DataCenter::getVerifyCodeId()
{
    return currentVerifyCodeId;
}

void DataCenter::changePhoneAsync(const QString &phone, const QString &verifyCodeId, const QString &verifyCode)
{
    netClient.changePhone(loginSessionId, phone, verifyCodeId, verifyCode);
}

void DataCenter::resetPhone(const QString &phone)
{
    if(myself == nullptr)
        return;
    myself->phone = phone;
}

void DataCenter::changeAvatarAsync(const QByteArray &imageBytes)
{
    netClient.changeAvatar(loginSessionId, imageBytes);
}

void DataCenter::resetAvatar(const QByteArray &avatar)
{
    if(myself == nullptr)
        return;
    myself->avatar = makeIcon(avatar);
}

void DataCenter::deleteFriendAsync(const QString &userId)
{
    netClient.deleteFriend(loginSessionId, userId);
}

void DataCenter::removeFriend(const QString &userId)
{
    // 遍历 好友列表 friendList
    if(friendList == nullptr || chatSessionList == nullptr)
        return;
    friendList->removeIf([=](const UserInfo& userInfo){
        return userInfo.userId == userId;
    });

    // 还要考虑 会话列表 chatSessionList
    chatSessionList->removeIf([=](const ChatSessionInfo& chatSessionInfo){
        if(chatSessionInfo.userId == "")
        {
            // 群聊
            return false;
        }
        if(chatSessionInfo.userId == userId)
        {
            if(chatSessionInfo.chatSessionId == this->currentChatSessionId)
                emit this->clearCurrentSession();

            return true;
        }

        return false;
    });
}

void DataCenter::addFriendApplyAsync(const QString &userId)
{
    netClient.addFriendApply(loginSessionId, userId);
}

void DataCenter::acceptFriendApplyAsync(const QString &userId)
{
    netClient.acceptFriendApply(loginSessionId, userId);
}

UserInfo DataCenter::removeFromApplyList(const QString &userId)
{
    if(applyList == nullptr)
        return UserInfo();

    for(auto it = applyList->begin(); it != applyList->end(); ++it)
    {
        if(it->userId == userId)
        {
            UserInfo toDelete = *it;
            it = applyList->erase(it);
            return toDelete;
        }
    }
    return UserInfo();
}

bool DataCenter::isInFriendList(const QString &userId)
{
    if(friendList == nullptr)
        return false;

    for(auto it = friendList->begin(); it != friendList->end(); ++it)
    {
        if(it->userId == userId)
        {
            return true;
        }
    }
    return false;
}

void DataCenter::rejectFriendApplyAsync(const QString &userId)
{
    netClient.rejectFriendApply(loginSessionId, userId);
}

void DataCenter::createGroupChatSessionAsync(const QList<QString> &userIdList)
{
    netClient.createGroupChatSession(loginSessionId, userIdList);
}

void DataCenter::exitGroupChatSessionAsync(const QString &chatSessionId)
{
    netClient.exitGroupChatSession(loginSessionId, chatSessionId);
}

void DataCenter::removeChatSession(const QString &chatSessionId)
{
    // 遍历 好友列表 friendList
    if(chatSessionList == nullptr)
        return;

    // 还要考虑 会话列表 chatSessionList
    chatSessionList->removeIf([=](const ChatSessionInfo& chatSessionInfo){
        if(chatSessionInfo.userId == "")
        {
            // 群聊
            if(chatSessionInfo.chatSessionId == chatSessionId)
            {
                if(chatSessionInfo.chatSessionId == this->currentChatSessionId)
                    emit this->clearCurrentSession();
            }
            return true;
        }
        return false;
    });
}

void DataCenter::getMemberListAsync(const QString &chatSessionId)
{
    netClient.getMemberList(loginSessionId, chatSessionId);
}

QList<UserInfo> *DataCenter::getMemberList(const QString& chatSessionId)
{
    if(!this->memberList->contains(chatSessionId))
        return nullptr;
    return &(*this->memberList)[chatSessionId];
}

void DataCenter::resetMemberList(const QString &chatSessionId, const QList<proto::UserInfo> &memberList)
{
    // 根据 chatSessionId，这个 key ，得到对应的 value
    QList<UserInfo>& currentMemberList = (*this->memberList)[chatSessionId];
    currentMemberList.clear();

    for(const auto& m : memberList)
    {
        UserInfo userInfo;
        userInfo.load(m);
        currentMemberList.push_back(userInfo);
    }
}

void DataCenter::inviteFriendJoinFroupAsync(const QString &chatSessionId, const QList<QString> &userIdList)
{
    netClient.inviteFriendJoinFroup(loginSessionId, chatSessionId, userIdList);
}

void DataCenter::changeGroupnameAsync(const QString &chatSessionId, const QString &groupname)
{
    netClient.changeGroupnameAsync(loginSessionId, chatSessionId, groupname);
}

void DataCenter::searchUserAsync(const QString &searchKey)
{
    netClient.searchUser(loginSessionId, searchKey);
}

QList<UserInfo> *DataCenter::getSearchUserResult()
{
    return searchUserResult;
}

void DataCenter::resetSearchUserResult(const QList<proto::UserInfo> &userList)
{
    if(searchUserResult == nullptr)
        searchUserResult = new QList<UserInfo>;

    searchUserResult->clear();

    for(const auto& u : userList)
    {
        UserInfo userInfo;
        userInfo.load(u);
        searchUserResult->push_back(userInfo);
    }
}

void DataCenter::searchMessageAsync(const QString &searchKey)
{
    netClient.searchMessage(loginSessionId, this->currentChatSessionId, searchKey);
}

void DataCenter::searchMessageByTimeAsync(const QDateTime &begTime, const QDateTime &endTime)
{
    netClient.searchMessageByTime(loginSessionId, this->currentChatSessionId, begTime, endTime);
}

QList<Message> *DataCenter::getSearchMessageResult()
{
    return this->searchMessageResult;
}

void DataCenter::resetSearchMessageResult(const QList<proto::MessageInfo> &msgList)
{
    if(this->searchMessageResult == nullptr)
        this->searchMessageResult = new QList<Message>();

    this->searchMessageResult->clear();

    for(const auto& m : msgList)
    {
        Message message;
        message.load(m);
        searchMessageResult->push_back(message);
    }
}

void DataCenter::userLoginAsync(const QString &username, const QString &password)
{
    netClient.userLogin(username, password);
}

void DataCenter::resetLoginSessionId(const QString &loginSessionId)
{
    this->loginSessionId = loginSessionId;

    // 一旦会话 ID 改变，就需要保存到硬盘上
    saveDataFile();
}

void DataCenter::userRegisterAsync(const QString &username, const QString &password)
{
    netClient.userRegister(username, password);
}

void DataCenter::phoneLoginAsync(const QString &phone, const QString &verifyCode)
{
    netClient.phoneLogin(phone, this->currentVerifyCodeId, verifyCode);
}

void DataCenter::phoneRegisterAsync(const QString &phone, const QString &verifyCode)
{
    netClient.phoneRegister(phone, this->currentVerifyCodeId, verifyCode);
}

void DataCenter::getSingleFileAsync(const QString &fileId)
{
    netClient.getSingleFile(loginSessionId, fileId);
}

void DataCenter::speechConvertTextAsync(const QString &fileId, const QByteArray &content)
{
    netClient.speechConvertText(loginSessionId, fileId, content);
}

void DataCenter::getFileIdAsync(const QString &messageId)
{
    netClient.getFileId(loginSessionId, messageId);
}

ChatSessionInfo *DataCenter::findChatSessionById(const QString &chatSessionId)
{
    if(chatSessionList == nullptr)
        return nullptr;
    for(auto& info : *chatSessionList)
    {
        if(info.chatSessionId == chatSessionId)
            return &info;
    }
    return nullptr;
}

ChatSessionInfo *DataCenter::findChatSessionByUserId(const QString &userId)
{
    if(chatSessionList == nullptr)
        return nullptr;
    for(auto& info : *chatSessionList)
    {
        if(info.userId == userId)
            return &info;
    }
    return nullptr;
}

void DataCenter::topChatSessionInfo(const ChatSessionInfo &chatSessionInfo)
{
    // 1. 从列表中找到这个元素
    if(chatSessionList == nullptr)
        return;
    auto it = chatSessionList->begin();
    for(; it != chatSessionList->end(); ++it)
    {
        if(it->chatSessionId == chatSessionInfo.chatSessionId)
            break;
    }

    if(it == chatSessionList->end())
        return;

    // 2. 把这个元素备份一下，然后删除
    ChatSessionInfo backup = chatSessionInfo;
    chatSessionList->erase(it);

    // 3. 把备份的元素插入到头部
    chatSessionList->push_front(backup);
}

UserInfo* DataCenter::findFriendById(const QString &userId)
{
    if(this->friendList == nullptr)
        return nullptr;
    for(UserInfo& f : *friendList)
    {
        if(f.userId == userId)
            return &f;
    }
    return nullptr;
}

void DataCenter::setCurrentChatSessionId(const QString &chatSessionId)
{
    currentChatSessionId = chatSessionId;
}

const QString &DataCenter::getCurrentChatSessionId()
{
    return currentChatSessionId;
}

void DataCenter::addMessage(const Message &message)
{
    if(recentMessages == nullptr)
        return;
    QList<Message>& messageList = (*recentMessages)[message.chatSessionId];
    messageList.push_back(message);
}



} // end namespace
