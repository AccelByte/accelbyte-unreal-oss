// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Utilities/AccelByteLoginQueuePoller.h"

#include "AsyncTasks/LoginQueue/OnlineAsyncTaskAccelByteLoginRefreshTicket.h"

DEFINE_LOG_CATEGORY(LogAccelByteLoginQueuePoll);

FAccelByteLoginQueuePoller::FAccelByteLoginQueuePoller()
{
	Poller = MakeShared<FAccelBytePoller, ESPMode::ThreadSafe>();
}

FAccelByteLoginQueuePoller::~FAccelByteLoginQueuePoller()
{
}

bool FAccelByteLoginQueuePoller::SetOnTicketRefreshed(const TicketRefreshedDelegate Delegate)
{
	OnTicketRefreshed = Delegate;
	return true;
}

bool FAccelByteLoginQueuePoller::SetOnPollStopped(const THandler<FOnlineErrorAccelByte> Delegate)
{
	OnPollingStopped = Delegate;
	return true;
}

bool FAccelByteLoginQueuePoller::UnbindOnTicketRefreshed()
{
	OnTicketRefreshed.Unbind();
	return true;
}

bool FAccelByteLoginQueuePoller::UnbindOnPollStopped()
{
	OnPollingStopped.Unbind();
	return true;
}

bool FAccelByteLoginQueuePoller::StartPoll(FOnlineSubsystemAccelByte* InSubsystem, int32 InLocalUserNum, const FAccelByteModelsLoginQueueTicketInfo& InTicket)
{
	if(InSubsystem == nullptr)
	{
		UE_LOG(LogAccelByteLoginQueuePoll, Log, TEXT("Failed to start login queue polling, Subsystem cannot be nullptr"));
		return false;
	}

	if(InTicket.Ticket.IsEmpty())
	{
		UE_LOG(LogAccelByteLoginQueuePoll, Log, TEXT("Failed to start login queue polling, ticket ID cannot be empty"));
		return false;
	}

	Subsystem = InSubsystem;
	LocalUserNum = InLocalUserNum;
	Ticket = InTicket;

	OnRefreshTicketHandle = OnPollExecute::CreateThreadSafeSP(AsShared(), &FAccelByteLoginQueuePoller::RefreshTicket);
	const bool bPollStarted = Poller->StartPolling(OnRefreshTicketHandle, Ticket.EstimatedWaitingTimeInSeconds);
	return bPollStarted;
}

bool FAccelByteLoginQueuePoller::StopPoll()
{
	EOnlineErrorResult ErrorResult;
	if(bQueueFinished)
	{
		ErrorResult = EOnlineErrorResult::Success;
	}
	else if(LastErrorCode.IsEmpty())
	{
		// if LastErrorCode is empty, that means poll was manually stopped
		ErrorResult = EOnlineErrorResult::Canceled;
	}
	else
	{
		ErrorResult = EOnlineErrorResult::RequestFailure;
	}
	
	OnPollingStopped.ExecuteIfBound(FOnlineErrorAccelByte::CreateError(TEXT("AccelByteLoginQueuePoller"), LastErrorCode, ErrorResult));
	
	return Poller->StopPolling();
}

void FAccelByteLoginQueuePoller::RefreshTicket()
{
	// Do refresh ticket here
	RefreshTicketCompleteHandler = FOnRefreshTicketCompleteDelegate::CreateThreadSafeSP(this, &FAccelByteLoginQueuePoller::OnRefreshTicketComplete);
	Subsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteLoginRefreshTicket>(Subsystem, LocalUserNum, Ticket.Ticket, RefreshTicketCompleteHandler);
}

void FAccelByteLoginQueuePoller::OnRefreshTicketComplete(bool bWasSuccessful,  const FAccelByteModelsLoginQueueTicketInfo& TicketInfo, const FOnlineErrorAccelByte& Error)
{
	RefreshTicketCompleteHandler.Unbind();
	
	if(bWasSuccessful)
	{
		// trigger OnTicketRefreshed here
		OnTicketRefreshed.ExecuteIfBound(TicketInfo);

		if(TicketInfo.Position == 0)
		{
			bQueueFinished = true;
			StopPoll();
		}

		int32 Delay = TicketInfo.EstimatedWaitingTimeInSeconds;
		if(Delay < MinPollDelay)
		{
			Delay = MinPollDelay;
		}
		else if(Delay > MaxPollDelay)
		{
			Delay = MaxPollDelay;
		}

		Poller->SetDelay(Delay);
	
		ConsecutiveErrorCount = 0;
	}
	else
	{
		// do something when refresh ticket error?
		UE_LOG(LogAccelByteLoginQueuePoll, Log, TEXT("Poll refresh ticket error with code %s, message %s"), *Error.ErrorCode, *Error.GetErrorMessage().ToString());
	
		ConsecutiveErrorCount++;
		if(ConsecutiveErrorCount == 3)
		{
			LastErrorCode = Error.GetErrorCode();
			StopPoll();
		}
	}
}
