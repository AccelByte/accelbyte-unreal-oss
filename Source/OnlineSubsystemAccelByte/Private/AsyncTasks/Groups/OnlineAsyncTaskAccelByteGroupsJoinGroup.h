// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "OnlineSubsystemAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"

class FOnlineAsyncTaskAccelByteGroupsJoinGroup : public FOnlineAsyncTaskAccelByte, public TSelfPtr<FOnlineAsyncTaskAccelByteGroupsJoinGroup, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteGroupsJoinGroup(
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
		return TEXT("FOnlineAsyncTaskAccelByteGroupsJoinGroup");
	}

private:
	void OnJoinGroupSuccess(const FAccelByteModelsJoinGroupResponse& Result);
	THandler<FAccelByteModelsJoinGroupResponse> OnSuccessDelegate;

	void OnJoinGroupError(int32 ErrorCode, const FString& ErrorMessage);
	FErrorHandler OnErrorDelegate;

	FAccelByteGroupsInfo GroupInfo;
	FAccelByteModelsJoinGroupResponse AccelByteModelsJoinGroupResponse;
	FOnGroupsRequestCompleted Delegate;
	FUniqueNetIdAccelByteResourcePtr UniqueNetIdAccelByteResource;
	int32 httpStatus;

	FString ErrorString{};
};
