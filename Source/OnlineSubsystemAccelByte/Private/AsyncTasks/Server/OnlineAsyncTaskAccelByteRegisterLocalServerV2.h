// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineSessionInterfaceV2AccelByte.h"

/**
 * Register a locally hosted server to Armada for a session to claim
 */
class FOnlineAsyncTaskAccelByteRegisterLocalServerV2 : public FOnlineAsyncTaskAccelByte
{
public:

	FOnlineAsyncTaskAccelByteRegisterLocalServerV2(FOnlineSubsystemAccelByte* const InABInterface, const FOnRegisterServerComplete& InDelegate);

    virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteRegisterLocalServerV2");
	}

private:
	/** Delegate fired when we finish registering server with backend */
	FOnRegisterServerComplete Delegate;

	/** Name of the server that we are registering to Armada */
	FString ServerName{};

	void OnRegisterServerSuccess();
	void OnRegisterServerError(int32 ErrorCode, const FString& ErrorMessage);

};

