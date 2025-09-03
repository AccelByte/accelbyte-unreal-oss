// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/Launch/Resources/Version.h"
#if ENGINE_MAJOR_VERSION >= 5
#include "Online/CoreOnline.h"
#else
#include "UObject/CoreOnline.h"
#endif
#include "OnlineSubsystemAccelByte.h"
#include "OnlineUserCacheAccelByte.h"
#include "Models/AccelByteChatModels.h"
#include "Interfaces/OnlineChatInterface.h"
#include "OnlineSubsystemAccelBytePackage.h"

struct FAccelByteChatRoomConfig {
	// Flag indicating whether users can join the chat room without an invite
	bool bIsJoinable{false};

	// Name for the chat room
	FString FriendlyName;
};

class FAccelByteChatMessage;
class FAccelByteChatRoomInfo;
class FAccelByteChatRoomMember;

using FAccelByteChatRoomInfoRef = TSharedRef<FAccelByteChatRoomInfo>;
using FAccelByteChatRoomInfoPtr = TSharedPtr<FAccelByteChatRoomInfo>;
using FAccelByteChatRoomMemberRef = TSharedRef<FAccelByteChatRoomMember>;
using FAccelByteChatRoomMemberPtr = TSharedPtr<FAccelByteChatRoomMember>;

//~ Begin custom delegates
DECLARE_MULTICAST_DELEGATE_FourParams(FOnConnectChatComplete, int32 /*LocalUserNum*/, bool /*bWasSuccessful*/, const FUniqueNetId& /*UserId*/, const FString& /*Error*/);
typedef FOnConnectChatComplete::FDelegate FOnConnectChatCompleteDelegate;

DECLARE_MULTICAST_DELEGATE_TwoParams(FAccelByteOnChatReconnectAttempted, int32 /*LocalUserNum*/, const AccelByte::FReconnectAttemptInfo& /*Reconnection attempt info*/);
typedef FAccelByteOnChatReconnectAttempted::FDelegate FAccelByteOnChatReconnectAttemptedDelegate;

DECLARE_MULTICAST_DELEGATE_TwoParams(FAccelByteOnChatMassiveOutageEvent, int32 /*LocalUserNum*/, const AccelByte::FMassiveOutageInfo& /*Reconnect exhaustion info*/);
typedef FAccelByteOnChatMassiveOutageEvent::FDelegate FAccelByteOnChatMassiveOutageEventDelegate;

DECLARE_MULTICAST_DELEGATE_FourParams(FOnChatConnectionClosed, int32 /*LocalUserNum*/, int32 /* StatusCode */, const FString& /* Reason */, bool /* bWasClean */);
typedef FOnChatConnectionClosed::FDelegate FOnChatConnectionClosedDelegate;

DECLARE_MULTICAST_DELEGATE_FourParams(FOnSendChatComplete, FString /*UserId*/, FString /*MsgBody*/, FString /*RoomId*/, bool /*bWasSuccessful*/);
typedef FOnSendChatComplete::FDelegate FOnSendChatCompleteDelegate;

DECLARE_MULTICAST_DELEGATE_FiveParams(FOnSendChatCompleteWithError, FString /*UserId*/, FString /*MsgBody*/, FString /*RoomId*/, bool /*bWasSuccessful*/, const FOnlineError& /*Error*/)
typedef FOnSendChatCompleteWithError::FDelegate FOnSendChatCompleteWithErrorDelegate;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnChatDisconnected, FString /*Message*/);
typedef FOnChatDisconnected::FDelegate FOnChatDisconnectedDelegate;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnReadChatReceived, TArray<FAccelByteModelsChatReadChatData> /*ReadChatsData*/)
typedef FOnReadChatReceived::FDelegate FOnReadChatReceivedDelegate;

DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnTopicAdded, FString /*ChatTopicName*/, FString /*TopicId*/, FString /*UserId*/)
typedef FOnTopicAdded::FDelegate FOnTopicAddedDelegate;

DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnTopicRemoved, FString /*ChatTopicName*/, FString /*TopicId*/, FString /*UserId*/)
typedef FOnTopicRemoved::FDelegate FOnTopicRemovedDelegate;

DECLARE_MULTICAST_DELEGATE_FourParams(FOnTopicUpdated, FString /*ChatTopicName*/, FString /*TopicId*/, FString /*SenderId*/, bool /*IsChannel */)
typedef FOnTopicUpdated::FDelegate FOnTopicUpdatedDelegate;

DECLARE_MULTICAST_DELEGATE_FourParams(FOnTopicDeleted, FString /*ChatTopicName*/, FString /*TopicId*/, FString /*SenderId*/, bool /*IsChannel */)
typedef FOnTopicDeleted::FDelegate FOnTopicDeletedDelegate;

DECLARE_MULTICAST_DELEGATE_FiveParams(FOnUserBanned, FString /*UserId*/, EBanType /*Ban*/, FString /*EndDate*/, EBanReason /*Reason*/, bool /*Enable*/)
typedef FOnUserBanned::FDelegate FOnUserBannedDelegate;

