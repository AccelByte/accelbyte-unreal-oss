// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteLoginQueue.h"
#include "OnlineAsyncTaskAccelByteLoginQueueClaimTicket.h"
#include "OnlineSubsystemAccelByteConfig.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteHelpers.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineAsyncTaskAccelByteLoginRefreshTicket"

FOnlineAsyncTaskAccelByteLoginQueue::FOnlineAsyncTaskAccelByteLoginQueue(FOnlineSubsystemAccelByte* const InABInterface,
	int32 InLocalUserNum, const FAccelByteModelsLoginQueueTicketInfo& InTicket)
	: FOnlineAsyncTaskAccelByte(InABInterface, InLocalUserNum)
	, Ticket(InTicket)
{
	bShouldUseTimeout = false;
	Poller = MakeShared<FAccelByteLoginQueuePoller, ESPMode::ThreadSafe>();
}

void FOnlineAsyncTaskAccelByteLoginQueue::Initialize()
{
	TRY_PIN_SUBSYSTEM();

	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("LocalUserNum: %d, TicketId: %s"), LocalUserNum, *Ticket.Ticket);

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
	
	Poller->StartPoll(SubsystemPin, LocalUserNum, Ticket);

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

	IdentityInterface->TriggerAccelByteOnLoginTicketStatusUpdatedDelegates(LocalUserNum, false, {},
			ONLINE_ERROR_ACCELBYTE(ErrorCode, EOnlineErrorResult::RequestFailure));

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteLoginQueue::OnPollTicketRefreshed(const FAccelByteModelsLoginQueueTicketInfo& InTicket)
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("LocalUserNum: %d, TicketId: %s"), LocalUserNum, *InTicket.Ticket);

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(SubsystemPin->GetIdentityInterface());
	if(!IdentityInterface.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Unable to act on login ticket refreshed, Identity interface is invalid"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}
	
	int64 PresentationThreshold{0};
	FOnlineSubsystemAccelByteConfigPtr Config = SubsystemPin->GetConfig();
	if (Config.IsValid())
	{
		PresentationThreshold = Config->GetLoginQueuePresentationThresholdSeconds().GetValue();
	}

	// only trigger ticket status updated when estimated wait time is above presentation threshold
	if(InTicket.EstimatedWaitingTimeInSeconds > PresentationThreshold)
	{
		IdentityInterface->TriggerAccelByteOnLoginTicketStatusUpdatedDelegates(LocalUserNum, true, InTicket,
			ONLINE_ERROR_ACCELBYTE(ErrorCode, EOnlineErrorResult::Success));
	}

	bool bManualClaimLoginQueueTicket { false };
	if (Config.IsValid())
	{
		bManualClaimLoginQueueTicket = Config->GetEnableManualLoginQueueClaim().GetValue();
	}

	if(InTicket.Position == 0)
	{
		if (bManualClaimLoginQueueTicket)
		{
			IdentityInterface->TriggerAccelByteOnLoginTicketReadyDelegates(LocalUserNum, true);
		}
		else
		{
			Super::ExecuteCriticalSectionAction(FVoidHandler::CreateLambda([this, &InTicket]()
				{
					TRY_PIN_SUBSYSTEM();
					SubsystemPin->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteLoginQueueClaimTicket>(SubsystemPin.Get(), LocalUserNum, InTicket.Ticket);
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

void FOnlineAsyncTaskAccelByteLoginQueue::OnLoginQueueCancelled(int32 InLocalUserNum)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Login queue cancelled, LocalUserNum: %d, Cancelled LocalUserNum: %d"), LocalUserNum, InLocalUserNum);

	if(InLocalUserNum == LocalUserNum)
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
