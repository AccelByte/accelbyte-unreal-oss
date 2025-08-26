// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteLoginServer.h"
#include "GameServerApi/AccelByteServerOauth2Api.h"
#include "OnlineIdentityInterfaceAccelByte.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineAccelByteServerLogin"

FOnlineAsyncTaskAccelByteLoginServer::FOnlineAsyncTaskAccelByteLoginServer(FOnlineSubsystemAccelByte* const InABInterface, int32 InLocalUserNum)
    : FOnlineAsyncTaskAccelByte(InABInterface, INVALID_CONTROLLERID, ASYNC_TASK_FLAG_BIT(EAccelByteAsyncTaskFlags::ServerTask))
{
    LocalUserNum = InLocalUserNum;
}

void FOnlineAsyncTaskAccelByteLoginServer::Initialize()
{
    Super::Initialize();

    AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

    SERVER_API_CLIENT_CHECK_GUARD();

	OnServerLoginSuccessDelegate = TDelegateUtils<FVoidHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteLoginServer::OnLoginServerSuccess);
	OnServerLoginErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteLoginServer::OnLoginServerError);
	ServerApiClient->ServerOauth2.LoginWithClientCredentials(OnServerLoginSuccessDelegate, OnServerLoginErrorDelegate);

    AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteLoginServer::Finalize()
{
    TRY_PIN_SUBSYSTEM();

    AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

    if (bWasSuccessful)
    {
		const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(SubsystemPin->GetIdentityInterface());
		if (!ensure(IdentityInterface.IsValid()))
        {
            AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to finalize server login as our identity interface is invalid!"));
            return;
        }

        IdentityInterface->AddAuthenticatedServer(LocalUserNum);
        SubsystemPin->SetLocalUserNumCached(LocalUserNum);
    }

    AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteLoginServer::TriggerDelegates()
{
    TRY_PIN_SUBSYSTEM();

    AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));
	
    const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(SubsystemPin->GetIdentityInterface());
	if (!ensure(IdentityInterface.IsValid()))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to trigger delegates for server login as our identity interface is invalid!"));
		return;
	}

    // #NOTE Deliberately passing an invalid user ID here as server's do not have user IDs. This shouldn't cause any issues for auto login.
	IdentityInterface->TriggerOnLoginCompleteDelegates(LocalUserNum, bWasSuccessful, FUniqueNetIdAccelByteUser::Invalid().Get(), ErrorString);
    IdentityInterface->TriggerAccelByteOnLoginCompleteDelegates(LocalUserNum, bWasSuccessful, FUniqueNetIdAccelByteUser::Invalid().Get(), ONLINE_ERROR_ACCELBYTE(FOnlineErrorAccelByte::PublicGetErrorKey(ErrorCode, ErrorString), bWasSuccessful ? EOnlineErrorResult::Success : EOnlineErrorResult::RequestFailure));

    AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteLoginServer::OnLoginServerSuccess()
{
    TRY_PIN_SUBSYSTEM();

    AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

    SERVER_API_CLIENT_CHECK_GUARD();

    // After logging in, create the user ID instance.
    FAccelByteUniqueIdComposite CompositeId;
    CompositeId.Id = ServerApiClient->ServerCredentialsRef->GetUserId();
    UserId = FUniqueNetIdAccelByteUser::Create(CompositeId);

    // If the user ID is valid,  we want to provide an Access Token on the Identity Interface.
    if (UserId.IsValid())
    {
        // Also create an account instance for them, this will be fed back to the identity interface after login
        TSharedPtr<FUserOnlineAccountAccelByte> Account;
        Account = MakeShared<FUserOnlineAccountAccelByte>(UserId.ToSharedRef()); 
        Account->SetCredentialsRef(ServerApiClient->ServerCredentialsRef);

        const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(SubsystemPin->GetIdentityInterface());
        if (IdentityInterface.IsValid())
        {
            IdentityInterface->AddNewAuthenticatedUser(LocalUserNum, UserId.ToSharedRef(), Account.ToSharedRef()); 
        }
    }  

    CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

    AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteLoginServer::OnLoginServerError(int32 InErrorCode, const FString& ErrorMessage)
{
    UE_LOG_AB(Warning, TEXT("Failed to authenticate server with AccelByte! Error code: %d; Error message: %s"), InErrorCode, *ErrorMessage);
    ErrorCode = InErrorCode;
    ErrorString = TEXT("server-login-failed");
    CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}

#undef ONLINE_ERROR_NAMESPACE