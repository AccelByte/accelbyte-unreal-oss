// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "Models/AccelByteSessionModels.h"
#include "OnlineSessionInterfaceV2AccelByte.h"

/**
 * Async task to kick a player from a party session
 */
class FOnlineAsyncTaskAccelByteKickV2Party : public FOnlineAsyncTaskAccelByte, public TSelfPtr<FOnlineAsyncTaskAccelByteKickV2Party, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteKickV2Party(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FName& InSessionName, const FUniqueNetId& InPlayerIdToKick, const FOnKickPlayerComplete& InDelegate);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteKickV2Party");
	}

private:
	/** Local name of the party session that we are trying to kick a player from */
	FName SessionName{};
	
	/** ID of the player that we are trying to kick from the party session */
	TSharedRef<const FUniqueNetIdAccelByteUser> PlayerIdToKick;

	/** Delegate fired when we finish kicking a player from the party session */
	FOnKickPlayerComplete Delegate{};

	/** Updated session data from the backend that we will use to update our local session copy */
	FAccelByteModelsV2PartySession UpdatedBackendSessionData;

	void OnKickUserFromPartySuccess(const FAccelByteModelsV2PartySession& Result);
	void OnKickUserFromPartyError(int32 ErrorCode, const FString& ErrorMessage);

};

