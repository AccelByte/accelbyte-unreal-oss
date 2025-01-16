// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "OnlineSubsystemAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineGroupsInterfaceAccelByte.h"

class FOnlineAsyncTaskAccelByteGroupsQueryGroupInvites
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteGroupsQueryGroupInvites, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteGroupsQueryGroupInvites(
		FOnlineSubsystemAccelByte* const InABInterface,
		const FUniqueNetId& ContextUserId,
		const FAccelByteModelsLimitOffsetRequest& InRequestContent,
		const FOnGroupsRequestCompleted& InDelegate);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;
	virtual void Finalize() override;

protected:
	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteGroupsQueryGroupInvites");
	}

private:
	void OnQueryGroupInvitesSuccess(const FAccelByteModelsGetMemberRequestsListResponse& Result);
	THandler<FAccelByteModelsGetMemberRequestsListResponse> OnSuccessDelegate;

	void OnQueryGroupInvitesError(int32 ErrorCode, const FString& ErrorMessage);
	FErrorHandler OnErrorDelegate;

	/* Incoming content specifications to query */
	FAccelByteModelsLimitOffsetRequest RequestContent;

	/* Temporary storage for the result of the query to be passed back to the requester */
	FAccelByteModelsGetMemberRequestsListResponse AccelByteModelsGetMemberRequestsListResponse;

	/* Generic delegate used to return Success or Failure status */
	FOnGroupsRequestCompleted Delegate;

	/* Used by Delegate to return Success or Failure status */
	FUniqueNetIdAccelByteResourcePtr UniqueNetIdAccelByteResource;

	/* Success or Failure status code */
	int32 httpStatus;

	/* Error message upon failure to perform requested action */
	FString ErrorString{};
};
