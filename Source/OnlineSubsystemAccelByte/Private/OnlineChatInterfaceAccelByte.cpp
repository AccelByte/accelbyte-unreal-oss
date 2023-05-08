// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineChatInterfaceAccelByte.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "OnlineSubsystemAccelByteInternalHelpers.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "AsyncTasks/Chat/OnlineAsyncTaskAccelByteConnectChat.h"
#include "AsyncTasks/Chat/OnlineAsyncTaskAccelByteChatCreateRoom.h"
#include "AsyncTasks/Chat/OnlineAsyncTaskAccelByteChatConfigureRoom.h"
#include "AsyncTasks/Chat/OnlineAsyncTaskAccelByteChatJoinPublicRoom.h"
#include "AsyncTasks/Chat/OnlineAsyncTaskAccelByteChatExitRoom.h"
#include "AsyncTasks/Chat/OnlineAsyncTaskAccelByteChatQueryRoom.h"
#include "AsyncTasks/Chat/OnlineAsyncTaskAccelByteChatQueryRoomById.h"
#include "AsyncTasks/Chat/OnlineAsyncTaskAccelByteChatSendPersonalChat.h"
#include "AsyncTasks/Chat/OnlineAsyncTaskAccelByteChatSendRoomChat.h"

bool FOnlineChatAccelByte::GetFromSubsystem(const IOnlineSubsystem* Subsystem, FOnlineChatAccelBytePtr& OutInterfaceInstance)
{
	OutInterfaceInstance = StaticCastSharedPtr<FOnlineChatAccelByte>(Subsystem->GetChatInterface());
	return OutInterfaceInstance.IsValid();
}

bool FOnlineChatAccelByte::GetFromWorld(const UWorld* World, FOnlineChatAccelBytePtr& OutInterfaceInstance)
{
	const IOnlineSubsystem* Subsystem = Online::GetSubsystem(World);
	if (Subsystem == nullptr)
	{
		OutInterfaceInstance = nullptr;
		return false;
	}

	return GetFromSubsystem(Subsystem, OutInterfaceInstance);
}

FOnlineChatAccelByte::FOnlineChatAccelByte(FOnlineSubsystemAccelByte* InSubsystem)
	: AccelByteSubsystem(InSubsystem)
{}

bool FOnlineChatAccelByte::Connect(int32 LocalUserNum)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d"), LocalUserNum);

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystem->GetIdentityInterface());
	if (!ensure(IdentityInterface.IsValid()))
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to register notifications for session updates as our identity interface is invalid!"));
		return false;
	}

	// Don't attempt to connect again if we are already reporting as connected
	if (IdentityInterface->GetLoginStatus(LocalUserNum) == ELoginStatus::LoggedIn)
	{
		const TSharedPtr<const FUniqueNetId> UserIdPtr = IdentityInterface->GetUniquePlayerId(LocalUserNum);
		TSharedPtr<FUserOnlineAccount> UserAccount;
		if (UserIdPtr.IsValid())
		{
			const FUniqueNetId& UserId = UserIdPtr.ToSharedRef().Get();
			UserAccount = IdentityInterface->GetUserAccount(UserId);
		}

		const TSharedPtr<FUserOnlineAccountAccelByte> UserAccountAccelByte = StaticCastSharedPtr<FUserOnlineAccountAccelByte>(UserAccount);
		if (UserAccountAccelByte->IsConnectedToChat())
		{
			const FString ErrorStr = TEXT("connect-failed-already-connected");
			AB_OSS_INTERFACE_TRACE_END(TEXT("User already connected to chat at user index '%d'!"), LocalUserNum);

			TriggerOnConnectChatCompleteDelegates(LocalUserNum, false, FUniqueNetIdAccelByteUser::Invalid().Get(), ErrorStr);
			return false;
		}
		
		AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteConnectChat>(AccelByteSubsystem, *IdentityInterface->GetUniquePlayerId(LocalUserNum).Get());
		AB_OSS_INTERFACE_TRACE_END(TEXT("Dispatching async task to attempt to connect chat!"));
		return true;
	}

	const FString ErrorStr = TEXT("connect-failed-not-logged-in");
	AB_OSS_INTERFACE_TRACE_END(TEXT("User not logged in at user index '%d'!"), LocalUserNum);

	TriggerOnConnectChatCompleteDelegates(LocalUserNum, false, FUniqueNetIdAccelByteUser::Invalid().Get(), ErrorStr);
	return false;
}

bool FOnlineChatAccelByte::CreateRoom(
	const FUniqueNetId& UserId,
	const FChatRoomId&,
	const FString& Nickname,
	const FChatRoomConfig&)
{
	// #NOTE: The chat service does not support the configuration options in FChatRoomConfig, so we just forward
	//		the user ID and nickname to the AccelByte version of this method
	return CreateRoom(UserId, Nickname, {});
}

bool FOnlineChatAccelByte::CreateRoom(const FUniqueNetId& UserId, const FString& Nickname, const FAccelByteChatRoomConfig& ChatRoomConfig)
{
	FReport::LogDeprecated(FString(__FUNCTION__), TEXT("Manual room creation is deprecated - please use V2 Sessions to auto-create chat rooms"));

	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("UserId: %s, Nickname: %s, bIsJoinable: %s"), *UserId.ToDebugString(), *Nickname, LOG_BOOL_FORMAT(ChatRoomConfig.bIsJoinable));

	// TODO: Store the supplied nickname

	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteChatCreateRoom>(AccelByteSubsystem, UserId, ChatRoomConfig);

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));

	return true;
}

