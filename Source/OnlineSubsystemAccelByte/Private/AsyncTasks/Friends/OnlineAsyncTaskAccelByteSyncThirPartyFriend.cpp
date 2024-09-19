// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteSyncThirPartyFriend.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineAsyncTaskAccelByteSyncThirPartyFriend"

FOnlineAsyncTaskAccelByteSyncThirPartyFriend::FOnlineAsyncTaskAccelByteSyncThirPartyFriend(FOnlineSubsystemAccelByte* const InABInterface, int32 InLocalUserNum, const FString& InNativeFriendListName, const FString& InAccelByteFriendListName)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, NativeFriendListName(InNativeFriendListName)
	, AccelByteFriendListName(InAccelByteFriendListName)
{
	LocalUserNum = InLocalUserNum;

	auto SubsystemPin = AccelByteSubsystem.Pin();
	if (!SubsystemPin.IsValid()) {
		return;
	}

	NativeSubSystem = SubsystemPin->GetNativePlatformSubsystem();
	OnlineError = FOnlineError();
}

void FOnlineAsyncTaskAccelByteSyncThirPartyFriend::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("LocalUserNum: %d"), LocalUserNum);

	if (!QueryNativeFriendList())
	{
		ErrorText = FText::FromString(TEXT("sync-other-platform-friend-failed-query-native-friend-list"));
		OnlineError = ONLINE_ERROR(EOnlineErrorResult::RequestFailure, FString(), ErrorText);
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Failed reading native friend list"));
		return;
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteSyncThirPartyFriend::Finalize()
{
	TRY_PIN_SUBSYSTEM()

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful)
	{
		const TSharedPtr<FOnlineFriendsAccelByte, ESPMode::ThreadSafe> FriendInterface = StaticCastSharedPtr<FOnlineFriendsAccelByte>(SubsystemPin->GetFriendsInterface());
		FriendInterface->AddFriendsToList(LocalUserNum, SyncedFriends);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteSyncThirPartyFriend::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM()

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s, ErrorMessage: %s"), LOG_BOOL_FORMAT(bWasSuccessful), *ErrorText.ToString());

	const TSharedPtr<FOnlineFriendsAccelByte, ESPMode::ThreadSafe> FriendInterface = StaticCastSharedPtr<FOnlineFriendsAccelByte>(SubsystemPin->GetFriendsInterface());

	FriendInterface->TriggerOnSyncThirdPartyPlatformFriendsCompleteDelegates(LocalUserNum, OnlineError);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

bool FOnlineAsyncTaskAccelByteSyncThirPartyFriend::QueryNativeFriendList()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("LocalUserNum: %d"), LocalUserNum);

	if (NativeSubSystem == nullptr)
	{
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Native subsystem is null, can't sync third party friend"));
		return false;
	}

	const IOnlineFriendsPtr NativeFriendInterface = NativeSubSystem->GetFriendsInterface();
	if (!NativeFriendInterface.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Native friend interface is invalid, can't sync third party friend"));
		return false;
	}

	Super::ExecuteCriticalSectionAction(FVoidHandler::CreateLambda([&]()
	{
		NativeFriendInterface->ReadFriendsList(LocalUserNum, NativeFriendListName, FOnReadFriendsListComplete::CreateLambda([this](int32 InLocalUserNum, bool bInWasSuccessful, const FString& InListName, const FString& ErrorMessage)
		{
			OnReadNativeFriendListComplete(InLocalUserNum, bInWasSuccessful, InListName, ErrorMessage);
		}));
	}));

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));

	return true;
}

void FOnlineAsyncTaskAccelByteSyncThirPartyFriend::OnReadNativeFriendListComplete(int32 InLocalUserNum, bool bInWasSuccessful, const FString& InListName, const FString& ErrorMessage)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("LocalUserNum: %d"), LocalUserNum);

	if (InLocalUserNum != LocalUserNum)
	{
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Native friend list is not from current user"));
		return;
	}

	if (bInWasSuccessful)
	{
		const IOnlineFriendsPtr NativeFriendInterface = NativeSubSystem->GetFriendsInterface();

		TArray<TSharedRef<FOnlineFriend>> OutNativeFriends;
		NativeFriendInterface->GetFriendsList(LocalUserNum, InListName, OutNativeFriends);

		if (OutNativeFriends.Num() <= 0)
		{
			OnlineError = ONLINE_ERROR(EOnlineErrorResult::Success, FString(), ErrorText);
			CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
			AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Native friend list is empty"));
			return;
		}

		TArray<FString> OtherPlatformUserIds;
		for (const auto& NativeFriend : OutNativeFriends)
		{
			OtherPlatformUserIds.Add(NativeFriend->GetUserId()->ToString());
		}

		FAccelByteUtilities::SplitArraysToNum(OtherPlatformUserIds, 100, SplitUserIds);

		BulkGetUserByOtherPlatformUserIds(SplitUserIds[LastSplitQueryIndex.GetValue()]);
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
	}
	else
	{
		ErrorText = FText::FromString(TEXT("sync-other-platform-friend-failed-query-native-friend-list"));
		OnlineError = ONLINE_ERROR(EOnlineErrorResult::RequestFailure, FString(), ErrorText);
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Failed to query native friend list, ListName: %s, ErrorMessage: %s"), *InListName, *ErrorMessage);
	}

}

