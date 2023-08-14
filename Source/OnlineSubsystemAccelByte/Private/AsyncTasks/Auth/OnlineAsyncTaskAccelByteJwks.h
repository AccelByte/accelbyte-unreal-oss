// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"

DECLARE_DELEGATE_OneParam(FOnJwksCompleted, const FJwkSet&);

/**
 * Get JwkSet to DS service.
 *
 * For dedicated sessions.
 */
class FOnlineAsyncTaskAccelByteJwks : public FOnlineAsyncTaskAccelByte, public TSelfPtr<FOnlineAsyncTaskAccelByteJwks, ESPMode::ThreadSafe>
{
public:
	/** Constructor to setup the Dedicated Server task */
	FOnlineAsyncTaskAccelByteJwks(FOnlineSubsystemAccelByte* const InABInterface, const FOnJwksCompleted& InDelegate);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteJwks");
	}

private:
	FJwkSet JwkSet;
	FOnJwksCompleted Delegate;

	void OnJwksSuccess(const FJwkSet& Response);
	void OnJwksError(int32 ErrorCode, const FString& ErrorMessage);
};
