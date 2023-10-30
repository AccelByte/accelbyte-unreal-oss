// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSessionInterfaceV2AccelByte.h"

class FOnlineAsyncTaskAccelByteCancelV2Matchmaking
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteCancelV2Matchmaking, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteCancelV2Matchmaking(FOnlineSubsystemAccelByte* const InABInterface, const TSharedRef<FOnlineSessionSearchAccelByte>& InSearchHandle, const FName& InSessionName);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteCancelV2Matchmaking");
	}

private:
	/** Search handle that we will be canceling matchmaking for */
	TSharedRef<FOnlineSessionSearchAccelByte> SearchHandle;

	/** Name of the session that we are canceling matchmaking for */
	FName SessionName{};

	FVoidHandler OnDeleteMatchTicketSuccessDelegate;
	void OnDeleteMatchTicketSuccess();

	FErrorHandler OnDeleteMatchTicketErrorDelegate;
	void OnDeleteMatchTicketError(int32 ErrorCode, const FString& ErrorMessage);
};
