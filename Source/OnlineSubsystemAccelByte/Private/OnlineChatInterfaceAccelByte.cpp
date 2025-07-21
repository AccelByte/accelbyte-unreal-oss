// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineChatInterfaceAccelByte.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "OnlinePredefinedEventInterfaceAccelByte.h"
#include "OnlineSubsystemAccelByteInternalHelpers.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "AsyncTasks/Chat/OnlineAsyncTaskAccelByteConnectChat.h"
#include "AsyncTasks/Chat/OnlineAsyncTaskAccelByteChatCreateRoom.h"
#include "AsyncTasks/Chat/OnlineAsyncTaskAccelByteChatConfigureRoom.h"
#include "AsyncTasks/Chat/OnlineAsyncTaskAccelByteChatDeleteSystemMessages.h"
#include "AsyncTasks/Chat/OnlineAsyncTaskAccelByteChatJoinPublicRoom.h"
#include "AsyncTasks/Chat/OnlineAsyncTaskAccelByteChatExitRoom.h"
#include "AsyncTasks/Chat/OnlineAsyncTaskAccelByteChatGetConfig.h"
#include "AsyncTasks/Chat/OnlineAsyncTaskAccelByteChatGetSystemMessagesStats.h"
#include "AsyncTasks/Chat/OnlineAsyncTaskAccelByteChatQueryRoom.h"
#include "AsyncTasks/Chat/OnlineAsyncTaskAccelByteChatQueryRoomById.h"
#include "AsyncTasks/Chat/OnlineAsyncTaskAccelByteChatQuerySystemMessages.h"
#include "AsyncTasks/Chat/OnlineAsyncTaskAccelByteChatQueryTransientSystemMessages.h"
#include "AsyncTasks/Chat/OnlineAsyncTaskAccelByteChatSendPersonalChat.h"
#include "AsyncTasks/Chat/OnlineAsyncTaskAccelByteChatSendRoomChat.h"
#include "AsyncTasks/Chat/OnlineAsyncTaskAccelByteChatUpdateSystemMessages.h"
#include "AsyncTasks/Chat/OnlineAsyncTaskAccelByteGetUserChatConfiguration.h"
#include "AsyncTasks/Chat/OnlineAsyncTaskAccelByteSetUserChatConfiguration.h"
#include "AsyncTasks/Chat/OnlineAsyncTaskAccelByteChatReportMessage.h"
#include "OnlineSubsystemAccelByteConfig.h"

using namespace AccelByte;

bool FOnlineChatAccelByte::GetFromSubsystem(const IOnlineSubsystem* Subsystem, FOnlineChatAccelBytePtr& OutInterfaceInstance)
{
	OutInterfaceInstance = StaticCastSharedPtr<FOnlineChatAccelByte>(Subsystem->GetChatInterface());
	return OutInterfaceInstance.IsValid();
}

bool FOnlineChatAccelByte::GetFromSubsystem(FOnlineSubsystemAccelByte* Subsystem, FOnlineChatAccelBytePtr& OutInterfaceInstance)
{
	if (Subsystem == nullptr)
	{
		return false;
	}

	OutInterfaceInstance = StaticCastSharedPtr<FOnlineChatAccelByte>(Subsystem->GetChatInterface());
	return OutInterfaceInstance.IsValid();
}

bool FOnlineChatAccelByte::GetFromWorld(const UWorld* World, FOnlineChatAccelBytePtr& OutInterfaceInstance)
{
	const IOnlineSubsystem* Subsystem = ::Online::GetSubsystem(World);
	if (Subsystem == nullptr)
	{
		OutInterfaceInstance = nullptr;
		return false;
	}

	return GetFromSubsystem(Subsystem, OutInterfaceInstance);
}

FOnlineChatAccelByte::FOnlineChatAccelByte(FOnlineSubsystemAccelByte* InSubsystem)
#if ENGINE_MAJOR_VERSION >= 5
	: AccelByteSubsystem(InSubsystem->AsWeak())
#else
	: AccelByteSubsystem(InSubsystem->AsShared())
#endif
{}

bool FOnlineChatAccelByte::Connect(int32 LocalUserNum)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d"), LocalUserNum);

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelByteSubsystemPtr.Get() is invalid"));
		return false;
	}

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystemPtr->GetIdentityInterface());
	if (!ensure(IdentityInterface.IsValid()))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to register notifications for session updates as our identity interface is invalid!"));
		return false;
	}

	// Don't attempt to connect again if we are already reporting as connected
	if (IdentityInterface->GetLoginStatus(LocalUserNum) != ELoginStatus::LoggedIn)
	{
		const FString ErrorStr = TEXT("chat-connect-failed-not-logged-in");
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("User not logged in at user index '%d'!"), LocalUserNum);

		TriggerOnConnectChatCompleteDelegates(LocalUserNum, false, FUniqueNetIdAccelByteUser::Invalid().Get(), ErrorStr);
		return false;
	}

	FUniqueNetIdPtr LocalUserId = IdentityInterface->GetUniquePlayerId(LocalUserNum);
	if (!LocalUserId.IsValid())
	{
		const FString ErrorStr = TEXT("chat-connect-failed-user-invalid");
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Local User at index '%d' is invalid!"), LocalUserNum);

		TriggerOnConnectChatCompleteDelegates(LocalUserNum, false, FUniqueNetIdAccelByteUser::Invalid().Get(), ErrorStr);
		return false;
	}

	TSharedPtr<FUserOnlineAccount> UserAccount = IdentityInterface->GetUserAccount(LocalUserId);
	if (!UserAccount.IsValid())
	{
		const FString ErrorStr = TEXT("chat-connect-failed-user-account-invalid");
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("User Account at index '%d' is invalid!"), LocalUserNum);

		TriggerOnConnectChatCompleteDelegates(LocalUserNum, false, *LocalUserId.Get(), ErrorStr);
		return false;
	}

	const TSharedPtr<FUserOnlineAccountAccelByte> UserAccountAccelByte = StaticCastSharedPtr<FUserOnlineAccountAccelByte>(UserAccount);
	if (UserAccountAccelByte->IsConnectedToChat())
	{
		const FString ErrorStr = TEXT("chat-connect-failed-already-connected");
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("User already connected to chat at user index '%d'!"), LocalUserNum);

		TriggerOnConnectChatCompleteDelegates(LocalUserNum, false, *LocalUserId.Get(), ErrorStr);
		return false;
	}
	
	bool bIsAutoCheckMaximumLimitChatMessage { false };
	FOnlineSubsystemAccelByteConfigPtr Config = AccelByteSubsystemPtr->GetConfig();
	if (Config.IsValid())
	{
		bIsAutoCheckMaximumLimitChatMessage = Config->GetAutoCheckMaximumChatMessageLimit().GetValue();
	}
	
	// If auto check max limit message config is true, then get the config first from backend
	if (bIsAutoCheckMaximumLimitChatMessage)
	{
		AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskSerial<FOnlineAsyncTaskAccelByteChatGetConfig>(AccelByteSubsystemPtr.Get(), *LocalUserId.Get());
		AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskSerial<FOnlineAsyncTaskAccelByteConnectChat>(AccelByteSubsystemPtr.Get(), *LocalUserId.Get());
	}
	else
	{
		AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteConnectChat>(AccelByteSubsystemPtr.Get(), *LocalUserId.Get());
	}

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Dispatching async task to attempt to connect chat!"));
	return true;
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

	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("UserId: %s, Nickname: %s, bIsJoinable: %s"), *UserId.ToDebugString(), *Nickname, LOG_BOOL_FORMAT(ChatRoomConfig.bIsJoinable));

	// TODO: Store the supplied nickname
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelByteSubsystemPtr.Get() is invalid"));
		return false;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteChatCreateRoom>(AccelByteSubsystemPtr.Get(), UserId, ChatRoomConfig);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));

	return true;
}

