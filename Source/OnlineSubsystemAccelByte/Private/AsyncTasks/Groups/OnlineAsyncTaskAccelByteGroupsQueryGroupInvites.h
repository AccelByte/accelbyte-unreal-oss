// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "OnlineSubsystemAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"

class FOnlineAsyncTaskAccelByteGroupsQueryGroupInvites
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteGroupsQueryGroupInvites, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteGroupsQueryGroupInvites(
		FOnlineSubsystemAccelByte* const InABInterface,
		const FUniqueNetId& ContextUserId,
		const FAccelByteModelsLimitOffsetRequest& InRequestContent,
		const FAccelByteGroupsInfo& InGroupInfo,
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

	FAccelByteModelsLimitOffsetRequest RequestContent;
	FAccelByteGroupsInfo GroupInfo;
	FAccelByteModelsGetMemberRequestsListResponse AccelByteModelsGetMemberRequestsListResponse;
	FOnGroupsRequestCompleted Delegate;
	FUniqueNetIdAccelByteResourcePtr UniqueNetIdAccelByteResource;
	int32 httpStatus;

	FString ErrorString{};
};
