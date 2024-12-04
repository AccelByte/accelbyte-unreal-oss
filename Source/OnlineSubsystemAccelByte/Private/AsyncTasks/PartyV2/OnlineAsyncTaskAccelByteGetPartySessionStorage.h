// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineSessionSettings.h"
#include "OnlineSessionInterfaceV2AccelByte.h"

class FOnlineAsyncTaskAccelByteGetPartySessionStorage
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteGetPartySessionStorage, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteGetPartySessionStorage(FOnlineSubsystemAccelByte* const InABInterface,
		FUniqueNetIdAccelByteUserPtr UserUniqueNetId,
		const FOnGetPartySessionStorageComplete& Delegate);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteGetPartySessionStorage");
	}

private:
	FAccelByteModelsV2PartySessionStorage PartySessionStorageResult{};
	FOnGetPartySessionStorageComplete Delegate;

	THandler<FAccelByteModelsV2PartySessionStorage> OnGetPartySessionStorageSuccessDelegate;
	void OnGetPartySessionStorageSuccess(const FAccelByteModelsV2PartySessionStorage& BackendSessionData);

	FErrorHandler OnGetPartySessionStorageErrorDelegate;
	void OnGetPartySessionStorageError(int32 ErrorCode, const FString& ErrorMessage);
};