DECLARE_MULTICAST_DELEGATE_FiveParams(FOnUserUnbanned, FString /*UserId*/, EBanType /*Ban*/, FString /*EndDate*/, EBanReason /*Reason*/, bool /*Enable*/)
typedef FOnUserUnbanned::FDelegate FOnUserUnbannedDelegate;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnSystemMessageReceived, const FUniqueNetId& /*UserId*/, const FSystemMessageNotifMessage& /*SystemMessage*/);
typedef FOnSystemMessageReceived::FDelegate FOnSystemMessageReceivedDelegate;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnTransientSystemMessageReceived, const FUniqueNetId& /*UserId*/, const FAccelByteModelsChatSystemMessageNotif& /*TransientSystemMessage*/);
typedef FOnTransientSystemMessageReceived::FDelegate FOnTransientSystemMessageReceivedDelegate;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnDeleteSystemMessagesComplete, const FUniqueNetId& /*UserId*/, const FOnlineError& /*ErrorInfo*/)
typedef FOnDeleteSystemMessagesComplete::FDelegate FOnDeleteSystemMessagesCompleteDelegate;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnUpdateSystemMessagesComplete, const FUniqueNetId& /*UserId*/, const FOnlineError& /*ErrorInfo*/)
typedef FOnUpdateSystemMessagesComplete::FDelegate FOnUpdateSystemMessagesCompleteDelegate;

DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnQuerySystemMessageComplete, const FUniqueNetId& /*UserId*/, const TArray<FSystemMessageNotifMessage>& /*SystemMessages*/, const FOnlineError& /*ErrorInfo*/)
typedef FOnQuerySystemMessageComplete::FDelegate FOnQuerySystemMessageCompleteDelegate;

DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnQueryTransientSystemMessageComplete, const FUniqueNetId& /*UserId*/, const TArray<FAccelByteModelsQuerySystemMessagesResponseItem>& /*TransientSystemMessages*/, const FOnlineError& /*ErrorInfo*/)
typedef FOnQueryTransientSystemMessageComplete::FDelegate FOnQueryTransientSystemMessageCompleteDelegate;

DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnGetSystemMessageStatsComplete, const FUniqueNetId& /*UserId*/, const FAccelByteGetSystemMessageStatsResponse& /*SystemMessagesStats*/, const FOnlineError& /*ErrorInfo*/)
typedef FOnGetSystemMessageStatsComplete::FDelegate FOnGetSystemMessageStatsCompleteDelegate;

DECLARE_MULTICAST_DELEGATE_FourParams(FOnGetChatConfigComplete, int32 /*LocalUserNum*/, bool /*bWasSuccessful*/, const FAccelByteModelsChatPublicConfigResponse& /*ChatPublicConfigResponse*/, const FString& /*Error*/);
typedef FOnGetChatConfigComplete::FDelegate FOnGetChatConfigCompleteDelegate;

DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnGetUserChatConfigurationComplete, const int32 /*LocalUserNum*/, const FAccelByteModelsGetUserChatConfigurationResponse& /*UserChatConfiguration*/, const FOnlineError& /*ErrorInfo*/)
typedef FOnGetUserChatConfigurationComplete::FDelegate FOnGetUserChatConfigurationCompleteDelegate;

DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnSetUserChatConfigurationComplete, const int32 /*LocalUserNum*/, const FAccelByteSetUserChatConfigurationResponse& /*SetUserChatConfigurationResponse*/, const FOnlineError& /*ErrorInfo*/)
typedef FOnSetUserChatConfigurationComplete::FDelegate FOnSetUserChatConfigurationCompleteDelegate;

DECLARE_DELEGATE_FourParams(FOnReportChatMessageComplete, const FUniqueNetId& /*UserId*/, bool /*bWasSuccessful*/, const FChatMessage& /*ReportedMessage*/, FAccelByteModelsReportingSubmitResponse /*ReportResponse*/);

DECLARE_DELEGATE_ThreeParams(FOnChatQueryRoomComplete, bool /*bWasSuccessful*/, TArray<FAccelByteChatRoomInfoRef> /*Result*/, int32 /*LocalUserNum*/);

//~ End custom delegates

using FChatRoomIdToChatMessages = TMap<FChatRoomId, TArray<TSharedRef<FChatMessage>>>;
using FUserIdToRoomChatMessages = TMap<TSharedRef<const FUniqueNetIdAccelByteUser>, FChatRoomIdToChatMessages, FDefaultSetAllocator, TUserUniqueIdConstSharedRefMapKeyFuncs<FChatRoomIdToChatMessages>>;

class ONLINESUBSYSTEMACCELBYTE_API FAccelByteChatMessage : public FChatMessage
{
public:

	/** Constructor */
	explicit FAccelByteChatMessage(const FUniqueNetIdRef& InSenderUserId
		, FString InSenderNickname
		, FString InMessageBody
		, FDateTime InTimestamp
		, FString InChatId
		, FString InTopicId
		, EAccelBytePlatformType InSenderPlatformId)
		: SenderUserId(InSenderUserId)
		, MessageBody(InMessageBody)
		, SenderNickname(InSenderNickname)
		, Timestamp(InTimestamp)
		, ChatId(InChatId)
		, TopicId(InTopicId)
		, SenderPlatformId(InSenderPlatformId)
	{
	}

