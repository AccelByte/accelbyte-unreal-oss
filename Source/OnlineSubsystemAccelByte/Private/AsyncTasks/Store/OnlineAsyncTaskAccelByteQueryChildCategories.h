// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "Interfaces/OnlineStoreInterfaceV2.h"

class FOnlineAsyncTaskAccelByteQueryChildCategories : public FOnlineAsyncTaskAccelByte, public TSelfPtr<FOnlineAsyncTaskAccelByteQueryChildCategories, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteQueryChildCategories(FOnlineSubsystemAccelByte* const InABSubsystem, const FUniqueNetId& InUserId, const FString& InCategoryPath, const FOnQueryOnlineStoreCategoriesComplete& InDelegate);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteQueryChildCategories");
	}

private:
	void HandleGetChildCategoriesSuccess(const TArray<FAccelByteModelsCategoryInfo>& AccelByteModelsCategoryInfos);
	void HandleAsyncTaskError(int32 Code, FString const& ErrMsg);

	FString CategoryPath;
	FOnQueryOnlineStoreCategoriesComplete Delegate;
	FString ErrorMsg;
	FString Language;

	THandler<TArray<FAccelByteModelsCategoryInfo>> OnGetChildCategoriesSuccess;
	FErrorHandler OnError;
	TMap<FString, FOnlineStoreCategory> CategoryMap;
};
