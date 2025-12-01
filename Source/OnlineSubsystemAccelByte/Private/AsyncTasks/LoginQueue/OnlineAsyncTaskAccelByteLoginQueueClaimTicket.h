// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "Utilities/AccelByteLoginQueuePoller.h"
#include "OnlineIdentityInterfaceAccelByte.h"

using namespace AccelByte;

class ONLINESUBSYSTEMACCELBYTE_API FOnlineAsyncTaskAccelByteLoginQueueClaimTicket
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteLoginQueueClaimTicket, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteLoginQueueClaimTicket(FOnlineSubsystemAccelByte* const InABInterface, int32 InLoginUserNum, const FString& InTicketId);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:
	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteLoginQueueClaimTicket");
	}

private:

	/** Ticket ID we want to claim */
	FString TicketId{};

	/** Error code produced if request errors out */
	int32 ErrorCode = 0;

	/** Error message produced if request errors out */
	FString ErrorMessage{};

	/** Error message produced if IAM request errors out */
	FErrorOAuthInfo ErrorOAuthInfo{};

	FVoidHandler OnClaimAccessTokenSuccessHandler;
	void OnClaimAccessTokenSuccess();

	FOAuthErrorHandler OnClaimAccessTokenErrorHandler;
	void OnClaimAccessTokenError(int32 InErrorCode, const FString& InErrorMessage, const FErrorOAuthInfo& InErrorObject);

	FDelegateHandle OnLoginQueueCancelledDelegateHandle;
	FAccelByteOnLoginQueueCanceledByUserDelegate OnLoginQueueCancelledDelegate;
	void OnLoginQueueCancelled(int32 InLocalUserNum);

	void CleanupDelegateHandler();

	/** Login queue poller for refreshing ticket until position is 0 */
	TSharedPtr<FAccelByteLoginQueuePoller, ESPMode::ThreadSafe> Poller;
};
