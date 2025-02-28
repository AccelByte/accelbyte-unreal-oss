// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteQueryStorefront.h"

#include "OnlineAsyncTaskAccelByteQueryActiveSections.h"

FOnlineAsyncTaskAccelByteQueryStorefront::FOnlineAsyncTaskAccelByteQueryStorefront(FOnlineSubsystemAccelByte* const InABSubsystem, const FUniqueNetId& InUserId, const FString& InStoreId, const FString& InViewId, const FString& InRegion, const EAccelBytePlatformMapping& InPlatform, const FOnQueryStorefrontComplete& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABSubsystem)
	, StoreId(InStoreId)
	, ViewId(InViewId)
	, Language(InABSubsystem->GetLanguage())
	, Region(InRegion)
	, Platform(InPlatform)
	, Delegate(InDelegate)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InUserId);
}

void FOnlineAsyncTaskAccelByteQueryStorefront::Initialize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	FOnlineAsyncTaskAccelByte::Initialize();

	OnQueryErrorDelegate = AccelByte::TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteQueryStorefront::HandleQueryError);
	OnLoadDisplaysSuccessDelegate = AccelByte::TDelegateUtils<THandler<TArray<FAccelByteModelsViewInfo>>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteQueryStorefront::OnLoadDisplaysSuccess);
	OnLoadSections =  AccelByte::TDelegateUtils<FOnQueryActiveSectionsComplete>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteQueryStorefront::OnListActiveSectionsSuccess);
	OnLoadItemMappingsSuccessDelegate= AccelByte::TDelegateUtils<THandler<FAccelByteModelsItemMappingsResponse>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteQueryStorefront::OnLoadItemMappingsSuccess);
	OnLoadItemMappingsErrorDelegate = AccelByte::TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteQueryStorefront::HandleLoadItemMappingsErrorError);

	API_FULL_CHECK_GUARD(StoreDisplay, ErrorMsg);
	API_CHECK_GUARD(Item, ErrorMsg);
	StoreDisplay->GetAllViews(StoreId, Language, OnLoadDisplaysSuccessDelegate, OnQueryErrorDelegate);
	Item->GetItemMappings(Platform, OnLoadItemMappingsSuccessDelegate, OnLoadItemMappingsErrorDelegate);
	ExecuteCriticalSectionAction(FVoidHandler::CreateLambda([&]()
	{
		TRY_PIN_SUBSYSTEM();

		SubsystemPin->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteQueryActiveSections>(SubsystemPin.Get(), UserId.ToSharedRef().Get(), StoreId, ViewId, Region, OnLoadSections);
	}));

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryStorefront::Finalize()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	FOnlineAsyncTaskAccelByte::Finalize();

	if (bWasSuccessful)
	{
		const FOnlineStoreV2AccelBytePtr StoreV2Interface = StaticCastSharedPtr<FOnlineStoreV2AccelByte>(SubsystemPin->GetStoreV2Interface());
		StoreV2Interface->EmplaceDisplays(DisplayMap);
		StoreV2Interface->EmplaceItemMappings(ItemMappings);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryStorefront::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	FOnlineAsyncTaskAccelByte::TriggerDelegates();

	TArray<FString> QueriedDisplays;
	DisplayMap.GenerateKeyArray(QueriedDisplays);

	TArray<FString> QueriedItemMappings;
	ItemMappings.GenerateKeyArray(QueriedItemMappings);

	Delegate.ExecuteIfBound(bWasSuccessful, QueriedDisplays, SectionIds, OfferIds, QueriedItemMappings, ErrorMsg);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}


void FOnlineAsyncTaskAccelByteQueryStorefront::OnLoadDisplaysSuccess(const TArray<FAccelByteModelsViewInfo>& Views)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	for (const FAccelByteModelsViewInfo& Display : Views)
	{
		TSharedRef<FAccelByteModelsViewInfo, ESPMode::ThreadSafe> DisplayRef = MakeShared<FAccelByteModelsViewInfo, ESPMode::ThreadSafe>(Display);
		DisplayMap.Add(Display.ViewId, DisplayRef);
	}

	bLoadedDisplays = true;
	HandleStepComplete();

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryStorefront::OnListActiveSectionsSuccess(bool bSectionsWasSuccessful, const TArray<FString>& InSectionIds, const TArray<FUniqueOfferId>& InOfferIds, const FString& Error)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	if (!bSectionsWasSuccessful)
	{
		ErrorMsg = Error;
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		return;
	}

	SectionIds = InSectionIds;
	OfferIds = InOfferIds;

	bLoadedSections = true;
	HandleStepComplete();

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryStorefront::OnLoadItemMappingsSuccess(const FAccelByteModelsItemMappingsResponse& MappingsResponse)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	for (const FAccelByteModelsItemMapping& Mapping : MappingsResponse.Data)
	{
		TSharedRef<FAccelByteModelsItemMapping, ESPMode::ThreadSafe> MappingRef = MakeShared<FAccelByteModelsItemMapping, ESPMode::ThreadSafe>(Mapping);
		ItemMappings.Add(Mapping.ItemIdentity, MappingRef);
	}

	bLoadedItemMappings = true;
	HandleStepComplete();

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryStorefront::HandleLoadItemMappingsErrorError(int32 Code, FString const& ErrMsg)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("%s"), *ErrMsg);

	// Not all environment has item mapping, starter environment is one of them.
	// We still consider this task success if the item mappings not found.
	if (Code == static_cast<int32>(AccelByte::ErrorCodes::ItemConfigNotFoundInNamespace))
	{
		bLoadedItemMappings = true;
		HandleStepComplete();
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryStorefront::HandleStepComplete()
{
	if (bIsComplete)
	{
		//if a previous step errored, ignore the results of whatever step just completed
		return;
	}

	if (bLoadedDisplays && bLoadedSections && bLoadedItemMappings)
	{
		CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	}
}

void FOnlineAsyncTaskAccelByteQueryStorefront::HandleQueryError(int32 Code, FString const& ErrMsg)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN_VERBOSITY(Error, TEXT("Code: %d; Message: %s"), Code, *ErrMsg);

	ErrorMsg = ErrMsg;
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}