bool FOnlineChatAccelByte::ConfigureRoom(const FUniqueNetId&, const FChatRoomId&, const FChatRoomConfig&)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT(""));

	// #NOTE Not doing anything, as none of the fields in FChatRoomConfig are supported by the backend. So a call to this method would be redundant

	AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Cannot configure chat room using FChatRoomConfig as its fields are not supported! Please supply an instance of FAccelByteChatRoomConfig instead!"));

	return false;
}

bool FOnlineChatAccelByte::ConfigureRoom(const FUniqueNetId& UserId, const FChatRoomId& RoomId, const FAccelByteChatRoomConfig& ChatRoomConfig)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("UserId: %s, RoomId: %s, bIsJoinable: %s"), *UserId.ToDebugString(), *RoomId, LOG_BOOL_FORMAT(ChatRoomConfig.bIsJoinable));

	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteChatConfigureRoom>(AccelByteSubsystem, UserId, RoomId, ChatRoomConfig);

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));

	return true;
}

bool FOnlineChatAccelByte::JoinPublicRoom(
	const FUniqueNetId& UserId,
	const FChatRoomId& RoomId,
	const FString& Nickname,
	const FChatRoomConfig&)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("UserId: %s, RoomId: %s, Nickname: %s"), *UserId.ToDebugString(), *RoomId, *Nickname);

	// TODO: Store the supplied nickname

	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteChatJoinPublicRoom>(AccelByteSubsystem, UserId, RoomId);

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));

	return true;
}

bool FOnlineChatAccelByte::JoinPrivateRoom(const FUniqueNetId&, const FChatRoomId&, const FString&, const FChatRoomConfig&)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT(""));

	// #NOTE Not doing anything, as the backend does not support private rooms

	AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Cannot join private chat room as it is unsupported!"));

	return false;
}

bool FOnlineChatAccelByte::ExitRoom(const FUniqueNetId& UserId, const FChatRoomId& RoomId)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("UserId: %s, RoomId: %s"), *UserId.ToDebugString(), *RoomId);

	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteChatExitRoom>(AccelByteSubsystem, UserId, RoomId);

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));

	return true;
}

bool FOnlineChatAccelByte::SendRoomChat(const FUniqueNetId& UserId, const FChatRoomId& RoomId, const FString& MsgBody)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("UserId: %s, RoomId: %s, MsgBody: %s"), *UserId.ToDebugString(), *RoomId, *MsgBody);

	const FUniqueNetIdAccelByteUserRef AccelByteUserId = FUniqueNetIdAccelByteUser::CastChecked(UserId);
	const FString AccelByteId = AccelByteUserId->GetAccelByteId();
	if (!IsJoinedTopic(AccelByteId, RoomId))
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Unable to send room chat as the user %s has not joined room %s"), *AccelByteId, *RoomId);
		return false;
	}

	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteChatSendRoomChat>(AccelByteSubsystem, UserId, RoomId, MsgBody);

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));

	return true;
}

bool FOnlineChatAccelByte::SendPrivateChat(const FUniqueNetId& UserId, const FUniqueNetId& RecipientId, const FString& MsgBody)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("UserId: %s, RecipientId: %s"), *UserId.ToDebugString(), *RecipientId.ToDebugString());

	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteChatSendPersonalChat>(AccelByteSubsystem, UserId, RecipientId, MsgBody);

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));

	return true;
}

bool FOnlineChatAccelByte::IsChatAllowed(const FUniqueNetId& UserId, const FUniqueNetId& RecipientId) const
{
	return true;
}

void FOnlineChatAccelByte::GetJoinedRooms(const FUniqueNetId& UserId, TArray<FChatRoomId>& OutRooms)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("UserId: %s"), *UserId.ToDebugString());

	const FUniqueNetIdAccelByteUserRef AccelByteUserId = FUniqueNetIdAccelByteUser::CastChecked(UserId);
	const FString AccelByteId = AccelByteUserId->GetAccelByteId();
	for (const auto& Entry : TopicIdToChatRoomInfoCached)
	{
		if (Entry.Value->HasMember(AccelByteId))
		{
			OutRooms.Add(Entry.Value->GetRoomId());
		}
	}

	AB_OSS_INTERFACE_TRACE_END(TEXT("Number of rooms: %d"), OutRooms.Num());
}

TSharedPtr<FChatRoomInfo> FOnlineChatAccelByte::GetRoomInfo(const FUniqueNetId& UserId, const FChatRoomId& RoomId)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("UserId: %s, RoomId: %s"), *UserId.ToDebugString(), *RoomId);

	FAccelByteChatRoomInfoRef* RoomInfo = TopicIdToChatRoomInfoCached.Find(RoomId);
	if (RoomInfo == nullptr)
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Room with ID %s not found"), *RoomId);
		return nullptr;		
	}

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));

	return *RoomInfo;
}

