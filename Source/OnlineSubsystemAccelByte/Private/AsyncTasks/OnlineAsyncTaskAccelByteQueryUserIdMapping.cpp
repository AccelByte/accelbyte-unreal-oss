// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteQueryUserIdMapping.h"

#include "Core/AccelByteRegistry.h"
#include "Core/AccelByteError.h"
#include "Api/AccelByteUserApi.h"

FOnlineAsyncTaskAccelByteQueryUserIdMapping::FOnlineAsyncTaskAccelByteQueryUserIdMapping
	( FOnlineSubsystemAccelByte* const InABSubsystem
	, const FUniqueNetId& InUserId
	, const FString& InDisplayNameOrEmail
	, const IOnlineUser::FOnQueryUserMappingComplete& InDelegate )
	: FOnlineAsyncTaskAccelByte(InABSubsystem)
	, DisplayNameOrEmail(InDisplayNameOrEmail)
	, Delegate(InDelegate)
{
	UserId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(InUserId.AsShared());
}

void FOnlineAsyncTaskAccelByteQueryUserIdMapping::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	// First, check if the display name or email we wish to query an ID for is blank, if so we want to bail out
	if (DisplayNameOrEmail.IsEmpty())
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Cannot query user ID mapping as the display name or email passed in is blank!"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	THandler<FPagedPublicUsersInfo> OnSuccess = THandler<FPagedPublicUsersInfo>::CreateRaw(this, &FOnlineAsyncTaskAccelByteQueryUserIdMapping::OnSearchUserSuccessResponse);
	FErrorHandler OnError = FErrorHandler::CreateRaw(this, &FOnlineAsyncTaskAccelByteQueryUserIdMapping::OnSearchUserErrorResponse);
	ApiClient->User.SearchUsers(DisplayNameOrEmail, EAccelByteSearchType::ALL, OnSuccess, OnError);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryUserIdMapping::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s; Is Delegate Bound: %s; bWasSuccessful: %s"), *UserId->ToString(), LOG_BOOL_FORMAT(Delegate.IsBound()), LOG_BOOL_FORMAT(bWasSuccessful));

	Delegate.ExecuteIfBound(bWasSuccessful, UserId.ToSharedRef().Get(), DisplayNameOrEmail, FoundUserId.ToSharedRef().Get(), ErrorString);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryUserIdMapping::OnSearchUserSuccessResponse(const FPagedPublicUsersInfo& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Result Amount: %d; UserId: %s"), Result.Data.Num(), *UserId->ToString());

	if (Result.Data.Num() < 1)
	{
		ErrorString = FString::Printf(TEXT("Query for '%s' had no results."), *DisplayNameOrEmail);
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Error, TEXT("%s"), *ErrorString);
		return;
	}

	FPublicUserInfo FoundUser;
	for (const FPublicUserInfo& User : Result.Data)
	{
		if (User.DisplayName == DisplayNameOrEmail)
		{
			FoundUser = User;
			break;
		}
	}

	if (FoundUser.UserId.IsEmpty())
	{
		ErrorString = FString::Printf(TEXT("Could not find user with '%s' for a display name or email."), *DisplayNameOrEmail);
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Error, TEXT("%s"), *ErrorString);
		return;
	}

	FAccelByteUniqueIdComposite CompositeId;
	CompositeId.Id = FoundUser.UserId;
	FoundUserId = FUniqueNetIdAccelByteUser::Create(CompositeId);
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Found user for query '%s'! ID of user is '%s'"), *DisplayNameOrEmail, *(FoundUserId->ToString()));
}

void FOnlineAsyncTaskAccelByteQueryUserIdMapping::OnSearchUserErrorResponse(int32 Code, const FString& Message)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN_VERBOSITY(Error, TEXT("Code: %d; Message: %s"), Code, *Message);

	ErrorString = Message;
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}