	//~ Begin FChatMessage overrides
	virtual const FUniqueNetIdRef& GetUserId() const override;
	virtual const FString& GetNickname() const override;
	virtual const FString& GetBody() const override;
	virtual const FDateTime& GetTimestamp() const override;
	//~ End FChatMessage overrides

	const FString& GetChatId() const;
	const FString& GetTopicId() const;
	EAccelBytePlatformType GetSenderPlatformId() const;

PACKAGE_SCOPE:
	FUniqueNetIdRef SenderUserId;
	FString MessageBody{};
	FString SenderNickname{};
	FDateTime Timestamp{};
	FString ChatId{};
	FString TopicId{};
	EAccelBytePlatformType SenderPlatformId{};
};

class ONLINESUBSYSTEMACCELBYTE_API FAccelByteChatRoomMember : public FChatRoomMember
{
public:
	FAccelByteChatRoomMember(const FUniqueNetIdAccelByteUserRef& InUserId, const FString& InNickname);
	static FAccelByteChatRoomMemberRef Create(const FUniqueNetIdAccelByteUserRef& InUserId, const FString& InNickname);
	
	virtual const FUniqueNetIdRef& GetUserId() const override;
	virtual const FString& GetNickname() const override;

PACKAGE_SCOPE:
	void SetNickname(const FString& InNickname);
	bool HasNickname() const;
	
private:
	FUniqueNetIdRef UserId;
	FString Nickname;
};

class ONLINESUBSYSTEMACCELBYTE_API FAccelByteChatRoomInfo : public FChatRoomInfo
{
public:
	//~ Begin FChatRoomInfo overrides
	virtual const FChatRoomId& GetRoomId() const override;
	virtual const FUniqueNetIdRef& GetOwnerId() const override;
	virtual const FString& GetSubject() const override;
	virtual bool IsPrivate() const override;
	virtual bool IsJoined() const override;
	virtual const FChatRoomConfig& GetRoomConfig() const override;
	virtual FString ToDebugString() const override;
	virtual void SetChatInfo(const TSharedRef<FJsonObject>& JsonInfo) override;
	//~ Begin FChatRoomInfo overrides

	static FAccelByteChatRoomInfoRef Create();
	
	bool HasMember(const FString& UserId) const;
	const TArray<FString>& GetMembers() const;

PACKAGE_SCOPE:
	void SetTopicData(const FAccelByteModelsChatUpdateUserTopicNotif& UserTopicData);
	void SetTopicData(const FAccelByteModelsChatTopicQueryData& InTopicData);
	void UpdateTopicData(const FAccelByteModelsChatUpdateTopicNotif& InUpdateTopic);
	void AddMember(const FString& UserId);
	void RemoveMember(const FString& UserId);
	
	FUniqueNetIdRef OwnerId{FUniqueNetIdAccelByteUser::Invalid()};
	bool bIsPrivate{};
	bool bIsJoined{};
	FChatRoomConfig RoomConfig;
	FAccelByteModelsChatTopicQueryData TopicData;
};

class ONLINESUBSYSTEMACCELBYTE_API FOnlineChatAccelByte : public IOnlineChat, public TSharedFromThis<FOnlineChatAccelByte, ESPMode::ThreadSafe>
{
private:
	/** Reference to the main AccelByte subsystem */
	FOnlineSubsystemAccelByteWPtr AccelByteSubsystem;
	
public:
	
	/**
	 * Convenience method to get an instance of this interface from the subsystem passed in.
	 *
	 * @param Subsystem Subsystem instance that we wish to get this interface from
	 * @param OutInterfaceInstance Instance of the interface that we got from the subsystem, or nullptr if not found
	 * @returns boolean that is true if we could get an instance of the interface, false otherwise
	 */
	static bool GetFromSubsystem(const IOnlineSubsystem* Subsystem, TSharedPtr<FOnlineChatAccelByte, ESPMode::ThreadSafe>& OutInterfaceInstance);
	static bool GetFromSubsystem(FOnlineSubsystemAccelByte* Subsystem, TSharedPtr<FOnlineChatAccelByte, ESPMode::ThreadSafe>& OutInterfaceInstance);

	/**
	 * Convenience method to get an instance of this interface from the subsystem associated with the world passed in.
	 *
	 * @param World World instance that we wish to get the interface from
	 * @param OutInterfaceInstance Instance of the interface that we got from the subsystem, or nullptr if not found
	 * @returns boolean that is true if we could get an instance of the interface, false otherwise
	 */
	static bool GetFromWorld(const UWorld* World, TSharedPtr<FOnlineChatAccelByte, ESPMode::ThreadSafe>& OutInterfaceInstance);

	FOnlineChatAccelByte(FOnlineSubsystemAccelByte* InSubsystem);

	DEFINE_ONLINE_PLAYER_DELEGATE_THREE_PARAM(MAX_LOCAL_PLAYERS, OnConnectChatComplete, bool /*bWasSuccessful*/, const FUniqueNetId& /*UserId*/, const FString& /*Error*/);