bool FOnlineChatAccelByte::GetMembers(const FUniqueNetId& UserId, const FChatRoomId& RoomId, TArray<TSharedRef<FChatRoomMember>>& OutMembers)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("UserId: %s, RoomId: %s"), *UserId.ToDebugString(), *RoomId);

	const FAccelByteChatRoomInfoRef* RoomInfo = TopicIdToChatRoomInfoCached.Find(RoomId);
	if (RoomInfo == nullptr)
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get members for room with ID %s as the room was not found!"), *RoomId);
		return false;
	}

	const TArray<FString>& Members = (*RoomInfo)->GetMembers();
	for (const auto& MemberAccelByteId : Members)
	{
		FAccelByteChatRoomMemberRef RoomMember = GetAccelByteChatRoomMember(MemberAccelByteId);
		OutMembers.Add(RoomMember);
	}

	AB_OSS_INTERFACE_TRACE_END(TEXT("Number of members: %d"), OutMembers.Num());

	return true;
}

TSharedPtr<FChatRoomMember> FOnlineChatAccelByte::GetMember(
	const FUniqueNetId& UserId,
	const FChatRoomId& RoomId,
	const FUniqueNetId& MemberId)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("UserId: %s, RoomId: %s, MemberId: %s"), *UserId.ToDebugString(), *RoomId, *MemberId.ToDebugString());

	const FUniqueNetIdAccelByteUserRef AccelByteUserId = FUniqueNetIdAccelByteUser::CastChecked(UserId);
	const FString AccelByteId = AccelByteUserId->GetAccelByteId();

	const FAccelByteChatRoomInfoRef* RoomInfo = TopicIdToChatRoomInfoCached.Find(RoomId);
	if (RoomInfo == nullptr)
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get member from room with ID %s as the room was not found!"), *RoomId);
		return nullptr;
	}
	
	if (!(*RoomInfo)->HasMember(AccelByteId))
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get member with ID %s from room with ID %s as the user is not a member of the room!"), *MemberId.ToDebugString(), *RoomId);
		return nullptr;
	}

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));

	return GetAccelByteChatRoomMember(AccelByteId);
}

bool FOnlineChatAccelByte::GetLastMessages(
	const FUniqueNetId& UserId,
	const FChatRoomId& RoomId,
	int32 NumMessages,
	TArray<TSharedRef<FChatMessage>>& OutMessages)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("UserId: %s, RoomId: %s, NumMessages: %d"), *UserId.ToDebugString(), *RoomId, NumMessages);

	const FUniqueNetIdAccelByteUserRef AccelByteUserId = FUniqueNetIdAccelByteUser::CastChecked(UserId);	
	FString RoomIdToLoad = RoomId;

	// Check if the room id is the user id, so it will load personal chat message
	if(HasPersonalChat(AccelByteUserId->GetAccelByteId(), RoomId))
	{
		RoomIdToLoad = PersonalChatTopicId(AccelByteUserId->GetAccelByteId(), RoomId);
	}

	FChatRoomIdToChatMessages* RoomIdToChatMessages = UserIdToChatRoomMessagesCached.Find(AccelByteUserId);
	if (RoomIdToChatMessages == nullptr)
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get last messages from room with ID %s as the room was not found!"), *RoomId);
		return false;
	}
	TArray<TSharedRef<FChatMessage>>* Messages = RoomIdToChatMessages->Find(RoomIdToLoad);
	if (Messages == nullptr)
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get last messages from room with ID %s as the room has no messages!"), *RoomId);
		return false;
	}

	TArray<TSharedRef<FChatMessage>> MsgList = *Messages;

	const int32 TotalMessages = MsgList.Num();
	const int32 LastIndex = FMath::Max(TotalMessages - NumMessages, 0);
	for (int32 i = TotalMessages - 1; i >= LastIndex; i--)
	{
		OutMessages.Add(MsgList[i]);
	}

	AB_OSS_INTERFACE_TRACE_END(TEXT("Number of messages: %d"), OutMessages.Num());

	return true;
}

bool FOnlineChatAccelByte::IsMessageFromLocalUser(const FUniqueNetId& UserId, const FChatMessage& Message, const bool bIncludeExternalInstances)
{
	return UserId.IsValid() && *Message.GetUserId() == UserId;
}

void FOnlineChatAccelByte::DumpChatState() const
{
}

FString FOnlineChatAccelByte::SessionV2IdToChatTopicId(const FString& SessionId)
{
	return FString::Printf(TEXT("s.%s"), *SessionId);
}

FString FOnlineChatAccelByte::PartyV2IdToChatTopicId(const FString& PartyId)
{
	return FString::Printf(TEXT("p.%s"), *PartyId);
}

FString FOnlineChatAccelByte::PartyV1IdToChatTopicId(const FString& PartyId)
{
	return FString::Printf(TEXT("pv1.%s"), *PartyId);
}

FString FOnlineChatAccelByte::PersonalChatTopicId(const FString& FromUserId, const FString& ToUserId)
{
	TArray<FString> UserIds = {FromUserId, ToUserId};
	UserIds.Sort();
	return FString::Printf(TEXT("#%s,%s"), *UserIds[0], *UserIds[1]);
}

EAccelByteChatRoomType FOnlineChatAccelByte::GetChatRoomType(const FString& TopicId)
{
	if (TopicId.StartsWith(TEXT("s.")))
	{
		return EAccelByteChatRoomType::SESSION_V2;
	}
	else if (TopicId.StartsWith(TEXT("p.")))
	{
		return EAccelByteChatRoomType::PARTY_V2;
	}
	else if (TopicId.StartsWith(TEXT("pv1.")))
	{
		return EAccelByteChatRoomType::PARTY_V1;
	}
	else if (TopicId.StartsWith(TEXT("#")))
	{
		return EAccelByteChatRoomType::PERSONAL;
	}
	return EAccelByteChatRoomType::NORMAL;
}

