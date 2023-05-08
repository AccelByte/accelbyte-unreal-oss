// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "Models/AccelByteSessionModels.h"

/**
 * Async task for retrieving player attributes from the session service, and updating current platform and crossplay preferences if necessary.
 */
class FOnlineAsyncTaskAccelByteInitializePlayerAttributes : public FOnlineAsyncTaskAccelByte, public TSelfPtr<FOnlineAsyncTaskAccelByteInitializePlayerAttributes, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteInitializePlayerAttributes(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId);

	virtual void Initialize() override;
	virtual void Finalize() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteInitializePlayerAttributes");
	}

private:
	/**
	 * Attributes that we have retrieved and potentially updated for the player. Will be stored in the session interface
	 * in Finalize.
	 */
	FAccelByteModelsV2PlayerAttributes Attributes{};

	/**
	 * Whether or not this player is allowed to crossplay on their local platform
	 */
	bool bIsCrossplayAllowed{false};

	void OnGetPlayerCrossplayPrivilege(const FUniqueNetId& LocalUserId, EUserPrivileges::Type Privilege, uint32 PrivilegeResult);

	AB_ASYNC_TASK_DECLARE_SDK_DELEGATES_WITH_RESULT(GetPlayerAttributes, FAccelByteModelsV2PlayerAttributes)

	void SendAttributeUpdateRequest(const FAccelByteModelsV2PlayerAttributes& PreviousAttributes);

	AB_ASYNC_TASK_DECLARE_SDK_DELEGATES_WITH_RESULT(StorePlayerAttributes, FAccelByteModelsV2PlayerAttributes)

};

