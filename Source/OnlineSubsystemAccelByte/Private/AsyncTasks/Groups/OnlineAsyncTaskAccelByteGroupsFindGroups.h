// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "OnlineSubsystemAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineGroupsInterfaceAccelByte.h"

class FOnlineAsyncTaskAccelByteGroupsFindGroups : public FOnlineAsyncTaskAccelByte, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteGroupsFindGroups, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteGroupsFindGroups(
		FOnlineSubsystemAccelByte* const InABInterface,
		const FUniqueNetId& InSearchingUserId,
		const FAccelByteModelsGetGroupListRequest& InRequestedContent,
		const FOnGroupsRequestCompleted& InDelegate);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;
	virtual void Finalize() override;

protected:
	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteGroupsFindGroups");
	}

private:
	void OnFindGroupsSuccess(const FAccelByteModelsGetGroupListResponse& Result);
	THandler<FAccelByteModelsGetGroupListResponse> OnSuccessDelegate;

	void OnFindGroupsError(int32 ErrorCode, const FString& ErrorMessage);
	FErrorHandler OnErrorDelegate;

	/* Temporary storage for the list of groups requested by the Query to be passed back to the requester */
	FAccelByteModelsGetGroupListResponse AccelByteModelsGetGroupListResponse;

	/* Requested query information */
	FAccelByteModelsGetGroupListRequest RequestedContent;

	/* Generic delegate used to return Success or Failure status */
	FOnGroupsRequestCompleted Delegate;

	/* Used by Delegate to return Success or Failure status */
	FUniqueNetIdAccelByteResourcePtr UniqueNetIdAccelByteResource;

	/* Success or Failure status code */
	int32 httpStatus;

	/* Error message upon failure to perform requested action */
	FString ErrorString{};
};
