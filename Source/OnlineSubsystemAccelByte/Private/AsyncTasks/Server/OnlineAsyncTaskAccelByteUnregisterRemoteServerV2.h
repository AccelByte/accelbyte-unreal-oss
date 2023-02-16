// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineSessionInterfaceV2AccelByte.h"

/**
 * Unregister a server in the cloud from the DSM
 */
class FOnlineAsyncTaskAccelByteUnregisterRemoteServerV2 : public FOnlineAsyncTaskAccelByte, public TSelfPtr<FOnlineAsyncTaskAccelByteUnregisterRemoteServerV2, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteUnregisterRemoteServerV2(FOnlineSubsystemAccelByte* const InABInterface, const FName& InSessionName, const FOnUnregisterServerComplete& InDelegate);

    virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteUnregisterRemoteServerV2");
	}

private:
	/**
	 * Name of the session we wish to associate with the server session, will be used if we have a session ID already for
	 * the server, and thus need to restore the session for it.
	 */
	FName SessionName;

	/** Delegate fired after we finish unregistering server */
	FOnUnregisterServerComplete Delegate;

	void OnUnregisterServerSuccess();
	void OnUnregisterServerError(int32 ErrorCode, const FString& ErrorMessage);

};

