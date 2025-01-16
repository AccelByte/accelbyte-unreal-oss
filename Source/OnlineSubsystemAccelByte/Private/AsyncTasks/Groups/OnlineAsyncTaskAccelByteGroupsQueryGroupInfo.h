// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "OnlineSubsystemAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "Interfaces/OnlineGroupsInterface.h"

class FOnlineAsyncTaskAccelByteGroupsQueryGroupInfo : public FOnlineAsyncTaskAccelByte, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteGroupsQueryGroupInfo, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteGroupsQueryGroupInfo(
		FOnlineSubsystemAccelByte* const InABInterface,
		const FUniqueNetId& ContextUserId,
		const FString& InGroupId,
		const FOnGroupsRequestCompleted& InDelegate);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;
	virtual void Finalize() override;

protected:
	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteGroupsQueryGroupInfo");
	}

private:
	void OnQueryGroupInfoSuccess(const FAccelByteModelsGroupInformation& Result);
	THandler<FAccelByteModelsGroupInformation> OnSuccessDelegate;

	void OnQueryGroupInfoError(int32 ErrorCode, const FString& ErrorMessage);
	FErrorHandler OnErrorDelegate;

	/* Incoming content specifications to query */
	FAccelByteModelsLimitOffsetRequest RequestContent;

	/* Incoming Group ID to to query */
	FString GroupId;

	/* Incoming information to change within the group */
	FAccelByteModelsGroupInformation AccelByteModelsGroupInformation;

	/* Generic delegate used to return Success or Failure status */
	FOnGroupsRequestCompleted Delegate;

	/* Used by Delegate to return Success or Failure status */
	FUniqueNetIdAccelByteResourcePtr UniqueNetIdAccelByteResource;

	/* Success or Failure status code */
	int32 httpStatus;

	/* Error message upon failure to perform requested action */
	FString ErrorString{};
};
