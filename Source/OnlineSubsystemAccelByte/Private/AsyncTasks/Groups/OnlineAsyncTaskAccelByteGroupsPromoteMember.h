// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "OnlineSubsystemAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"

class FOnlineAsyncTaskAccelByteGroupsPromoteMember
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteGroupsPromoteMember, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteGroupsPromoteMember(
		FOnlineSubsystemAccelByte* const InABInterface,
		const int32& GroupAdmin,
		const FUniqueNetId& GroupMemberUserId,
		const FAccelByteGroupsInfo& InGroupInfo,
		const FString& MemberRoleId,
		const FOnGroupsRequestCompleted& InDelegate);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;
	virtual void Finalize() override;

protected:
	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteGroupsPromoteMember");
	}

private:
	void OnPromoteMemberSuccess(const FAccelByteModelsGetUserGroupInfoResponse& Result);
	THandler<FAccelByteModelsGetUserGroupInfoResponse> OnSuccessDelegate;

	void OnPromoteMemberError(int32 ErrorCode, const FString& ErrorMessage);
	FErrorHandler OnErrorDelegate;

	TSharedRef<const FUniqueNetIdAccelByteUser> MemberId;
	FAccelByteGroupsInfo GroupInfo;
	FString RoleId;
	FAccelByteModelsGetUserGroupInfoResponse AccelByteModelsGetUserGroupInfoResponse;
	FOnGroupsRequestCompleted Delegate;
	FUniqueNetIdAccelByteResourcePtr UniqueNetIdAccelByteResource;
	int32 httpStatus;

	FString ErrorString{};
};