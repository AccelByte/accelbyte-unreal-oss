// Copyright (c) 2026 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineSessionInterfaceV2AccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteHelpers.h"

/**
 * Update the password for a PASSWORD_PROTECTED game session. Only the session leader may
 * call this endpoint; the backend returns 403 otherwise.
 */
class FOnlineAsyncTaskAccelByteUpdateV2GameSessionPassword
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteUpdateV2GameSessionPassword, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteUpdateV2GameSessionPassword(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FName& InSessionName, const FString& InNewPassword, const FOnUpdateGameSessionPasswordComplete& InDelegate);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteUpdateV2GameSessionPassword");
	}

private:
	/** Name of the local session whose password we are updating */
	FName SessionName{};

	/** New password to set on the session */
	FString NewPassword{};

	/** Delegate fired when the update completes */
	FOnUpdateGameSessionPasswordComplete Delegate{};

	FVoidHandler OnUpdatePasswordSuccessDelegate;
	FErrorHandler OnUpdatePasswordErrorDelegate;
	AB_ASYNC_TASK_DECLARE_SDK_DELEGATES(UpdatePassword)
};
