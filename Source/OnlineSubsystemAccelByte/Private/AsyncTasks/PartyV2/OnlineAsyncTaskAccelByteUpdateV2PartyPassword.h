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
 * Update the password for a PASSWORD_PROTECTED party. Only the party leader may call this
 * endpoint; the backend returns 403 otherwise.
 */
class FOnlineAsyncTaskAccelByteUpdateV2PartyPassword
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteUpdateV2PartyPassword, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteUpdateV2PartyPassword(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FName& InSessionName, const FString& InNewPassword, const FOnUpdatePartyPasswordComplete& InDelegate);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteUpdateV2PartyPassword");
	}

private:
	/** Name of the local party whose password we are updating */
	FName SessionName{};

	/** New password to set on the party */
	FString NewPassword{};

	/** Delegate fired when the update completes */
	FOnUpdatePartyPasswordComplete Delegate{};

	FVoidHandler OnUpdatePasswordSuccessDelegate;
	FErrorHandler OnUpdatePasswordErrorDelegate;
	AB_ASYNC_TASK_DECLARE_SDK_DELEGATES(UpdatePassword)
};
