// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteCreateUserProfile.h"

#include "Core/AccelByteMultiRegistry.h"
#include "Core/AccelByteError.h"
#include "Core/AccelByteUtilities.h"
#include "Api/AccelByteUserApi.h"
#include "OnlineError.h"
#include "OnlinePredefinedEventInterfaceAccelByte.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineUserAccelByte"

FOnlineAsyncTaskAccelByteCreateUserProfile::FOnlineAsyncTaskAccelByteCreateUserProfile
	( FOnlineSubsystemAccelByte* const InABSubsystem
	, const FUniqueNetId& InUserId )
	: FOnlineAsyncTaskAccelByte(InABSubsystem)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InUserId);
	bWasCreated = false;
}

void FOnlineAsyncTaskAccelByteCreateUserProfile::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(Subsystem->GetIdentityInterface());

	if (IsRunningDedicatedServer())
	{
		ErrorString = TEXT("request-failed-create-userprofile-error");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to create userProfile, this request is not intended for DS!"));
		return;
	}
	
	if (!IdentityInterface.IsValid())
	{
		ErrorString = TEXT("request-failed-create-user-userprofile-error");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to create userProfile, identity interface is invalid!"));
		return;
	}

	auto LoginStatus = IdentityInterface->GetLoginStatus(*UserId.Get());
	if (LoginStatus != ELoginStatus::LoggedIn)
	{
		ErrorString = TEXT("request-failed-create-user-userprofile-error");
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to create userProfile, user not logged in!"));
		return;
	}

	Account = StaticCastSharedPtr<FUserOnlineAccountAccelByte>(IdentityInterface->GetUserAccount(*UserId.Get()));
	if (!Account.IsValid())
	{
		Account = MakeShared<FUserOnlineAccountAccelByte>(UserId.ToSharedRef());
		Account->SetDisplayName(ApiClient->CredentialsRef->GetUserDisplayName());
		Account->SetAccessToken(ApiClient->CredentialsRef->GetAccessToken());

		IdentityInterface->AddNewAuthenticatedUser(LocalUserNum, UserId.ToSharedRef(), Account.ToSharedRef());
	}

	CreateUserProfile();

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteCreateUserProfile::Finalize()
{
	if (bWasSuccessful && UserProfileUpdatedFieldsPayload.JsonObject.IsValid())
	{
		FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = Subsystem->GetPredefinedEventInterface();
		TSharedPtr<FAccelByteModelsUserProfileCreatedPayload> UserProfileCreatedPayload = MakeShared<FAccelByteModelsUserProfileCreatedPayload>();
		
		UserProfileCreatedPayload->UpdatedFields = UserProfileUpdatedFieldsPayload;
		PredefinedEventInterface->SendEvent(LocalUserNum, UserProfileCreatedPayload.ToSharedRef());
	}
}

void FOnlineAsyncTaskAccelByteCreateUserProfile::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s; bWasSuccessful: %s"), *UserId->ToString(), LOG_BOOL_FORMAT(bWasSuccessful));

	TSharedPtr<FOnlineUserAccelByte, ESPMode::ThreadSafe> UserInterface = StaticCastSharedPtr<FOnlineUserAccelByte>(Subsystem->GetUserInterface());

	EOnlineErrorResult Result = ((bWasSuccessful) ? EOnlineErrorResult::Success : EOnlineErrorResult::RequestFailure);
	UserInterface->TriggerOnCreateUserProfileCompleteDelegates(LocalUserNum, bWasSuccessful, ONLINE_ERROR(Result, ErrorString));

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteCreateUserProfile::OnCreateUserProfileSuccess(const FAccelByteModelsUserProfileInfo& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	TSharedPtr<FOnlineUserAccelByte, ESPMode::ThreadSafe> UserInterface = StaticCastSharedPtr<FOnlineUserAccelByte>(Subsystem->GetUserInterface());
	TSharedPtr<FOnlineUserCacheAccelByte, ESPMode::ThreadSafe> UserCache = StaticCastSharedPtr<FOnlineUserCacheAccelByte>(Subsystem->GetUserCache());

	// Set attributes for the user account based on profile data
	Account->SetUserAttribute(TEXT("AvatarURL"), Result.AvatarUrl);
	Account->SetUserAttribute(TEXT("AvatarSmallURL"), Result.AvatarSmallUrl);
	Account->SetUserAttribute(TEXT("AvatarLargeURL"), Result.AvatarLargeUrl);
	Account->SetUserAttribute(TEXT("UserLanguage"), Result.Language.IsEmpty() ? TEXT("en") : Result.Language);

	if (Result.PublicId.IsEmpty())
	{
		UE_LOG_AB(Warning, TEXT("Failed to set FriendCode text. Player FriendCode (PublicCode) is empty!"));
	}
	else
	{
		Account->SetPublicCode(Result.PublicId);
		UserCache->AddPublicCodeToCache(*UserId.Get(), Result.PublicId);
	}

	// Add the new user information instance to the user interface's cache so the developer can retrieve it in the delegate handler
	if (UserInterface != nullptr)
	{
		UserInterface->AddUserInfo(UserId.ToSharedRef(), Account.ToSharedRef());
	}

	UserProfileUpdatedFieldsPayload.JsonObject = FJsonObjectConverter::UStructToJsonObject(Result);
	FAccelByteUtilities::RemoveEmptyStrings(UserProfileUpdatedFieldsPayload.JsonObject);
	
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Successfully have a user profile!"));
}

void FOnlineAsyncTaskAccelByteCreateUserProfile::CreateUserProfile()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	// NOTE(Maxwell, 5/20/2021): There's not really much here we can set up for helpful defaults for a profile.
	// However, we will set language and timezone based on the values in FPlatformMisc, which will just be blank
	// if these cannot be found.
	// NOTE(Afif, 08/02/2021): Use FInternationalization to get current language inside the OS culture.
	FAccelByteModelsUserProfileCreateRequest CreateRequest;
	CreateRequest.Language = FInternationalization::Get().GetCurrentLanguage()->GetName();
	CreateRequest.Timezone = FOnlineSubsystemAccelByteUtils::GetLocalTimeOffsetFromUTC();
	Account->GetUserAttribute(ACCELBYTE_ACCOUNT_GAME_AVATAR_URL, CreateRequest.AvatarUrl);
	Account->GetUserAttribute(ACCELBYTE_ACCOUNT_GAME_AVATAR_URL, CreateRequest.AvatarSmallUrl);
	Account->GetUserAttribute(ACCELBYTE_ACCOUNT_GAME_AVATAR_URL, CreateRequest.AvatarLargeUrl);

	// We can use the same method as the get profile success handler, as they both will need to do the same operations of
	// setting extra attributes for the user
	OnCreateProfileSuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsUserProfileInfo>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteCreateUserProfile::OnCreateUserProfileSuccess);
	OnCreateProfileErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteCreateUserProfile::OnCreateUserProfileError);
	ApiClient->UserProfile.CreateUserProfile(CreateRequest, OnCreateProfileSuccessDelegate, OnCreateProfileErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Fired off request to create a new default user profile for user '%s'!"), *UserId->ToDebugString());
}

void FOnlineAsyncTaskAccelByteCreateUserProfile::OnCreateUserProfileError(int32 ErrorCode, const FString& ErrorMessage)
{
	ErrorString = TEXT("create-user-profile-error-response");
	UE_LOG_AB(Warning, TEXT("Failed to create profile for user '%s'! Error Code: %d; Error Message: %s"), *UserId->ToString(), ErrorCode, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}

#undef ONLINE_ERROR_NAMESPACE