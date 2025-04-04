// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "OnlineSubsystemAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineGroupsInterfaceAccelByte.h"

class FOnlineAsyncTaskAccelByteGroupsCancelJoinRequest : public FOnlineAsyncTaskAccelByte, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteGroupsCancelJoinRequest, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteGroupsCancelJoinRequest(
		FOnlineSubsystemAccelByte* const InABInterface,
		const FUniqueNetId& UserIdCancelingRequest,
		const FString& InGroupId,
		const FOnGroupsRequestCompleted& InDelegate);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;
	virtual void Finalize() override;

protected:
	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteGroupsCancelJoinRequest");
	}

private:
	void OnCancelJoinRequestSuccess(const FAccelByteModelsMemberRequestGroupResponse& Result);
	THandler<FAccelByteModelsMemberRequestGroupResponse> OnSuccessDelegate;

	void OnCancelJoinRequestError(int32 ErrorCode, const FString& ErrorMessage);
	FErrorHandler OnErrorDelegate;

	/* Incoming Group Id Storage Variable */
	FString GroupId;

	/* Temporary data storage to be passed back to the requesting user or group */
	FAccelByteModelsMemberRequestGroupResponse AccelByteModelsMemberRequestGroupResponse;

	/* Generic delegate used to return Success or Failure status */
	FOnGroupsRequestCompleted Delegate;

	/* Used by Delegate to return Success or Failure status */
	FUniqueNetIdAccelByteResourcePtr UniqueNetIdAccelByteResource;

	/* Success or Failure status code */
	int32 httpStatus;

	/* Error message upon failure to perform requested action */
	FString ErrorString{};
};