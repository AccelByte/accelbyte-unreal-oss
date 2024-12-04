// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineSessionSettings.h"

class FOnlineAsyncTaskAccelByteUpdateReservedPartyStorage
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteUpdateReservedPartyStorage, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteUpdateReservedPartyStorage(FOnlineSubsystemAccelByte* const InABInterface,
		FUniqueNetIdAccelByteUserPtr UserUniqueNetId,
		FAccelByteModelsV2PartySessionStorageReservedData UserDataToStoreOnReservedStorage);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteUpdateReservedPartyStorage");
	}

private:

	/**
	 * Collection of user cache that need to be cached to the reserved slot in the party storage.
	 */
	FAccelByteModelsV2PartySessionStorageReservedData UserDataToStoreOnReservedStorage{};

	THandler<FAccelByteModelsV2PartySessionStorageReservedData> OnUpdatePartyReservedStorageSuccessDelegate;
	void OnUpdatePartySessionSuccess(const FAccelByteModelsV2PartySessionStorageReservedData& BackendSessionData);

	FErrorHandler OnUpdatePartyReservedStorageErrorDelegate;
	void OnUpdatePartySessionError(int32 ErrorCode, const FString& ErrorMessage);

	FOnlineError OnlineError{false};

	FAccelByteModelsV2PartySessionStorageReservedData UserReservedDataResult{};
};
