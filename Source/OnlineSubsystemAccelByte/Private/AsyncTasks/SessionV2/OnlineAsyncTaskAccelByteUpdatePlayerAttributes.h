// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "Models/AccelByteSessionModels.h"
#include "OnlineSessionInterfaceV2AccelByte.h"

/**
 * Async task for updating player attributes on the session service. Specifically only updates crossplay and data
 * attributes for the player.
 */
class FOnlineAsyncTaskAccelByteUpdatePlayerAttributes : public FOnlineAsyncTaskAccelByte, public TSelfPtr<FOnlineAsyncTaskAccelByteUpdatePlayerAttributes, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteUpdatePlayerAttributes(FOnlineSubsystemAccelByte* const InABInterface, 
		const FUniqueNetId& InLocalUserId,
		const FOnlineSessionV2AccelBytePlayerAttributes& InAttributesToUpdate,
		const FOnUpdatePlayerAttributesComplete& InDelegate);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteUpdatePlayerAttributes");
	}

private:
	/**
	 * Structure with the attributes the player wishes to update.
	 */
	FOnlineSessionV2AccelBytePlayerAttributes AttributesToUpdate{};

	/**
	 * Delegate fired when we finish updating attributes for the given player.
	 */
	FOnUpdatePlayerAttributesComplete Delegate{};

	/**
	 * Attributes that we have received back after sending the update request. Will be stored in the session interface
	 * upon completion.
	 */
	FAccelByteModelsV2PlayerAttributes Attributes{};

	void OnGetPlayerCrossplayPrivilege(const FUniqueNetId& LocalUserId, EUserPrivileges::Type Privilege, uint32 PrivilegeResult);

	AB_ASYNC_TASK_DECLARE_SDK_DELEGATES_WITH_RESULT(StorePlayerAttributes, FAccelByteModelsV2PlayerAttributes)

};
