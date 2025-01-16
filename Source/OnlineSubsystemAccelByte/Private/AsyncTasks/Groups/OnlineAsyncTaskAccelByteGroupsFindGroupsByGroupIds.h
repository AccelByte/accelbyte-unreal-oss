// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "OnlineSubsystemAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineGroupsInterfaceAccelByte.h"

class FOnlineAsyncTaskAccelByteGroupsFindGroupsByGroupIds : public FOnlineAsyncTaskAccelByte, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteGroupsFindGroupsByGroupIds, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteGroupsFindGroupsByGroupIds(
		FOnlineSubsystemAccelByte* const InABInterface,
		const FUniqueNetId& InSearchingUserId,
		const TArray<FString> InGroupIds,
		const FOnGroupsRequestCompleted& InDelegate);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;
	virtual void Finalize() override;

protected:
	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteGroupsFindGroupsByGroupIds");
	}

private:
	void OnFindGroupsByGroupIdsSuccess(const FAccelByteModelsGetGroupListResponse& Result);
	THandler<FAccelByteModelsGetGroupListResponse> OnSuccessDelegate;

	void OnFindGroupsByGroupIdsError(int32 ErrorCode, const FString& ErrorMessage);
	FErrorHandler OnErrorDelegate;

	/* Temporary storage for the list of groups requested by the Query to be passed back to the requester */
	FAccelByteModelsGetGroupListResponse AccelByteModelsGetGroupListResponse;

	/* Group IDs to search for */
	TArray<FString> GroupIds;

	/* Generic delegate used to return Success or Failure status */
	FOnGroupsRequestCompleted Delegate;

	/* Used by Delegate to return Success or Failure status */
	FUniqueNetIdAccelByteResourcePtr UniqueNetIdAccelByteResource;

	/* Success or Failure status code */
	int32 httpStatus;

	/* Error message upon failure to perform requested action */
	FString ErrorString{};
};
