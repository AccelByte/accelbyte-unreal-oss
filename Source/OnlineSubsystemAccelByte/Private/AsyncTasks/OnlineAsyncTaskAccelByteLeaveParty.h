// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "OnlineAsyncTaskAccelByte.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include <OnlinePartyInterfaceAccelByte.h>

/**
 * Task for leaving a party
 */
class FOnlineAsyncTaskAccelByteLeaveParty : public FOnlineAsyncTaskAccelByte
{
public:

	FOnlineAsyncTaskAccelByteLeaveParty(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FOnlinePartyId& InPartyId, bool InBSynchronizeLeave, const FOnLeavePartyComplete& InDelegate);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteLeaveParty");
	}

private:

	/** ID of the party that the user wishes to leave */
	TSharedRef<const FOnlinePartyIdAccelByte> PartyId;

	/** Whether or not we want to synchronize the party leave with the backend */
	bool bSynchronizeLeave;

	/** Delegate fired when we leave a party, or fail to leave one */
	FOnLeavePartyComplete Delegate;

	/** State of the call to leave the party specified, passed to delegate */
	ELeavePartyCompletionResult CompletionResult;

	/** Delegate handler for when we get a response back from the backend about leaving the party specified in this task */
	void OnLeavePartyResponse(const FAccelByteModelsLeavePartyResponse& Result);

};
