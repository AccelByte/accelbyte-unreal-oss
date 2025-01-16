// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "OnlineSubsystemAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineGroupsInterfaceAccelByte.h"

class FOnlineAsyncTaskAccelByteGroupsAcceptUser
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteGroupsAcceptUser, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteGroupsAcceptUser(
		FOnlineSubsystemAccelByte* const InABInterface,
		const int32& GroupAdmin,
		const FUniqueNetId& GroupMemberUserId,
		const FString& InGroupId,
		const FOnGroupsRequestCompleted& InDelegate);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;
	virtual void Finalize() override;

protected:
	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteGroupsAcceptUser");
	}

private:
	void OnAcceptUserSuccess(const FAccelByteModelsMemberRequestGroupResponse& Result);
	THandler<FAccelByteModelsMemberRequestGroupResponse> OnSuccessDelegate;

	void OnAcceptUserError(int32 ErrorCode, const FString& ErrorMessage);
	FErrorHandler OnErrorDelegate;

	/* Incoming User ID Storage Variable */
	TSharedRef<const FUniqueNetIdAccelByteUser> MemberId;

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
