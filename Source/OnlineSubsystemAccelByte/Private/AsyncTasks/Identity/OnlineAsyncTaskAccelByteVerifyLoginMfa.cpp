// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteVerifyLoginMfa.h"

#include "OnlineAgreementInterfaceAccelByte.h"
#include "OnlineSessionInterfaceV2AccelByte.h"
#include "OnlineUserInterfaceAccelByte.h"
#include "AsyncTasks/LoginQueue/OnlineAsyncTaskAccelByteLoginQueue.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineAsyncTaskAccelByteVerifyLoginMfa"

FOnlineAsyncTaskAccelByteVerifyLoginMfa::FOnlineAsyncTaskAccelByteVerifyLoginMfa(FOnlineSubsystemAccelByte* const InABSubsystem, int32 InLocalUserNum
	, const EAccelByteLoginAuthFactorType InFactorType, const FString& InCode, const FString& InMfaToken, const bool bInRememberDevice)
	: FOnlineAsyncTaskAccelByte(InABSubsystem)
	, LoginUserNum(InLocalUserNum)
	, FactorType(InFactorType), Code(InCode), MfaToken(InMfaToken) , bRememberDevice(bInRememberDevice)
{
	LocalUserNum = INVALID_CONTROLLERID;
}

void FOnlineAsyncTaskAccelByteVerifyLoginMfa::Initialize()
{
	TRY_PIN_SUBSYSTEM()

	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("LoginUserNum: %d"), LoginUserNum);

	GConfig->GetInt(TEXT("OnlineSubsystemAccelByte"), TEXT("LoginQueuePresentationThreshold"), LoginQueuePresentationThreshold, GEngineIni);

	if (SubsystemPin->IsMultipleLocalUsersEnabled())
	{
		SetApiClient(FMultiRegistry::GetApiClient(FString::Printf(TEXT("%d"), LoginUserNum)));
	}
	else
	{
		SetApiClient(FMultiRegistry::GetApiClient());
	}

	API_CLIENT_CHECK_GUARD();

	const auto OnSuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsLoginQueueTicketInfo>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteVerifyLoginMfa::OnVerifyLoginSuccess);
	const auto OnErrorDelegate = TDelegateUtils<FOAuthErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteVerifyLoginMfa::OnVerifyLoginError);
	ApiClient->User.VerifyLoginWithNewDevice2FAEnabledV4(MfaToken, FactorType, Code, OnSuccessDelegate, OnErrorDelegate, bRememberDevice);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""))
}

void FOnlineAsyncTaskAccelByteVerifyLoginMfa::Finalize()
{
	TRY_PIN_SUBSYSTEM()

	Super::Finalize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (SubsystemPin.IsValid())
	{
		SubsystemPin->SetLocalUserNumCached(LoginUserNum);
	}

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(SubsystemPin->GetIdentityInterface());
	if(!IdentityInterface.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Failed to clear AccelByteOnLoginQueueCanceledByUserDelegate_Handle, Identity interface is invalid!"));
		return;
	}

	IdentityInterface->ClearAccelByteOnLoginQueueCanceledByUserDelegate_Handle(OnLoginQueueCancelledDelegateHandle);
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteVerifyLoginMfa::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM()

	Super::TriggerDelegates();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(SubsystemPin->GetIdentityInterface());
	const TSharedRef<const FUniqueNetIdAccelByteUser> ReturnId = (bWasSuccessful) ? UserId.ToSharedRef() : FUniqueNetIdAccelByteUser::Invalid();

	if (!IdentityInterface.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Failed to trigger delegates as our identity interface is invalid"));
		return;
	}

	IdentityInterface->SetLoginStatus(LoginUserNum, bWasSuccessful ? ELoginStatus::LoggedIn : ELoginStatus::NotLoggedIn);
	
	IdentityInterface->TriggerOnLoginCompleteDelegates(LoginUserNum, bWasSuccessful, ReturnId.Get(), ErrorStr);
	IdentityInterface->TriggerOnLoginWithOAuthErrorCompleteDelegates(LoginUserNum, bWasSuccessful, ReturnId.Get(), ErrorOAuthObject);
	IdentityInterface->TriggerAccelByteOnLoginCompleteDelegates(LoginUserNum, bWasSuccessful, ReturnId.Get(), ONLINE_ERROR_ACCELBYTE(FOnlineErrorAccelByte::PublicGetErrorKey(ErrorCode, ErrorStr), bWasSuccessful ? EOnlineErrorResult::Success : EOnlineErrorResult::RequestFailure));

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteVerifyLoginMfa::OnTaskTimedOut()
{
	Super::OnTaskTimedOut();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("LoginUserNum: %d"), LoginUserNum);

	OnlineError = ONLINE_ERROR(EOnlineErrorResult::RequestFailure
		, FString::FromInt(static_cast<int32>(EOnlineErrorResult::RequestFailure))
		, FText::FromString(TEXT("Verify login Mfa task timeout while waiting for response from backend.")));

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteVerifyLoginMfa::Tick()
{
	Super::Tick();

	if(!bLoginInQueue)
	{
		return;
	}

	API_CLIENT_CHECK_GUARD();
	if(ApiClient->CredentialsRef->IsSessionValid())
	{
		OnLoginSuccess();
	}
}

void FOnlineAsyncTaskAccelByteVerifyLoginMfa::OnVerifyLoginSuccess(const FAccelByteModelsLoginQueueTicketInfo& Response)
{
	TRY_PIN_SUBSYSTEM()

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("LoginUserNum: %d, TicketId: %s"), LoginUserNum, *Response.Ticket);

	if (Response.Ticket.IsEmpty())
	{
		OnLoginSuccess();
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
		return;
	}

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(SubsystemPin->GetIdentityInterface());
	if(!IdentityInterface.IsValid())
	{
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Failed to queue login ticket, Identity interface is invalid!"));
		return;
	}

	// Only trigger login queued delegate if estimated waiting time is above presentation threshold
	if(Response.EstimatedWaitingTimeInSeconds > LoginQueuePresentationThreshold)
	{
		IdentityInterface->TriggerAccelByteOnLoginQueuedDelegates(LoginUserNum, Response);
	}

	OnLoginQueueCancelledDelegate = TDelegateUtils<FAccelByteOnLoginQueueCanceledByUserDelegate>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteVerifyLoginMfa::OnLoginQueueCancelled);
	OnLoginQueueCancelledDelegateHandle = IdentityInterface->AddAccelByteOnLoginQueueCanceledByUserDelegate_Handle(OnLoginQueueCancelledDelegate);
	
	OnLoginQueueClaimTicketCompleteDelegate = TDelegateUtils<FAccelByteOnLoginQueueClaimTicketCompleteDelegate>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteVerifyLoginMfa::OnLoginQueueTicketClaimed);
	OnLoginQueueClaimTicketCompleteDelegateHandle = IdentityInterface->AddAccelByteOnLoginQueueClaimTicketCompleteDelegate_Handle(LoginUserNum, OnLoginQueueClaimTicketCompleteDelegate);

	bShouldUseTimeout = false;
	bLoginInQueue = true;
	this->ExecuteCriticalSectionAction(FVoidHandler::CreateLambda([this, &Response]()
	{
		TRY_PIN_SUBSYSTEM()
		SubsystemPin->CreateAndDispatchAsyncTaskSerial<FOnlineAsyncTaskAccelByteLoginQueue>(SubsystemPin.Get(), LoginUserNum, Response);
	}));
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("LoginQueue task dispatched."));
}