	/**
	 * Called when the disconnected chat websocket trying to establish the connection again
	 */
	DEFINE_ONLINE_PLAYER_DELEGATE_ONE_PARAM(MAX_LOCAL_PLAYERS, AccelByteOnChatReconnectAttempted, const AccelByte::FReconnectAttemptInfo& /*Info*/);	

	/**
	 * Called when the chat has been disconnected for longer than usual and can't be reconnected yet
	 */
	DEFINE_ONLINE_PLAYER_DELEGATE_ONE_PARAM(MAX_LOCAL_PLAYERS, AccelByteOnChatMassiveOutageEvent, const AccelByte::FMassiveOutageInfo& /*Info*/);

	bool Connect(int32 LocalUserNum);

	/**
	 * Create a new chat room containing the given user. Deprecated.
	 *
	 * @param UserId ID of the user creating the chat room
	 * @param RoomId #NOTE Unsupported field as the backend will generate an ID
	 * @param Nickname The local nickname of the chat room
	 * @param ChatRoomConfig #NOTE Unsupported field as the backend does not support these configuration options
	 * @returns A boolean indicating whether the process was started successfully
	 *
	 * @deprecated Manual room creation is deprecated - please use V2 Sessions to auto-create chat rooms!
	 */
	virtual bool CreateRoom(const FUniqueNetId& UserId, const FChatRoomId& RoomId, const FString& Nickname, const FChatRoomConfig& ChatRoomConfig) override;

	/**
	 * Create a new chat room containing the given user. Deprecated.
	 *
	 * @param UserId ID of the user creating the chat room
	 * @param Nickname The local nickname of the chat room
	 * @param ChatRoomConfig Configuration for the chat room
	 * @returns A boolean indicating whether the process was started successfully
	 *
	 * @deprecated Manual room creation is deprecated - please use V2 Sessions to auto-create chat rooms!
	 */
	virtual bool CreateRoom(const FUniqueNetId& UserId, const FString& Nickname, const FAccelByteChatRoomConfig& ChatRoomConfig);

	/**
	 * Update a room's configuration
	 *
	 * @param UserId ID of the user updating the chat room
	 * @param RoomId The ID of the chat room
	 * @param ChatRoomConfig #NOTE Unsupported
	 * @returns A boolean indicating whether the process was started successfully
	 */
	virtual bool ConfigureRoom(const FUniqueNetId& UserId, const FChatRoomId& RoomId, const FChatRoomConfig& ChatRoomConfig) override;

	/**
	 * Update a room's configuration
	 *
	 * @param UserId ID of the user updating the chat room
	 * @param RoomId The ID of the chat room
	 * @param ChatRoomConfig Configuration for the chat room
	 * @returns A boolean indicating whether the process was started successfully
	 */
	virtual bool ConfigureRoom(const FUniqueNetId& UserId, const FChatRoomId& RoomId, const FAccelByteChatRoomConfig& ChatRoomConfig);

	/**
	 * Join a chat room
	 *
	 * @param UserId ID of the user joining the chat room
	 * @param RoomId The ID of the chat room
	 * @param Nickname Local nickname for the chat room
	 * @param ChatRoomConfig #NOTE Unsupported
	 * @returns A boolean indicating whether the process was started successfully
	 */
	virtual bool JoinPublicRoom(const FUniqueNetId& UserId, const FChatRoomId& RoomId, const FString& Nickname, const FChatRoomConfig& ChatRoomConfig) override;

	/**
	 * Join a private chat room - not supported!
	 */
	virtual bool JoinPrivateRoom(const FUniqueNetId& UserId, const FChatRoomId& RoomId, const FString& Nickname, const FChatRoomConfig& ChatRoomConfig) override;

	/**
	 * Exit a chat room
	 *
	 * @param UserId ID of the user exiting the chat room
	 * @param RoomId The ID of the chat room
	 * @returns A boolean indicating whether the process was started successfully
	 */
	virtual bool ExitRoom(const FUniqueNetId& UserId, const FChatRoomId& RoomId) override;

	/**
	 * Send a chat message to a room
	 *
	 * @param UserId ID of the user sending the message
	 * @param RoomId The ID of the chat room
	 * @param MsgBody The message to send
	 * @returns A boolean indicating whether the process was started successfully
	 */
	virtual bool SendRoomChat(const FUniqueNetId& UserId, const FChatRoomId& RoomId, const FString& MsgBody) override;
	
	/**
	 * Kick off request for sending a chat message privately between users
	 * 
	 * @param UserId id of user that is sending the message
	 * @param RecipientId id of user to send the chat to
	 * @param MsgBody plain text of message body
	 *
	 * @return if successfully started the async operation
	 */
	virtual bool SendPrivateChat(const FUniqueNetId& UserId, const FUniqueNetId& RecipientId, const FString& MsgBody) override;

	/**
	 * Delete system message(s) in user system inbox based on the message ids.
	 * Listen to OnDeleteSystemMessagesComplete delegate for the result after the action completed.
	 *
	 * @param UserId id of user that perform this action
	 * @param MessageIds list of message ids to be deleted
	 * @return if successfully started the async operation
	 */
	bool DeleteSystemMessages(const FUniqueNetId& UserId, const TSet<FString>& MessageIds);

