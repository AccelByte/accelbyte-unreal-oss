// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "Utilities/AccelByteLoginQueuePoller.h"
#include "OnlineIdentityInterfaceAccelByte.h"

using namespace AccelByte;

class ONLINESUBSYSTEMACCELBYTE_API FOnlineAsyncTaskAccelByteLoginQueue
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteLoginQueue, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteLoginQueue(FOnlineSubsystemAccelByte* const InABInterface, int32 InLoginUserNum, const FAccelByteModelsLoginQueueTicketInfo& InTicket);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:
	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteLoginQueue");
	}

private:

	/** Initial ticket queue */
	FAccelByteModelsLoginQueueTicketInfo Ticket;
	
	/** Login queue poller for refreshing ticket until position is 0 */
	TSharedPtr<FAccelByteLoginQueuePoller, ESPMode::ThreadSafe> Poller;

	/** Result delegate when ticket is successfully claimed */
	FVoidHandler Delegate;

	/** Error code produced if request errors out */
	FString ErrorCode;

	THandler<FAccelByteModelsLoginQueueTicketInfo> OnPollTicketRefreshedHandler;
	THandler<FOnlineErrorAccelByte> OnPollStoppedHandler;

	void OnPollTicketRefreshed(const FAccelByteModelsLoginQueueTicketInfo& InTicket);
	void OnPollTicketStopped(const FOnlineErrorAccelByte& Error);

	FDelegateHandle OnLoginQueueCancelledDelegateHandle;
	FAccelByteOnLoginQueueCanceledByUserDelegate OnLoginQueueCancelledDelegate;
	void OnLoginQueueCancelled(int32 InLoginUserNum);

	void CleanupDelegateHandler();
};
