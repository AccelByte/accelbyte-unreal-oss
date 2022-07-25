// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "../OnlineAsyncTaskAccelByte.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineSessionInterfaceV2AccelByte.h"

/**
 * Unregister a locally hosted server from Armada
 */
class FOnlineAsyncTaskAccelByteUnregisterLocalServerV2 : public FOnlineAsyncTaskAccelByte
{
public:

	FOnlineAsyncTaskAccelByteUnregisterLocalServerV2(FOnlineSubsystemAccelByte* const InABInterface, const FOnUnregisterServerComplete& InDelegate);

    virtual void Initialize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteUnregisterLocalServerV2");
	}

private:
	/** Delegate fired when we finish unregistering server from the backend */
	FOnUnregisterServerComplete Delegate;

	void OnUnregisterServerSuccess();
	void OnUnregisterServerError(int32 ErrorCode, const FString& ErrorMessage);

};

