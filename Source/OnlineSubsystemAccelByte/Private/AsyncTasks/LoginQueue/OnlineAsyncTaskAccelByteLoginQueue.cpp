// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteLoginQueue.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineAsyncTaskAccelByteLoginRefreshTicket"

FOnlineAsyncTaskAccelByteLoginQueue::FOnlineAsyncTaskAccelByteLoginQueue(FOnlineSubsystemAccelByte* const InABInterface,
	int32 InLoginUserNum, const FAccelByteModelsLoginQueueTicketInfo& InTicket)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, LoginUserNum(InLoginUserNum)
	, Ticket(InTicket)
	, PresentationThreshold(0)
{
	LocalUserNum = INVALID_CONTROLLERID;
	bShouldUseTimeout = false;
	Poller = MakeShared<FAccelByteLoginQueuePoller, ESPMode::ThreadSafe>();
}

void FOnlineAsyncTaskAccelByteLoginQueue::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("LocalUserNum: %d, TicketId: %s"), LoginUserNum, *Ticket.Ticket);

	GConfig->GetInt(TEXT("OnlineSubsystemAccelByte"), TEXT("LoginQueuePresentationThreshold"), PresentationThreshold, GEngineIni);

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(Subsystem->GetIdentityInterface());
	if(!IdentityInterface.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Unable to initialize login queue, Identity interface is invalid"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	IdentityInterface->InitializeLoginQueue(LoginUserNum, Ticket.Ticket);

	OnLoginQueueCancelledDelegate = TDelegateUtils<FAccelByteOnLoginQueueCanceledByUserDelegate>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteLoginQueue::OnLoginQueueCancelled);
	OnLoginQueueCancelledDelegateHandle = IdentityInterface->AddAccelByteOnLoginQueueCanceledByUserDelegate_Handle(OnLoginQueueCancelledDelegate);

	OnPollTicketRefreshedHandler = TDelegateUtils<TicketRefreshedDelegate>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteLoginQueue::OnPollTicketRefreshed);
	Poller->SetOnTicketRefreshed(OnPollTicketRefreshedHandler);
	
	OnPollStoppedHandler = TDelegateUtils<THandler<FOnlineErrorAccelByte>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteLoginQueue::OnPollTicketStopped);
	Poller->SetOnPollStopped(OnPollStoppedHandler);
	
	Poller->StartPoll(Subsystem, LoginUserNum, Ticket);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteLoginQueue::Finalize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), bWasSuccessful ? TEXT("True") : TEXT("False"));

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(Subsystem->GetIdentityInterface());
	if(!IdentityInterface.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Failed to finalize login queue process failed, Identity Interface is invalid!"));
		return;
	}

	IdentityInterface->FinalizeLoginQueue(LoginUserNum);
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteLoginQueue::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), bWasSuccessful ? TEXT("True") : TEXT("False"));
	
	if(bWasSuccessful)
	{
		// all is well, nothing to do here
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Login queue process complete, not trigger any delegate"));
		return;
	}

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(Subsystem->GetIdentityInterface());
	if(!IdentityInterface.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Failed to broadcast login queue process failed, Identity Interface is invalid!"));
		
		return;
	}

	IdentityInterface->TriggerAccelByteOnLoginTicketStatusUpdatedDelegates(LoginUserNum, false, {},
			ONLINE_ERROR_ACCELBYTE(ErrorCode, EOnlineErrorResult::RequestFailure));

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteLoginQueue::OnPollTicketRefreshed(const FAccelByteModelsLoginQueueTicketInfo& InTicket)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("LocalUserNum: %d, TicketId: %s"), LoginUserNum, *InTicket.Ticket);

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(Subsystem->GetIdentityInterface());
	if(!IdentityInterface.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Unable to act on login ticket refreshed, Identity interface is invalid"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	// only trigger ticket status updated when estimated wait time is above presentation threshold
	if(InTicket.EstimatedWaitingTimeInSeconds > PresentationThreshold)
	{
		IdentityInterface->TriggerAccelByteOnLoginTicketStatusUpdatedDelegates(LoginUserNum, true, InTicket,
			ONLINE_ERROR_ACCELBYTE(ErrorCode, EOnlineErrorResult::Success));
	}
	
	if(InTicket.Position == 0)
	{
		CleanupDelegateHandler();
		ClaimAccessToken(InTicket.Ticket);
		
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Login ticket refreshed, position is 0, claiming access token"));
		return;
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteLoginQueue::OnPollTicketStopped(const FOnlineErrorAccelByte& Error)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("poll ticket %s stopped with error %s, %s"), *Ticket.Ticket, *Error.GetErrorCode(), *Error.GetErrorMessage().ToString());
	
	CleanupDelegateHandler();
	
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteLoginQueue::ClaimAccessToken(const FString& InTicketId)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("LocalUserNum: %d, TicketId: %s"), LoginUserNum, *InTicketId);

	if (Subsystem->IsMultipleLocalUsersEnabled())
	{
		SetApiClient(FMultiRegistry::GetApiClient(FString::Printf(TEXT("%d"), LoginUserNum)));
	}
	else
	{
		SetApiClient(FMultiRegistry::GetApiClient());
	}
	
	if(!IsApiClientValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Unable to claim access token, ApiClient is invalid"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}
	
	OnClaimAccessTokenSuccessHandler = TDelegateUtils<FVoidHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteLoginQueue::OnClaimAccessTokenSuccess);
	OnClaimAccessTokenErrorHandler = TDelegateUtils<FOAuthErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteLoginQueue::OnClaimAccessTokenError);
	
	API_CLIENT_CHECK_GUARD();
	ApiClient->User.ClaimAccessToken(InTicketId, OnClaimAccessTokenSuccessHandler, OnClaimAccessTokenErrorHandler);
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteLoginQueue::OnLoginQueueCancelled(int32 InLoginUserNum)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Login queue cancelled, LoginUserNum: %d, Cancelled LoginUserNum: %d"), LoginUserNum, InLoginUserNum);

	if(InLoginUserNum == LoginUserNum)
	{
		CleanupDelegateHandler();
		
		Poller->StopPoll();
		CompleteTask(EAccelByteAsyncTaskCompleteState::Incomplete);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteLoginQueue::OnClaimAccessTokenSuccess()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("TicketId: %s"), *Ticket.Ticket);

	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteLoginQueue::OnClaimAccessTokenError(int32 InErrorCode, const FString& InErrorMessage,
	const FErrorOAuthInfo& InErrorObject)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Claim access token failed for ticket %s with error %d, %s"), *Ticket.Ticket, InErrorCode, *InErrorMessage);

	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteLoginQueue::CleanupDelegateHandler()
{
	Poller->UnbindOnPollStopped();
	Poller->UnbindOnTicketRefreshed();

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(Subsystem->GetIdentityInterface());
	if(!IdentityInterface.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Failed to broadcast login queue process failed, Identity Interface is invalid!"));
		
		return;
	}
	IdentityInterface->ClearAccelByteOnLoginQueueCanceledByUserDelegate_Handle(OnLoginQueueCancelledDelegateHandle);
}

#undef ONLINE_ERROR_NAMESPACE
