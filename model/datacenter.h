#ifndef DATACENTER_H
#define DATACENTER_H

#include <QWidget>
#include "data.h"
#include "../network/netclient.h"

namespace model{


class DataCenter : public QObject
{
    Q_OBJECT
public:
    static DataCenter* getInstance();
    ~DataCenter();
private:
    DataCenter();
    static DataCenter* instance;

    // DataCenter 中要组织管理的所有的数据

    // 当前客户端登录到服务器对应的登录会话id
    QString loginSessionId = "";

    // 当前的用户信息
    UserInfo* myself = nullptr;

    // 好友列表
    QList<UserInfo>* friendList = nullptr;

    // 会话列表
    QList<ChatSessionInfo>* chatSessionList = nullptr;
    // 记录当前选中的会话是哪个
    QString currentChatSessionId = "";
    // 记录每个会话中，都有哪些成员（主要针对群聊），key 为 chatSessionId，value 为成员列表
    QHash<QString, QList<UserInfo>>* memberList = nullptr;

    // 待处理的好友申请列表
    QList<UserInfo>* applyList = nullptr;

    // 每个会话的最近消息列表，key 为 chatSessionId，value 为消息列表
    QHash<QString ,QList<Message>>* recentMessages = nullptr;

    // 存储每个会话，未读消息的个数，key 为 chatSessionId，value 为未读消息个数
    QHash<QString, int>* unreadMessageCount = nullptr;

    // 用户添加好友的好友搜索结果
    QList<UserInfo>* searchUserResult = nullptr;

    // 历史消息搜索结果
    QList<Message>* searchMessageResult = nullptr;

    // 短信验证码的验证 id
    QString currentVerifyCodeId = "";

    // 让 DataCenter 持有 NetClient 实例
    network::NetClient netClient;

public:
    // 初始化数据文件
    void initDataFile();
    // 存储数据到文件
    void saveDataFile();
    // 从数据文件中加载数据到内存
    void loadDataFile();
    // 清空未读消息数目
    void clearUnread(const QString& chatSessionId);
    // 增加未读消息数目
    void addUnread(const QString& chatSessionId);
    // 获取未读消息数目
    int getUnread(const QString& chatSessionId);

    // 获取到当前的登录会话
    const QString& getLoginSessionId()const{return loginSessionId;}
    // 验证网络连通性
    void ping() {netClient.ping();}
    void initWebSocket(){netClient.initWebSocket();}

    /////////////////////////////////////////////////////////////////
    /// 核心函数
    /////////////////////////////////////////////////////////////////
    // 通过网络获取到用户的个人信息(异步，只负责发送请求)
    void getMyselfAsync();
    UserInfo* getMyself();
    void resetMyself(std::shared_ptr<proto::GetUserInfoRsp> resp);

    // 通过网络获取好友列表
    void getFriendListAsync();
    QList<UserInfo>* getFriendList();
    void resetFriendList(std::shared_ptr<proto::GetFriendListRsp> resp);

    // 获取会话列表
    void getSessionListAsync();
    QList<ChatSessionInfo>* getSessionList();
    void resetChatSessionList(std::shared_ptr<proto::GetChatSessionListRsp> resp);

    // 获取好友申请列表
    void getApplyListAsync();
    QList<UserInfo>* getApplyList();
    void resetApplyList(std::shared_ptr<proto::GetPendingFriendEventListRsp> resp);

    // 获取最近消息列表
    void getRecentMessageListAsync(const QString& chatSessionId, bool updateUI);
    QList<Message>* getRecentMessageList(const QString& chatSessionId);
    void resetRecentMessageList(const QString& chatSessionId, std::shared_ptr<proto::GetRecentMsgRsp> resp);

    // 发送信息给服务器
    void senTextMessageAsync(const QString& chatSessionId, const QString& content);

    // 修改用户昵称
    void changeNicknameAsync(const QString& nickname);
    void resetNickname(const QString& nickname);

    // 修改用户签名
    void changeDescriptionAsync(const QString& desc);
    void resetDescription(const QString& desc);

    // 获取短信验证码
    void getVerifyCodeAsync(const QString& phone);
    void resetVerifyCodeId(const QString& verifyCodeId);
    const QString& getVerifyCodeId();

    // 修改手机号码
    void changePhoneAsync(const QString& phone, const QString& verifyCodeId, const QString& verifyCode);
    void resetPhone(const QString& phone);

    // 修改头像
    void changeAvatarAsync(const QByteArray& imageBytes);
    void resetAvatar(const QByteArray& avatar);

    // 删除好友
    void deleteFriendAsync(const QString& userId);
    void removeFriend(const QString& userId);

    // 发送好友申请
    void addFriendApplyAsync(const QString& userId);

    // 发送同意好友申请操作
    void acceptFriendApplyAsync(const QString& userId);
    UserInfo removeFromApplyList(const QString& userId);
    bool isInFriendList(const QString& userId);

    // 拒绝好友申请操作
    void rejectFriendApplyAsync(const QString& userId);

    // 创建群聊
    void createGroupChatSessionAsync(const QList<QString>& userIdList);

    /////////////////////////////////////////////////////////////////
    /// 辅助函数
    /////////////////////////////////////////////////////////////////
    // 根据会话Id查询会话信息
    ChatSessionInfo* findChatSessionById(const QString& chatSessionId);
    //根据用户Id查询会话信息
    ChatSessionInfo* findChatSessionByUserId(const QString& userId);
    // 把指定的会话信息放到列表头部
    void topChatSessionInfo(const ChatSessionInfo& chatSessionInfo);\
    // 根据用户 Id 查询好友信息
    UserInfo* findFriendById(const QString& userId);

    // 设置/获取当前选中的会话
    void setCurrentChatSessionId(const QString& chatSessionId);
    const QString& getCurrentChatSessionId();

    // 添加消息到 dataCenter 中
    void addMessage(const Message& message);

signals:
    // 自定义信号
    void getMyselfDone();
    void getFriendListDone();
    void getChatSessionListDone();
    void getApplyListDone();
    void getRecentMessageListDone(const QString& chatSessionId);
    void getRecentMessageListDoneNoUI(const QString& chatSessionId);
    void sendMessageDone(MessageType messageType, const QByteArray& content, const QString& extraInfo);
    void updateLastMessage(const QString& currentChatSessionId);
    void receiveMessageDone(const model::Message& lastMessage);
    void changeNicknameDone();
    void changeDescriptionDone();
    void getVerifyCodeDone();
    void changePhoneDone();
    void changeAvatarDone();
    void deleteFriendDone();
    void clearCurrentSession();
    void addFriendApplyDone();
    void receiveFriendApplyDone();
    void acceptFriendApplyDone(const QString& nickName);
    void receiveFriendProcessDone(const QString& nickName, bool agree);
    void rejectFriendApplyDone();
    void updateApplyListUI();
    void createGroupChatSessionDone();
    void receiveSessionCreateDone();
};


} // end namespace


#endif // DATACENTER_H
