// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "AccelBytePoller.h"
#include "OnlineSubsystemAccelByte.h"
#include "AsyncTasks/LoginQueue/OnlineAsyncTaskAccelByteLoginRefreshTicket.h"
#include "Core/AccelByteApiClient.h"

DECLARE_LOG_CATEGORY_EXTERN(LogAccelByteLoginQueuePoll, Log, All);

DECLARE_DELEGATE_OneParam(TicketRefreshedDelegate, const FAccelByteModelsLoginQueueTicketInfo& /* UpdatedTicket */)

class ONLINESUBSYSTEMACCELBYTE_API FAccelByteLoginQueuePoller : public TSharedFromThis<FAccelByteLoginQueuePoller, ESPMode::ThreadSafe>
{
public:
	FAccelByteLoginQueuePoller();
	virtual ~FAccelByteLoginQueuePoller();

	bool SetOnTicketRefreshed(const TicketRefreshedDelegate Delegate);
	bool SetOnPollStopped(const THandler<FOnlineErrorAccelByte> Delegate);
	bool UnbindOnTicketRefreshed();
	bool UnbindOnPollStopped();

	virtual bool StartPoll(FOnlineSubsystemAccelByte* InSubsystem, int32 LocalUserNum, const FAccelByteModelsLoginQueueTicketInfo& InTicket);
	virtual bool StopPoll();

protected:
	FOnlineSubsystemAccelByte* Subsystem {nullptr};
	FAccelByteModelsLoginQueueTicketInfo Ticket;
	TSharedPtr<FAccelBytePoller, ESPMode::ThreadSafe> Poller;
	int32 ConsecutiveErrorCount {0};
	int32 LocalUserNum {0};
	bool bStoppedByErrors {false};

	TicketRefreshedDelegate OnTicketRefreshed;
	THandler<FOnlineErrorAccelByte> OnPollingStopped;

	bool bQueueFinished {false};

	FString LastErrorCode;
	const int32 MaxPollDelay {30};
	const int32 MinPollDelay {3};
	OnPollExecute OnRefreshTicketHandle;
	const int32 DelayJitterRange {10};

	virtual void RefreshTicket();
	int32 CalculatePollDelay(const FAccelByteModelsLoginQueueTicketInfo& TicketInfo) const;
	virtual void OnRefreshTicketComplete(bool bWasSuccessful,  const FAccelByteModelsLoginQueueTicketInfo& TicketInfo, const FOnlineErrorAccelByte& Error);
	FOnRefreshTicketCompleteDelegate RefreshTicketCompleteHandler;
};