bool FOnlineChatAccelByte::ConfigureRoom(const FUniqueNetId&, const FChatRoomId&, const FChatRoomConfig&)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));

	// #NOTE Not doing anything, as none of the fields in FChatRoomConfig are supported by the backend. So a call to this method would be redundant

	AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Cannot configure chat room using FChatRoomConfig as its fields are not supported! Please supply an instance of FAccelByteChatRoomConfig instead!"));

	return false;
}

bool FOnlineChatAccelByte::ConfigureRoom(const FUniqueNetId& UserId, const FChatRoomId& RoomId, const FAccelByteChatRoomConfig& ChatRoomConfig)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("UserId: %s, RoomId: %s, bIsJoinable: %s"), *UserId.ToDebugString(), *RoomId, LOG_BOOL_FORMAT(ChatRoomConfig.bIsJoinable));

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelByteSubsystemPtr.Get() is invalid"));
		return false;
	}
	
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteChatConfigureRoom>(AccelByteSubsystemPtr.Get(), UserId, RoomId, ChatRoomConfig);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));

	return true;
}

bool FOnlineChatAccelByte::JoinPublicRoom(
	const FUniqueNetId& UserId,
	const FChatRoomId& RoomId,
	const FString& Nickname,
	const FChatRoomConfig&)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("UserId: %s, RoomId: %s, Nickname: %s"), *UserId.ToDebugString(), *RoomId, *Nickname);

	// TODO: Store the supplied nickname
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelByteSubsystemPtr.Get() is invalid"));
		return false;
	}

	FOnlineAsyncTaskInfo TaskInfo;
	TaskInfo.Type = ETypeOfOnlineAsyncTask::Parallel;
	TaskInfo.bCreateEpicForThis = true;
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTask<FOnlineAsyncTaskAccelByteChatJoinPublicRoom>(TaskInfo, AccelByteSubsystemPtr.Get(), UserId, RoomId);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));

	return true;
}

bool FOnlineChatAccelByte::JoinPrivateRoom(const FUniqueNetId&, const FChatRoomId&, const FString&, const FChatRoomConfig&)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));

	// #NOTE Not doing anything, as the backend does not support private rooms

	AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Cannot join private chat room as it is unsupported!"));

	return false;
}

bool FOnlineChatAccelByte::ExitRoom(const FUniqueNetId& UserId, const FChatRoomId& RoomId)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("UserId: %s, RoomId: %s"), *UserId.ToDebugString(), *RoomId);

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelByteSubsystemPtr.Get() is invalid"));
		return false;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteChatExitRoom>(AccelByteSubsystemPtr.Get(), UserId, RoomId);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));

	return true;
}

bool FOnlineChatAccelByte::SendRoomChat(const FUniqueNetId& UserId, const FChatRoomId& RoomId, const FString& MsgBody)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("UserId: %s, RoomId: %s, MsgBody: %s"), *UserId.ToDebugString(), *RoomId, *MsgBody);

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelByteSubsystemPtr.Get() is invalid"));
		return false;
	}

	const FUniqueNetIdAccelByteUserRef AccelByteUserId = FUniqueNetIdAccelByteUser::CastChecked(UserId);
	const FString AccelByteId = AccelByteUserId->GetAccelByteId();
	if (!IsJoinedTopic(AccelByteId, RoomId))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Unable to send room chat as the user %s has not joined room %s"), *AccelByteId, *RoomId);
		return false;
	}

	// If max chat message length is not an index none, then check the message body length compare with max chat message length
	if (MaxChatMessageLength != INDEX_NONE)
	{
		if (MsgBody.Len() > MaxChatMessageLength)
		{
			AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Unable to send room chat as the message body is more than limit of %d characters"), MaxChatMessageLength);
			return false;
		}
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteChatSendRoomChat>(AccelByteSubsystemPtr.Get(), UserId, RoomId, MsgBody);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));

	return true;
}

bool FOnlineChatAccelByte::SendPrivateChat(const FUniqueNetId& UserId, const FUniqueNetId& RecipientId, const FString& MsgBody)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("UserId: %s, RecipientId: %s"), *UserId.ToDebugString(), *RecipientId.ToDebugString());

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelByteSubsystemPtr.Get() is invalid"));
		return false;
	}

	// If max chat message length is not an index none, then check the message body length compare with max chat message length
	if (MaxChatMessageLength != INDEX_NONE)
	{
		if (MsgBody.Len() > MaxChatMessageLength)
		{
			AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Unable to send room chat as the message body is more than limit of %d characters"), MaxChatMessageLength);
			return false;
		}
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteChatSendPersonalChat>(AccelByteSubsystemPtr.Get(), UserId, RecipientId, MsgBody);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));

	return true;
}

bool FOnlineChatAccelByte::DeleteSystemMessages(const FUniqueNetId& UserId, const TSet<FString>& MessageIds)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("UserId: %s"), *UserId.ToDebugString());

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelByteSubsystemPtr.Get() is invalid"));
		return false;
	}

	if (MessageIds.Num() <= 0)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Can't delete system message, the message ids is empty"));
		return false;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteChatDeleteSystemMessages>(AccelByteSubsystemPtr.Get(), UserId, MessageIds);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));

	return true;
}

bool FOnlineChatAccelByte::UpdateSystemMessages(const FUniqueNetId& UserId, const TArray<FAccelByteModelsActionUpdateSystemMessage>& ActionUpdateSystemMessages)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("UserId: %s"), *UserId.ToDebugString());

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelByteSubsystemPtr.Get() is invalid"));
		return false;
	}

	if (ActionUpdateSystemMessages.Num() <= 0)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Can't update system messages, the actions is empty"));
		return false;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteChatUpdateSystemMessages>(AccelByteSubsystemPtr.Get(), UserId, ActionUpdateSystemMessages);
	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));

	return true;
}

