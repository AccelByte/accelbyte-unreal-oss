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
		const FString& InGroupId,
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

	/* Incoming User ID Storage Variable */
	TSharedRef<const FUniqueNetIdAccelByteUser> MemberId;

	/* Incoming Group Id Storage Variable */
	FString GroupId;

	/* Temporary storage of the return kick response to be sent back to the requester */
	FAccelByteModelsKickGroupMemberResponse AccelByteModelsKickGroupMemberResponse;

	/* Generic delegate used to return Success or Failure status */
	FOnGroupsRequestCompleted Delegate;

	/* Used by Delegate to return Success or Failure status */
	FUniqueNetIdAccelByteResourcePtr UniqueNetIdAccelByteResource;

	/* Success or Failure status code */
	int32 httpStatus;

	/* Error message upon failure to perform requested action */
	FString ErrorString{};
};