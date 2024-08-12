// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "OnlineSubsystemAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"

class FOnlineAsyncTaskAccelByteSetUserChatConfiguration
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteSetUserChatConfiguration, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteSetUserChatConfiguration(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId
		, const FAccelByteModelsSetUserChatConfigurationRequest& InRequest);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;
	virtual void OnTaskTimedOut() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteSetUserChatConfiguration");
	}

private:
	/** Request for updating user chat configuration. */
	FAccelByteModelsSetUserChatConfigurationRequest Request;
	
	/** Response for set user chat configuration returned from backend. */
	FAccelByteSetUserChatConfigurationResponse SetUserChatConfigurationResponse;
	
	/** Online error information in case any error happen. */
	FOnlineError OnlineError;

	/** Handler OnSetUserChatConfigurationSuccess.*/
	void OnSetUserChatConfigurationSuccess(FAccelByteSetUserChatConfigurationResponse const& Response);

	/** Handler OnSetUserChatConfigurationFailed.*/
	void OnSetUserChatConfigurationFailed(const int32 ErrorCode, const FString& ErrorMessage);
};