bool FOnlineChatAccelByte::IsJoinedTopic(const FString& UserId, const FString& TopicId)
{
	const FAccelByteChatRoomInfoRef* ChatRoomInfo = TopicIdToChatRoomInfoCached.Find(TopicId);
	return ChatRoomInfo != nullptr && (*ChatRoomInfo)->HasMember(UserId);
}

bool FOnlineChatAccelByte::HasPersonalChat(const FString& FromUserId, const FString& ToUserId)
{
	const FString TopicId = PersonalChatTopicId(FromUserId, ToUserId);
	const FAccelByteChatRoomInfoRef* ChatRoomInfo = TopicIdToChatRoomInfoCached.Find(TopicId);
	return ChatRoomInfo != nullptr;
}

void FOnlineChatAccelByte::RemoveMemberFromTopic(const FString& UserId, const FString& TopicId)
{
	const FAccelByteChatRoomInfoRef* ChatRoomInfo = TopicIdToChatRoomInfoCached.Find(TopicId);
	if (ChatRoomInfo != nullptr)
	{
		(*ChatRoomInfo)->RemoveMember(UserId);
	}
}

void FOnlineChatAccelByte::AddTopic(const FAccelByteChatRoomInfoRef& ChatRoomInfo)
{
	TopicIdToChatRoomInfoCached.Add(ChatRoomInfo->GetRoomId(), ChatRoomInfo);
}

void FOnlineChatAccelByte::RemoveTopic(const FString& TopicId)
{
	TopicIdToChatRoomInfoCached.Remove(TopicId);
}

FAccelByteChatRoomInfoPtr FOnlineChatAccelByte::GetTopic(const FString& TopicId)
{
	const FAccelByteChatRoomInfoRef* ChatRoomInfo = TopicIdToChatRoomInfoCached.Find(TopicId);
	if (ChatRoomInfo != nullptr)
	{
		return *ChatRoomInfo;
	}
	return nullptr;
}

void FOnlineChatAccelByte::AddChatRoomMembers(TArray<TSharedRef<FAccelByteUserInfo>> Users)
{
	for (const auto& UserInfo : Users)
	{
		const FAccelByteChatRoomMemberRef* RoomMemberPtr = UserIdToChatRoomMemberCached.Find(UserInfo->Id->GetAccelByteId());
		if (RoomMemberPtr != nullptr)
		{
			(*RoomMemberPtr)->SetNickname(UserInfo->DisplayName);
		}
		else
		{
			FAccelByteChatRoomMemberRef RoomMember = FAccelByteChatRoomMember::Create(UserInfo->Id.ToSharedRef(), UserInfo->DisplayName);
			UserIdToChatRoomMemberCached.Add(UserInfo->Id->GetAccelByteId(), RoomMember);
		}
	}
}

void FOnlineChatAccelByte::AddChatMessage(FUniqueNetIdAccelByteUserRef AccelByteUserId, const FChatRoomId& ChatRoomId, TSharedRef<FChatMessage> ChatMessage)
{
	FChatRoomIdToChatMessages& RoomIdToChatMessages = UserIdToChatRoomMessagesCached.FindOrAdd(AccelByteUserId);
	TArray<TSharedRef<FChatMessage>>& ChatMessages = RoomIdToChatMessages.FindOrAdd(ChatRoomId);
	ChatMessages.Add(ChatMessage);
	if (ChatMessages.Num() > 1000)
	{
		// TODO: optimize this expensive operation
		ChatMessages.RemoveAt(0);
	}
}

FAccelByteChatRoomMemberRef FOnlineChatAccelByte::GetAccelByteChatRoomMember(const FString& UserId)
{
	FAccelByteChatRoomMemberRef* RoomMember = UserIdToChatRoomMemberCached.Find(UserId);
	if (RoomMember != nullptr)
	{
		return *RoomMember;
	}
	
	// fallback if somehow nickname can't be retrieved, the nickname will be updated when retrieving user info
	const FUniqueNetIdAccelByteUserRef MemberUserId = FUniqueNetIdAccelByteUser::Create(FAccelByteUniqueIdComposite(UserId));
	FAccelByteChatRoomMemberRef Member = FAccelByteChatRoomMember::Create(MemberUserId, UserId);
	UserIdToChatRoomMemberCached.Add(UserId, Member);
	return Member;
}

