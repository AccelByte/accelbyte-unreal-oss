// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"

class FOnlineAsyncTaskAccelByteQueryActiveSections
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteQueryActiveSections, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteQueryActiveSections(FOnlineSubsystemAccelByte* const InABSubsystem, const FUniqueNetId& InUserId, const FString& InStoreId, const FString& InViewId, const FString& InRegion, const FOnQueryActiveSectionsComplete& InDelegate);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:
	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteQueryActiveSections");
	}

private:
	THandler<TArray<FAccelByteModelsSectionInfo>> OnListActiveSectionContentsSuccessDelegate;
	void OnListActiveSectionContentsSuccess(const TArray<FAccelByteModelsSectionInfo>& InSections);

	FErrorHandler OnListActiveSectionContentsErrorDelegate;
	void OnListActiveSectionContentsError(int32 Code, FString const& ErrMsg);

	FString StoreId;
	FString ViewId;
	FString Language;
	FString Region;
	FOnQueryActiveSectionsComplete Delegate;

	FString ErrorMsg;
	TMap<FUniqueOfferId, FOnlineStoreOfferAccelByteRef> OfferMap{};
	TMultiMap<FString, FOnlineStoreOfferAccelByteRef> OffersBySection;
	TMap<FString, TSharedRef<FAccelByteModelsSectionInfo, ESPMode::ThreadSafe>> Sections;
};
