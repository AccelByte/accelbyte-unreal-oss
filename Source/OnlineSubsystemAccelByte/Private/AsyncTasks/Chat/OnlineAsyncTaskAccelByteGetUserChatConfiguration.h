// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "OnlineSubsystemAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"

class FOnlineAsyncTaskAccelByteGetUserChatConfiguration
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteGetUserChatConfiguration, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteGetUserChatConfiguration(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;
	virtual void OnTaskTimedOut() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteGetUserChatconfiguration");
	}

private:
	/** Current user chat configuration returned from backend. */
	FAccelByteModelsGetUserChatConfigurationResponse UserChatConfiguration;
	
	/** Online error information in case any error happen. */
	FOnlineError OnlineError;

	/** Handler OnGetUserChatConfigurationSuccess.*/
	void OnGetUserChatConfigurationSuccess(FAccelByteModelsGetUserChatConfigurationResponse const& Response);

	/** Handler OnGetUserChatConfigurationFailed.*/
	void OnGetUserChatConfigurationFailed(const int32 ErrorCode, const FString& ErrorMessage);
};
