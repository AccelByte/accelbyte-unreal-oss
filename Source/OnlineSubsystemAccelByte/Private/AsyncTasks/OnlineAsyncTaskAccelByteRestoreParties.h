// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "OnlineAsyncTaskAccelByte.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "Interfaces/OnlinePartyInterface.h"
#include "Models/AccelByteLobbyModels.h"
#include "OnlineAsyncTaskAccelByteQueryPartyInfo.h"

/**
 * Async task to restore parties if the user exits a game while still in a party. Does not work if Auto Kick on Disconnect is enabled in the admin portal.
 */
class FOnlineAsyncTaskAccelByteRestoreParties : public FOnlineAsyncTaskAccelByte
{
public:

	FOnlineAsyncTaskAccelByteRestoreParties(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FOnRestorePartiesComplete& InCompletionDelegate);

	virtual void Initialize() override;
	virtual void Tick() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteRestoreParties");
	}

private:

	/** Delegate that is fired when the task to restore a party completes */
	FOnRestorePartiesComplete CompletionDelegate;

	/** Array of party member IDs to their display names, used in Finalize to construct member instances */
	TArray<TSharedRef<FAccelByteUserInfo>> PartyMemberInfo;

	/** Flag representing whether we have any party info to restore, as this call can succeed but have no current party */
	FThreadSafeBool bUserHasPartyToRestore = false;

	/** Object used to construct a new party from the restore info */
	FAccelByteModelsInfoPartyResponse PartyInfo;

	/**
	 * Party data instance that will be initialized to the party object created in Finalize. We always want to have an
	 * instance of this associated with a party, even if there is no data to read.
	 */
	TSharedRef<FOnlinePartyData> PartyData;

	/**
	 * Pointer to the party ID that was instantiated for the restored party. Only valid if bUserHasPartyToRestore is true.
	 */
	TSharedPtr<const class FOnlinePartyIdAccelByte> PartyId;

	/** Code associated with the party being restored */
	FString PartyCode;

	/** Delegate handler for when we get a response back from the backend on current party info */
	void OnGetPartyInfoResponse(const FAccelByteModelsInfoPartyResponse& Result);

	/** Delegate handler for when our request to query all party info completes */
	void OnQueryPartyInfoComplete(bool bIsSuccessful, const FAccelBytePartyInfo& Result);

	/** Delegate handler for when our query to get a party code from the backend completes */
	void OnPartyGetCodeResponse(const FAccelByteModelsPartyGetCodeResponse& Result);

	/** Delegate handler for when get party data from the backend success */
	void OnGetPartyDataSuccess(const FAccelByteModelsPartyData& InPartyData);

	/** Delegate handler for when get party data from the backend error */
	void OnGetPartyDataError(int32 ErrorCode, const FString& ErrorMessage);
};

