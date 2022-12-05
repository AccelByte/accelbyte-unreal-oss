#include "OnlineAsyncTaskAccelByteQueryEntitlements.h"

#include "OnlineEntitlementsInterfaceAccelByte.h"
#include "Interfaces/OnlineEntitlementsInterface.h"

FOnlineAsyncTaskAccelByteQueryEntitlements::FOnlineAsyncTaskAccelByteQueryEntitlements(FOnlineSubsystemAccelByte* const InABSubsystem, const FUniqueNetId& InUserId, const FString& InNamespace, const FPagedQuery& InPage)
	: FOnlineAsyncTaskAccelByte(InABSubsystem),
	Namespace(InNamespace),
	PagedQuery(InPage)
{
	UserId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(InUserId.AsShared());
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
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	FOnlineAsyncTaskAccelByte::TriggerDelegates();
	
	Subsystem->GetEntitlementsInterface()->TriggerOnQueryEntitlementsCompleteDelegates(bWasSuccessful, *UserId, Namespace, ErrorMessage);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryEntitlements::QueryEntitlement(int32 Offset, int32 Limit)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Starting Query entitlement, Offset: %d, Limit: %d"), Offset, Limit);
	THandler<FAccelByteModelsEntitlementPagingSlicedResult> OnQueryEntitlementSuccess =
		THandler<FAccelByteModelsEntitlementPagingSlicedResult>::CreateRaw(this, &FOnlineAsyncTaskAccelByteQueryEntitlements::HandleQueryEntitlementSuccess);
	FErrorHandler OnError = FErrorHandler::CreateRaw(this, &FOnlineAsyncTaskAccelByteQueryEntitlements::HandleQueryEntitlementError);
	ApiClient->Entitlement.QueryUserEntitlements(TEXT(""), TEXT(""), Offset, Limit, OnQueryEntitlementSuccess, OnError, EAccelByteEntitlementClass::NONE, EAccelByteAppType::NONE);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryEntitlements::HandleQueryEntitlementSuccess(FAccelByteModelsEntitlementPagingSlicedResult const& Result)
{	
	for(FAccelByteModelsEntitlementInfo const& EntInfo : Result.Data)
	{
		TSharedRef<FOnlineEntitlement> Entitlement = MakeShared<FOnlineEntitlement>();
		Entitlement->Id = EntInfo.Id;
		Entitlement->Name = EntInfo.Name;
		Entitlement->Namespace = EntInfo.Namespace;
		Entitlement->Status = FAccelByteUtilities::GetUEnumValueAsString<EAccelByteEntitlementStatus>(EntInfo.Status);
		Entitlement->bIsConsumable = EntInfo.Type == EAccelByteEntitlementType::CONSUMABLE;
		// Note(Damar), should read from history, currently not available from sdk
		//Entitlement->ConsumedCount = EntInfo.UseCount;
		Entitlement->EndDate = EntInfo.EndDate;
		Entitlement->ItemId = EntInfo.ItemId;
		Entitlement->RemainingCount = EntInfo.UseCount;
		Entitlement->StartDate = EntInfo.StartDate;
		
		const TSharedPtr<FOnlineEntitlementsAccelByte, ESPMode::ThreadSafe> EntitlementsInterface = StaticCastSharedPtr<FOnlineEntitlementsAccelByte>(Subsystem->GetEntitlementsInterface());
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
			for(FString Param : ParamsArray)
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
