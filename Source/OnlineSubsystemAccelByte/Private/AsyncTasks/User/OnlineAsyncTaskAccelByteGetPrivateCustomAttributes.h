// Copyright (c) 2026 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "JsonObjectWrapper.h"
#include "OnlineUserInterfaceAccelByte.h"

class FOnlineAsyncTaskAccelByteGetPrivateCustomAttributes
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteGetPrivateCustomAttributes, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteGetPrivateCustomAttributes(FOnlineSubsystemAccelByte* const InABSubsystem, int32 InLocalUserNum, const FOnGetPrivateCustomAttributesComplete& InDelegate);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteGetPrivateCustomAttributes");
	}

private:
	FJsonObjectWrapper PrivateCustomAttributes;
	TSharedPtr<FUserOnlineAccountAccelByte> Account;
	FString ErrorString;
	FOnGetPrivateCustomAttributesComplete Delegate;

	void OnGetPrivateCustomAttributesSuccess(const FJsonObjectWrapper& Result);
	THandler<FJsonObjectWrapper> OnGetPrivateCustomAttributesSuccessDelegate;

	void OnGetPrivateCustomAttributesError(int32 ErrorCode, const FString& ErrorMessage);
	FErrorHandler OnGetPrivateCustomAttributesErrorDelegate;

	void GetPrivateCustomAttributes();
};
