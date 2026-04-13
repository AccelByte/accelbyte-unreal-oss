// Copyright (c) 2026 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "JsonObjectWrapper.h"
#include "OnlineUserInterfaceAccelByte.h"

class FOnlineAsyncTaskAccelByteUpdatePrivateCustomAttributes
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteUpdatePrivateCustomAttributes, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteUpdatePrivateCustomAttributes(FOnlineSubsystemAccelByte* const InABSubsystem, int32 InLocalUserNum, const TSharedRef<FJsonObject>& InPrivateAttributes, const FOnUpdatePrivateCustomAttributesComplete& InDelegate);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteUpdatePrivateCustomAttributes");
	}

private:
	TSharedRef<FJsonObject> PrivateAttributesRequest;
	FJsonObjectWrapper UpdatedPrivateCustomAttributes;
	TSharedPtr<FUserOnlineAccountAccelByte> Account;
	FString ErrorString;
	FOnUpdatePrivateCustomAttributesComplete Delegate;

	void OnUpdatePrivateCustomAttributesSuccess(const FJsonObjectWrapper& Result);
	THandler<FJsonObjectWrapper> OnUpdatePrivateCustomAttributesSuccessDelegate;

	void OnUpdatePrivateCustomAttributesError(int32 ErrorCode, const FString& ErrorMessage);
	FErrorHandler OnUpdatePrivateCustomAttributesErrorDelegate;

	void UpdatePrivateCustomAttributes();
};
