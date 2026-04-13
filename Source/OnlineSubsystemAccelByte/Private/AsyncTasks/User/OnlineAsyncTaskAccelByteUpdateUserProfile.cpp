// Copyright (c) 2026 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteUpdateUserProfile.h"

#include "Core/AccelByteError.h"
#include "Core/AccelByteUtilities.h"
#include "Api/AccelByteUserApi.h"
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

FOnlineAsyncTaskAccelByteUpdateUserProfile::FOnlineAsyncTaskAccelByteUpdateUserProfile
	( FOnlineSubsystemAccelByte* const InABSubsystem
	, int32 InLocalUserNum
	, const FUniqueNetId& InUserId
	, const FAccelByteModelsUserProfileUpdateRequest& InUpdateRequest )
	: FOnlineAsyncTaskAccelByte(InABSubsystem, InLocalUserNum)
	, UpdateRequest(InUpdateRequest)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InUserId);
}

void FOnlineAsyncTaskAccelByteUpdateUserProfile::Initialize()
{
	TRY_PIN_SUBSYSTEM();

	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	// Check if this is a dedicated server - user profile operations are not intended for DS
	TOptional<bool> IsDS = SubsystemPin->IsDedicatedServer(LocalUserNum);
	if (IsDS.IsSet() && IsDS.GetValue())
	{
		ErrorString = TEXT("request-failed-update-userprofile-ds-not-allowed");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to update userProfile, this request is not intended for DS!"));
		return;
	}

	FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(SubsystemPin->GetIdentityInterface());

	if (!IdentityInterface.IsValid())
	{
		ErrorString = TEXT("request-failed-update-userprofile-identity-invalid");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to update userProfile, identity interface is invalid!"));
		return;
	}

	auto LoginStatus = IdentityInterface->GetLoginStatus(*UserId.Get());
	if (LoginStatus != ELoginStatus::LoggedIn)
	{
		ErrorString = TEXT("request-failed-update-userprofile-not-logged-in");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to update userProfile, user not logged in!"));
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

	UpdateUserProfile();

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteUpdateUserProfile::UpdateUserProfile()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	// Create success and error delegates
	OnUpdateProfileSuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsUserProfileInfo>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteUpdateUserProfile::OnUpdateUserProfileSuccess);
	OnUpdateProfileErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteUpdateUserProfile::OnUpdateUserProfileError);

	API_FULL_CHECK_GUARD(UserProfile, ErrorString);
	UserProfile->UpdateUserProfile(UpdateRequest, OnUpdateProfileSuccessDelegate, OnUpdateProfileErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Fired off request to update user profile for user '%s'!"), *UserId->ToDebugString());
}

void FOnlineAsyncTaskAccelByteUpdateUserProfile::Finalize()
{
	TRY_PIN_SUBSYSTEM();

	if (bWasSuccessful && UserProfileUpdatedFieldsPayload.JsonObject.IsValid())
	{
		FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = SubsystemPin->GetPredefinedEventInterface();
		if (PredefinedEventInterface.IsValid())
		{
			TSharedPtr<FAccelByteModelsUserProfileUpdatedPayload> UserProfileUpdatedPayload = MakeShared<FAccelByteModelsUserProfileUpdatedPayload>();
			UserProfileUpdatedPayload->UpdatedFields = UserProfileUpdatedFieldsPayload;
			PredefinedEventInterface->SendEvent(LocalUserNum, UserProfileUpdatedPayload.ToSharedRef());
		}
	}
}

void FOnlineAsyncTaskAccelByteUpdateUserProfile::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM();

    AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s; bWasSuccessful: %s"), *UserId->ToString(), LOG_BOOL_FORMAT(bWasSuccessful));

    TSharedPtr<FOnlineUserAccelByte, ESPMode::ThreadSafe> UserInterface = StaticCastSharedPtr<FOnlineUserAccelByte>(SubsystemPin->GetUserInterface());

	if (UserInterface.IsValid())
	{
		EOnlineErrorResult Result = ((bWasSuccessful) ? EOnlineErrorResult::Success : EOnlineErrorResult::RequestFailure);
		UserInterface->TriggerOnUpdateUserProfileCompleteDelegates(LocalUserNum, bWasSuccessful, ONLINE_ERROR(Result, ErrorString));
	}
	else
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Cannot trigger delegates, user interface is invalid!"));
	}

    AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteUpdateUserProfile::OnUpdateUserProfileSuccess(const FAccelByteModelsUserProfileInfo& Result)
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	// Set attributes for the user account based on updated profile data
	Account->SetUserAttribute(TEXT("AvatarURL"), Result.AvatarUrl);
	Account->SetUserAttribute(TEXT("AvatarSmallURL"), Result.AvatarSmallUrl);
	Account->SetUserAttribute(TEXT("AvatarLargeURL"), Result.AvatarLargeUrl);
	Account->SetUserAttribute(TEXT("FirstName"), Result.FirstName);
	Account->SetUserAttribute(TEXT("LastName"), Result.LastName);
	Account->SetUserAttribute(TEXT("Timezone"), Result.Timezone);
	Account->SetUserAttribute(TEXT("DateOfBirth"), Result.DateOfBirth);
	Account->SetUserAttribute(TEXT("ZipCode"), Result.ZipCode);
	// Only update language if non-empty; preserve cached value otherwise
	if (!Result.Language.IsEmpty())
	{
		Account->SetUserAttribute(TEXT("UserLanguage"), Result.Language);
	}

	if (Result.PublicId.IsEmpty())
	{
		UE_LOG_AB(Warning, TEXT("Failed to set FriendCode text. Player FriendCode (PublicCode) is empty!"));
	}
	else
	{
		Account->SetPublicCode(Result.PublicId);
		TSharedPtr<FOnlineUserCacheAccelByte, ESPMode::ThreadSafe> UserCache = StaticCastSharedPtr<FOnlineUserCacheAccelByte>(SubsystemPin->GetUserCache());
		if (UserCache.IsValid())
		{
			UserCache->AddPublicCodeToCache(*UserId.Get(), Result.PublicId);
		}
	}

	// Update the user information instance in the user interface's cache
	TSharedPtr<FOnlineUserAccelByte, ESPMode::ThreadSafe> UserInterface = StaticCastSharedPtr<FOnlineUserAccelByte>(SubsystemPin->GetUserInterface());

	if (UserInterface.IsValid())
	{
		UserInterface->AddUserInfo(UserId.ToSharedRef(), Account.ToSharedRef());
	}

	UserProfileUpdatedFieldsPayload.JsonObject = FJsonObjectConverter::UStructToJsonObject(Result);
	FAccelByteUtilities::RemoveEmptyStrings(UserProfileUpdatedFieldsPayload.JsonObject);

	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Successfully updated user profile!"));
}

void FOnlineAsyncTaskAccelByteUpdateUserProfile::OnUpdateUserProfileError(int32 ErrorCode, const FString& ErrorMessage)
{
	ErrorString = TEXT("update-user-profile-error-response");
	UE_LOG_AB(Warning, TEXT("Failed to update profile for user '%s'! Error Code: %d; Error Message: %s"), *UserId->ToString(), ErrorCode, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}

#undef ONLINE_ERROR_NAMESPACE
