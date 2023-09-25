// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "OnlineSubsystemAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"

class FOnlineAsyncTaskAccelByteGroupsCreateGroup
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteGroupsCreateGroup, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteGroupsCreateGroup(
		FOnlineSubsystemAccelByte* const InABInterface,
		const FUniqueNetId& InContextUserId,
		const FAccelByteGroupsInfo& InGroupInfo,
		const FOnGroupsRequestCompleted& InDelegate);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;
	virtual void Finalize() override;

protected:
	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteGroupsCreateGroup");
	}

private:
	void OnCreateGroupSuccess(const FAccelByteModelsGroupInformation& Result);
	THandler<FAccelByteModelsGroupInformation> OnSuccessDelegate;

	void OnCreateGroupError(int32 ErrorCode, const FString& ErrorMessage);
	FErrorHandler OnErrorDelegate;

	/* Incoming information to change within the group */
	FAccelByteModelsGroupInformation AccelByteModelsGroupInformation;

	/* Incoming Group Information Storage Variable */
	FAccelByteGroupsInfo GroupInfo;

	/* Generic delegate used to return Success or Failure status */
	FOnGroupsRequestCompleted Delegate;

	/* Used by Delegate to return Success or Failure status */
	FUniqueNetIdAccelByteResourcePtr UniqueNetIdAccelByteResource;

	/* Success or Failure status code */
	int32 httpStatus;

	/* Error message upon failure to perform requested action */
	FString ErrorString{};
};