bool FOnlineChatAccelByte::QuerySystemMessage(const FUniqueNetId& UserId, const FQuerySystemMessageOptions& OptionalParams)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("UserId: %s"), *UserId.ToDebugString());
	
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelByteSubsystemPtr.Get() is invalid"));
		return false;
	}
	
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteChatQuerySystemMessages>(AccelByteSubsystemPtr.Get(), UserId, OptionalParams);
	
	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));

	return true;
}

bool FOnlineChatAccelByte::QueryTransientSystemMessage(const FUniqueNetId& UserId, const FQuerySystemMessageOptions& OptionalParams)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("UserId: %s"), *UserId.ToDebugString());

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelByteSubsystemPtr.Get() is invalid"));
		return false;
	}
	
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteChatQueryTransientSystemMessages>(AccelByteSubsystemPtr.Get(), UserId, OptionalParams);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));

	return true;
}

bool FOnlineChatAccelByte::GetSystemMessageStats(const FUniqueNetId& UserId, const FAccelByteGetSystemMessageStatsRequest& Request)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("UserId: %s"), *UserId.ToDebugString());

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelByteSubsystemPtr.Get() is invalid"));
		return false;
	}
	
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteChatGetSystemMessagesStats>(AccelByteSubsystemPtr.Get(), UserId, Request);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));

	return true;
}

bool FOnlineChatAccelByte::GetChatConfig(int32 LocalUserNum)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %i"), LocalUserNum);

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelByteSubsystemPtr.Get() is invalid"));
		return false;
	}

	if (IsRunningDedicatedServer())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Get chat config is not supported for game server"));
		return false;
	}
	const TSharedPtr<const FUniqueNetId> UserIdPtr = AccelByteSubsystemPtr->GetIdentityInterface()->GetUniquePlayerId(LocalUserNum);
	GetChatConfig(*UserIdPtr.Get());

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));

	return true;
}

bool FOnlineChatAccelByte::GetChatConfig(const FUniqueNetId& UserId)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("UserId: %s"), *UserId.ToDebugString());

	if (IsRunningDedicatedServer())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Get chat config is not supported for game server"));
		return false;
	}

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelByteSubsystemPtr.Get() is invalid"));
		return false;
	}
	
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteChatGetConfig>(AccelByteSubsystemPtr.Get(), UserId);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));

	return true;
}

bool FOnlineChatAccelByte::GetUserConfiguration(const int32 LocalUserNum)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %i"), LocalUserNum);

	if (IsRunningDedicatedServer())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Get user chat configuration is not supported for game server"));
		return false;
	}

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelByteSubsystemPtr.Get() is invalid"));
		return false;
	}

	const FUniqueNetIdPtr UserIdPtr = AccelByteSubsystemPtr->GetIdentityInterface()->GetUniquePlayerId(LocalUserNum);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
	
	return GetUserConfiguration(*UserIdPtr);
}

bool FOnlineChatAccelByte::GetUserConfiguration(const FUniqueNetId& UserId)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("UserId: %s"), *UserId.ToDebugString());

	if (IsRunningDedicatedServer())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Get user chat configuration is not supported for game server"));
		return false;
	}

	if (!UserId.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Get user chat configuration failed as the user id is invalid"));
		return false;
	}

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelByteSubsystemPtr.Get() is invalid"));
		return false;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGetUserChatConfiguration>(AccelByteSubsystemPtr.Get(), UserId);
	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));

	return true;
}

bool FOnlineChatAccelByte::SetUserConfiguration(const int32 LocalUserNum
	, const FAccelByteModelsSetUserChatConfigurationRequest& Configuration)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %i"), LocalUserNum);

	if (IsRunningDedicatedServer())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Set user chat configuration is not supported for game server"));
		return false;
	}

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelByteSubsystemPtr.Get() is invalid"));
		return false;
	}

	const TSharedPtr<const FUniqueNetId> UserIdPtr = AccelByteSubsystemPtr->GetIdentityInterface()->GetUniquePlayerId(LocalUserNum);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));

	return SetUserConfiguration(*UserIdPtr, Configuration);
}

bool FOnlineChatAccelByte::SetUserConfiguration(const FUniqueNetId& UserId
	, const FAccelByteModelsSetUserChatConfigurationRequest& Configuration)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("UserId: %s"), *UserId.ToDebugString());

	if (IsRunningDedicatedServer())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Set user chat configuration is not supported for game server"));
		return false;
	}

	if (!UserId.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Set user chat configuration failed as the user id is invalid"));
		return false;
	}

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelByteSubsystemPtr.Get() is invalid"));
		return false;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteSetUserChatConfiguration>(AccelByteSubsystemPtr.Get(), UserId, Configuration);
	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));

	return true;
}

bool FOnlineChatAccelByte::IsChatAllowed(const FUniqueNetId& UserId, const FUniqueNetId& RecipientId) const
{
	return true;
}

void FOnlineChatAccelByte::GetJoinedRooms(const FUniqueNetId& UserId, TArray<FChatRoomId>& OutRooms)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("UserId: %s"), *UserId.ToDebugString());

	const FUniqueNetIdAccelByteUserRef AccelByteUserId = FUniqueNetIdAccelByteUser::CastChecked(UserId);
	const FString AccelByteId = AccelByteUserId->GetAccelByteId();
	for (const auto& Entry : TopicIdToChatRoomInfoCached)
	{
		if (Entry.Value->HasMember(AccelByteId))
		{
			OutRooms.Add(Entry.Value->GetRoomId());
		}
	}

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Number of rooms: %d"), OutRooms.Num());
}

TSharedPtr<FChatRoomInfo> FOnlineChatAccelByte::GetRoomInfo(const FUniqueNetId& UserId, const FChatRoomId& RoomId)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("UserId: %s, RoomId: %s"), *UserId.ToDebugString(), *RoomId);

	FAccelByteChatRoomInfoRef* RoomInfo = TopicIdToChatRoomInfoCached.Find(RoomId);
	if (RoomInfo == nullptr)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Room with ID %s not found"), *RoomId);
		return nullptr;		
	}

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));

	return *RoomInfo;
}

bool FOnlineChatAccelByte::GetMembers(const FUniqueNetId& UserId, const FChatRoomId& RoomId, TArray<TSharedRef<FChatRoomMember>>& OutMembers)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("UserId: %s, RoomId: %s"), *UserId.ToDebugString(), *RoomId);

	const FAccelByteChatRoomInfoRef* RoomInfo = TopicIdToChatRoomInfoCached.Find(RoomId);
	if (RoomInfo == nullptr)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get members for room with ID %s as the room was not found!"), *RoomId);
		return false;
	}

	const TArray<FString>& Members = (*RoomInfo)->GetMembers();
	for (const auto& MemberAccelByteId : Members)
	{
		FAccelByteChatRoomMemberRef RoomMember = GetAccelByteChatRoomMember(MemberAccelByteId);
		OutMembers.Add(RoomMember);
	}

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Number of members: %d"), OutMembers.Num());

	return true;
}

