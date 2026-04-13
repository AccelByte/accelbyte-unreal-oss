// Copyright (c) 2026 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "JsonObjectWrapper.h"
#include "OnlineUserInterfaceAccelByte.h"

class FOnlineAsyncTaskAccelByteUpdateCustomAttributes
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteUpdateCustomAttributes, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteUpdateCustomAttributes(FOnlineSubsystemAccelByte* const InABSubsystem, int32 InLocalUserNum, const TSharedRef<FJsonObject>& InCustomAttributes, const FOnUpdateCustomAttributesComplete& InDelegate);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteUpdateCustomAttributes");
	}

private:
	/** Custom attributes to update (thread-safe shared reference) */
	TSharedRef<FJsonObject> CustomAttributesRequest;

	/** Updated custom attributes from the backend */
	FJsonObjectWrapper UpdatedCustomAttributes;

	/** Online user account for the user requested the task */
	TSharedPtr<FUserOnlineAccountAccelByte> Account;

	/** String containing an error if one was encountered */
	FString ErrorString;

	/** Delegate to call when the update custom attributes request completes */
	FOnUpdateCustomAttributesComplete Delegate;

	void OnUpdateCustomAttributesSuccess(const FJsonObject& Result);
	THandler<FJsonObject> OnUpdateCustomAttributesSuccessDelegate;

	void OnUpdateCustomAttributesError(int32 ErrorCode, const FString& ErrorMessage);
	FErrorHandler OnUpdateCustomAttributesErrorDelegate;

	void UpdateCustomAttributes();
};
