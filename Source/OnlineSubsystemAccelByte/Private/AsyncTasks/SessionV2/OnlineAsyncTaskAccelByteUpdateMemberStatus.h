// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineSessionInterfaceV2AccelByte.h"

/**
 * Server async task to update the status of a member of a session.
 */
class FOnlineAsyncTaskAccelByteUpdateMemberStatus : public FOnlineAsyncTaskAccelByte, public TSelfPtr<FOnlineAsyncTaskAccelByteUpdateMemberStatus, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteUpdateMemberStatus(FOnlineSubsystemAccelByte* const InABInterface, FName InSessionName, const FUniqueNetId& InPlayerId, const EAccelByteV2SessionMemberStatus& InStatus, const FOnSessionMemberStatusUpdateComplete& InDelegate);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteUpdateMemberStatus");
	}

private:
	/**
	 * Local name of the session that contains the member that we want to update status of.
	 */
	FName SessionName{};

	/**
	 * ID of the player that we want to update status of.
	 */
	FUniqueNetIdAccelByteUserPtr PlayerId{nullptr};

	/**
	 * Status enum that we wish to update the player to have.
	 */
	EAccelByteV2SessionMemberStatus Status{EAccelByteV2SessionMemberStatus::EMPTY};

	/**
	 * Delegate fired when this async task completes.
	 */
	FOnSessionMemberStatusUpdateComplete Delegate{};

	AB_ASYNC_TASK_DECLARE_SDK_DELEGATES(UpdateMemberStatus);

};

