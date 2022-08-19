// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "OnlineAsyncTaskAccelByte.h"

class FOnlineAsyncTaskAccelByteQueryOfferById : public FOnlineAsyncTaskAccelByte
{
public:
	FOnlineAsyncTaskAccelByteQueryOfferById(FOnlineSubsystemAccelByte* const InABSubsystem, const FUniqueNetId& InUserId, const TArray<FUniqueOfferId>& InOfferIds,
		const FOnQueryOnlineStoreOffersComplete& InDelegate);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteQueryOfferById");
	}

private:
	void HandleGetItemByIds(const TArray<FAccelByteModelsItemInfo>& Result);
	void HandleAsyncTaskError(int32 Code, FString const& ErrMsg);

	FString ErrorMsg;
	FString Language;
	TArray<FUniqueOfferId> OfferIds;
	THandler<TArray<FAccelByteModelsItemInfo>> OnSuccess;
	FErrorHandler OnError;
	FOnQueryOnlineStoreOffersComplete Delegate;
	TMap<FUniqueOfferId, FOnlineStoreOfferRef> OfferMap;
};