void FOnlineChatAccelByte::RegisterChatDelegates(const FUniqueNetId& PlayerId)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("PlayerId: %s"), *PlayerId.ToDebugString());

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystem->GetIdentityInterface());
	if (!ensure(IdentityInterface.IsValid()))
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to register notifications for session updates as our identity interface is invalid!"));
		return;
	}

	int32 LocalUserNum = 0;
	if (!ensure(IdentityInterface->GetLocalUserNum(PlayerId, LocalUserNum)))
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to register notifications for session updates as we could not get a local user index for player with ID '%s'!"), *PlayerId.ToDebugString());
		return;
	}

	AccelByte::FApiClientPtr ApiClient = IdentityInterface->GetApiClient(PlayerId);
	if (!ensure(ApiClient.IsValid()))
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to register notifications for session updates as player '%s' has an invalid API client!"), *PlayerId.ToDebugString());
		return;
	}

	// Begin Chat Notifications
	typedef AccelByte::Api::Chat::FAddRemoveFromTopicNotif FAddRemoveFromTopicNotificationDelegate;
	const FAddRemoveFromTopicNotificationDelegate OnRemoveFromTopicNotificationDelegate = FAddRemoveFromTopicNotificationDelegate::CreateThreadSafeSP(SharedThis(this), &FOnlineChatAccelByte::OnRemoveFromTopicNotification, LocalUserNum);
	ApiClient->Chat.SetRemoveFromTopicNotifDelegate(OnRemoveFromTopicNotificationDelegate);

	const FAddRemoveFromTopicNotificationDelegate OnAddToTopicNotificationDelegate = FAddRemoveFromTopicNotificationDelegate::CreateThreadSafeSP(SharedThis(this), &FOnlineChatAccelByte::OnAddToTopicNotification, LocalUserNum);
	ApiClient->Chat.SetAddToTopicNotifDelegate(OnAddToTopicNotificationDelegate);

	typedef AccelByte::Api::Chat::FChatDisconnectNotif FChatDisconnectNotificationDelegate;
	const FChatDisconnectNotificationDelegate OnChatDisconnectNotificationDelegate = FChatDisconnectNotificationDelegate::CreateThreadSafeSP(SharedThis(this), &FOnlineChatAccelByte::OnChatDisconnectedNotification, LocalUserNum);
	ApiClient->Chat.SetDisconnectNotifDelegate(OnChatDisconnectNotificationDelegate);

	typedef AccelByte::Api::Chat::FChatNotif FReceivedChatNotificationDelegate;
	const FReceivedChatNotificationDelegate OnReceivedChatNotificationDelegate = FReceivedChatNotificationDelegate::CreateThreadSafeSP(SharedThis(this), &FOnlineChatAccelByte::OnReceivedChatNotification, LocalUserNum);
	ApiClient->Chat.SetChatNotifDelegate(OnReceivedChatNotificationDelegate);

	typedef AccelByte::Api::Chat::FDeleteUpdateTopicNotif FDeleteUpdateTopicNotificationDelegate;
	const FDeleteUpdateTopicNotificationDelegate OnUpdateTopicNotificationDelegate = FDeleteUpdateTopicNotificationDelegate::CreateThreadSafeSP(SharedThis(this), &FOnlineChatAccelByte::OnUpdateTopicNotification, LocalUserNum);
	ApiClient->Chat.SetUpdateTopicNotifDelegate(OnUpdateTopicNotificationDelegate);

	const FDeleteUpdateTopicNotificationDelegate OnDeleteTopicNotificationDelegate = FDeleteUpdateTopicNotificationDelegate::CreateThreadSafeSP(SharedThis(this), &FOnlineChatAccelByte::OnDeleteTopicNotification, LocalUserNum);
	ApiClient->Chat.SetDeleteTopicNotifDelegate(OnDeleteTopicNotificationDelegate);

	typedef AccelByte::Api::Chat::FReadChatNotif FReadChatNotificationDelegate;
	const FReadChatNotificationDelegate OnReadChatNotificationDelegate = FReadChatNotificationDelegate::CreateThreadSafeSP(SharedThis(this), &FOnlineChatAccelByte::OnReadChatNotification, LocalUserNum);
	ApiClient->Chat.SetReadChatNotifDelegate(OnReadChatNotificationDelegate);

	typedef AccelByte::Api::Chat::FUserBanUnbanNotif FUserBanUnbanNotificationDelegate;
	const FUserBanUnbanNotificationDelegate OnUserBanNotificationDelegate = FUserBanUnbanNotificationDelegate::CreateThreadSafeSP(SharedThis(this), &FOnlineChatAccelByte::OnUserBanNotification, LocalUserNum);
	ApiClient->Chat.SetUserBanNotifDelegate(OnUserBanNotificationDelegate);

	const FUserBanUnbanNotificationDelegate OnUserUnBanNotificationDelegate = FUserBanUnbanNotificationDelegate::CreateThreadSafeSP(SharedThis(this), &FOnlineChatAccelByte::OnUserUnbanNotification, LocalUserNum);
	ApiClient->Chat.SetUserUnbanNotifDelegate(OnUserUnBanNotificationDelegate);
	//~ End Chat Notifications

	// Cache topic data
	FAccelByteModelsChatQueryTopicRequest QueryTopicRequest;
	// TODO: get all topic data if the user has more that 200 topics (should be working for most of cases now)
	QueryTopicRequest.Offset = 0;
	QueryTopicRequest.Limit = 200;
	const FOnChatQueryRoomComplete OnQueryTopicResponse = FOnChatQueryRoomComplete::CreateThreadSafeSP(SharedThis(this), &FOnlineChatAccelByte::OnQueryChatRoomInfoComplete);
	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteChatQueryRoom>(AccelByteSubsystem, PlayerId, QueryTopicRequest, OnQueryTopicResponse);

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineChatAccelByte::OnChatDisconnectedNotification(const FAccelByteModelsChatDisconnectNotif& DisconnectEvent, int32 LocalUserNum)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("Message: %s"), *DisconnectEvent.Message);

	TriggerOnChatDisconnectedDelegates(DisconnectEvent.Message);

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineChatAccelByte::OnRemoveFromTopicNotification(const FAccelByteModelsChatUpdateUserTopicNotif& RemoveTopicEvent, int32 LocalUserNum)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("TopicId: %s"), *RemoveTopicEvent.TopicId);

	RemoveMemberFromTopic(RemoveTopicEvent.SenderId, RemoveTopicEvent.TopicId);
	TriggerOnTopicRemovedDelegates(RemoveTopicEvent.Name, RemoveTopicEvent.TopicId, RemoveTopicEvent.UserId);
	const TSharedPtr<const FUniqueNetId> UserIdPtr = AccelByteSubsystem->GetIdentityInterface()->GetUniquePlayerId(LocalUserNum);
	if (UserIdPtr.IsValid())
	{
		TSharedPtr<const FUniqueNetId> SenderUserId = FUniqueNetIdAccelByteUser::Create(FAccelByteUniqueIdComposite(RemoveTopicEvent.SenderId));
		TriggerOnChatRoomMemberExitDelegates(*UserIdPtr, RemoveTopicEvent.TopicId, *SenderUserId);
	}
	
	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineChatAccelByte::OnAddToTopicNotification(const FAccelByteModelsChatUpdateUserTopicNotif& AddTopicEvent, int32 LocalUserNum)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("TopicId: %s"), *AddTopicEvent.TopicId);

	const IOnlineIdentityPtr IdentityInterface = AccelByteSubsystem->GetIdentityInterface();
	if (!ensure(IdentityInterface.IsValid()))
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to add to topic notification as our identity interface is invalid!"));
		return;
	}

	const TSharedPtr<const FUniqueNetId> UserIdPtr = AccelByteSubsystem->GetIdentityInterface()->GetUniquePlayerId(LocalUserNum);
	if (!ensure(UserIdPtr.IsValid()))
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to add to topic notification as our UserIdPtr is invalid!"));
		return;
	}
	
	// Find the room first, if the room not found fetch the room info
	// If room found, but no member info, fetch user information
	// If room info and member info found, trigger the ChatRoomMemberJoinDelegate
	TSharedPtr<const FUniqueNetId> SenderUserId = FUniqueNetIdAccelByteUser::Create(FAccelByteUniqueIdComposite(AddTopicEvent.SenderId));
	const FAccelByteChatRoomInfoRef* ChatRoomInfo = TopicIdToChatRoomInfoCached.Find(AddTopicEvent.TopicId);
	if (ChatRoomInfo == nullptr)
	{
		UE_LOG_AB(Verbose, TEXT("ChatRoomInfo not found by room ID %s!"), *AddTopicEvent.TopicId);
		// we don't have room data locally
		const FOnChatQueryRoomByIdComplete OnQueryTopicResponse = FOnChatQueryRoomByIdComplete::CreateThreadSafeSP(SharedThis(this), &FOnlineChatAccelByte::OnQueryChatRoomById_TriggerChatRoomMemberJoin, UserIdPtr, SenderUserId);
		AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteChatQueryRoomById>(AccelByteSubsystem, *UserIdPtr, AddTopicEvent.TopicId, OnQueryTopicResponse);
	}
	else
	{
		UE_LOG_AB(Verbose, TEXT("ChatRoomInfo for room ID %s found. Current member num: %d, adding member with ID %s"), *AddTopicEvent.TopicId, (*ChatRoomInfo)->GetMembers().Num(), *AddTopicEvent.SenderId);
		(*ChatRoomInfo)->AddMember(AddTopicEvent.SenderId);
				
		FAccelByteChatRoomMemberRef* MemberPtr = UserIdToChatRoomMemberCached.Find(AddTopicEvent.SenderId);
		// member is not cached get the info
		if (MemberPtr == nullptr || !(*MemberPtr)->HasNickname())
		{
			const FOnlineUserCacheAccelBytePtr UserStore = AccelByteSubsystem->GetUserCache();
			if (!UserStore.IsValid())
			{
				UE_LOG_AB(Warning, TEXT("Unable to get member info as our user store instance is invalid!"));
				TriggerOnChatRoomMemberJoinDelegates(*UserIdPtr, AddTopicEvent.TopicId, *SenderUserId);
			}
			else
			{
				const TArray<FString> UserIds = {AddTopicEvent.SenderId};
				const FOnQueryUsersComplete OnQueryUsersCompleteDelegate = FOnQueryUsersComplete::CreateThreadSafeSP(SharedThis(this), &FOnlineChatAccelByte::OnQueryChatMemberInfo_TriggerChatRoomMemberJoin, AddTopicEvent.TopicId, UserIdPtr, SenderUserId);
				UserStore->QueryUsersByAccelByteIds(LocalUserNum, UserIds, OnQueryUsersCompleteDelegate);
			}
		}
		else
		{
			TriggerOnChatRoomMemberJoinDelegates(*UserIdPtr, AddTopicEvent.TopicId, *SenderUserId);
		}
	}	
	
	TriggerOnTopicAddedDelegates(AddTopicEvent.Name, AddTopicEvent.TopicId, AddTopicEvent.UserId);
	
	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineChatAccelByte::OnReceivedChatNotification(const FAccelByteModelsChatNotif& ChatNotif, int32 LocalUserNum)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("TopicId: %s"), *ChatNotif.TopicId);

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystem->GetIdentityInterface());
	if (!ensure(IdentityInterface.IsValid()))
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle received chat notification as our identity interface is invalid!"));
		return;
	}

	const TSharedPtr<const FUniqueNetId> UserIdPtr = IdentityInterface->GetUniquePlayerId(LocalUserNum);
	if (!UserIdPtr.IsValid())
	{
		AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle received chat notification as UserIdPtr is invalid!"));
		return;
	}

	FAccelByteUniqueIdComposite SenderCompositeId;
	SenderCompositeId.Id = ChatNotif.From;
	TSharedPtr<const FUniqueNetIdAccelByteUser> SenderUserId = FUniqueNetIdAccelByteUser::Create(SenderCompositeId);

	FChatRoomId OutChatRoomId = ChatNotif.TopicId;
	const EAccelByteChatRoomType RoomType = GetChatRoomType(ChatNotif.TopicId);

	FAccelByteChatRoomMemberRef Member = GetAccelByteChatRoomMember(ChatNotif.From);
	TSharedRef<FAccelByteChatMessage> OutChatMessage = MakeShared<FAccelByteChatMessage>(SenderUserId.ToSharedRef(), Member->GetNickname(), ChatNotif.Message, ChatNotif.CreatedAt);

	const FUniqueNetIdAccelByteUserRef AccelByteUserId = FUniqueNetIdAccelByteUser::CastChecked(UserIdPtr.ToSharedRef());
	AddChatMessage(AccelByteUserId, OutChatRoomId, OutChatMessage);

	if (RoomType == EAccelByteChatRoomType::PERSONAL)
	{
		TriggerOnChatPrivateMessageReceivedDelegates(UserIdPtr.ToSharedRef().Get(), OutChatMessage);
	}
	else
	{		
		TriggerOnChatRoomMessageReceivedDelegates(UserIdPtr.ToSharedRef().Get(), ChatNotif.TopicId, OutChatMessage);
	}

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineChatAccelByte::OnUpdateTopicNotification(const FAccelByteModelsChatUpdateTopicNotif& UpdateTopicNotif, int32 LocalUserNum)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("TopicId: %s"), *UpdateTopicNotif.TopicId);

	FAccelByteChatRoomInfoRef* RoomInfoPtr = TopicIdToChatRoomInfoCached.Find(UpdateTopicNotif.TopicId);
	if (RoomInfoPtr != nullptr)
	{
		(*RoomInfoPtr)->UpdateTopicData(UpdateTopicNotif);
	}
	
	TriggerOnTopicUpdatedDelegates(UpdateTopicNotif.Name, UpdateTopicNotif.TopicId, UpdateTopicNotif.SenderId, UpdateTopicNotif.IsChannel);

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineChatAccelByte::OnDeleteTopicNotification(const FAccelByteModelsChatUpdateTopicNotif& DeleteTopicNotif, int32 LocalUserNum)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("TopicId: %s"), *DeleteTopicNotif.TopicId);

	RemoveTopic(DeleteTopicNotif.TopicId);
	TriggerOnTopicDeletedDelegates(DeleteTopicNotif.Name, DeleteTopicNotif.TopicId, DeleteTopicNotif.SenderId, DeleteTopicNotif.IsChannel);

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineChatAccelByte::OnReadChatNotification(const FAccelByteModelsReadChatNotif& ReadChatNotif, int32 LocalUserNum)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT(""));

	TriggerOnReadChatReceivedDelegates(ReadChatNotif.ReadChat);

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineChatAccelByte::OnUserBanNotification(const FAccelByteModelsChatUserBanUnbanNotif& UserBanNotif, int32 LocalUserNum)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT(""));

	TriggerOnUserBannedDelegates(UserBanNotif.UserId, UserBanNotif.Ban, UserBanNotif.EndDate, UserBanNotif.Reason, UserBanNotif.Enable);

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineChatAccelByte::OnUserUnbanNotification(const FAccelByteModelsChatUserBanUnbanNotif& UserUnbanNotif, int32 LocalUserNum)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT(""));

	TriggerOnUserUnbannedDelegates(UserUnbanNotif.UserId, UserUnbanNotif.Ban, UserUnbanNotif.EndDate, UserUnbanNotif.Reason, UserUnbanNotif.Enable);

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineChatAccelByte::OnQueryChatRoomInfoComplete(bool bWasSuccessful, TArray<FAccelByteChatRoomInfoRef> RoomList, int32 LocalUserNum)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("Room length %d"), RoomList.Num());

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineChatAccelByte::OnQueryChatMemberInfo_TriggerChatRoomMemberJoin(bool bIsSuccessful, TArray<TSharedRef<FAccelByteUserInfo>> UsersQueried, FString RoomId, TSharedPtr<const FUniqueNetId> InUserId, TSharedPtr<const FUniqueNetId> InMemberId)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("RoomId %s MemberId %s"), *RoomId, *InMemberId->ToDebugString());

	if(!bIsSuccessful)
	{
		AB_OSS_INTERFACE_TRACE_END(TEXT("Query chat member info on ChatRoomMemberJoin failed"));
		return;
	}
	
	TriggerOnChatRoomMemberJoinDelegates(*InUserId, RoomId, *InMemberId);

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineChatAccelByte::OnQueryChatRoomById_TriggerChatRoomMemberJoin(bool bWasSuccessful, FAccelByteChatRoomInfoPtr RoomInfo, int32 LocalUserNum, TSharedPtr<const FUniqueNetId> InUserId, TSharedPtr<const FUniqueNetId> InMemberId)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("MemberId %s"), *InMemberId->ToDebugString());
	
	if(!bWasSuccessful)
	{
		AB_OSS_INTERFACE_TRACE_END(TEXT("Query room info on ChatRoomMemberJoin failed"));
		return;
	}
	
	TriggerOnChatRoomMemberJoinDelegates(*InUserId, RoomInfo->GetRoomId(), *InMemberId);

	AB_OSS_INTERFACE_TRACE_END(TEXT(""));
}

