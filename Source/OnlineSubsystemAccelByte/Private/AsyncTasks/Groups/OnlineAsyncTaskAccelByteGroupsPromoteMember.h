// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "OnlineSubsystemAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "Interfaces/OnlineGroupsInterface.h"

class FOnlineAsyncTaskAccelByteGroupsPromoteMember
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteGroupsPromoteMember, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteGroupsPromoteMember(
		FOnlineSubsystemAccelByte* const InABInterface,
		const int32& GroupAdmin,
		const FUniqueNetId& GroupMemberUserId,
		const FString& InGroupId,
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

	/* Incoming User ID Storage Variable */
	TSharedRef<const FUniqueNetIdAccelByteUser> MemberId;

	/* Incoming Group Id Storage Variable */
	FString GroupId;

	/* When promoting member, this will be their new Role ID */
	FString RoleId;

	/* Temporary storage for the response of promoting a member to be sent back to the requester */
	FAccelByteModelsGetUserGroupInfoResponse AccelByteModelsGetUserGroupInfoResponse;

	/* Generic delegate used to return Success or Failure status */
	FOnGroupsRequestCompleted Delegate;

	/* Used by Delegate to return Success or Failure status */
	FUniqueNetIdAccelByteResourcePtr UniqueNetIdAccelByteResource;

	/* Success or Failure status code */
	int32 httpStatus;

	/* Error message upon failure to perform requested action */
	FString ErrorString{};
};
