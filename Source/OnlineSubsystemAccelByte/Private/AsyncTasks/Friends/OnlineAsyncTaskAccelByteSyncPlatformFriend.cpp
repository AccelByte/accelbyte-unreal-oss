// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteSyncPlatformFriend.h"
#include "OnlineFriendsInterfaceAccelByte.h"
#include "Core/Platform/AccelBytePlatformHandler.h"

#define ONLINE_ERROR_NAMESPACE "FOnlineAsyncTaskAccelByteSyncPlatformFriend"

FOnlineAsyncTaskAccelByteSyncPlatformFriend::FOnlineAsyncTaskAccelByteSyncPlatformFriend(FOnlineSubsystemAccelByte* const InABInterface
	, const FUniqueNetId& InLocalUserNetId
	, const EAccelBytePlatformType InNativePlatform)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, NativePlatform(InNativePlatform)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InLocalUserNetId);
	OnlineError = FOnlineError();
}

void FOnlineAsyncTaskAccelByteSyncPlatformFriend::Initialize()
{
	FOnlineAsyncTaskAccelByte::Initialize();
	
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserID: %s, LocalUserNum: %d"), *UserId->ToDebugString(), LocalUserNum);

	if (!QueryPlatformFriendList())
	{
		ErrorText = FText::FromString(TEXT("sync-other-platform-friend-failed-query-platform-friend-list"));
		OnlineError = ONLINE_ERROR(EOnlineErrorResult::RequestFailure, FString(), ErrorText);
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Failed reading platform friend list"));
		return;
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteSyncPlatformFriend::Finalize()
{
	FOnlineAsyncTaskAccelByte::Finalize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s, UserID: %s, LocalUserNum: %d"),LOG_BOOL_FORMAT(bWasSuccessful), *UserId->ToDebugString(), LocalUserNum);

	if (!bWasSuccessful)
	{
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Sync platform friend failed, skip adding friend to the friend interface."));
		return;
	}

	TRY_PIN_SUBSYSTEM();
	const TSharedPtr<FOnlineFriendsAccelByte, ESPMode::ThreadSafe> FriendInterface = StaticCastSharedPtr<FOnlineFriendsAccelByte>(SubsystemPin->GetFriendsInterface());

	if (!FriendInterface.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Can't add friends to list as our friend interface is invalid."));
		return;
	}

	FriendInterface->AddFriendsToList(LocalUserNum, SyncedFriends);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteSyncPlatformFriend::TriggerDelegates()
{
	FOnlineAsyncTaskAccelByte::TriggerDelegates();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserID: %s, LocalUserNum: %d"), *UserId->ToDebugString(), LocalUserNum);

	TRY_PIN_SUBSYSTEM();

	const TSharedPtr<FOnlineFriendsAccelByte, ESPMode::ThreadSafe> FriendInterface = StaticCastSharedPtr<FOnlineFriendsAccelByte>(SubsystemPin->GetFriendsInterface());

	if (!FriendInterface.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Can't trigger delegate as our friend interface is invalid."));
		return;
	}

	FriendInterface->TriggerOnSyncThirdPartyPlatformFriendsCompleteDelegates(LocalUserNum, OnlineError);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

bool FOnlineAsyncTaskAccelByteSyncPlatformFriend::QueryPlatformFriendList()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserID: %s, LocalUserNum: %d"), *UserId->ToDebugString(), LocalUserNum);

	AccelByte::FAccelBytePlatformHandler PlatformHandler;
	const AccelByte::AccelBytePlatformWrapperWPtr PlatformWrapperWPtr = PlatformHandler.GetPlatformWrapper(NativePlatform);
	const AccelByte::AccelBytePlatformWrapperPtr PlatformWrapper = PlatformWrapperWPtr.Pin();

	if (!PlatformWrapper.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Error, TEXT("Can't fetch platform friends as platform wrapper is invalid."));
		return false;
	}

	const auto OnSuccess = AccelByte::TDelegateUtils<TDelegate<void(const TArray<AccelByte::FPlatformUser>&)>>::CreateThreadSafeSelfPtr(this
		, &FOnlineAsyncTaskAccelByteSyncPlatformFriend::OnQueryPlatformFriendListSuccess);
	const auto OnError = AccelByte::TDelegateUtils<TDelegate<void(const AccelByte::FPlatformHandlerError&)>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteSyncPlatformFriend::OnQueryPlatformFriendListError);
	
	PlatformWrapper->FetchPlatformFriends(OnSuccess, OnError);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
	return true;
}

void FOnlineAsyncTaskAccelByteSyncPlatformFriend::OnQueryPlatformFriendListSuccess(const TArray<AccelByte::FPlatformUser>& PlatformFriends)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserID: %s, LocalUserNum: %d, Found %d friends"), *UserId->ToDebugString(), LocalUserNum, PlatformFriends.Num());

	TArray<FString> OtherPlatformUserIds;
	for (const auto& PlatformFriend : PlatformFriends)
	{
		OtherPlatformUserIds.Emplace(PlatformFriend.ID);
	}

	FAccelByteUtilities::SplitArraysToNum(OtherPlatformUserIds, 100, SplitUserIds);

	BulkGetUserByOtherPlatformUserIds(SplitUserIds[LastSplitQueryIndex.GetValue()]);
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteSyncPlatformFriend::OnQueryPlatformFriendListError(const AccelByte::FPlatformHandlerError& Response)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN_VERBOSITY(Warning, TEXT("UserID: %s, LocalUserNum: %d"), *UserId->ToDebugString(), LocalUserNum);

	OnlineError.bSucceeded = false;
	OnlineError.ErrorCode = Response.ErrorCode;
	OnlineError.ErrorMessage = FText::FromString(Response.ErrorMessage);
	OnlineError.ErrorRaw = Response.ErrorMessage;

	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

	AB_OSS_ASYNC_TASK_TRACE_BEGIN_VERBOSITY(Warning, TEXT(""));
}

