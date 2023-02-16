// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineSessionInterfaceV2AccelByte.h"

/**
 * Register a server spawned in the cloud to Armada for a session to claim
 */
class FOnlineAsyncTaskAccelByteRegisterRemoteServerV2 : public FOnlineAsyncTaskAccelByte, public TSelfPtr<FOnlineAsyncTaskAccelByteRegisterRemoteServerV2, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteRegisterRemoteServerV2(FOnlineSubsystemAccelByte* const InABInterface, const FName& InSessionName, const FOnRegisterServerComplete& InDelegate);

    virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteRegisterRemoteServerV2");
	}

private:
	/**
	 * Name of the session we wish to associate with the server session, will be used if we have a session ID already for
	 * the server, and thus need to restore the session for it.
	 */
	FName SessionName;

	/** Delegate fired after we finish registering server */
	FOnRegisterServerComplete Delegate;

	/** Try and get the port that the server is currently bound to */
	bool GetServerPort(int32& OutPort);

	void OnRegisterServerSuccess();
	void OnRegisterServerError(int32 ErrorCode, const FString& ErrorMessage);

};

