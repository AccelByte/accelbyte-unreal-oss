// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"

DECLARE_DELEGATE_TwoParams(FOnAuthUserCompleted, bool /*bWasSuccessful*/, const FString& /*UserId*/);

/**
 * Ban User to DS service.
 *
 * For dedicated sessions, this requires the permission "ADMIN:NAMESPACE:{namespace}:BAN:USER:{userId} [READ]".
 */
class FOnlineAsyncTaskAccelByteAuthUser : public FOnlineAsyncTaskAccelByte, public TSelfPtr<FOnlineAsyncTaskAccelByteAuthUser, ESPMode::ThreadSafe>
{
public:
	/** Constructor to setup the Dedicated Server task */
	FOnlineAsyncTaskAccelByteAuthUser(FOnlineSubsystemAccelByte* const InABInterface, const FString& InUserId, const FOnAuthUserCompleted& InDelegate);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteAuthUser");
	}

private:
	FString UserId;

	bool bRequestResult;
	FOnAuthUserCompleted Delegate;

	void OnAuthSuccess(const FGetUserBansResponse& Result);
	void OnAuthError(int32 ErrorCode, const FString& ErrorMessage);
};
