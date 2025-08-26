// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteQueryActiveSections.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteQueryActiveSections::FOnlineAsyncTaskAccelByteQueryActiveSections(FOnlineSubsystemAccelByte* const InABSubsystem, const FUniqueNetId& InUserId, const FString& InStoreId, const FString& InViewId, const FString& InRegion, const FOnQueryActiveSectionsComplete& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABSubsystem)
	, StoreId(InStoreId)
	, ViewId(InViewId)
	, Language(InABSubsystem->GetLanguage())
	, Region(InRegion)
	, Delegate(InDelegate)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InUserId);
}


void FOnlineAsyncTaskAccelByteQueryActiveSections::Initialize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	FOnlineAsyncTaskAccelByte::Initialize();

	OnListActiveSectionContentsSuccessDelegate = TDelegateUtils<THandler<TArray<FAccelByteModelsSectionInfo>>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteQueryActiveSections::OnListActiveSectionContentsSuccess);
	OnListActiveSectionContentsErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteQueryActiveSections::OnListActiveSectionContentsError);
	API_FULL_CHECK_GUARD(StoreDisplay, ErrorMsg);
	StoreDisplay->ListActiveSectionContents(StoreId, ViewId, Region, Language, OnListActiveSectionContentsSuccessDelegate, OnListActiveSectionContentsErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryActiveSections::Finalize()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	FOnlineAsyncTaskAccelByte::Finalize();

	if (bWasSuccessful)
	{
		const FOnlineStoreV2AccelBytePtr StoreV2Interface = StaticCastSharedPtr<FOnlineStoreV2AccelByte>(SubsystemPin->GetStoreV2Interface());
		StoreV2Interface->EmplaceOffers(OfferMap);
		StoreV2Interface->EmplaceSections(UserId.ToSharedRef().Get(), Sections);
		StoreV2Interface->EmplaceOffersBySection(UserId.ToSharedRef().Get(), OffersBySection);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryActiveSections::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	FOnlineAsyncTaskAccelByte::TriggerDelegates();

	TArray<FString> QueriedSectionIds;
	TArray<FString> QueriedOfferIds;
	OfferMap.GenerateKeyArray(QueriedOfferIds);
	Sections.GenerateKeyArray(QueriedSectionIds);

	Delegate.ExecuteIfBound(bWasSuccessful, QueriedSectionIds, QueriedOfferIds, ErrorMsg);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryActiveSections::OnListActiveSectionContentsSuccess(const TArray<FAccelByteModelsSectionInfo>& InSections)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	for (const FAccelByteModelsSectionInfo& Section : InSections)
	{
		Sections.Add(Section.SectionId, MakeShared<FAccelByteModelsSectionInfo, ESPMode::ThreadSafe>(Section));
		for (const FAccelByteModelsItemInfo& Item : Section.CurrentRotationItems)
		{
			FOnlineStoreOfferAccelByteRef Offer = MakeShared<FOnlineStoreOfferAccelByte>();
			Offer->SetItem(Item);

			// Offers populated by the ListActiveSections task have a expiration of the rotation time or the section end date, whichever is closer
			if (Section.CurrentRotationExpireAt > 0 && Section.CurrentRotationExpireAt < Section.EndDate)
			{
				Offer->ExpirationDate = Section.CurrentRotationExpireAt;
			} else
			{
				Offer->ExpirationDate = Section.EndDate;
			}

			OfferMap.Add(Offer->OfferId, Offer);
			OffersBySection.Add(Section.SectionId, Offer);
		}
	}

	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryActiveSections::OnListActiveSectionContentsError(int32 Code, FString const& ErrMsg)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN_VERBOSITY(Error, TEXT("Code: %d; Message: %s"), Code, *ErrMsg);

	ErrorMsg = ErrMsg;
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}
