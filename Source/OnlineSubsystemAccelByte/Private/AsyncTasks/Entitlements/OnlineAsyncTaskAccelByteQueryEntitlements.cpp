#include "OnlineAsyncTaskAccelByteQueryEntitlements.h"

#include "OnlineEntitlementsInterfaceAccelByte.h"
#include "Interfaces/OnlineEntitlementsInterface.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteQueryEntitlements::FOnlineAsyncTaskAccelByteQueryEntitlements(FOnlineSubsystemAccelByte* const InABSubsystem, const FUniqueNetId& InUserId, const FString& InNamespace, const FPagedQuery& InPage)
	: FOnlineAsyncTaskAccelByte(InABSubsystem),
	Namespace(InNamespace),
	PagedQuery(InPage)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InUserId);
}

void FOnlineAsyncTaskAccelByteQueryEntitlements::Initialize()
{
	FOnlineAsyncTaskAccelByte::Initialize();
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	if (PagedQuery.Count == -1 || PagedQuery.Count >= 100)
	{
		QueryEntitlement(PagedQuery.Start, 100);
	}
	else
	{
		QueryEntitlement(PagedQuery.Start, PagedQuery.Count);
	}
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryEntitlements::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM()

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	FOnlineAsyncTaskAccelByte::TriggerDelegates();
	
	SubsystemPin->GetEntitlementsInterface()->TriggerOnQueryEntitlementsCompleteDelegates(bWasSuccessful, *UserId, Namespace, ErrorMessage);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryEntitlements::QueryEntitlement(int32 Offset, int32 Limit)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Starting Query entitlement, Offset: %d, Limit: %d"), Offset, Limit);
	THandler<FAccelByteModelsEntitlementPagingSlicedResult> OnQueryEntitlementSuccess =
		TDelegateUtils<THandler<FAccelByteModelsEntitlementPagingSlicedResult>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteQueryEntitlements::HandleQueryEntitlementSuccess);
	FErrorHandler OnError = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteQueryEntitlements::HandleQueryEntitlementError);
	API_CLIENT_CHECK_GUARD(ErrorMessage);
	ApiClient->Entitlement.QueryUserEntitlements(TEXT(""), TEXT(""), Offset, Limit, OnQueryEntitlementSuccess, OnError, EAccelByteEntitlementClass::NONE, EAccelByteAppType::NONE);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryEntitlements::HandleQueryEntitlementSuccess(FAccelByteModelsEntitlementPagingSlicedResult const& Result)
{	
	TRY_PIN_SUBSYSTEM()

	const TSharedPtr<FOnlineEntitlementsAccelByte, ESPMode::ThreadSafe> EntitlementsInterface = StaticCastSharedPtr<FOnlineEntitlementsAccelByte>(SubsystemPin->GetEntitlementsInterface());
	if (!EntitlementsInterface.IsValid())
	{
		ErrorMessage = TEXT("Entitlement Interface is not valid!");
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}
	for(FAccelByteModelsEntitlementInfo const& EntInfo : Result.Data)
	{
		TSharedRef<FOnlineEntitlementAccelByte> Entitlement = MakeShared<FOnlineEntitlementAccelByte>();
		Entitlement->Id = EntInfo.Id;
		Entitlement->Name = EntInfo.Name;
		Entitlement->Namespace = EntInfo.Namespace;
		Entitlement->Status = FAccelByteUtilities::GetUEnumValueAsString<EAccelByteEntitlementStatus>(EntInfo.Status);
		Entitlement->bIsConsumable = EntInfo.Type == EAccelByteEntitlementType::CONSUMABLE;
		Entitlement->EndDate = EntInfo.EndDate;
		Entitlement->ItemId = EntInfo.ItemId;
		Entitlement->RemainingCount = EntInfo.UseCount;
		Entitlement->StartDate = EntInfo.StartDate;
		Entitlement->SetBackendEntitlementInfo(EntInfo);
		
		const auto CachedEntitlement = EntitlementsInterface->GetEntitlement(*UserId.Get(), EntInfo.Id);
		if (CachedEntitlement.IsValid())
		{
			Entitlement->ConsumedCount = CachedEntitlement->ConsumedCount;
		}

		EntitlementsInterface->AddEntitlementToMap(UserId.ToSharedRef(), Entitlement);
		
		if(PagedQuery.Count != -1)
		{
			PagedQuery.Count -= 1;

			if(PagedQuery.Count == 0)
			{
				CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
			}
		}
	}

	// this should global function, can be accessed anywhere
	if(!Result.Paging.Next.IsEmpty())
	{
		FString UrlOut;
		FString Params;
		Result.Paging.Next.Split(TEXT("?"), &UrlOut, &Params);
		if(!Params.IsEmpty())
		{
			TArray<FString> ParamsArray;
			Params.ParseIntoArray(ParamsArray, TEXT("&"));
			int32 Offset = -1;
			int32 Limit = -1;
			for(const FString& Param : ParamsArray)
			{
				FString Key;
				FString Value;
				Param.Split(TEXT("="), &Key, &Value);
				if(Key.Equals(TEXT("offset")) && Value.IsNumeric())
				{
					Offset = FCString::Atoi(*Value);
				}
				else if(Key.Equals(TEXT("limit")) && Value.IsNumeric())
				{
					Limit = FCString::Atoi(*Value);
				}

				if(Offset != -1 && Limit != -1)
				{
					QueryEntitlement(Offset, Limit);
					return;
				}
			}
		}
	}
	
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
}

void FOnlineAsyncTaskAccelByteQueryEntitlements::HandleQueryEntitlementError(int32 Code, FString const& ErrMsg)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN_VERBOSITY(Error, TEXT("Code: %d; Message: %s"), Code, *ErrMsg);

	ErrorMessage = ErrMsg;
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}
