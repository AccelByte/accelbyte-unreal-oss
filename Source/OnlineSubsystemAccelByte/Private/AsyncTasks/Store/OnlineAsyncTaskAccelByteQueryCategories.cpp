﻿// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteQueryCategories.h"

#include "OnlineStoreInterfaceV2AccelByte.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteQueryCategories::FOnlineAsyncTaskAccelByteQueryCategories(
	FOnlineSubsystemAccelByte* const InABSubsystem, const FUniqueNetId& InUserId, const FOnQueryOnlineStoreCategoriesComplete& InDelegate) 
	: FOnlineAsyncTaskAccelByte(InABSubsystem, true)
	, Delegate(InDelegate)
	, Language(InABSubsystem->GetLanguage())
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InUserId);
}

void FOnlineAsyncTaskAccelByteQueryCategories::Initialize()
{
	Super::Initialize();
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	
	OnGetRootCategoriesSuccess = TDelegateUtils<THandler<TArray<FAccelByteModelsCategoryInfo>>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteQueryCategories::HandleGetRootCategorySuccess);
	OnError = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteQueryCategories::HandleAsyncTaskError);
	
	API_FULL_CHECK_GUARD(Category, ErrorMsg);
	Category->GetRootCategories(Language, OnGetRootCategoriesSuccess, OnError);
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryCategories::Finalize()
{
	TRY_PIN_SUBSYSTEM();

	Super::Finalize();
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	const FOnlineStoreV2AccelBytePtr StoreV2Interface = StaticCastSharedPtr<FOnlineStoreV2AccelByte>(SubsystemPin->GetStoreV2Interface());
	TArray<FOnlineStoreCategory> Categories;
	CategoryMap.GenerateValueArray(Categories);
	StoreV2Interface->ReplaceCategories(Categories);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryCategories::TriggerDelegates()
{
	Super::TriggerDelegates();
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	Delegate.ExecuteIfBound(bWasSuccessful, ErrorMsg);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryCategories::Tick()
{
	Super::Tick();

	if (TaskLeft == 0)
	{
		CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	}
}

void FOnlineAsyncTaskAccelByteQueryCategories::HandleGetRootCategorySuccess(const TArray<FAccelByteModelsCategoryInfo>& AccelByteModelsCategoryInfos)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	TaskLeft = AccelByteModelsCategoryInfos.Num();
	for(const FAccelByteModelsCategoryInfo& CategoryInfo : AccelByteModelsCategoryInfos)
	{
		FOnlineStoreCategory& OnlineCategory = CategoryMap.FindOrAdd(CategoryInfo.CategoryPath);
		OnlineCategory.Id = CategoryInfo.CategoryPath;
		OnlineCategory.Description = FText::FromString(CategoryInfo.DisplayName);
		OnGetDescendantCategoriesSuccess = TDelegateUtils<THandler<TArray<FAccelByteModelsCategoryInfo>>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteQueryCategories::HandleGetDescendantCategoriesSuccess);
		API_FULL_CHECK_GUARD(Category, ErrorMsg);
		Category->GetDescendantCategories(Language, CategoryInfo.CategoryPath, OnGetDescendantCategoriesSuccess, OnError);
	}
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryCategories::HandleGetDescendantCategoriesSuccess(const TArray<FAccelByteModelsCategoryInfo>& AccelByteModelsCategoryInfos)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	for(const FAccelByteModelsCategoryInfo& CategoryInfo : AccelByteModelsCategoryInfos)
	{
		FOnlineStoreCategory& OnlineCategory = CategoryMap.FindOrAdd(CategoryInfo.ParentCategoryPath);
		OnlineCategory.Id = CategoryInfo.ParentCategoryPath;
		FOnlineStoreCategory Descendant;
		Descendant.Id = CategoryInfo.CategoryPath;
		Descendant.Description = FText::FromString(CategoryInfo.DisplayName);
		OnlineCategory.SubCategories.Add(Descendant);
		CategoryMap.Add(Descendant.Id, Descendant);
	}
	TaskLeft--;
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryCategories::HandleAsyncTaskError(int32 Code, FString const& ErrMsg)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN_VERBOSITY(Error, TEXT("Code: %d; Message: %s"), Code, *ErrMsg);

	ErrorMsg = ErrMsg;
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}
