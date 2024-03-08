// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"

DECLARE_DELEGATE_ThreeParams(FOnRefreshTicketCompleteDelegate, bool /*bWasSuccessful*/,  const FAccelByteModelsLoginQueueTicketInfo& /*TicketInfo*/, const FOnlineErrorAccelByte& /*Error*/);

class ONLINESUBSYSTEMACCELBYTE_API FOnlineAsyncTaskAccelByteLoginRefreshTicket
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteLoginRefreshTicket, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteLoginRefreshTicket(FOnlineSubsystemAccelByte* const InABInterface, int32 InLoginUserNum, const FString& InTicketId, const FOnRefreshTicketCompleteDelegate InDelegate);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteLoginRefreshTicket");
	}

private:

	/** the usernum of user who want to login*/
	int32 LoginUserNum;
	
	/** Ticket ID we want to refresh */
	FString TicketId;

	/** Cached result of refresh login ticket action */
	FAccelByteModelsLoginQueueTicketInfo Result;

	/** Delegate result to trigger after refresh ticket done */
	FOnRefreshTicketCompleteDelegate Delegate;

	/** Error description if endpoints response with failed */
	FString ErrorStr;

	/** Error code if endpoints response with failed */
	int32 ErrorCode;

	THandler<FAccelByteModelsLoginQueueTicketInfo> OnRefreshTicketSuccessHandler;
	void OnRefreshTicketSuccess(const FAccelByteModelsLoginQueueTicketInfo& TicketInfo);

	FErrorHandler OnRefreshTicketErrorHandler;
	void OnRefreshTicketError(int32 ErrorCode, const FString& ErrorMessage);
};
