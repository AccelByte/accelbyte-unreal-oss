// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"

class FOnlineAsyncTaskAccelByteGetUserPlatformLinks
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteGetUserPlatformLinks, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteGetUserPlatformLinks(
		FOnlineSubsystemAccelByte* const InABSubsystem, 
		const int32 InLocalUserNum);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;
	virtual void Finalize() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteGetUserPlatformLinks");
	}

private:
	void OnGetUserPlatformLinksSuccess(const FPagedPlatformLinks& Result);
	THandler<FPagedPlatformLinks> OnSuccessDelegate;

	void OnGetUserPlatformLinksError(int32 ErrorCode, const FString& ErrorMessage);
	FErrorHandler OnErrorDelegate;

	/* Incoming information about user 3rd party platform information */
	TArray<FPlatformLink> AccelByteModelsUserPlatformLinks = {};

	/* Incoming platform information */
	TArray<FPlatformLink> OutUserPlatformLinks = {};

	/* Success or Failure status code */
	int32 HttpStatus = 0;

	/* Error message upon failure to perform requested action */
	FString ErrorString{};
};