TSharedPtr<FChatRoomMember> FOnlineChatAccelByte::GetMember(
	const FUniqueNetId& UserId,
	const FChatRoomId& RoomId,
	const FUniqueNetId& MemberId)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("UserId: %s, RoomId: %s, MemberId: %s"), *UserId.ToDebugString(), *RoomId, *MemberId.ToDebugString());

	const FUniqueNetIdAccelByteUserRef AccelByteUserId = FUniqueNetIdAccelByteUser::CastChecked(UserId);
	const FString AccelByteId = AccelByteUserId->GetAccelByteId();

	const FAccelByteChatRoomInfoRef* RoomInfo = TopicIdToChatRoomInfoCached.Find(RoomId);
	if (RoomInfo == nullptr)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get member from room with ID %s as the room was not found!"), *RoomId);
		return nullptr;
	}
	
	if (!(*RoomInfo)->HasMember(AccelByteId))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get member with ID %s from room with ID %s as the user is not a member of the room!"), *MemberId.ToDebugString(), *RoomId);
		return nullptr;
	}

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));

	return GetAccelByteChatRoomMember(AccelByteId);
}

bool FOnlineChatAccelByte::GetLastMessages(
	const FUniqueNetId& UserId,
	const FChatRoomId& RoomId,
	int32 NumMessages,
	TArray<TSharedRef<FChatMessage>>& OutMessages)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("UserId: %s, RoomId: %s, NumMessages: %d"), *UserId.ToDebugString(), *RoomId, NumMessages);

	const FUniqueNetIdAccelByteUserRef AccelByteUserId = FUniqueNetIdAccelByteUser::CastChecked(UserId);	
	FString RoomIdToLoad = RoomId;

	// Check if the room id is the user id, so it will load personal chat message
	auto OptionalRoomIdToLoad = GetPersonalChatTopicId(AccelByteUserId->GetAccelByteId(), RoomId);
	if(OptionalRoomIdToLoad.IsSet())
	{
		RoomIdToLoad = OptionalRoomIdToLoad.GetValue();
	}

	FChatRoomIdToChatMessages* RoomIdToChatMessages = UserIdToChatRoomMessagesCached.Find(AccelByteUserId);
	if (RoomIdToChatMessages == nullptr)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get last messages from room with ID %s as the room was not found!"), *RoomId);
		return false;
	}
	TArray<TSharedRef<FChatMessage>>* Messages = RoomIdToChatMessages->Find(RoomIdToLoad);
	if (Messages == nullptr)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get last messages from room with ID %s as the room has no messages!"), *RoomIdToLoad);
		return false;
	}

	TArray<TSharedRef<FChatMessage>> MsgList = *Messages;

	const int32 TotalMessages = MsgList.Num();
	const int32 LastIndex = FMath::Max(TotalMessages - NumMessages, 0);
	for (int32 i = TotalMessages - 1; i >= LastIndex; i--)
	{
		OutMessages.Add(MsgList[i]);
	}

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Number of messages: %d"), OutMessages.Num());

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
	FReport::LogDeprecated(FString(__FUNCTION__),
		TEXT("PersonalChatTopicId is deprecated - unreliable method to obtain the topic ID and might be different from backend's sorting result. Please use FOnlineChatAccelByte::GetPersonalChatTopicId."));

	TArray<FString> UserIds = { FromUserId, ToUserId };
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
	return GetPersonalChatTopicId(FromUserId, ToUserId).IsSet();
}

