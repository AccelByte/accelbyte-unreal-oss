// Copyright (c) 2026 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "JsonObjectWrapper.h"
#include "OnlineUserInterfaceAccelByte.h"

class FOnlineAsyncTaskAccelByteGetCustomAttributes
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteGetCustomAttributes, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteGetCustomAttributes(FOnlineSubsystemAccelByte* const InABSubsystem, int32 InLocalUserNum, const FOnGetCustomAttributesComplete& InDelegate);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteGetCustomAttributes");
	}

private:
	/**
	 * Public custom attributes from the backend
	 */
	FJsonObjectWrapper CustomAttributes;

	/**
	 * Online user account for the user requested the task
	 */
	TSharedPtr<FUserOnlineAccountAccelByte> Account;

	/** String containing an error if one was encountered */
	FString ErrorString;

	/** Delegate to call when the get custom attributes request completes */
	FOnGetCustomAttributesComplete Delegate;

	/**
	 * Delegate handler for when getting the current user's custom attributes from the AccelByte SDK succeeds.
	 *
	 * @param Result Custom attributes as JSON object
	 */
	void OnGetCustomAttributesSuccess(const FJsonObject& Result);
	THandler<FJsonObject> OnGetCustomAttributesSuccessDelegate;

	/**
	 * Delegate handler for when getting the current user's custom attributes fails.
	 *
	 * @param ErrorCode Code returned from the backend that represents the error encountered for the request
	 * @param ErrorMessage Message from the backend that describes the error encountered
	 */
	void OnGetCustomAttributesError(int32 ErrorCode, const FString& ErrorMessage);
	FErrorHandler OnGetCustomAttributesErrorDelegate;

	/**
	 * Method to get the authenticated user's custom attributes
	 */
	void GetCustomAttributes();
};
