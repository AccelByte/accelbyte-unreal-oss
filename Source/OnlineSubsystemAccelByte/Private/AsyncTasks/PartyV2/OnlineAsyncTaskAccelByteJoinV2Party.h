// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "../OnlineAsyncTaskAccelByte.h"
#include "Models/AccelByteSessionModels.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "Interfaces/OnlineSessionInterface.h"

/**
 * Async Task to join a party session 
 */
class FOnlineAsyncTaskAccelByteJoinV2Party : public FOnlineAsyncTaskAccelByte
{
public:

	FOnlineAsyncTaskAccelByteJoinV2Party(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FName& InSessionName);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteJoinV2Party");
	}

private:
	/** Name of the session to create if join is successful */
	FName SessionName;

	/** Information on the party just obtained from the backend */
	FAccelByteModelsV2PartySession PartyInfo;

	/** Enum used to signal what result occurred with the join session call */
	EOnJoinSessionCompleteResult::Type JoinSessionResult;

	void OnJoinPartySuccess(const FAccelByteModelsV2PartySession& Result);
	void OnJoinPartyError(int32 ErrorCode, const FString& ErrorMessage);
};