void FOnlineAsyncTaskAccelByteSyncPlatformFriend::BulkGetUserByOtherPlatformUserIds(const TArray<FString>& InUserIds)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserID: %s, LocalUserNum: %d"), *UserId->ToDebugString(), LocalUserNum);

	const THandler<FBulkPlatformUserIdResponse> OnBulkGetUserSuccess = AccelByte::TDelegateUtils<THandler<FBulkPlatformUserIdResponse>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteSyncPlatformFriend::OnBulkGetUserByOtherPlatformUserIdsSuccess);
	const FErrorHandler OnBulkGetUserError = AccelByte::TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteSyncPlatformFriend::OnBulkGetUserByOtherPlatformUserIdsError);
	API_FULL_CHECK_GUARD(User, OnlineError);
	User->BulkGetUserByOtherPlatformUserIdsV4(NativePlatform, InUserIds, OnBulkGetUserSuccess, OnBulkGetUserError, false, EAccelBytePidType::OCULUS_APP_USER_ID);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteSyncPlatformFriend::OnBulkGetUserByOtherPlatformUserIdsSuccess(const FBulkPlatformUserIdResponse& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserID: %s, LocalUserNum: %d"), *UserId->ToDebugString(), LocalUserNum);
	
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

	TRY_PIN_SUBSYSTEM();

	const FOnlineUserCacheAccelBytePtr UserStore = SubsystemPin->GetUserCache();
	if (!UserStore.IsValid())
	{
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Could not query synced friends as our user store instance is invalid!"));
		return;
	}

	const FOnQueryUsersComplete OnQuerySyncedFriendCompleteDelegate = AccelByte::TDelegateUtils<FOnQueryUsersComplete>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteSyncPlatformFriend::OnQuerySyncedFriendComplete);
	UserStore->QueryUsersByAccelByteIds(LocalUserNum, UserIds, OnQuerySyncedFriendCompleteDelegate, true);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteSyncPlatformFriend::OnBulkGetUserByOtherPlatformUserIdsError(int32 ErrorCode,
	const FString& ErrorMessage)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserID: %s, LocalUserNum: %d"), *UserId->ToDebugString(), LocalUserNum);

	ErrorText = FText::FromString(TEXT("sync-other-platform-friend-failed-cannot-retrieve-accelbyte-ids"));
	OnlineError = ONLINE_ERROR(EOnlineErrorResult::RequestFailure, FString::FromInt(ErrorCode), ErrorText);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Failed to get accelbyte user ids using 3rd party user ids, ErrorCode: %d, ErrorMessage: %s"), ErrorCode, *ErrorMessage);
}

void FOnlineAsyncTaskAccelByteSyncPlatformFriend::OnQuerySyncedFriendComplete(const bool bIsSuccessful, TArray<FAccelByteUserInfoRef> UsersQueried)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserID: %s, LocalUserNum: %d"), *UserId->ToDebugString(), LocalUserNum);

	TRY_PIN_SUBSYSTEM();

	if (!bIsSuccessful)
	{
		ErrorText = FText::FromString(TEXT("sync-other-platform-friend-failed-get-user-info"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed retrieving friend info"));
		return;
	}
	
	FAccelByteModelsBulkFriendsRequest BulkFriendsRequest;

	for (const auto& User : UsersQueried)
	{
		if (SubsystemPin->GetFriendsInterface()->IsFriend(LocalUserNum, *User->Id, TEXT("")))
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
	const FVoidHandler OnBulkFriendRequestSuccessHandler = AccelByte::TDelegateUtils<FVoidHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteSyncPlatformFriend::OnBulkFriendRequestSuccess);
	const FErrorHandler OnBulkFriendRequestErrorHandler = AccelByte::TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteSyncPlatformFriend::OnBulkFriendRequestError);
	API_FULL_CHECK_GUARD(Lobby, OnlineError);
	Lobby->BulkFriendRequest(BulkFriendsRequest, OnBulkFriendRequestSuccessHandler, OnBulkFriendRequestErrorHandler);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteSyncPlatformFriend::OnBulkFriendRequestSuccess()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserID: %s, LocalUserNum: %d"), *UserId->ToDebugString(), LocalUserNum);
	OnlineError = ONLINE_ERROR(EOnlineErrorResult::Success, FString(), FText());
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteSyncPlatformFriend::OnBulkFriendRequestError(int32 ErrorCode, const FString& ErrorMessage)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserID: %s, LocalUserNum: %d"), *UserId->ToDebugString(), LocalUserNum);
	ErrorText = FText::FromString(TEXT("sync-other-platform-friend-failed-adding-friends"));
	OnlineError = ONLINE_ERROR(EOnlineErrorResult::RequestFailure, FString::FromInt(ErrorCode), ErrorText);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Failed to sync third party friend, ErrorCode: %d, ErrorMessage: %s"), ErrorCode, *ErrorMessage);
}

#undef ONLINE_ERROR_NAMESPACE
