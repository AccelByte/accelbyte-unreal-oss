// Copyright (c) 2026 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "Models/AccelByteSessionModels.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineSessionInterfaceV2AccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteHelpers.h"

/**
 * Request the plaintext password for a PASSWORD_PROTECTED party. Only active members
 * (status JOINED or CONNECTED) may call this.
 */
class FOnlineAsyncTaskAccelByteGetV2PartyPassword
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteGetV2PartyPassword, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteGetV2PartyPassword(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FName& InSessionName, const FOnGetPartyPasswordComplete& InDelegate);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteGetV2PartyPassword");
	}

private:
	/** Name of the local party whose password we are fetching */
	FName SessionName{};

	/** Delegate fired when the password fetch completes */
	FOnGetPartyPasswordComplete Delegate{};

	/** Password response returned by the backend on success */
	FAccelByteModelsV2SessionPasswordResponse PasswordResponse{};

	THandler<FAccelByteModelsV2SessionPasswordResponse> OnGetPasswordSuccessDelegate;
	FErrorHandler OnGetPasswordErrorDelegate;
	AB_ASYNC_TASK_DECLARE_SDK_DELEGATES_WITH_RESULT(GetPassword, FAccelByteModelsV2SessionPasswordResponse)
};
