#include "OnlineAsyncTaskAccelByteConsumeEntitlement.h"

#include "OnlineEntitlementsInterfaceAccelByte.h"
#include "Interfaces/OnlineEntitlementsInterface.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineAccelByteConsumeEntitlement"

FOnlineAsyncTaskAccelByteConsumeEntitlement::FOnlineAsyncTaskAccelByteConsumeEntitlement(FOnlineSubsystemAccelByte* const InABSubsystem, const FUniqueNetId& InUserId, const FUniqueEntitlementId& InEntitlementId, int32 InUseCount, TArray<FString> InOptions, const FString& InRequestId)
	: FOnlineAsyncTaskAccelByte(InABSubsystem)
	, EntitlementId(InEntitlementId)
	, UseCount(InUseCount)
	, Options(InOptions)
	, RequestId(InRequestId)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InUserId);
	Entitlement = MakeShared<FOnlineEntitlement>();
	Entitlement->Id = EntitlementId;
}

void FOnlineAsyncTaskAccelByteConsumeEntitlement::Initialize()
{
	FOnlineAsyncTaskAccelByte::Initialize();
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	THandler<FAccelByteModelsEntitlementInfo> OnConsumeEntitlementSuccess =
		TDelegateUtils<THandler<FAccelByteModelsEntitlementInfo>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteConsumeEntitlement::HandleConsumeEntitlementSuccess);
	FErrorHandler OnError = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteConsumeEntitlement::HandleConsumeEntitlementError);

	if (!IsRunningDedicatedServer())
	{
		API_CLIENT_CHECK_GUARD(ErrorMessage);
		ApiClient->Entitlement.ConsumeUserEntitlement(EntitlementId, UseCount, OnConsumeEntitlementSuccess, OnError, Options, RequestId);
	}
	else
	{
		FServerApiClientPtr ServerApiClient = FMultiRegistry::GetServerApiClient();
		ServerApiClient->ServerEcommerce.ConsumeUserEntitlement(UserId->GetAccelByteId(), EntitlementId, UseCount, OnConsumeEntitlementSuccess, OnError, Options, RequestId);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteConsumeEntitlement::Finalize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	FOnlineAsyncTaskAccelByte::TriggerDelegates();
	const FOnlineEntitlementsAccelBytePtr EntitlementInterface = StaticCastSharedPtr<FOnlineEntitlementsAccelByte>(Subsystem->GetEntitlementsInterface());
	if (EntitlementInterface.IsValid() && bWasSuccessful)
	{
		EntitlementInterface->AddEntitlementToMap(UserId.ToSharedRef(), Entitlement.ToSharedRef());
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteConsumeEntitlement::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	FOnlineAsyncTaskAccelByte::TriggerDelegates();
	const FOnlineEntitlementsAccelBytePtr EntitlementInterface = StaticCastSharedPtr<FOnlineEntitlementsAccelByte>(Subsystem->GetEntitlementsInterface());
	if (EntitlementInterface.IsValid())
	{
		EntitlementInterface->TriggerOnConsumeEntitlementCompleteDelegates(bWasSuccessful, *UserId.Get(), Entitlement, ONLINE_ERROR(bWasSuccessful ? EOnlineErrorResult::Success : EOnlineErrorResult::RequestFailure, FString::Printf(TEXT("%d"), ErrorCode), FText::FromString(ErrorMessage)));
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteConsumeEntitlement::HandleConsumeEntitlementSuccess(FAccelByteModelsEntitlementInfo const& Result)
{	
	Entitlement->Id = Result.Id;
	Entitlement->Name = Result.Name;
	Entitlement->Namespace = Result.Namespace;
	Entitlement->Status = FAccelByteUtilities::GetUEnumValueAsString<EAccelByteEntitlementStatus>(Result.Status);
	Entitlement->bIsConsumable = Result.Type == EAccelByteEntitlementType::CONSUMABLE;
	Entitlement->EndDate = Result.EndDate;
	Entitlement->ItemId = Result.ItemId;
	Entitlement->ConsumedCount = UseCount;
	Entitlement->RemainingCount = Result.UseCount;
	Entitlement->StartDate = Result.StartDate;

	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
}

void FOnlineAsyncTaskAccelByteConsumeEntitlement::HandleConsumeEntitlementError(int32 Code, FString const& ErrMsg)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN_VERBOSITY(Warning, TEXT("Code: %d; Message: %s"), Code, *ErrMsg);

	ErrorCode = Code;
	ErrorMessage = ErrMsg;
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

#undef ONLINE_ERROR_NAMESPACE