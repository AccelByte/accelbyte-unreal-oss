// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "OnlineAsyncTaskAccelByte.h"

class FOnlineAsyncTaskAccelByteQueryEntitlements : public FOnlineAsyncTaskAccelByte
{
public:
	FOnlineAsyncTaskAccelByteQueryEntitlements(FOnlineSubsystemAccelByte* const InABSubsystem, const FUniqueNetId& InUserId, const FString& InNamespace, const FPagedQuery& InPage);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteQueryEntitlements");
	}

private:
	void QueryEntitlement(int32 Offset, int32 Limit);
	void HandleQueryEntitlementSuccess(FAccelByteModelsEntitlementPagingSlicedResult const& Result);
	void HandleQueryEntitlementError(int32 Code, FString const& ErrMsg);

	FString Namespace;
	FPagedQuery PagedQuery;
	FString ErrorMessage;
};
