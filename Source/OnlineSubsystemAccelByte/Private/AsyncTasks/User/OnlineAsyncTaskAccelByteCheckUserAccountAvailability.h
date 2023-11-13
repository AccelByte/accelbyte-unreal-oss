// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"  
#include "OnlineUserInterfaceAccelByte.h"

class FOnlineAsyncTaskAccelByteCheckUserAccountAvailability
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteCheckUserAccountAvailability, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteCheckUserAccountAvailability(FOnlineSubsystemAccelByte* const InABSubsystem, const FUniqueNetId& InUserId, const FString& DisplayName);

	virtual void Initialize() override; 
	virtual void TriggerDelegates() override;

protected:
	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteCheckUserAccountAvailability");
	}

private:
	bool bUserExisted;
	
	// Endpoint Handlers
	void HandleSuccess();
	void HandleError(int32 ErrorCode, const FString& ErrorMessage);

	// Error Information
	int32 ErrorCode;
	FString ErrorMessage; 
	
	// Input Variables 
	FString DisplayName; 
};
