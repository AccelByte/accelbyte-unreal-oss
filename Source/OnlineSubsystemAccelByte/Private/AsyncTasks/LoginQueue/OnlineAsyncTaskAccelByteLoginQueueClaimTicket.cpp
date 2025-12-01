// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteLoginQueueClaimTicket.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteHelpers.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineAsyncTaskAccelByteLoginQueueClaimTicket"

FOnlineAsyncTaskAccelByteLoginQueueClaimTicket::FOnlineAsyncTaskAccelByteLoginQueueClaimTicket(FOnlineSubsystemAccelByte* const InABInterface,
	int32 InLocalUserNum,
	const FString& InTicketId)
	: FOnlineAsyncTaskAccelByte(InABInterface, InLocalUserNum)
	, TicketId(InTicketId)
{
	Poller = MakeShared<FAccelByteLoginQueuePoller, ESPMode::ThreadSafe>();
}

void FOnlineAsyncTaskAccelByteLoginQueueClaimTicket::Initialize()
{
	TRY_PIN_SUBSYSTEM();

	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("LocalUserNum: %d, TicketId: %s"), LocalUserNum, *TicketId);

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(SubsystemPin->GetIdentityInterface());
	if (!IdentityInterface.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Unable to initialize login queue, Identity interface is invalid"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	FAccelByteInstancePtr AccelByteInstance = SubsystemPin->GetAccelByteInstance().Pin();

	if(!AccelByteInstance.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Unable to initialize login queue, AccelByteInstance is invalid"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}
	
	FApiClientPtr LocalApiClient = SubsystemPin->GetApiClient(LocalUserNum);

	if (!LocalApiClient.IsValid())
	{
		LocalApiClient = AccelByteInstance->GetApiClient(FString::Printf(TEXT("%d"), LocalUserNum));
	}

	SetApiClient(LocalApiClient);
	
	if (!IsApiClientValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Unable to claim access token, ApiClient is invalid"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	OnLoginQueueCancelledDelegate = TDelegateUtils<FAccelByteOnLoginQueueCanceledByUserDelegate>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteLoginQueueClaimTicket::OnLoginQueueCancelled);
	OnLoginQueueCancelledDelegateHandle = IdentityInterface->AddAccelByteOnLoginQueueCanceledByUserDelegate_Handle(OnLoginQueueCancelledDelegate);

	OnClaimAccessTokenSuccessHandler = TDelegateUtils<FVoidHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteLoginQueueClaimTicket::OnClaimAccessTokenSuccess);
	OnClaimAccessTokenErrorHandler = TDelegateUtils<FOAuthErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteLoginQueueClaimTicket::OnClaimAccessTokenError);

	API_FULL_CHECK_GUARD(User);
	User->ClaimAccessToken(TicketId, OnClaimAccessTokenSuccessHandler, OnClaimAccessTokenErrorHandler);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteLoginQueueClaimTicket::Finalize()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), bWasSuccessful ? TEXT("True") : TEXT("False"));

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(SubsystemPin->GetIdentityInterface());
	if(!IdentityInterface.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Failed to finalize login queue process failed, Identity Interface is invalid!"));
		return;
	}

	IdentityInterface->FinalizeLoginQueue(LocalUserNum);
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteLoginQueueClaimTicket::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), bWasSuccessful ? TEXT("True") : TEXT("False"));
	
	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(SubsystemPin->GetIdentityInterface());
	if (!IdentityInterface.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Failed to broadcast login queue process failed, Identity Interface is invalid!"));

		return;
	}

	IdentityInterface->TriggerAccelByteOnLoginQueueClaimTicketCompleteDelegates(LocalUserNum, bWasSuccessful, ErrorOAuthInfo);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Login queue, process to claim ticket complete"));
}

void FOnlineAsyncTaskAccelByteLoginQueueClaimTicket::OnClaimAccessTokenSuccess()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("TicketId: %s"), *TicketId);

	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteLoginQueueClaimTicket::OnClaimAccessTokenError(int32 InErrorCode, const FString& InErrorMessage,
	const FErrorOAuthInfo& InErrorObject)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Claim access token failed for ticket %s with error %d, %s"), *TicketId, InErrorCode, *InErrorMessage);

	ErrorCode = InErrorCode;
	ErrorMessage = InErrorMessage;
	ErrorOAuthInfo = InErrorObject;

	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteLoginQueueClaimTicket::OnLoginQueueCancelled(int32 InLocalUserNum)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Login queue cancelled, LocalUserNum: %d, Cancelled LocalUserNum: %d"), LocalUserNum, InLocalUserNum);

	if (InLocalUserNum == LocalUserNum)
	{
		CleanupDelegateHandler();

		Poller->StopPoll();
		CompleteTask(EAccelByteAsyncTaskCompleteState::Incomplete);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteLoginQueueClaimTicket::CleanupDelegateHandler()
{
	TRY_PIN_SUBSYSTEM();

	Poller->UnbindOnPollStopped();
	Poller->UnbindOnTicketRefreshed();

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(SubsystemPin->GetIdentityInterface());
	if (!IdentityInterface.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Failed to broadcast login queue process failed, Identity Interface is invalid!"));

		return;
	}
	IdentityInterface->ClearAccelByteOnLoginQueueCanceledByUserDelegate_Handle(OnLoginQueueCancelledDelegateHandle);
}

#undef ONLINE_ERROR_NAMESPACE