	/**
	 * Update system message(s) in user system inbox. This can be used for marking message as keep or as read.
	 * Listen to OnUpdateSystemMessagesComplete delegate for the result after the action completed.
	 *
	 * @param UserId id of user that perform this action
	 * @param ActionUpdateSystemMessages list of the action will be performed to the system message
	 * @return if successfully started the async operation
	 */
	bool UpdateSystemMessages(const FUniqueNetId& UserId, const TArray<FAccelByteModelsActionUpdateSystemMessage>& ActionUpdateSystemMessages);

	/**
	 * Query system message(s) in user system inbox.
	 * Listen to OnQuerySystemMessageComplete delegate for the result after the action completed.
	 *
	 * @param UserId id of user that perform this action
	 * @param OptionalParams optional query params, if left empty it will query all of the system messages
	 * @return if successfully started the async operation
	 */
	bool QuerySystemMessage(const FUniqueNetId& UserId, const FQuerySystemMessageOptions& OptionalParams = {});

	/**
	 * Query transient system message(s) in user system inbox.
	 * Listen to OnQueryTransientSystemMessageComplete delegate for the result after the action completed.
	 *
	 * @param UserId id of user that perform this action
	 * @param OptionalParams optional query params, if left empty it will query all of the system messages
	 * @return if successfully started the async operation
	 */
	bool QueryTransientSystemMessage(const FUniqueNetId& UserId, const FQuerySystemMessageOptions& OptionalParams = {});

	/**
	 * Get system message(s) statistic in user system inbox. Currently the stats have oldest unread and unread count of system messages.
	 * Listen to OnGetSystemMessageStatsComplete delegate for the result after the action completed.
	 *
	 * @param UserId id of user that perform this action
	 * @param Request optional params for getting system message stats, currently it is still empty and can be ignored
	 * @return if successfully started the async operation
	 */
	bool GetSystemMessageStats(const FUniqueNetId& UserId, const FAccelByteGetSystemMessageStatsRequest& Request = {});

	/**
	 * Get chat configuration of the namespace
	 * Listen to OnGetChatConfigComplete delegate for the result after the action completed
	 * 
	 * @param LocalUserNum target local user num that perform this action
	 * @return if successfully started the async operation
	 */
	bool GetChatConfig(int32 LocalUserNum);

	/**
	 * Get chat configuration of the namespace
	 * Listen to OnGetChatConfigComplete delegate for the result after the action completed
	 * 
	 * @param UserId id of user that perform this action
	 * @return if successfully started the async operation
	 */
	bool GetChatConfig(const FUniqueNetId& UserId);

	/**
	 * Get chat configuration for current user.
	 * Listen to OnGetUserChatConfigurationComplete delegate for the result after the action completed.
	 * 
	 * @param LocalUserNum target local user num that perform this action.
	 * @return if successfully started the async operation.
	 */
	bool GetUserConfiguration(const int32 LocalUserNum);

	/**
	 * Get chat configuration for current user.
	 * Listen to OnGetUserChatConfigurationComplete delegate for the result after the action completed.
	 * 
	 * @param UserId id of user that perform this action.
	 * @return if successfully started the async operation.
	 */
	bool GetUserConfiguration(const FUniqueNetId& UserId);

	/**
	 * Set chat configuration for current user.
	 * Listen to OnSetUserChatConfigurationComplete delegate for the result after the action completed.
	 * 
	 * @param LocalUserNum target local user num that perform this action.
	 * @param Configuration user chat configuration.
	 * @return if successfully started the async operation
	 */
	bool SetUserConfiguration(const int32 LocalUserNum, const FAccelByteModelsSetUserChatConfigurationRequest& Configuration);

	/**
	 * Set chat configuration for current user
	 * Listen to OnSetUserChatConfigurationComplete delegate for the result after the action completed
	 * 
	 * @param UserId id of user that perform this action
	 * @param Configuration user chat configuration.
	 * @return if successfully started the async operation
	 */
	bool SetUserConfiguration(const FUniqueNetId& UserId, const FAccelByteModelsSetUserChatConfigurationRequest& Configuration);

	/**
	 * Report a chat message for abuse.
	 *
	 * @param UserId ID of the user that is reporting the given chat message
	 * @param Message Shared reference to the chat message to report
	 * @param Reason String describing the reason for reporting the chat message
	 * @param Comment String giving more detail on the report
	 * @param CompletionDelegate Delegate run on completion of report task to inform success state
	 * @return boolean that is true if async operation started, false otherwise
	 */
	bool ReportChatMessage(const FUniqueNetId& UserId
		, const TSharedRef<FChatMessage>& Message
		, const FString& Reason
		, const FString& Comment
		, FOnReportChatMessageComplete CompletionDelegate);

