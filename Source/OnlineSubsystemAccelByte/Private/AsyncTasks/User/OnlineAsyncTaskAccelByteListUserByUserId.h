// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "Models/AccelByteUserModels.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "OnlineUserInterfaceAccelByte.h"

class FOnlineAsyncTaskAccelByteListUserByUserId
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteListUserByUserId, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteListUserByUserId(FOnlineSubsystemAccelByte* const InABInterface, const int32 InLocalUserNum,
		const TArray<FString>& InUserIds);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FFOnlineAsyncTaskAccelByteListUserByUserId");
	}

private: 
	THandler<FListUserDataResponse> OnListUserDataSuccess; 
	void HandleListUsersByUserId(const FListUserDataResponse& Result);
	
	FErrorHandler OnError;
	void HandleAsyncTaskError(int32 Code, const FString& ErrMsg);

	int32 LocalUserNum;
	FString AccelByteUserId;
	TArray<FString> UserIds{}; 
	FListUserDataResponse ListUserDataResult; 
	FOnlineError OnlineError;
};
