// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "OnlineSubsystemAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"

typedef TSharedPtr<FAccelByteModelsGroupInformation> FAccelByteModelsGroupInformationPtr;

class FOnlineAsyncTaskAccelByteGroupsGetGroup : public FOnlineAsyncTaskAccelByte, public TSelfPtr<FOnlineAsyncTaskAccelByteGroupsGetGroup, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteGroupsGetGroup(
		FOnlineSubsystemAccelByte* const InABInterface,
		const FUniqueNetId& InContextUserId,
		const FUniqueNetId& InGroupId,
		const FOnFindGroupsCompleted& InDelegate);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;
	virtual void Finalize() override;

protected:
	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteGroupsGetGroup");
	}

private:
	void OnGetGroupSuccess(const FAccelByteModelsGroupInformation& Result);
	THandler<FAccelByteModelsGroupInformation> OnSuccessDelegate;

	void OnGetGroupError(int32 ErrorCode, const FString& ErrorMessage);
	FErrorHandler OnErrorDelegate;

	FUniqueNetIdRef GroupId;
	FAccelByteModelsGroupInformation AccelByteModelsGroupInformation;
	FOnFindGroupsCompleted Delegate;
	FUniqueNetIdAccelByteResourcePtr UniqueNetIdAccelByteResource;
	int32 httpStatus;

	FString ErrorString{};
};
