// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteLoginQueue.h"
#include "OnlineAsyncTaskAccelByteLoginQueueClaimTicket.h"

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
	TRY_PIN_SUBSYSTEM();

	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("LocalUserNum: %d, TicketId: %s"), LoginUserNum, *Ticket.Ticket);

	GConfig->GetInt(TEXT("OnlineSubsystemAccelByte"), TEXT("LoginQueuePresentationThreshold"), PresentationThreshold, GEngineIni);
	GConfig->GetBool(TEXT("OnlineSubsystemAccelByte"), TEXT("bEnableManualClaimLoginQueue"), bManualClaimLoginQueueTicket, GEngineIni);

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(SubsystemPin->GetIdentityInterface());
	if(!IdentityInterface.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Unable to initialize login queue, Identity interface is invalid"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}
	
	OnLoginQueueCancelledDelegate = TDelegateUtils<FAccelByteOnLoginQueueCanceledByUserDelegate>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteLoginQueue::OnLoginQueueCancelled);
	OnLoginQueueCancelledDelegateHandle = IdentityInterface->AddAccelByteOnLoginQueueCanceledByUserDelegate_Handle(OnLoginQueueCancelledDelegate);

	OnPollTicketRefreshedHandler = TDelegateUtils<TicketRefreshedDelegate>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteLoginQueue::OnPollTicketRefreshed);
	Poller->SetOnTicketRefreshed(OnPollTicketRefreshedHandler);
	
	OnPollStoppedHandler = TDelegateUtils<THandler<FOnlineErrorAccelByte>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteLoginQueue::OnPollTicketStopped);
	Poller->SetOnPollStopped(OnPollStoppedHandler);
	
	Poller->StartPoll(SubsystemPin.Get(), LoginUserNum, Ticket);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteLoginQueue::Finalize()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), bWasSuccessful ? TEXT("True") : TEXT("False"));

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(SubsystemPin->GetIdentityInterface());
	if(!IdentityInterface.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Failed to finalize login queue process failed, Identity Interface is invalid!"));
		return;
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteLoginQueue::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), bWasSuccessful ? TEXT("True") : TEXT("False"));

	if(bWasSuccessful)
	{
		// all is well, nothing to do here
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Login queue process complete, not trigger any delegate"));
		return;
	}

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(SubsystemPin->GetIdentityInterface());
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
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("LocalUserNum: %d, TicketId: %s"), LoginUserNum, *InTicket.Ticket);

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(SubsystemPin->GetIdentityInterface());
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
		if (bManualClaimLoginQueueTicket)
		{
			IdentityInterface->TriggerAccelByteOnLoginTicketReadyDelegates(LoginUserNum, true);
		}
		else
		{
			Super::ExecuteCriticalSectionAction(FVoidHandler::CreateLambda([this, &InTicket]()
				{
					TRY_PIN_SUBSYSTEM();
					SubsystemPin->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteLoginQueueClaimTicket>(SubsystemPin.Get(), LoginUserNum, InTicket.Ticket);
				}));
		}
		CleanupDelegateHandler();
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

void FOnlineAsyncTaskAccelByteLoginQueue::CleanupDelegateHandler()
{
	TRY_PIN_SUBSYSTEM();

	Poller->UnbindOnPollStopped();
	Poller->UnbindOnTicketRefreshed();
	Poller->StopPoll();
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(SubsystemPin->GetIdentityInterface());
	if(!IdentityInterface.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Failed to broadcast login queue process failed, Identity Interface is invalid!"));
		
		return;
	}
	IdentityInterface->ClearAccelByteOnLoginQueueCanceledByUserDelegate_Handle(OnLoginQueueCancelledDelegateHandle);
}

#undef ONLINE_ERROR_NAMESPACE
