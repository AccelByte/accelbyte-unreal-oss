// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "OnlineSubsystemAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"

class FOnlineAsyncTaskAccelByteGroupsDeclineInvite
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteGroupsDeclineInvite, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteGroupsDeclineInvite(
		FOnlineSubsystemAccelByte* const InABInterface,
		const FUniqueNetId& UserIdRejectingInvite,
		const FAccelByteGroupsInfo& InGroupInfo,
		const FOnGroupsRequestCompleted& InDelegate);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;
	virtual void Finalize() override;

protected:
	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteGroupsDeclineInvite");
	}

private:
	void OnDeclineInviteSuccess(const FAccelByteModelsMemberRequestGroupResponse& Result);
	THandler<FAccelByteModelsMemberRequestGroupResponse> OnSuccessDelegate;

	void OnDeclineInviteError(int32 ErrorCode, const FString& ErrorMessage);
	FErrorHandler OnErrorDelegate;

	FAccelByteGroupsInfo GroupInfo;
	FAccelByteModelsMemberRequestGroupResponse AccelByteModelsMemberRequestGroupResponse;
	FOnGroupsRequestCompleted Delegate;
	FUniqueNetIdAccelByteResourcePtr UniqueNetIdAccelByteResource;
	int32 httpStatus;

	FString ErrorString{};
};
