// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineSessionInterfaceV2AccelByte.h"

class FOnlineAsyncTaskAccelByteUpdateMemberSessionV2Storage
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteUpdateMemberSessionV2Storage, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteUpdateMemberSessionV2Storage(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FNamedOnlineSession* InSessionToUpdate, FJsonObjectWrapper const& InData);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteUpdateMemberSessionV2Storage");
	}

private:
	/** Session pointer which the storage will be updated. */
	const FNamedOnlineSession* SessionToUpdate;

	/** Data for updating leader session storage. */
	FJsonObjectWrapper Data;

	/** AccelByte user id of user performing this task. */
	FString AccelByteUserId;

	/** Text representing the error that occurred in the request, if one did. */
	FText ErrorText;

	/** Online error information in case any error happen. */
	FOnlineError OnlineError;

	/** Handler for successful updating member session storage. */
	void OnUpdateMemberStorageSuccess(const FJsonObjectWrapper& ResponseData);

	/**	Handler for failed updating member session storage */
	void OnUpdateMemberStorageError(int32 ErrorCode, const FString& ErrorMessage);
};