void FOnlineAsyncTaskAccelByteVerifyLoginMfa::OnVerifyLoginError(const int32 ResponseErrorCode, const FString& ErrorMessage, const FErrorOAuthInfo& ErrorObject)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("LoginUserNum: %d, ErrorCode: %d, ErrorMessage: %s")
		, LoginUserNum, ErrorCode, *ErrorMessage);

	ErrorCode = ResponseErrorCode;
	ErrorStr = ErrorMessage;
	
	ErrorOAuthObject = ErrorObject;

	OnlineError = ONLINE_ERROR(EOnlineErrorResult::RequestFailure
		, FString::FromInt(ErrorCode)
		, FText::FromString(ErrorMessage));
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteVerifyLoginMfa::OnLoginQueueCancelled(int32 InLoginUserNum)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Login queue cancelled for user %d"), LoginUserNum);

	if(LoginUserNum != InLoginUserNum)
	{
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("OnLoginQueueCancelled not processed as the login user num is %d not %d"), InLoginUserNum, LoginUserNum);
		return;
	}

	bLoginInQueue = false;

	ErrorStr = TEXT("login process in queue is cancelled");
	ErrorCode = static_cast<int32>(ErrorCodes::LoginQueueCanceled);
	
	ErrorOAuthObject.ErrorCode = ErrorCode;
	ErrorOAuthObject.ErrorMessage = ErrorStr;
	
	OnlineError = ONLINE_ERROR(EOnlineErrorResult::RequestFailure
		, FString::FromInt(static_cast<int32>(ErrorCodes::LoginQueueCanceled))
		, FText::FromString(TEXT("login process in queue is cancelled")));
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteVerifyLoginMfa::OnLoginQueueTicketClaimed(int32 InLoginUserNum, bool bWasClaimSuccessful, const FErrorOAuthInfo& ErrorObject)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Login queue ticket is claimed for user %d, bWasClaimSuccessful: %s"), InLoginUserNum, bWasClaimSuccessful ? TEXT("True") : TEXT("False"));

	bWasSuccessful = bWasClaimSuccessful;

	if (LoginUserNum != InLoginUserNum)
	{
		return;
	}

	bLoginInQueue = false;

	if (!bWasSuccessful)
	{
		ErrorOAuthObject = ErrorObject;

		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
	}
	else
	{
		OnLoginSuccess();
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteVerifyLoginMfa::OnLoginSuccess()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	// Create a new user ID instance for the user that we just logged in as
	FAccelByteUniqueIdComposite CompositeId;
	API_CLIENT_CHECK_GUARD();

	CompositeId.Id = ApiClient->CredentialsRef->GetUserId();

	UserId = FUniqueNetIdAccelByteUser::Create(CompositeId);
	if (!UserId.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to process OnLoginSuccess as our UserId is invalid."));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}
	
	// Also create an account instance for them, this will be fed back to the identity interface after login
	Account = MakeShared<FUserOnlineAccountAccelByte>(UserId.ToSharedRef());
	Account->SetUserCountry(ApiClient->CredentialsRef->GetAccountUserData().Country);
	Account->SetDisplayName(ApiClient->CredentialsRef->GetUserDisplayName());
	Account->SetCredentialsRef(ApiClient->CredentialsRef);
	Account->SetPlatformUserId(ApiClient->CredentialsRef->GetPlatformUserId());
	Account->SetSimultaneousPlatformID(ApiClient->CredentialsRef->GetSimultaneousPlatformId());
	Account->SetSimultaneousPlatformUserID(ApiClient->CredentialsRef->GetSimultaneousPlatformUserId());
	Account->SetUniqueDisplayName(ApiClient->CredentialsRef->GetUniqueDisplayName());
	
	// Retrieve the platform user information array from the account user data.
	const TArray<FAccountUserPlatformInfo>& PlatformInfos = ApiClient->CredentialsRef->GetAccountUserData().PlatformInfos;

	// Iterate over platform user information retrieved from the account data.
	for (const FAccountUserPlatformInfo& Info : PlatformInfos)
	{
		FOnlinePlatformUserAccelByte LocalPlatformUser;
		LocalPlatformUser.SetDisplayName(Info.PlatformDisplayName);
		LocalPlatformUser.SetAvatarUrl(Info.PlatformAvatarUrl);
		LocalPlatformUser.SetPlatformId(Info.PlatformId);
		LocalPlatformUser.SetPlatformGroup(Info.PlatformGroup);
		LocalPlatformUser.SetPlatformUserId(Info.PlatformUserId);

		// Prepare the linked platform user information for caching.
		FAccelByteLinkedUserInfo LinkedUserInfo;
		TArray<FAccelByteLinkedUserInfo> LinkedPlatformUsers;

		LinkedUserInfo.DisplayName = Info.PlatformDisplayName;
		LinkedUserInfo.PlatformId = Info.PlatformId;
		LinkedUserInfo.AvatarUrl = Info.PlatformAvatarUrl;
		LinkedPlatformUsers.Add(LinkedUserInfo);

		// Add each platform user to the related account.
		Account->AddPlatformUser(LocalPlatformUser);
	}
	
	TRY_PIN_SUBSYSTEM()

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(SubsystemPin->GetIdentityInterface());
	if (!IdentityInterface.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to add new authenticated user as our IdentityInterface is invalid."));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	IdentityInterface->AddNewAuthenticatedUser(LoginUserNum, UserId.ToSharedRef(), Account.ToSharedRef());
	FMultiRegistry::RemoveApiClient(UserId->GetAccelByteId());
	FMultiRegistry::RegisterApiClient(UserId->GetAccelByteId(), ApiClient);
	if (SubsystemPin->IsMultipleLocalUsersEnabled())
	{
		FMultiRegistry::RemoveApiClient(FString::Printf(TEXT("%d"), LoginUserNum));
	}
	else
	{
		FMultiRegistry::RemoveApiClient();
	}

	// Grab our user interface and kick off a task to get information about the newly logged in player from it, namely
	// their avatar URL. No need to register a delegate to update the account from the query, the query task will check
	// if an account instance exists for the player in the identity interface, and if so will update it.
	FOnlineUserAccelBytePtr UserInterface = nullptr;
	const bool bUserInterfaceRetrieved = FOnlineUserAccelByte::GetFromSubsystem(SubsystemPin.Get(), UserInterface);
	if (!bUserInterfaceRetrieved || !UserInterface.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to query user info as our UserInterface is invalid."));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	UserInterface->AddOnQueryUserProfileCompleteDelegate_Handle(LoginUserNum, FOnQueryUserProfileCompleteDelegate::CreateThreadSafeSP(UserInterface.Get(), &FOnlineUserAccelByte::PostLoginBulkGetUserProfileCompleted));
	UserInterface->QueryUserInfo(LoginUserNum, { UserId.ToSharedRef() });

	// If we are using V2 sessions, send a request to update stored platform data in the session service for native sync and crossplay
#if AB_USE_V2_SESSIONS
	FOnlineSessionV2AccelBytePtr SessionInterface = nullptr;
	const bool bSessionInterfaceRetrieved = FOnlineSessionV2AccelByte::GetFromSubsystem(SubsystemPin.Get(), SessionInterface);
	if (!bSessionInterfaceRetrieved || !SessionInterface.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to initialize player attributes as our SessionInterface is invalid."));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	SessionInterface->InitializePlayerAttributes(UserId.ToSharedRef().Get());	
#endif 

	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

#undef ONLINE_ERROR_NAMESPACE
