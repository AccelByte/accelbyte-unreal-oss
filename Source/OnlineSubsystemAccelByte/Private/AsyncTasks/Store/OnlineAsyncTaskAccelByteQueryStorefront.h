// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "Models/AccelByteEcommerceModels.h"
#include "OnlineStoreInterfaceV2AccelByte.h"

class FOnlineAsyncTaskAccelByteQueryStorefront
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteQueryStorefront, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteQueryStorefront(FOnlineSubsystemAccelByte* const InABSubsystem, const FUniqueNetId& InUserId, const FString& InStoreId, const FString& InViewId, const FString& InRegion, const EAccelBytePlatformMapping& Platform, const FOnQueryStorefrontComplete& InDelegate);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:
	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteQueryStorefront");
	}

private:
	THandler<TArray<FAccelByteModelsViewInfo>> OnLoadDisplaysSuccessDelegate;
	void OnLoadDisplaysSuccess(const TArray<FAccelByteModelsViewInfo>& Views);

	THandler<TArray<FAccelByteModelsSectionInfo>> OnListActiveSectionsSuccessDelegate;
	void OnListActiveSectionsSuccess(bool bSectionsWasSuccessful, const TArray<FString>& SectionIds, const TArray<FUniqueOfferId>& OfferIds, const FString& Error);

	THandler<FAccelByteModelsItemMappingsResponse> OnLoadItemMappingsSuccessDelegate;
	void OnLoadItemMappingsSuccess(const FAccelByteModelsItemMappingsResponse& MappingsResponse);

	FErrorHandler OnLoadItemMappingsErrorDelegate;
	void HandleLoadItemMappingsErrorError(int32 Code, FString const& ErrMsg);
	
	FErrorHandler OnQueryErrorDelegate;
	void HandleQueryError(int32 Code, FString const& ErrMsg);

	void HandleStepComplete();

	std::atomic<bool> bLoadedDisplays;
	std::atomic<bool> bLoadedItemMappings;
	std::atomic<bool> bLoadedSections;

	FString StoreId;
	FString ViewId;
	FString Language;
	FString Region;
	EAccelBytePlatformMapping Platform;
	FOnQueryStorefrontComplete Delegate;

	FString ErrorMsg;
	FOnQueryActiveSectionsComplete OnLoadSections;

	TMap<FString, TSharedRef<FAccelByteModelsViewInfo, ESPMode::ThreadSafe>> DisplayMap;
	TMap<FString, TSharedRef<FAccelByteModelsItemMapping, ESPMode::ThreadSafe>> ItemMappings;
	TArray<FString> SectionIds;
	TArray<FString> OfferIds;
};
