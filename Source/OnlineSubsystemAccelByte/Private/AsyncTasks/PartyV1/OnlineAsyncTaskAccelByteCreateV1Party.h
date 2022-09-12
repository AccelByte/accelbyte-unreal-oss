// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "Interfaces/OnlinePartyInterface.h"
#include "Models/AccelByteLobbyModels.h"

/**
 * Async task to create a party for the user on the backend
 */
class FOnlineAsyncTaskAccelByteCreateV1Party : public FOnlineAsyncTaskAccelByte
{
public:

	FOnlineAsyncTaskAccelByteCreateV1Party(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FOnlinePartyTypeId InPartyTypeId, const FPartyConfiguration& InPartyConfig, const FOnCreatePartyComplete& InDelegate);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteCreateParty");
	}

private:

	/** ID of the type of party that is being created, ignored for now */
	FOnlinePartyTypeId PartyTypeId;

	/** Configuration of the party that we wish to create, ignored for now as parties are largely non-configurable */
	FPartyConfiguration PartyConfig;

	/** Delegate to be fired after creating (or failing to create) a party */
	FOnCreatePartyComplete Delegate;

	/** Information on the party just created from the backend */
	FAccelByteModelsCreatePartyResponse PartyInfo;

	/** Shared pointer to the party ID that was created, or nullptr if no party was created */
	TSharedPtr<const FOnlinePartyId> PartyId;

	/** Result of the party creation on the backend, sent to the delegate fired afterwards */
	ECreatePartyCompletionResult CompletionResult;

	/** Code associated with the current party, used for platform party invites */
	FString PartyCode;

	void OnGetPartyInfoResponse(const FAccelByteModelsInfoPartyResponse& Result);
	
	void OnCreatePartyResponse(const FAccelByteModelsCreatePartyResponse& Result);

	void OnPartyGetCodeResponse(const FAccelByteModelsPartyGetCodeResponse& Result);

};

