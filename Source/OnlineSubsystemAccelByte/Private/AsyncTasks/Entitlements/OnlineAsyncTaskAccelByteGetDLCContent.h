// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineEntitlementsInterfaceAccelByte.h"

/**
 * Async task to retrieve user DLC reward contents for a given platform type.
 */
class FOnlineAsyncTaskAccelByteGetDLCContent
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteGetDLCContent, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteGetDLCContent(FOnlineSubsystemAccelByte* const InABInterface
		, const FUniqueNetId& InUserId
		, EAccelByteDLCType InDLCType
		, bool bInIncludeAllNamespaces);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteGetDLCContent");
	}

private:

	EAccelByteDLCType DLCType;
	bool bIncludeAllNamespaces;
	FOnlineError OnlineError;
	TArray<FAccelByteModelsSimpleUserDLCRewardContent> Contents;

	void OnGetDLCContentSuccess(const FAccelByteModelsSimpleUserDLCRewardContentsResponse& Response);
	void OnGetDLCContentFailed(int32 ErrorCode, const FString& ErrorMessage);
};