	virtual bool IsChatAllowed(const FUniqueNetId& UserId, const FUniqueNetId& RecipientId) const override;
	virtual void GetJoinedRooms(const FUniqueNetId& UserId, TArray<FChatRoomId>& OutRooms) override;
	virtual TSharedPtr<FChatRoomInfo> GetRoomInfo(const FUniqueNetId& UserId, const FChatRoomId& RoomId) override;
	virtual bool GetMembers(const FUniqueNetId& UserId, const FChatRoomId& RoomId, TArray<TSharedRef<FChatRoomMember>>& OutMembers) override;
	virtual TSharedPtr<FChatRoomMember> GetMember(const FUniqueNetId& UserId, const FChatRoomId& RoomId, const FUniqueNetId& MemberId) override;
	/** Return only live messages */
	virtual bool GetLastMessages(const FUniqueNetId& UserId, const FChatRoomId& RoomId, int32 NumMessages, TArray<TSharedRef<FChatMessage>>& OutMessages) override;
	virtual bool IsMessageFromLocalUser(const FUniqueNetId& UserId, const FChatMessage& Message, const bool bIncludeExternalInstances) override;
	virtual void DumpChatState() const override;

	//~ Begin Chat Utility functions
	/**
	 * Convert Game Session Id to TopicId
	 * Note: Session chat only available when TextChat configuration is true
	 */
	static FString SessionV2IdToChatTopicId(const FString &SessionId);
	/**
	 * Convert Session Party Id to Topic Id
	 * Note: Party chat only available when TextChat configuration is true
	 */
	static FString PartyV2IdToChatTopicId(const FString &PartyId);/**
	 * Convert Lobby Party Id to Topic Id
	 * Note: Party chat only available when TextChat configuration is true
	 */
	static FString PartyV1IdToChatTopicId(const FString &PartyId);
	
	/**
	 * [DEPRECATED] Unreliable method to obtain the topic ID and might be different from backend's sorting result.
	 * Convert User Id to personal Topic Id
	 * To be able to send personal chat, personal topic need to be created first
	 */
	static FString PersonalChatTopicId(const FString& FromUserId, const FString& ToUserId);

	/**
	 * Determine chat room type based on topic Id
	 */
	static EAccelByteChatRoomType GetChatRoomType(const FString &TopicId);
	/**
	 * Check if user has joined topic
	 * Can be use to check if user joined session chat or party chat
	 */

	bool IsJoinedTopic(const FString &UserId, const FString &TopicId);

	/**
	 * Check if user has personal chat topic to target user
	 */
	bool HasPersonalChat(const FString& FromUserId, const FString& ToUserId);

	/**
	 * Get user's personal chat topic to target user
	 * 
	 * @param FromUserId
	 * @param ToUserId
	 * @return TOptional<FString> Value will be set if personal chat is found
	 */
	TOptional<FString> GetPersonalChatTopicId(const FString& FromUserId, const FString& ToUserId);

	/**
	* Manually query chat room
	* This function spawn AsyncTask to query room and will call OnQueryComplete once finishes.
	* 
	* @param PlayerId
	* @param Request
	* @param OnComplete
	* @return bool Will return true if the AsyncTask was successfully launched, false otherwise.
	*/
	bool QueryChatRoom(const FUniqueNetId& PlayerId, const FAccelByteModelsChatQueryTopicRequest& Request, const FOnChatQueryRoomComplete& OnComplete);
	//~ End Chat Utility functions

	/**
	 * Delegate fired when a notification is received regarding disconnection from the chat service
	 */
	DEFINE_ONLINE_DELEGATE_ONE_PARAM(OnChatDisconnected, FString /*Message*/);

	/**
	 * Delegate fired when a sending a chat message is completed
	 */
	DEFINE_ONLINE_DELEGATE_FOUR_PARAM(OnSendChatComplete, FString /*UserId*/, FString /*MsgBody*/, FString /*RoomId*/, bool /*bWasSuccessful*/);
	
	/**
	* Delegate fired when a sending a chat message is completed
	*/
	DEFINE_ONLINE_DELEGATE_FIVE_PARAM(OnSendChatCompleteWithError, FString /*UserId*/, FString /*MsgBody*/, FString /*RoomId*/, bool /*bWasSuccessful*/, const FOnlineError& /*Error*/);

	/**
	* Delegate fired when a notification is received regarding a read chat
	*/
	DEFINE_ONLINE_DELEGATE_ONE_PARAM(OnReadChatReceived, TArray<FAccelByteModelsChatReadChatData> /*ReadChatsData*/);

	/**
	* Delegate fired when a notification is received regarding being added to a topic
	*/
	DEFINE_ONLINE_DELEGATE_THREE_PARAM(OnTopicAdded, FString /*ChatTopicName*/, FString /*TopicId*/, FString /*UserId*/);

	/**
	* Delegate fired when a notification is received regarding being removed from a topic
	*/
	DEFINE_ONLINE_DELEGATE_THREE_PARAM(OnTopicRemoved, FString /*ChatTopicName*/, FString /*TopicId*/, FString /*UserId*/);

	/**
	* Delegate fired when a notification is received regarding a topic being updated
	*/
	DEFINE_ONLINE_DELEGATE_FOUR_PARAM(OnTopicUpdated, FString /*ChatTopicName*/, FString /*TopicId*/, FString /*SenderId*/, bool /*IsChannel*/);