const FUniqueNetIdRef& FAccelByteChatMessage::GetUserId() const
{
	return SenderUserId;
}

const FString& FAccelByteChatMessage::GetNickname() const
{
	return SenderNickname;
}

const FString& FAccelByteChatMessage::GetBody() const
{
	return MessageBody;
}

const FDateTime& FAccelByteChatMessage::GetTimestamp() const
{
	return Timestamp;
}

FAccelByteChatRoomMember::FAccelByteChatRoomMember(const FUniqueNetIdAccelByteUserRef& InUserId, const FString& InNickname)
	: UserId(InUserId), Nickname(InNickname)
{
}

FAccelByteChatRoomMemberRef FAccelByteChatRoomMember::Create(const FUniqueNetIdAccelByteUserRef& InUserId, const FString& InNickname)
{
	return MakeShared<FAccelByteChatRoomMember>(InUserId, InNickname);
}

const FUniqueNetIdRef& FAccelByteChatRoomMember::GetUserId() const
{
	return UserId;
}

const FString& FAccelByteChatRoomMember::GetNickname() const
{
	return Nickname;
}

void FAccelByteChatRoomMember::SetNickname(const FString& InNickname)
{
	Nickname = InNickname;
}

bool FAccelByteChatRoomMember::HasNickname() const
{
	const FUniqueNetIdAccelByteUserRef AccelByteUserId = FUniqueNetIdAccelByteUser::CastChecked(UserId);
	return Nickname != AccelByteUserId->GetAccelByteId();
}