TOptional<FString> FOnlineChatAccelByte::GetPersonalChatTopicId(const FString& FromUserId, const FString& ToUserId)
{
	TSet<FString> KeyResult{};
	auto KeyCount = TopicIdToChatRoomInfoCached.GetKeys(KeyResult);
	for (const auto& Key : KeyResult)
	{
		if (Key.Contains(FromUserId, ESearchCase::IgnoreCase) && Key.Contains(ToUserId, ESearchCase::IgnoreCase))
		{
			return Key;
		}
	}

	return {};
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
	TopicIdToChatRoomInfoCached.Emplace(ChatRoomInfo->GetRoomId(), ChatRoomInfo);
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

void FOnlineChatAccelByte::AddChatRoomMembers(TArray<FAccelByteUserInfoRef> Users)
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
	UE_LOG_AB(Verbose, TEXT("Add chat message to userId %s with room ID %s!"), *AccelByteUserId->ToDebugString(), *ChatRoomId);
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
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("PlayerId: %s"), *PlayerId.ToDebugString());

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelByteSubsystemPtr.Get() is invalid"));
		return;
	}

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystemPtr->GetIdentityInterface());
	if (!ensure(IdentityInterface.IsValid()))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to register notifications for session updates as our identity interface is invalid!"));
		return;
	}

	int32 LocalUserNum = 0;
	if (!ensure(IdentityInterface->GetLocalUserNum(PlayerId, LocalUserNum)))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to register notifications for session updates as we could not get a local user index for player with ID '%s'!"), *PlayerId.ToDebugString());
		return;
	}

	AccelByte::FApiClientPtr ApiClient = AccelByteSubsystemPtr->GetApiClient(PlayerId);
	if (!ensure(ApiClient.IsValid()))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to register notifications for session updates as player '%s' has an invalid API client!"), *PlayerId.ToDebugString());
		return;
	}

	const auto Chat = ApiClient->GetChatApi().Pin();
	if (!ensure(Chat.IsValid()))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to register notifications for session updates as player '%s' has an invalid Chat API!"), *PlayerId.ToDebugString());
		return;
	}

	// Begin Chat Notifications
	typedef AccelByte::Api::Chat::FAddRemoveFromTopicNotif FAddRemoveFromTopicNotificationDelegate;
	const FAddRemoveFromTopicNotificationDelegate OnRemoveFromTopicNotificationDelegate = FAddRemoveFromTopicNotificationDelegate::CreateThreadSafeSP(SharedThis(this), &FOnlineChatAccelByte::OnRemoveFromTopicNotification, LocalUserNum);
	Chat->SetRemoveFromTopicNotifDelegate(OnRemoveFromTopicNotificationDelegate);

	const FAddRemoveFromTopicNotificationDelegate OnAddToTopicNotificationDelegate = FAddRemoveFromTopicNotificationDelegate::CreateThreadSafeSP(SharedThis(this), &FOnlineChatAccelByte::OnAddToTopicNotification, LocalUserNum);
	Chat->SetAddToTopicNotifDelegate(OnAddToTopicNotificationDelegate);

	typedef AccelByte::Api::Chat::FChatDisconnectNotif FChatDisconnectNotificationDelegate;
	const FChatDisconnectNotificationDelegate OnChatDisconnectNotificationDelegate = FChatDisconnectNotificationDelegate::CreateThreadSafeSP(SharedThis(this), &FOnlineChatAccelByte::OnChatDisconnectedNotification, LocalUserNum);
	Chat->SetDisconnectNotifDelegate(OnChatDisconnectNotificationDelegate);

	typedef AccelByte::Api::Chat::FChatConnectionClosed FChatConnectionClosedDelegate;
	const FChatConnectionClosedDelegate OnChatConnectionClosedDelegate = FChatConnectionClosedDelegate::CreateThreadSafeSP(SharedThis(this), &FOnlineChatAccelByte::OnChatConnectionClosed, LocalUserNum);
	Chat->SetConnectionClosedDelegate(OnChatConnectionClosedDelegate);

	typedef AccelByte::Api::Chat::FChatNotif FReceivedChatNotificationDelegate;
	const FReceivedChatNotificationDelegate OnReceivedChatNotificationDelegate = FReceivedChatNotificationDelegate::CreateThreadSafeSP(SharedThis(this), &FOnlineChatAccelByte::OnReceivedChatNotification, LocalUserNum);
	Chat->SetChatNotifDelegate(OnReceivedChatNotificationDelegate);

	typedef AccelByte::Api::Chat::FDeleteUpdateTopicNotif FDeleteUpdateTopicNotificationDelegate;
	const FDeleteUpdateTopicNotificationDelegate OnUpdateTopicNotificationDelegate = FDeleteUpdateTopicNotificationDelegate::CreateThreadSafeSP(SharedThis(this), &FOnlineChatAccelByte::OnUpdateTopicNotification, LocalUserNum);
	Chat->SetUpdateTopicNotifDelegate(OnUpdateTopicNotificationDelegate);

	const FDeleteUpdateTopicNotificationDelegate OnDeleteTopicNotificationDelegate = FDeleteUpdateTopicNotificationDelegate::CreateThreadSafeSP(SharedThis(this), &FOnlineChatAccelByte::OnDeleteTopicNotification, LocalUserNum);
	Chat->SetDeleteTopicNotifDelegate(OnDeleteTopicNotificationDelegate);

	typedef AccelByte::Api::Chat::FReadChatNotif FReadChatNotificationDelegate;
	const FReadChatNotificationDelegate OnReadChatNotificationDelegate = FReadChatNotificationDelegate::CreateThreadSafeSP(SharedThis(this), &FOnlineChatAccelByte::OnReadChatNotification, LocalUserNum);
	Chat->SetReadChatNotifDelegate(OnReadChatNotificationDelegate);

	typedef AccelByte::Api::Chat::FUserBanUnbanNotif FUserBanUnbanNotificationDelegate;
	const FUserBanUnbanNotificationDelegate OnUserBanNotificationDelegate = FUserBanUnbanNotificationDelegate::CreateThreadSafeSP(SharedThis(this), &FOnlineChatAccelByte::OnUserBanNotification, LocalUserNum);
	Chat->SetUserBanNotifDelegate(OnUserBanNotificationDelegate);

	const FUserBanUnbanNotificationDelegate OnUserUnBanNotificationDelegate = FUserBanUnbanNotificationDelegate::CreateThreadSafeSP(SharedThis(this), &FOnlineChatAccelByte::OnUserUnbanNotification, LocalUserNum);
	Chat->SetUserUnbanNotifDelegate(OnUserUnBanNotificationDelegate);

	typedef AccelByte::Api::Chat::FSystemMessageNotif FSystemMessageNotificationDelegate;
	const FSystemMessageNotificationDelegate OnSystemMessageDelegate = FSystemMessageNotificationDelegate::CreateThreadSafeSP(SharedThis(this), &FOnlineChatAccelByte::OnSystemMessageNotification, LocalUserNum);
	Chat->SetSystemMessageNotifDelegate(OnSystemMessageDelegate);

	Chat->OnReconnectAttemptedMulticastDelegate().AddThreadSafeSP(SharedThis(this), &FOnlineChatAccelByte::OnChatReconnectAttempted, LocalUserNum);

	Chat->OnMassiveOutageMulticastDelegate().AddThreadSafeSP(SharedThis(this), &FOnlineChatAccelByte::OnChatMassiveOutageEvent, LocalUserNum);
	//~ End Chat Notifications

	// Cache topic data
	FAccelByteModelsChatQueryTopicRequest QueryTopicRequest;
	// TODO: get all topic data if the user has more that 200 topics (should be working for most of cases now)
	QueryTopicRequest.Offset = 0;
	QueryTopicRequest.Limit = 200;
	const FOnChatQueryRoomComplete OnQueryTopicResponse = FOnChatQueryRoomComplete::CreateThreadSafeSP(SharedThis(this), &FOnlineChatAccelByte::OnQueryChatRoomInfoComplete);

	FOnlineAsyncTaskInfo TaskInfo;
	TaskInfo.Type = ETypeOfOnlineAsyncTask::Parallel;
	TaskInfo.bCreateEpicForThis = true;
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTask<FOnlineAsyncTaskAccelByteChatQueryRoom>(TaskInfo, AccelByteSubsystemPtr.Get(), PlayerId, QueryTopicRequest, OnQueryTopicResponse);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineChatAccelByte::OnChatDisconnectedNotification(const FAccelByteModelsChatDisconnectNotif& DisconnectEvent, int32 LocalUserNum)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("Message: %s"), *DisconnectEvent.Message);

	TriggerOnChatDisconnectedDelegates(DisconnectEvent.Message);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineChatAccelByte::OnChatConnectionClosed(int32 StatusCode, const FString& Reason, bool bWasClean, int32 LocalUserNum)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("Chat connection closed: status code %d, reason %s, bWasClean %s"), StatusCode, *Reason, bWasClean?TEXT("true"):TEXT("false"));

	UpdateUserAccount(LocalUserNum);

	SendDisconnectPredefinedEvent(StatusCode, LocalUserNum);

	TriggerOnChatConnectionClosedDelegates(LocalUserNum, StatusCode, Reason, bWasClean);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineChatAccelByte::OnRemoveFromTopicNotification(const FAccelByteModelsChatUpdateUserTopicNotif& RemoveTopicEvent, int32 LocalUserNum)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("TopicId: %s"), *RemoveTopicEvent.TopicId);

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelByteSubsystemPtr.Get() is invalid"));
		return;
	}

	RemoveMemberFromTopic(RemoveTopicEvent.SenderId, RemoveTopicEvent.TopicId);
	TriggerOnTopicRemovedDelegates(RemoveTopicEvent.Name, RemoveTopicEvent.TopicId, RemoveTopicEvent.UserId);
	const FUniqueNetIdPtr LocalUserId = AccelByteSubsystemPtr->GetIdentityInterface()->GetUniquePlayerId(LocalUserNum);
	if (LocalUserId.IsValid())
	{
		FUniqueNetIdPtr SenderUserId = FUniqueNetIdAccelByteUser::Create(FAccelByteUniqueIdComposite(RemoveTopicEvent.SenderId));
		TriggerOnChatRoomMemberExitDelegates(*LocalUserId, RemoveTopicEvent.TopicId, *SenderUserId);
	}
	
	const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = AccelByteSubsystemPtr->GetPredefinedEventInterface();
	if (PredefinedEventInterface.IsValid())
	{
		FAccelByteModelsChatV2TopicUserRemovedPayload TopicUserRemovedPayload{};
		TopicUserRemovedPayload.UserId = RemoveTopicEvent.UserId;
		TopicUserRemovedPayload.TopicId = RemoveTopicEvent.TopicId;
		PredefinedEventInterface->SendEvent(LocalUserNum, MakeShared<FAccelByteModelsChatV2TopicUserRemovedPayload>(TopicUserRemovedPayload));
	}

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineChatAccelByte::OnAddToTopicNotification(const FAccelByteModelsChatUpdateUserTopicNotif& AddTopicEvent, int32 LocalUserNum)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("TopicId: %s"), *AddTopicEvent.TopicId);

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelByteSubsystemPtr.Get() is invalid"));
		return;
	}

	const IOnlineIdentityPtr IdentityInterface = AccelByteSubsystemPtr->GetIdentityInterface();
	if (!ensure(IdentityInterface.IsValid()))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to add to topic notification as our identity interface is invalid!"));
		return;
	}

	const FUniqueNetIdPtr LocalUserId = AccelByteSubsystemPtr->GetIdentityInterface()->GetUniquePlayerId(LocalUserNum);
	if (!ensure(LocalUserId.IsValid()))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to add to topic notification as our LocalUserId is invalid!"));
		return;
	}
	
	// Find the room first, if the room not found fetch the room info
	// If room found, but no member info, fetch user information
	// If room info and member info found, trigger the ChatRoomMemberJoinDelegate
	FUniqueNetIdPtr SenderUserId = FUniqueNetIdAccelByteUser::Create(FAccelByteUniqueIdComposite(AddTopicEvent.SenderId));
	const FAccelByteChatRoomInfoRef* ChatRoomInfo = TopicIdToChatRoomInfoCached.Find(AddTopicEvent.TopicId);
	if (ChatRoomInfo == nullptr)
	{
		UE_LOG_AB(Verbose, TEXT("ChatRoomInfo not found by room ID %s!"), *AddTopicEvent.TopicId);
		// we don't have room data locally

		FAccelByteChatRoomInfoPtr RoomInfo = FAccelByteChatRoomInfo::Create();
		RoomInfo->SetTopicData(AddTopicEvent);
		AddTopic(RoomInfo.ToSharedRef());

		const FOnChatQueryRoomByIdComplete OnQueryTopicResponse = FOnChatQueryRoomByIdComplete::CreateThreadSafeSP(SharedThis(this), &FOnlineChatAccelByte::OnQueryChatRoomById_TriggerChatRoomMemberJoin, LocalUserId, SenderUserId);
		FOnlineAsyncTaskInfo TaskInfo;
		TaskInfo.Type = ETypeOfOnlineAsyncTask::Parallel;
		TaskInfo.bCreateEpicForThis = true;
		AccelByteSubsystemPtr->CreateAndDispatchAsyncTask<FOnlineAsyncTaskAccelByteChatQueryRoomById>(TaskInfo, AccelByteSubsystemPtr.Get(), *LocalUserId, AddTopicEvent.TopicId, OnQueryTopicResponse);
	}
	else
	{
		UE_LOG_AB(Verbose, TEXT("ChatRoomInfo for room ID %s found. Current member num: %d, adding member with ID %s"), *AddTopicEvent.TopicId, (*ChatRoomInfo)->GetMembers().Num(), *AddTopicEvent.SenderId);
		(*ChatRoomInfo)->AddMember(AddTopicEvent.SenderId);
				
		FAccelByteChatRoomMemberRef* MemberPtr = UserIdToChatRoomMemberCached.Find(AddTopicEvent.SenderId);
		// member is not cached get the info
		if (MemberPtr == nullptr || !(*MemberPtr)->HasNickname())
		{
			const FOnlineUserCacheAccelBytePtr UserStore = AccelByteSubsystemPtr->GetUserCache();
			if (!UserStore.IsValid())
			{
				UE_LOG_AB(Warning, TEXT("Unable to get member info as our user store instance is invalid!"));
				TriggerOnChatRoomMemberJoinDelegates(*LocalUserId, AddTopicEvent.TopicId, *SenderUserId);
			}
			else
			{
				const TArray<FString> UserIds = {AddTopicEvent.SenderId};
				const FOnQueryUsersComplete OnQueryUsersCompleteDelegate = FOnQueryUsersComplete::CreateThreadSafeSP(SharedThis(this), &FOnlineChatAccelByte::OnQueryChatMemberInfo_TriggerChatRoomMemberJoin, AddTopicEvent.TopicId, LocalUserId, SenderUserId);
				UserStore->QueryUsersByAccelByteIds(LocalUserNum, UserIds, OnQueryUsersCompleteDelegate);
			}
		}
		else
		{
			TriggerOnChatRoomMemberJoinDelegates(*LocalUserId, AddTopicEvent.TopicId, *SenderUserId);
		}
	}	
	
	TriggerOnTopicAddedDelegates(AddTopicEvent.Name, AddTopicEvent.TopicId, AddTopicEvent.UserId);
	
	const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = AccelByteSubsystemPtr->GetPredefinedEventInterface();
	if (PredefinedEventInterface.IsValid())
	{
		FAccelByteModelsChatV2TopicUserAddedPayload TopicUserAddedPayload{};
		TopicUserAddedPayload.UserId = AddTopicEvent.UserId;
		TopicUserAddedPayload.TopicId = AddTopicEvent.TopicId;
		PredefinedEventInterface->SendEvent(LocalUserNum, MakeShared<FAccelByteModelsChatV2TopicUserAddedPayload>(TopicUserAddedPayload));
	}

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineChatAccelByte::OnReceivedChatNotification(const FAccelByteModelsChatNotif& ChatNotif, int32 LocalUserNum)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("TopicId: %s"), *ChatNotif.TopicId);

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelByteSubsystemPtr.Get() is invalid"));
		return;
	}

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystemPtr->GetIdentityInterface());
	if (!ensure(IdentityInterface.IsValid()))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle received chat notification as our identity interface is invalid!"));
		return;
	}

	const FUniqueNetIdPtr LocalUserId = IdentityInterface->GetUniquePlayerId(LocalUserNum);
	if (!LocalUserId.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle received chat notification as LocalUserId is invalid!"));
		return;
	}

	FAccelByteUniqueIdComposite SenderCompositeId;
	SenderCompositeId.Id = ChatNotif.From;
	FUniqueNetIdAccelByteUserPtr SenderUserId = FUniqueNetIdAccelByteUser::Create(SenderCompositeId);

	FChatRoomId OutChatRoomId = ChatNotif.TopicId;
	const EAccelByteChatRoomType RoomType = GetChatRoomType(ChatNotif.TopicId);

	FAccelByteChatRoomMemberRef Member = GetAccelByteChatRoomMember(ChatNotif.From);
	TSharedRef<FAccelByteChatMessage> OutChatMessage = MakeShared<FAccelByteChatMessage>(SenderUserId.ToSharedRef()
		, Member->GetNickname()
		, ChatNotif.Message
		, ChatNotif.CreatedAt
		, ChatNotif.ChatId
		, ChatNotif.TopicId
		, ChatNotif.SenderPlatformId);

	const FUniqueNetIdAccelByteUserRef AccelByteUserId = FUniqueNetIdAccelByteUser::CastChecked(LocalUserId.ToSharedRef());
	AddChatMessage(AccelByteUserId, OutChatRoomId, OutChatMessage);

	if (RoomType == EAccelByteChatRoomType::PERSONAL)
	{
		TriggerOnChatPrivateMessageReceivedDelegates(*LocalUserId, OutChatMessage);
	}
	else
	{		
		TriggerOnChatRoomMessageReceivedDelegates(*LocalUserId, ChatNotif.TopicId, OutChatMessage);
	}

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineChatAccelByte::OnUpdateTopicNotification(const FAccelByteModelsChatUpdateTopicNotif& UpdateTopicNotif, int32 LocalUserNum)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("TopicId: %s"), *UpdateTopicNotif.TopicId);

	FAccelByteChatRoomInfoRef* RoomInfoPtr = TopicIdToChatRoomInfoCached.Find(UpdateTopicNotif.TopicId);
	if (RoomInfoPtr != nullptr)
	{
		(*RoomInfoPtr)->UpdateTopicData(UpdateTopicNotif);
	}
	
	TriggerOnTopicUpdatedDelegates(UpdateTopicNotif.Name, UpdateTopicNotif.TopicId, UpdateTopicNotif.SenderId, UpdateTopicNotif.IsChannel);

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelByteSubsystemPtr.Get() is invalid"));
		return;
	}

	const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = AccelByteSubsystemPtr->GetPredefinedEventInterface();
	if (PredefinedEventInterface.IsValid())
	{
		FAccelByteModelsChatV2TopicUpdatedPayload TopicUpdatedPayload{};
		TopicUpdatedPayload.UserId = UpdateTopicNotif.SenderId;
		TopicUpdatedPayload.Name = UpdateTopicNotif.Name;
		TopicUpdatedPayload.IsChannel = UpdateTopicNotif.IsChannel;
		PredefinedEventInterface->SendEvent(LocalUserNum, MakeShared<FAccelByteModelsChatV2TopicUpdatedPayload>(TopicUpdatedPayload));
	}

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineChatAccelByte::OnDeleteTopicNotification(const FAccelByteModelsChatUpdateTopicNotif& DeleteTopicNotif, int32 LocalUserNum)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("TopicId: %s"), *DeleteTopicNotif.TopicId);

	RemoveTopic(DeleteTopicNotif.TopicId);
	TriggerOnTopicDeletedDelegates(DeleteTopicNotif.Name, DeleteTopicNotif.TopicId, DeleteTopicNotif.SenderId, DeleteTopicNotif.IsChannel);

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
    if (!AccelByteSubsystemPtr.IsValid())
    {
        AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelByteSubsystemPtr.Get() is invalid"));
        return;
    }

	const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = AccelByteSubsystemPtr->GetPredefinedEventInterface();
	if (PredefinedEventInterface.IsValid())
	{
		FAccelByteModelsChatV2TopicDeletedPayload TopicDeletedPayload{};
		TopicDeletedPayload.UserId = DeleteTopicNotif.SenderId;
		TopicDeletedPayload.TopicId = DeleteTopicNotif.Name;
		PredefinedEventInterface->SendEvent(LocalUserNum, MakeShared<FAccelByteModelsChatV2TopicDeletedPayload>(TopicDeletedPayload));
	}
	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineChatAccelByte::OnReadChatNotification(const FAccelByteModelsReadChatNotif& ReadChatNotif, int32 LocalUserNum)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));

	TriggerOnReadChatReceivedDelegates(ReadChatNotif.ReadChat);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineChatAccelByte::OnUserBanNotification(const FAccelByteModelsChatUserBanUnbanNotif& UserBanNotif, int32 LocalUserNum)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));

	TriggerOnUserBannedDelegates(UserBanNotif.UserId, UserBanNotif.Ban, UserBanNotif.EndDate, UserBanNotif.Reason, UserBanNotif.Enable);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineChatAccelByte::OnUserUnbanNotification(const FAccelByteModelsChatUserBanUnbanNotif& UserUnbanNotif, int32 LocalUserNum)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));

	TriggerOnUserUnbannedDelegates(UserUnbanNotif.UserId, UserUnbanNotif.Ban, UserUnbanNotif.EndDate, UserUnbanNotif.Reason, UserUnbanNotif.Enable);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineChatAccelByte::OnSystemMessageNotification(const FAccelByteModelsChatSystemMessageNotif& SystemMessageNotif, int32 LocalUserNum)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT(""));

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
    if (!AccelByteSubsystemPtr.IsValid())
    {
        AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelByteSubsystemPtr.Get() is invalid"));
        return;
    }

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystemPtr->GetIdentityInterface());
	if (!ensure(IdentityInterface.IsValid()))
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle received system message notification as our identity interface is invalid!"));
		return;
	}

	const FUniqueNetIdPtr LocalUserId = IdentityInterface->GetUniquePlayerId(LocalUserNum);
	if (!LocalUserId.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed to handle received system message notification as LocalUserId is invalid!"));
		return;
	}

	if (SystemMessageNotif.Category.IsEmpty()) // Empty category means this is regular system message
	{
		FSystemMessageNotifMessage Message;
		SystemMessageNotif.GetSystemMessageData(Message);
		TriggerOnSystemMessageReceivedDelegates(*LocalUserId, Message);
	}
	else // this is a transient system message
	{
		TriggerOnTransientSystemMessageReceivedDelegates(*LocalUserId, SystemMessageNotif);
	}

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineChatAccelByte::OnChatReconnectAttempted(const FReconnectAttemptInfo& Info, int32 InLocalUserNum)
{
	TriggerAccelByteOnChatReconnectAttemptedDelegates(InLocalUserNum, Info);
}

