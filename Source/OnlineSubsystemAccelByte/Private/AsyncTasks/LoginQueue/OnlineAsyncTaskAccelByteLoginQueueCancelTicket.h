// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"

DECLARE_DELEGATE_ThreeParams(FOnRefreshTicketCompleteDelegate, bool /*bWasSuccessful*/,  const FAccelByteModelsLoginQueueTicketInfo& /*TicketInfo*/, const FOnlineErrorAccelByte& /*Error*/);

class ONLINESUBSYSTEMACCELBYTE_API FOnlineAsyncTaskAccelByteLoginQueueCancelTicket
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteLoginQueueCancelTicket, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteLoginQueueCancelTicket(FOnlineSubsystemAccelByte* const InABInterface, int32 InLoginUserNum, const FString& InTicketId);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteLoginQueueCancelTicket");
	}

private:

	/** the UserNum of user who want to cancel login*/
	int32 LoginUserNum;
	
	/** Ticket ID we want to cancel */
	FString TicketId;

	/** Error description if endpoints response with failed */
	FString ErrorStr;

	/** Error code if endpoints response with failed */
	int32 ErrorCode;

	FVoidHandler OnCancelTicketSuccessHandler;
	void OnCancelTicketSuccess();

	FErrorHandler OnCancelTicketErrorHandler;
	void OnCancelTicketError(int32 InErrorCode, const FString& InErrorMessage);
};
