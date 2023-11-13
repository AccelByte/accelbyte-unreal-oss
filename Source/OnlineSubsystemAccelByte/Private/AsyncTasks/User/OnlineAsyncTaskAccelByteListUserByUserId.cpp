// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteListUserByUserId.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineError.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "Core/AccelByteError.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineUserSystemAccelByte"

FOnlineAsyncTaskAccelByteListUserByUserId::FOnlineAsyncTaskAccelByteListUserByUserId(FOnlineSubsystemAccelByte* const InABInterface, const int32 InLocalUserNum,
	const TArray<FString>& InUserIds)
	: FOnlineAsyncTaskAccelByte(InABInterface, true)
	, LocalUserNum(InLocalUserNum)
	, UserIds(InUserIds) 
{
	ListUserDataResult = FListUserDataResponse{};
	OnlineError = FOnlineError();
} 
void FOnlineAsyncTaskAccelByteListUserByUserId::Initialize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	Super::Initialize();

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(Subsystem->GetIdentityInterface());
	if (!IdentityInterface.IsValid())
	{
		FString ErrorCode = FString::Printf(TEXT("%d"), ErrorCodes::InvalidRequest);
		FText ErrorMessage = FText::FromString(TEXT("request-failed-list-users-by-user-id-error-invalid-identity-interface"));
		OnlineError =  ONLINE_ERROR(EOnlineErrorResult::InvalidCreds, ErrorCode, ErrorMessage); 
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to list users by user id, identity interface is invalid!"));
		return;
	}

	const ELoginStatus::Type LoginStatus = IdentityInterface->GetLoginStatus(LocalUserNum);
	if (LoginStatus != ELoginStatus::LoggedIn)
	{
		FString ErrorCode = FString::Printf(TEXT("%d"), ErrorCodes::StatusUnauthorized);
		FText ErrorMessage = FText::FromString(TEXT("request-failed-list-users-by-user-id-error-user-not-login"));
		OnlineError =  ONLINE_ERROR(EOnlineErrorResult::InvalidResults, ErrorCode, ErrorMessage); 
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to list users by user id, user not logged in!"));
		return;
	}

	if (IsRunningDedicatedServer())
	{
		OnListUserDataSuccess = TDelegateUtils<THandler<FListUserDataResponse>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteListUserByUserId::HandleListUsersByUserId);
		OnError = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteListUserByUserId::HandleAsyncTaskError);
		
		const FServerApiClientPtr ServerApiClient = FMultiRegistry::GetServerApiClient();
		FListUserDataRequest RequestBody{};
		RequestBody.UserIds = UserIds;
		ServerApiClient->ServerUser.ListUserByUserId(RequestBody, OnListUserDataSuccess, OnError);
	}
	else
	{
		FString ErrorCode = FString::Printf(TEXT("%d"), ErrorCodes::StatusUnauthorized);
		FText ErrorMessage = FText::FromString(TEXT("request-failed-list-users-by-user-id-error-not-dedicated-server"));
		OnlineError =  ONLINE_ERROR(EOnlineErrorResult::InvalidResults, ErrorCode,  ErrorMessage); 	 
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to list users by user id, user not a DS!"));
		return;
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("")); 
}

void FOnlineAsyncTaskAccelByteListUserByUserId::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

    Super::TriggerDelegates();
 
	const FOnlineUserAccelBytePtr UserInterface = StaticCastSharedPtr<FOnlineUserAccelByte>(Subsystem->GetUserInterface());  
	if (UserInterface.IsValid())
	{
		if (bWasSuccessful)
		{ 
			UserInterface->TriggerOnListUserByUserIdCompleteDelegates(LocalUserNum, true, ListUserDataResult, OnlineError);   
		}
		else
		{
			UserInterface->TriggerOnListUserByUserIdCompleteDelegates(LocalUserNum, false, FListUserDataResponse{}, OnlineError);
		}
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
} 

void FOnlineAsyncTaskAccelByteListUserByUserId::HandleListUsersByUserId(const FListUserDataResponse& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	
	ListUserDataResult = Result;
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteListUserByUserId::HandleAsyncTaskError(int32 Code, FString const& ErrMsg)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN_VERBOSITY(Error, TEXT("Code: %d; Message: %s"), Code, *ErrMsg);

	FString ErrorCode = FString::Printf(TEXT("%d"), Code); 
	OnlineError =  ONLINE_ERROR(EOnlineErrorResult::RequestFailure, ErrorCode, FText::FromString(ErrMsg)); 
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

#undef ONLINE_ERROR_NAMESPACE