void FOnlineChatAccelByte::OnChatMassiveOutageEvent(const FMassiveOutageInfo& Info, int32 InLocalUserNum)
{
	TriggerAccelByteOnChatMassiveOutageEventDelegates(InLocalUserNum, Info);
}

void FOnlineChatAccelByte::OnQueryChatRoomInfoComplete(bool bWasSuccessful, TArray<FAccelByteChatRoomInfoRef> RoomList, int32 LocalUserNum)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("Room length %d"), RoomList.Num());

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineChatAccelByte::OnQueryChatMemberInfo_TriggerChatRoomMemberJoin(bool bIsSuccessful, TArray<FAccelByteUserInfoRef> UsersQueried, FString RoomId, FUniqueNetIdPtr InUserId, FUniqueNetIdPtr InMemberId)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("RoomId %s MemberId %s"), *RoomId, *InMemberId->ToDebugString());

	if(!bIsSuccessful)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Query chat member info on ChatRoomMemberJoin failed"));
		return;
	}
	
	TriggerOnChatRoomMemberJoinDelegates(*InUserId, RoomId, *InMemberId);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineChatAccelByte::OnQueryChatRoomById_TriggerChatRoomMemberJoin(bool bWasSuccessful, FAccelByteChatRoomInfoPtr RoomInfo, int32 LocalUserNum, FUniqueNetIdPtr InUserId, FUniqueNetIdPtr InMemberId)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("MemberId %s"), *InMemberId->ToDebugString());
	
	if(!bWasSuccessful)
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Query room info on ChatRoomMemberJoin failed"));
		return;
	}
	
	TriggerOnChatRoomMemberJoinDelegates(*InUserId, RoomInfo->GetRoomId(), *InMemberId);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
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