	/**
	* Delegate fired when a notification is received regarding a topic being deleted
	*/
	DEFINE_ONLINE_DELEGATE_FOUR_PARAM(OnTopicDeleted, FString /*ChatTopicName*/, FString /*TopicId*/, FString /*SenderId*/, bool /*IsChannel*/);

	/**
	* Delegate fired when a notification is received regarding being banned from chat
	*/
	DEFINE_ONLINE_DELEGATE_FIVE_PARAM(OnUserBanned, FString /*UserId*/, EBanType /*Ban*/, FString /*EndDate*/, EBanReason /*Reason*/, bool /*Enable*/);

	/**
	* Delegate fired when a notification is received regarding being un-banned from chat
	*/
	DEFINE_ONLINE_DELEGATE_FIVE_PARAM(OnUserUnbanned, FString /*UserId*/, EBanType /*Ban*/, FString /*EndDate*/, EBanReason /*Reason*/, bool /*Enable*/);

	/**
	* Delegate fired when a system message received.
	*/
	DEFINE_ONLINE_DELEGATE_TWO_PARAM(OnSystemMessageReceived, const FUniqueNetId& /*UserId*/, const FSystemMessageNotifMessage& /*SystemMessage*/);

	/**
	* Delegate fired when a transient system message received.
	*/
	DEFINE_ONLINE_DELEGATE_TWO_PARAM(OnTransientSystemMessageReceived, const FUniqueNetId& /*UserId*/, const FAccelByteModelsChatSystemMessageNotif& /*TransientSystemMessage*/);

	/**
	* Delegate fired when action to delete system messages completed.
	*/
	DEFINE_ONLINE_DELEGATE_TWO_PARAM(OnDeleteSystemMessagesComplete, const FUniqueNetId& /*UserId*/, const FOnlineError& /*ErrorInfo*/);

	/**
	* Delegate fired when action to update system messages completed.
	*/
	DEFINE_ONLINE_DELEGATE_TWO_PARAM(OnUpdateSystemMessagesComplete, const FUniqueNetId& /*UserId*/, const FOnlineError& /*ErrorInfo*/);

	/**
	* Delegate fired when a query for system message completed.
	*/
	DEFINE_ONLINE_DELEGATE_THREE_PARAM(OnQuerySystemMessageComplete, const FUniqueNetId& /*UserId*/, const TArray<FSystemMessageNotifMessage>& /*SystemMessages*/, const FOnlineError& /*ErrorInfo*/);

	/**
	* Delegate fired when a query for transient system message completed.
	*/
	DEFINE_ONLINE_DELEGATE_THREE_PARAM(OnQueryTransientSystemMessageComplete, const FUniqueNetId& /*UserId*/, const TArray<FAccelByteModelsQuerySystemMessagesResponseItem>& /*TransientSystemMessages*/, const FOnlineError& /*ErrorInfo*/);

	/**
	* Delegate fired when action to get system message stats completed.
	*/
	DEFINE_ONLINE_DELEGATE_THREE_PARAM(OnGetSystemMessageStatsComplete, const FUniqueNetId& /*UserId*/, const FAccelByteGetSystemMessageStatsResponse& /*SystemMessagesStats*/, const FOnlineError& /*ErrorInfo*/);

	/**
	* Delegate fired when action to get chat config completed.
	*/
	DEFINE_ONLINE_PLAYER_DELEGATE_THREE_PARAM(MAX_LOCAL_PLAYERS, OnGetChatConfigComplete, bool /*bWasSuccessful*/, const FAccelByteModelsChatPublicConfigResponse& /*ChatPublicConfigResponse*/, const FString& /*Error*/);

	/**
	* Delegate fired when chat connection closed.
	*/
	DEFINE_ONLINE_PLAYER_DELEGATE_THREE_PARAM(MAX_LOCAL_PLAYERS, OnChatConnectionClosed, int32 /* StatusCode */, const FString& /* Reason */, bool /* bWasClean */);

	/*
	No trigger for this delegate
	DEFINE_ONLINE_DELEGATE_THREE_PARAM(OnChatRoomMemberUpdate, const FUniqueNetId&, const FChatRoomId&, const FUniqueNetId&);
	*/

	/**
	* Delegate fired when action to get user chat configuration completed.
	*/
	DEFINE_ONLINE_PLAYER_DELEGATE_TWO_PARAM(MAX_LOCAL_PLAYERS, OnGetUserChatConfigurationComplete, const FAccelByteModelsGetUserChatConfigurationResponse& /*UserChatConfiguration*/, const FOnlineError& /*ErrorInfo*/)

	/**
	* Delegate fired when action to set user chat configuration completed.
	*/
	DEFINE_ONLINE_PLAYER_DELEGATE_TWO_PARAM(MAX_LOCAL_PLAYERS, OnSetUserChatConfigurationComplete, const FAccelByteSetUserChatConfigurationResponse& /*UserChatConfiguration*/, const FOnlineError& /*ErrorInfo*/)
	
PACKAGE_SCOPE:
	/**
	* Need to be performed by FOnlineAsyncTaskAccelByteConnectChat after success.
	* This function spawn AsyncTask to query room and after complete, the interface will call RegisterChatDelegates
	*/
	void QueryRoomAfterChatConnectEstablished(const FUniqueNetId& PlayerId);

