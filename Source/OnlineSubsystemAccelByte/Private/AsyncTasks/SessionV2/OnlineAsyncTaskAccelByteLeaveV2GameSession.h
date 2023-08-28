// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineSessionInterfaceV2AccelByte.h"

/**
 * Fill out information about your async task here.
 */
class FOnlineAsyncTaskAccelByteLeaveV2GameSession
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteLeaveV2GameSession, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteLeaveV2GameSession(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FString& InSessionId, const FOnLeaveSessionComplete& InDelegate);

    virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteLeaveV2GameSession");
	}

private:
	/** ID of the game session that we are trying to leave */
	FString SessionId{};

	/** Delegate fired when we finish leaving the game session */
	FOnLeaveSessionComplete Delegate{};

	/** Flag denoting whether we destroyed a named session along with leaving */
	bool bRemovedNamedSession{ false };

	/** Name of the session removed when leaving, only valid when bRemovedNameSession is true */
	FName RemovedSessionName{};

	/** Flag denoting whether we destroyed a restore session along with leaving */
	bool bRemovedRestoreSession{ false };

	void OnLeaveGameSessionSuccess();
	void OnLeaveGameSessionError(int32 ErrorCode, const FString& ErrorMessage);

};