const FString& FAccelByteChatMessage::GetChatId() const
{
	return ChatId;
}

const FString& FAccelByteChatMessage::GetTopicId() const
{
	return TopicId;
}

EAccelBytePlatformType FAccelByteChatMessage::GetSenderPlatformId() const
{
	return SenderPlatformId;
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

void FAccelByteChatRoomInfo::SetTopicData(const FAccelByteModelsChatUpdateUserTopicNotif& UserTopicData)
{
	FAccelByteModelsChatTopicQueryData ChatTopicData{};

	ChatTopicData.TopicId = UserTopicData.TopicId;
	ChatTopicData.Name = UserTopicData.Name;
	ChatTopicData.Type = UserTopicData.Type;
	ChatTopicData.Members.AddUnique(UserTopicData.SenderId);
	ChatTopicData.Members.AddUnique(UserTopicData.UserId);

	SetTopicData(ChatTopicData);
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

bool FOnlineChatAccelByte::UpdateUserAccount(const int32 LocalUserNum)
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelByteSubsystemPtr.Get() is invalid"));
		return false;
	}
	
	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystemPtr->GetIdentityInterface());
	if (!IdentityInterface.IsValid())
	{
		UE_LOG_AB(Warning, TEXT("Failed updating user account due to invalid IdentityInterface"));
		return false;
	}

	const FUniqueNetIdPtr LocalUserId = IdentityInterface->GetUniquePlayerId(LocalUserNum);
	if (!LocalUserId.IsValid())
	{
		UE_LOG_AB(Warning, TEXT("Failed updating user account due to invalidLocal User"));
		return false;
	}

	TSharedPtr<FUserOnlineAccount> UserAccount = IdentityInterface->GetUserAccount(LocalUserId);
	if (!UserAccount.IsValid())
	{
		UE_LOG_AB(Warning, TEXT("Failed updating user account due to invalid User Account"));
		return false;
	}

	const TSharedPtr<FUserOnlineAccountAccelByte> UserAccountAccelByte = StaticCastSharedPtr<FUserOnlineAccountAccelByte>(UserAccount);
	UserAccountAccelByte->SetConnectedToChat(false);
	return true;
}