const FChatRoomId& FAccelByteChatRoomInfo::GetRoomId() const
{
	return TopicData.TopicId;
}

const FUniqueNetIdRef& FAccelByteChatRoomInfo::GetOwnerId() const
{
	return OwnerId;
}

const FString& FAccelByteChatRoomInfo::GetSubject() const
{
	return TopicData.Name;
}

bool FAccelByteChatRoomInfo::IsPrivate() const
{
	return bIsPrivate;
}

bool FAccelByteChatRoomInfo::IsJoined() const
{
	return bIsJoined;
}

const FChatRoomConfig& FAccelByteChatRoomInfo::GetRoomConfig() const
{
	return RoomConfig;
}

FString FAccelByteChatRoomInfo::ToDebugString() const
{
	return FString::Printf(TEXT(""));
}

void FAccelByteChatRoomInfo::SetChatInfo(const TSharedRef<FJsonObject>& JsonInfo)
{
	FAccelByteModelsChatTopicQueryData Data;
	if (!FJsonObjectConverter::JsonObjectToUStruct(JsonInfo, &Data))
	{
		UE_LOG_AB(Warning, TEXT("Unable to deserialize chat info json"));
		return;
	}

	SetTopicData(Data);
}

FAccelByteChatRoomInfoRef FAccelByteChatRoomInfo::Create()
{
	return MakeShared<FAccelByteChatRoomInfo>();
}

