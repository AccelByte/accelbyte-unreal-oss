// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "OnlineSubsystemAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"

class FOnlineAsyncTaskAccelByteGroupsRemoveMember
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteGroupsRemoveMember, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteGroupsRemoveMember(
		FOnlineSubsystemAccelByte* const InABInterface,
		const int32& GroupAdmin,
		const FUniqueNetId& GroupMemberUserId,
		const FAccelByteGroupsInfo& InGroupInfo,
		const FOnGroupsRequestCompleted& InDelegate);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;
	virtual void Finalize() override;

protected:
	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteGroupsRemoveMember");
	}

private:
	void OnRemoveMemberSuccess(const FAccelByteModelsKickGroupMemberResponse& Result);
	THandler<FAccelByteModelsKickGroupMemberResponse> OnSuccessDelegate;

	void OnRemoveMemberError(int32 ErrorCode, const FString& ErrorMessage);
	FErrorHandler OnErrorDelegate;

	TSharedRef<const FUniqueNetIdAccelByteUser> MemberId;
	FAccelByteGroupsInfo GroupInfo;
	FAccelByteModelsKickGroupMemberResponse AccelByteModelsKickGroupMemberResponse;
	FOnGroupsRequestCompleted Delegate;
	FUniqueNetIdAccelByteResourcePtr UniqueNetIdAccelByteResource;
	int32 httpStatus;

	FString ErrorString{};
};