void FOnlineChatAccelByte::SendDisconnectPredefinedEvent(int32 StatusCode, int32 LocalUserNum)
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelByteSubsystemPtr.Get() is invalid"));
		return;
	}
	
	const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = AccelByteSubsystemPtr->GetPredefinedEventInterface();
	if (PredefinedEventInterface.IsValid())
	{
		const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystemPtr->GetIdentityInterface());
		if (!IdentityInterface.IsValid())
		{
			UE_LOG_AB(Warning, TEXT("Failed sending disconnect predefined event due to invalid IdentityInterface"));
			return;
		}

		const FUniqueNetIdPtr LocalUserId = IdentityInterface->GetUniquePlayerId(LocalUserNum);
		if (!LocalUserId.IsValid())
		{
			UE_LOG_AB(Warning, TEXT("Failed  sending disconnect predefined event due to invalidLocal User"));
			return;
		}

		const FUniqueNetIdAccelByteUserPtr AccelByteUserId = FUniqueNetIdAccelByteUser::TryCast(LocalUserId.ToSharedRef());
		if (AccelByteUserId.IsValid())
		{
			FAccelByteModelsChatV2DisconnectedPayload DisconnectedPayload{};
			DisconnectedPayload.UserId = AccelByteUserId->GetAccelByteId();
			DisconnectedPayload.StatusCode = StatusCode;
			PredefinedEventInterface->SendEvent(LocalUserNum, MakeShared<FAccelByteModelsChatV2DisconnectedPayload>(DisconnectedPayload));
		}
	}
}

bool FOnlineChatAccelByte::ReportChatMessage(const FUniqueNetId& UserId
	, const TSharedRef<FChatMessage>& Message
	, const FString& Reason
	, const FString& Comment
	, FOnReportChatMessageComplete CompletionDelegate)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("UserId: %s; Reason: %s; Comment: %s"), *UserId.ToDebugString(), *Reason, *Comment);

	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelByteSubsystemPtr.Get() is invalid"));
		return false;
	}

	TSharedRef<FAccelByteChatMessage> AbChatMessage = StaticCastSharedRef<FAccelByteChatMessage>(Message);

	FOnlineAsyncTaskInfo TaskInfo{};
	TaskInfo.Type = ETypeOfOnlineAsyncTask::Parallel;
	TaskInfo.bCreateEpicForThis = true;
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTask<FOnlineAsyncTaskAccelByteChatReportMessage>(TaskInfo
		, AccelByteSubsystemPtr.Get()
		, UserId
		, AbChatMessage
		, Reason
		, Comment
		, CompletionDelegate);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
	return true;
}