	void RegisterChatDelegates(int32 LocalUserNum);

	//~ Begin Utility functions
	/**
	* Remove member to topic cache. Called on EventRemovedFromTopic
	*/
	void RemoveMemberFromTopic(const FString &UserId, const FString &TopicId);
	/**
	* Add topic to cache. Called on after connect success
	*/
	void AddTopic(const FAccelByteChatRoomInfoRef &ChatRoomInfo);
	/**
	* Remove topicId from cache. Called on EventTopicDeleted
	*/
	void RemoveTopic(const FString &TopicId);
	/**
	* Get topic from cache
	*/
	FAccelByteChatRoomInfoPtr GetTopic(const FString& TopicId);
	/**
	* Add Chat Room Member Information to cache
	*/
	void AddChatRoomMembers(TArray<FAccelByteUserInfoRef> Users);
	/**
	* Add Chat message to cache
	*/
	void AddChatMessage(FUniqueNetIdAccelByteUserRef AccelByteUserId, const FChatRoomId& ChatRoomId, TSharedRef<FChatMessage> ChatMessage);
	/**
	* Get cached room member
	*/
	FAccelByteChatRoomMemberRef GetAccelByteChatRoomMember(const FString& UserId);
	/**
	* Cache current maximum chat message length
	*/
	void SetMaxChatMessageLength(int32 InMaxChatMessageLength) { MaxChatMessageLength = InMaxChatMessageLength; }
	/**
	* Get current maximum chat message length
	*/
	int32 GetMaxChatMessageLength() const { return MaxChatMessageLength; }
	//~ End Utility functions

private:
	//~ Begin Chat Notification Handlers
	void OnChatDisconnectedNotification(const FAccelByteModelsChatDisconnectNotif& DisconnectEvent, int32 LocalUserNum);
	void OnChatConnectionClosed(int32 StatusCode, const FString& Reason, bool bWasClean, int32 LocalUserNum);
	void OnRemoveFromTopicNotification(const FAccelByteModelsChatUpdateUserTopicNotif& RemoveTopicEvent, int32 LocalUserNum);
	void OnAddToTopicNotification(const FAccelByteModelsChatUpdateUserTopicNotif& AddTopicEvent, int32 LocalUserNum);
	void OnReceivedChatNotification(const FAccelByteModelsChatNotif& ChatNotif, int32 LocalUserNum);
	void OnUpdateTopicNotification(const FAccelByteModelsChatUpdateTopicNotif& UpdateTopicNotif, int32 LocalUserNum);
	void OnDeleteTopicNotification(const FAccelByteModelsChatUpdateTopicNotif& DeleteTopicNotif, int32 LocalUserNum);
	void OnReadChatNotification(const FAccelByteModelsReadChatNotif& ReadChatNotif, int32 LocalUserNum);
	void OnUserBanNotification(const FAccelByteModelsChatUserBanUnbanNotif& UserBanNotif, int32 LocalUserNum);
	void OnUserUnbanNotification(const FAccelByteModelsChatUserBanUnbanNotif& UserUnbanNotif, int32 LocalUserNum);
	void OnSystemMessageNotification(const FAccelByteModelsChatSystemMessageNotif& SystemMessageNotif, int32 LocalUserNum);
	void OnChatReconnectAttempted(const AccelByte::FReconnectAttemptInfo& Info, int32 InLocalUserNum);
	void OnChatMassiveOutageEvent(const AccelByte::FMassiveOutageInfo& Info, int32 InLocalUserNum);
	//~ End Chat Notification Handlers

	//~ Begin Chat Internal Handlers
	void OnQueryChatRoomInfoCompleteAfterConnectionEstablished(bool bWasSuccessful, TArray<FAccelByteChatRoomInfoRef> RoomList, int32 LocalUserNum);
	void OnQueryChatMemberInfo_TriggerChatRoomMemberJoin(bool bIsSuccessful, TArray<FAccelByteUserInfoRef> UsersQueried, FString RoomId, FUniqueNetIdPtr UserId, FUniqueNetIdPtr MemberId);
	void OnQueryChatRoomById_TriggerChatRoomMemberJoin(bool bWasSuccessful, FAccelByteChatRoomInfoPtr RoomInfo, int32 LocalUserNum, FUniqueNetIdPtr UserId, FUniqueNetIdPtr MemberId);
	//~ End Chat Internal Handlers

	/** Cache chat room info. Populated after connect and updated on topic related events */
	TMap<FString, FAccelByteChatRoomInfoRef> TopicIdToChatRoomInfoCached;
	/** Cache chat room member. Populated along with the topic events */
	TMap<FString, FAccelByteChatRoomMemberRef> UserIdToChatRoomMemberCached;
	/** Cache live chat messages */
	FUserIdToRoomChatMessages UserIdToChatRoomMessagesCached;

	/** Cache maximum chat message length*/
	int32 MaxChatMessageLength{INDEX_NONE};

	bool UpdateUserAccount(int32 LocalUserNum);
	void SendDisconnectPredefinedEvent(int32 StatusCode, int32 LocalUserNum);
};