void FOnlineAsyncTaskAccelByteSyncThirPartyFriend::OnBulkGetUserByOtherPlatformUserIdsSuccess(const FBulkPlatformUserIdResponse& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("LocalUserNum: %d"), LocalUserNum);
	
	QueriedUserMapByPlatformUserIds.Append(Result.UserIdPlatforms);
	LastSplitQueryIndex.Increment();
	SetLastUpdateTimeToCurrentTime();
	
	if(LastSplitQueryIndex.GetValue() < SplitUserIds.Num())
	{
		BulkGetUserByOtherPlatformUserIds(SplitUserIds[LastSplitQueryIndex.GetValue()]);
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Querying next platform user Ids"));
		return;
	}

	if (QueriedUserMapByPlatformUserIds.Num() <= 0)
	{
		OnlineError = ONLINE_ERROR(EOnlineErrorResult::Success, FString(), ErrorText);
		CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Native friend is not linked to AccelByte service"));
		return;
	}

	TArray<FString> UserIds;
	for (const auto& UserData : QueriedUserMapByPlatformUserIds)
	{
		UserIds.Add(UserData.UserId);
	}

	TRY_PIN_SUBSYSTEM()

	const FOnlineUserCacheAccelBytePtr UserStore = SubsystemPin->GetUserCache();
	if (!UserStore.IsValid())
	{
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Could not query synced friends as our user store instance is invalid!"));
		return;
	}

	const FOnQueryUsersComplete OnQuerySyncedFriendCompleteDelegate = TDelegateUtils<FOnQueryUsersComplete>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteSyncThirPartyFriend::OnQuerySyncedFriendComplete);
	UserStore->QueryUsersByAccelByteIds(LocalUserNum, UserIds, OnQuerySyncedFriendCompleteDelegate, true);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteSyncThirPartyFriend::OnBulkGetUserByOtherPlatformUserIdsError(int32 ErrorCode, const FString& ErrorMessage)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("LocalUserNum: %d"), LocalUserNum);
	ErrorText = FText::FromString(TEXT("sync-other-platform-friend-failed-cannot-retrieve-accelbyte-ids"));
	OnlineError = ONLINE_ERROR(EOnlineErrorResult::RequestFailure, FString::FromInt(ErrorCode), ErrorText);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Failed to get accelbyte user ids using 3rd party user ids, ErrorCode: %d, ErrorMessage: %s"), ErrorCode, *ErrorMessage);
}

void FOnlineAsyncTaskAccelByteSyncThirPartyFriend::OnQuerySyncedFriendComplete(bool bIsSuccessful,	TArray<FAccelByteUserInfoRef> UsersQueried)
{
	TRY_PIN_SUBSYSTEM()

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("LocalUserNum: %d"), LocalUserNum);

	if (bIsSuccessful)
	{
		FAccelByteModelsBulkFriendsRequest BulkFriendsRequest;

		for (const auto& User : UsersQueried)
		{
			if (SubsystemPin->GetFriendsInterface()->IsFriend(LocalUserNum, *User->Id, AccelByteFriendListName))
			{
				continue;
			}

			SyncedFriends.Add(MakeShared<FOnlineFriendAccelByte>(User, EInviteStatus::Accepted));
			BulkFriendsRequest.FriendIds.Add(User->Id->GetAccelByteId());
		}

		// if all third party friends already synced, complete the task
		if (BulkFriendsRequest.FriendIds.Num() <= 0)
		{
			OnlineError = ONLINE_ERROR(EOnlineErrorResult::Success, FString(), FText());
			CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
			AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Native friend already synced"));
			return;
		}

		// Send the request to sync third party platform friend
		const FVoidHandler OnBulkFriendRequestSuccessHandler = TDelegateUtils<FVoidHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteSyncThirPartyFriend::OnBulkFriendRequestSuccess);
		const FErrorHandler OnBulkFriendRequestErrorHandler = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteSyncThirPartyFriend::OnBulkFriendRequestError);
		API_CLIENT_CHECK_GUARD(OnlineError);
		ApiClient->Lobby.BulkFriendRequest(BulkFriendsRequest, OnBulkFriendRequestSuccessHandler, OnBulkFriendRequestErrorHandler);
	}
	else
	{
		ErrorText = FText::FromString(TEXT("sync-other-platform-friend-failed-get-user-info"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
	}
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteSyncThirPartyFriend::OnBulkFriendRequestSuccess()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("LocalUserNum: %d"), LocalUserNum);
	OnlineError = ONLINE_ERROR(EOnlineErrorResult::Success, FString(), FText());
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteSyncThirPartyFriend::OnBulkFriendRequestError(int32 ErrorCode, const FString& ErrorMessage)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("LocalUserNum: %d"), LocalUserNum);
	ErrorText = FText::FromString(TEXT("sync-other-platform-friend-failed-adding-friends"));
	OnlineError = ONLINE_ERROR(EOnlineErrorResult::RequestFailure, FString::FromInt(ErrorCode), ErrorText);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Failed to sync third party friend, ErrorCode: %d, ErrorMessage: %s"), ErrorCode, *ErrorMessage);
}

void FOnlineAsyncTaskAccelByteSyncThirPartyFriend::BulkGetUserByOtherPlatformUserIds(const TArray<FString>& InUserIds)
{
	const EAccelBytePlatformType PlatformType = FOnlineSubsystemAccelByteUtils::GetCurrentAccelBytePlatformType(NativeSubSystem->GetSubsystemName());
	const THandler<FBulkPlatformUserIdResponse> OnBulkGetUserSuccess = TDelegateUtils<THandler<FBulkPlatformUserIdResponse>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteSyncThirPartyFriend::OnBulkGetUserByOtherPlatformUserIdsSuccess);
	const FErrorHandler OnBulkGetUserError = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteSyncThirPartyFriend::OnBulkGetUserByOtherPlatformUserIdsError);
	API_CLIENT_CHECK_GUARD(OnlineError);
	ApiClient->User.BulkGetUserByOtherPlatformUserIdsV4(PlatformType, InUserIds, OnBulkGetUserSuccess, OnBulkGetUserError);
}

#undef ONLINE_ERROR_NAMESPACE
