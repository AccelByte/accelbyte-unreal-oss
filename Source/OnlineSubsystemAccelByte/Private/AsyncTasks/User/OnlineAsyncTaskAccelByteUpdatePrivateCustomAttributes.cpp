// Copyright (c) 2026 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteUpdatePrivateCustomAttributes.h"

#include "Core/AccelByteError.h"
#include "Core/AccelByteUtilities.h"
#include "Api/AccelByteUserProfileApi.h"
#include "OnlineError.h"
#include "OnlinePredefinedEventInterfaceAccelByte.h"
#include "OnlineSubsystemAccelByteUtils.h"
#include "OnlineUserCacheAccelByte.h"
#include "OnlineUserInterfaceAccelByte.h"
#include "OnlineSubsystemAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteHelpers.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineUserAccelByte"

FOnlineAsyncTaskAccelByteUpdatePrivateCustomAttributes::FOnlineAsyncTaskAccelByteUpdatePrivateCustomAttributes
	( FOnlineSubsystemAccelByte* const InABSubsystem
	, int32 InLocalUserNum
	, const TSharedRef<FJsonObject>& InPrivateAttributes
	, const FOnUpdatePrivateCustomAttributesComplete& InDelegate )
	: FOnlineAsyncTaskAccelByte(InABSubsystem, InLocalUserNum)
	, PrivateAttributesRequest(InPrivateAttributes)
	, Delegate(InDelegate)
{
}

void FOnlineAsyncTaskAccelByteUpdatePrivateCustomAttributes::Initialize()
{
	TRY_PIN_SUBSYSTEM();
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	TOptional<bool> IsDS = SubsystemPin->IsDedicatedServer(LocalUserNum);
	if (IsDS.IsSet() && IsDS.GetValue())
	{
		ErrorString = TEXT("request-failed-update-private-custom-attributes-ds-not-allowed");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to update private custom attributes, this request is not intended for DS!"));
		return;
	}

	FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(SubsystemPin->GetIdentityInterface());
	if (!IdentityInterface.IsValid())
	{
		ErrorString = TEXT("request-failed-update-private-custom-attributes-identity-invalid");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to update private custom attributes, identity interface is invalid!"));
		return;
	}

	UserId = FUniqueNetIdAccelByteUser::Create(*IdentityInterface->GetUniquePlayerId(LocalUserNum));
	if (!UserId.IsValid())
	{
		ErrorString = TEXT("request-failed-update-private-custom-attributes-userid-invalid");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to update private custom attributes, user ID is invalid!"));
		return;
	}

	auto LoginStatus = IdentityInterface->GetLoginStatus(*UserId.Get());
	if (LoginStatus != ELoginStatus::LoggedIn)
	{
		ErrorString = TEXT("request-failed-update-private-custom-attributes-not-logged-in");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to update private custom attributes, user not logged in!"));
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

	UpdatePrivateCustomAttributes();
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteUpdatePrivateCustomAttributes::UpdatePrivateCustomAttributes()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	OnUpdatePrivateCustomAttributesSuccessDelegate = TDelegateUtils<THandler<FJsonObjectWrapper>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteUpdatePrivateCustomAttributes::OnUpdatePrivateCustomAttributesSuccess);
	OnUpdatePrivateCustomAttributesErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteUpdatePrivateCustomAttributes::OnUpdatePrivateCustomAttributesError);

	API_FULL_CHECK_GUARD(UserProfile, ErrorString);
	UserProfile->UpdatePrivateCustomAttributes(*PrivateAttributesRequest, OnUpdatePrivateCustomAttributesSuccessDelegate, OnUpdatePrivateCustomAttributesErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Fired off request to update private custom attributes for user '%s'!"), *UserId->ToDebugString());
}

void FOnlineAsyncTaskAccelByteUpdatePrivateCustomAttributes::OnUpdatePrivateCustomAttributesSuccess(const FJsonObjectWrapper& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	UpdatedPrivateCustomAttributes = Result;
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteUpdatePrivateCustomAttributes::OnUpdatePrivateCustomAttributesError(int32 ErrorCode, const FString& ErrorMessage)
{
	ErrorString = TEXT("request-failed-update-private-custom-attributes-error");
	UE_LOG_AB(Warning, TEXT("Failed to update private custom attributes! Error Code: %d; Error Message: %s"), ErrorCode, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}

void FOnlineAsyncTaskAccelByteUpdatePrivateCustomAttributes::Finalize()
{
	TRY_PIN_SUBSYSTEM();
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful)
	{
		FOnlineUserAccelBytePtr UserInterface = StaticCastSharedPtr<FOnlineUserAccelByte>(SubsystemPin->GetUserInterface());
		if (UserInterface.IsValid() && Account.IsValid())
		{
			UserInterface->AddUserInfo(UserId.ToSharedRef(), Account.ToSharedRef());
		}
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteUpdatePrivateCustomAttributes::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM();
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	FOnlineUserAccelBytePtr UserInterface = StaticCastSharedPtr<FOnlineUserAccelByte>(SubsystemPin->GetUserInterface());
	if (UserInterface.IsValid())
	{
		EOnlineErrorResult Result = ((bWasSuccessful) ? EOnlineErrorResult::Success : EOnlineErrorResult::RequestFailure);
		if (bWasSuccessful)
		{
			UserInterface->TriggerOnUpdatePrivateCustomAttributesCompleteDelegates(LocalUserNum, bWasSuccessful, UpdatedPrivateCustomAttributes, ONLINE_ERROR(Result));
		}
		else
		{
			UserInterface->TriggerOnUpdatePrivateCustomAttributesCompleteDelegates(LocalUserNum, bWasSuccessful, FJsonObjectWrapper{}, ONLINE_ERROR(Result, ErrorString));
		}
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}