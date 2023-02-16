// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineSessionInterfaceV2AccelByte.h"

/**
 * Async task to leave a party related session
 */
class FOnlineAsyncTaskAccelByteLeaveV2Party : public FOnlineAsyncTaskAccelByte, public TSelfPtr<FOnlineAsyncTaskAccelByteLeaveV2Party, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteLeaveV2Party(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FString& InSessionId, const FOnLeaveSessionComplete& InDelegate);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteLeaveV2Party");
	}

private:
	/** ID of the party that we are trying to leave */
	FString SessionId{};

	/** Delegate fired when we finish leaving the party session */
	FOnLeaveSessionComplete Delegate{};

	/** Flag denoting whether we destroyed a named session along with leaving */
	bool bRemovedNamedSession{false};

	/** Name of the session removed when leaving, only valid when bRemovedNameSession is true */
	FName RemovedSessionName{};

	/** Flag denoting whether we destroyed a restore session along with leaving */
	bool bRemovedRestoreSession{false};

	void OnLeavePartySuccess();
	void OnLeavePartyError(int32 ErrorCode, const FString& ErrorMessage);
};
