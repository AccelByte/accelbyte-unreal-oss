// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "OnlineAsyncTaskAccelByte.h"
#include "OnlineAsyncTaskAccelByteUtils.h"
#include "Interfaces/OnlineStoreInterfaceV2.h"

class FOnlineAsyncTaskAccelByteQueryCategories : public FOnlineAsyncTaskAccelByte, public TSelfPtr<FOnlineAsyncTaskAccelByteQueryCategories, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteQueryCategories(FOnlineSubsystemAccelByte* const InABSubsystem, const FUniqueNetId& InUserId, const FOnQueryOnlineStoreCategoriesComplete& InDelegate);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;
	virtual void Tick() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteQueryCategories");
	}

private:
	void HandleGetDescendantCategoriesSuccess(const TArray<FAccelByteModelsCategoryInfo>& AccelByteModelsCategoryInfos);
	void HandleGetRootCategorySuccess(const TArray<FAccelByteModelsCategoryInfo>& AccelByteModelsCategoryInfos);
	void HandleAsyncTaskError(int32 Code, FString const& ErrMsg);

	FString Language;
	THandler<TArray<FAccelByteModelsCategoryInfo>> OnGetRootCategoriesSuccess;
	THandler<TArray<FAccelByteModelsCategoryInfo>> OnGetDescendantCategoriesSuccess;
	FErrorHandler OnError;
	FOnQueryOnlineStoreCategoriesComplete Delegate;
	// Key is root path
	TMap<FString, FOnlineStoreCategory> CategoryMap;
	int32 TaskLeft = -1;

	FString ErrorMsg;
};
