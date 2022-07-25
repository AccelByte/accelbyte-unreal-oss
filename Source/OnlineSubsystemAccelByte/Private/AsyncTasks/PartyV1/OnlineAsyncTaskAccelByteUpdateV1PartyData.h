// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "../OnlineAsyncTaskAccelByte.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "Interfaces/OnlinePartyInterface.h"
#include "Models/AccelByteLobbyModels.h"
#include "OnlinePartyInterfaceAccelByte.h"

/**
 * Task for updating party storage for a party
 */
class FOnlineAsyncTaskAccelByteUpdateV1PartyData : public FOnlineAsyncTaskAccelByte
{
public:

	FOnlineAsyncTaskAccelByteUpdateV1PartyData(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FOnlinePartyId& InPartyId, const FName& InNamespace, const FOnlinePartyData& InPartyData);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteUpdatePartyData");
	}

private:

	/** ID of the party that we want to update data for */
	TSharedRef<const FOnlinePartyIdAccelByte> PartyId;

	/** Namespace that we are trying to update data on, this is ignored for now as party data is not namespaced */
	FName Namespace;

	/** Data that we wish to use to update the party */
	TSharedRef<FOnlinePartyData> PartyData;

	/** Delegate handler for when the request to update party storage was a success */
	void OnWritePartyStorageSuccess(const FAccelByteModelsPartyDataNotif& Result);

	/** Delegate handler for when the request to update party storage failed */
	void OnWritePartyStorageError(int32 ErrorCode, const FString& ErrorMessage);

};

