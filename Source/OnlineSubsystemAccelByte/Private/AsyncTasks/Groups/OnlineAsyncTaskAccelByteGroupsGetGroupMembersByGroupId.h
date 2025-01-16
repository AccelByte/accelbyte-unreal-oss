// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "OnlineSubsystemAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineGroupsInterfaceAccelByte.h"

class FOnlineAsyncTaskAccelByteGroupsGetGroupMembersByGroupId : public FOnlineAsyncTaskAccelByte, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteGroupsGetGroupMembersByGroupId, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteGroupsGetGroupMembersByGroupId(
		FOnlineSubsystemAccelByte* const InABInterface,
		const FUniqueNetId& InSearchingUserId,
		const FString& InGroupId,
		const FAccelByteModelsGetGroupMembersListByGroupIdRequest& InRequestedContent,
		const FOnGroupsRequestCompleted& InDelegate);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;
	virtual void Finalize() override;

protected:
	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteGroupsGetGroupMembersByGroupId");
	}

private:
	void OnGetGroupMembersByGroupIdsSuccess(const FAccelByteModelsGetGroupMemberListResponse& Result);
	THandler<FAccelByteModelsGetGroupMemberListResponse> OnSuccessDelegate;

	void OnGetGroupMembersByGroupIdsError(int32 ErrorCode, const FString& ErrorMessage);
	FErrorHandler OnErrorDelegate;

	/* Temporary storage of requested members by the query to be sent back to the requester */
	FAccelByteModelsGetGroupMemberListResponse AccelByteModelsGetGroupMemberListResponse;

	/* Incoming Group ID to to query */
	FString GroupId;

	/* Requested query information */
	FAccelByteModelsGetGroupMembersListByGroupIdRequest RequestedContent;

	/* Generic delegate used to return Success or Failure status */
	FOnGroupsRequestCompleted Delegate;

	/* Used by Delegate to return Success or Failure status */
	FUniqueNetIdAccelByteResourcePtr UniqueNetIdAccelByteResource;

	/* Success or Failure status code */
	int32 httpStatus;

	/* Error message upon failure to perform requested action */
	FString ErrorString{};
};