bool FAccelByteChatRoomInfo::HasMember(const FString& UserId) const
{
	return TopicData.Members.Contains(UserId);
}

const TArray<FString>& FAccelByteChatRoomInfo::GetMembers() const
{
	return TopicData.Members;
}

void FAccelByteChatRoomInfo::SetTopicData(const FAccelByteModelsChatTopicQueryData& InTopicData)
{
	TopicData = InTopicData;

    // NOTE: actually the topic don't have an owner, so we set first member as the owner
	if (TopicData.Members.Num() > 0)
	{
		FAccelByteUniqueIdComposite SenderCompositeId;
		SenderCompositeId.Id = TopicData.Members[0];
		OwnerId = FUniqueNetIdAccelByteUser::Create(SenderCompositeId);
	}

	const EAccelByteChatRoomType RoomType = FOnlineChatAccelByte::GetChatRoomType(TopicData.TopicId);
	switch (RoomType) {
	case EAccelByteChatRoomType::PERSONAL:
	case EAccelByteChatRoomType::PARTY_V2:
	case EAccelByteChatRoomType::PARTY_V1:
	case EAccelByteChatRoomType::SESSION_V2:
		// auto create room should be private and cannot be joined
		bIsPrivate = true;
		break;
	case EAccelByteChatRoomType::NORMAL:
		// currently we don't support private room
		bIsPrivate = false;
		break;
	default: break;
	}
	
	// currently only able to get joined room
	bIsJoined = true;
}

void FAccelByteChatRoomInfo::UpdateTopicData(const FAccelByteModelsChatUpdateTopicNotif& InUpdateTopic)
{
	TopicData.Name = InUpdateTopic.Name;
}

void FAccelByteChatRoomInfo::AddMember(const FString& UserId)
{
	TopicData.Members.AddUnique(UserId);
}

void FAccelByteChatRoomInfo::RemoveMember(const FString& UserId)
{
	TopicData.Members.Remove(UserId);
}
