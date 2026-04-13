// Copyright (c) 2026 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteGetCustomAttributes.h"

#include "Core/AccelByteError.h"
#include "Core/AccelByteUtilities.h"
#include "Api/AccelByteUserProfileApi.h"
#include "OnlineError.h"
#include "OnlinePredefinedEventInterfaceAccelByte.h"
#include "OnlineSubsystemAccelByteUtils.h"
#include "OnlineUserCacheAccelByte.h"
#include "OnlineSubsystemAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteHelpers.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineUserAccelByte"

FOnlineAsyncTaskAccelByteGetCustomAttributes::FOnlineAsyncTaskAccelByteGetCustomAttributes
	( FOnlineSubsystemAccelByte* const InABSubsystem
	, int32 InLocalUserNum
	, const FOnGetCustomAttributesComplete& InDelegate )
	: FOnlineAsyncTaskAccelByte(InABSubsystem, InLocalUserNum)
	, Delegate(InDelegate)
{
}

void FOnlineAsyncTaskAccelByteGetCustomAttributes::Initialize()
{
	TRY_PIN_SUBSYSTEM();

	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	// Check if this is a dedicated server - user profile operations are not intended for DS
	TOptional<bool> IsDS = SubsystemPin->IsDedicatedServer(LocalUserNum);
	if (IsDS.IsSet() && IsDS.GetValue())
	{
		ErrorString = TEXT("request-failed-get-custom-attributes-ds-not-allowed");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get custom attributes, this request is not intended for DS!"));
		return;
	}

	FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(SubsystemPin->GetIdentityInterface());

	if (!IdentityInterface.IsValid())
	{
		ErrorString = TEXT("request-failed-get-custom-attributes-identity-invalid");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get custom attributes, identity interface is invalid!"));
		return;
	}

	UserId = FUniqueNetIdAccelByteUser::Create(*IdentityInterface->GetUniquePlayerId(LocalUserNum));

	if (!UserId.IsValid())
	{
		ErrorString = TEXT("request-failed-get-custom-attributes-userid-invalid");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get custom attributes, user ID is invalid!"));
		return;
	}

	auto LoginStatus = IdentityInterface->GetLoginStatus(*UserId.Get());
	if (LoginStatus != ELoginStatus::LoggedIn)
	{
		ErrorString = TEXT("request-failed-get-custom-attributes-not-logged-in");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get custom attributes, user not logged in!"));
		return;
	}

	API_CLIENT_CHECK_GUARD(ErrorString);

	Account = StaticCastSharedPtr<FUserOnlineAccountAccelByte>(IdentityInterface->GetUserAccount(*UserId.Get()));
	if (!Account.IsValid())
	{
		Account = MakeShared<FUserOnlineAccountAccelByte>(UserId.ToSharedRef());
		Account->SetDisplayName(ApiClient->CredentialsRef->GetUserDisplayName());
		Account->SetCredentialsRef(ApiClient->CredentialsRef);
		Account->SetUniqueDisplayName(ApiClient->CredentialsRef->GetUniqueDisplayName());

		IdentityInterface->AddNewAuthenticatedUser(LocalUserNum, UserId.ToSharedRef(), Account.ToSharedRef());
	}

	GetCustomAttributes();

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetCustomAttributes::GetCustomAttributes()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	// Create success and error delegates
	OnGetCustomAttributesSuccessDelegate = TDelegateUtils<THandler<FJsonObject>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGetCustomAttributes::OnGetCustomAttributesSuccess);
	OnGetCustomAttributesErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGetCustomAttributes::OnGetCustomAttributesError);

	API_FULL_CHECK_GUARD(UserProfile, ErrorString);
	UserProfile->GetCustomAttributes(OnGetCustomAttributesSuccessDelegate, OnGetCustomAttributesErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Fired off request to get custom attributes for user '%s'!"), *UserId->ToDebugString());
}

void FOnlineAsyncTaskAccelByteGetCustomAttributes::OnGetCustomAttributesSuccess(const FJsonObject& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	CustomAttributes.JsonObject = MakeShared<FJsonObject>(Result);

	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetCustomAttributes::OnGetCustomAttributesError(int32 ErrorCode, const FString& ErrorMessage)
{
	ErrorString = TEXT("request-failed-get-custom-attributes-error");
	UE_LOG_AB(Warning, TEXT("Failed to get custom attributes! Error Code: %d; Error Message: %s"), ErrorCode, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}

void FOnlineAsyncTaskAccelByteGetCustomAttributes::Finalize()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful)
	{
		// Cache user in the interface
		FOnlineUserAccelBytePtr UserInterface = StaticCastSharedPtr<FOnlineUserAccelByte>(SubsystemPin->GetUserInterface());
		if (UserInterface.IsValid() && Account.IsValid())
		{
			UserInterface->AddUserInfo(UserId.ToSharedRef(), Account.ToSharedRef());
		}
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetCustomAttributes::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	FOnlineUserAccelBytePtr UserInterface = StaticCastSharedPtr<FOnlineUserAccelByte>(SubsystemPin->GetUserInterface());
	if (UserInterface.IsValid())
	{
		EOnlineErrorResult Result = ((bWasSuccessful) ? EOnlineErrorResult::Success : EOnlineErrorResult::RequestFailure);
		if (bWasSuccessful)
		{
			UserInterface->TriggerOnGetCustomAttributesCompleteDelegates(LocalUserNum, bWasSuccessful, CustomAttributes, ONLINE_ERROR(Result));
		}
		else
		{
			UserInterface->TriggerOnGetCustomAttributesCompleteDelegates(LocalUserNum, bWasSuccessful, FJsonObjectWrapper{}, ONLINE_ERROR(Result, ErrorString));
		}
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

#undef ONLINE_ERROR_NAMESPACE