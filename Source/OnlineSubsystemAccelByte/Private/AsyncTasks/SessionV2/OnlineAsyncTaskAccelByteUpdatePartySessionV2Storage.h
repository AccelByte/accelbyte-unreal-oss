// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineSessionInterfaceV2AccelByte.h"

class FOnlineAsyncTaskAccelByteUpdatePartySessionV2Storage
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteUpdatePartySessionV2Storage, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteUpdatePartySessionV2Storage(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FNamedOnlineSession* InSessionToUpdate, FJsonObjectWrapper const& InData);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteUpdatePartySessionV2Storage");
	}

private:
	/** Party session pointer which the storage will be updated. */
	const FNamedOnlineSession* SessionToUpdate;

	/** Data for updating party session storage. */
	FJsonObjectWrapper Data;

	/** AccelByte user id of user performing this task. */
	FString AccelByteUserId;

	/** String representing the error that occurred in the request, if one did. */
	FString ErrorStr;

	/** Online error information in case any error happen. */
	FOnlineError OnlineError;

	/** Handler for successful updating party session storage. */
	THandler<FJsonObjectWrapper> OnUpdateStorageSuccessDelegate;
	void OnUpdatePartyStorageSuccess(const FJsonObjectWrapper& ResponseData);

	/**	Handler for failed updating party session storage */
	FErrorHandler OnUpdateStorageErrorDelegate;
	void OnUpdatePartyStorageError(int32 ErrorCode, const FString& ErrorMessage);
};
