#include "OnlineAsyncTaskAccelByteConsumeEntitlement.h"

#include "OnlineEntitlementsInterfaceAccelByte.h"
#include "Interfaces/OnlineEntitlementsInterface.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteHelpers.h"

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
	EntitlementItem = MakeShared<FOnlineEntitlementAccelByte>();
	EntitlementItem->Id = EntitlementId;
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
		API_FULL_CHECK_GUARD(Entitlement, ErrorMessage);
		Entitlement->ConsumeUserEntitlement(EntitlementId, UseCount, OnConsumeEntitlementSuccess, OnError, Options, RequestId);
	}
	else
	{
		
		SERVER_API_CLIENT_CHECK_GUARD(ErrorMessage);
		ServerApiClient->ServerEcommerce.ConsumeUserEntitlement(UserId->GetAccelByteId(), EntitlementId, UseCount, OnConsumeEntitlementSuccess, OnError, Options, RequestId);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteConsumeEntitlement::Finalize()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	FOnlineAsyncTaskAccelByte::TriggerDelegates();
	const FOnlineEntitlementsAccelBytePtr EntitlementInterface = StaticCastSharedPtr<FOnlineEntitlementsAccelByte>(SubsystemPin->GetEntitlementsInterface());
	if (EntitlementInterface.IsValid() && bWasSuccessful)
	{
		EntitlementInterface->AddEntitlementToMap(UserId.ToSharedRef(), EntitlementItem.ToSharedRef());
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteConsumeEntitlement::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	FOnlineAsyncTaskAccelByte::TriggerDelegates();
	const FOnlineEntitlementsAccelBytePtr EntitlementInterface = StaticCastSharedPtr<FOnlineEntitlementsAccelByte>(SubsystemPin->GetEntitlementsInterface());
	if (EntitlementInterface.IsValid())
	{
		EntitlementInterface->TriggerOnConsumeEntitlementCompleteDelegates(bWasSuccessful, *UserId.Get(), EntitlementItem, ONLINE_ERROR(bWasSuccessful ? EOnlineErrorResult::Success : EOnlineErrorResult::RequestFailure, FString::Printf(TEXT("%d"), ErrorCode), FText::FromString(ErrorMessage)));
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteConsumeEntitlement::HandleConsumeEntitlementSuccess(FAccelByteModelsEntitlementInfo const& Result)
{	
	EntitlementItem->Id = Result.Id;
	EntitlementItem->Name = Result.Name;
	EntitlementItem->Namespace = Result.Namespace;
	EntitlementItem->Status = FAccelByteUtilities::GetUEnumValueAsString<EAccelByteEntitlementStatus>(Result.Status);
	EntitlementItem->bIsConsumable = Result.Type == EAccelByteEntitlementType::CONSUMABLE;
	EntitlementItem->EndDate = Result.EndDate;
	EntitlementItem->ItemId = Result.ItemId;
	EntitlementItem->ConsumedCount = UseCount;
	EntitlementItem->RemainingCount = Result.UseCount;
	EntitlementItem->StartDate = Result.StartDate;
	EntitlementItem->SetBackendEntitlementInfo(Result);

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