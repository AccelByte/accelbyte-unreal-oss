#include "OnlineAsyncTaskAccelByteGetUserEntitlementHistory.h"

#include "OnlineEntitlementsInterfaceAccelByte.h"
#include "Interfaces/OnlineEntitlementsInterface.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineAsyncTaskAccelByteGetUserEntitlementHistory"

FOnlineAsyncTaskAccelByteGetUserEntitlementHistory::FOnlineAsyncTaskAccelByteGetUserEntitlementHistory(FOnlineSubsystemAccelByte* const InABSubsystem
	, const FUniqueNetId& InLocalUserId
	, const FUniqueEntitlementId& InEntitlementId
	, bool InForceUpdate)
	: FOnlineAsyncTaskAccelByte(InABSubsystem)
	, bForceUpdateEntitlementHistory(InForceUpdate)
	, EntitlementId(InEntitlementId)
{
	LocalTargetUserId = FUniqueNetIdAccelByteUser::CastChecked(InLocalUserId);
}

FOnlineAsyncTaskAccelByteGetUserEntitlementHistory::FOnlineAsyncTaskAccelByteGetUserEntitlementHistory(FOnlineSubsystemAccelByte* const InABSubsystem
	, int32 InLocalTargetUserNum
	, const FUniqueEntitlementId& InEntitlementId
	, bool InForceUpdate)
	: FOnlineAsyncTaskAccelByte(InABSubsystem)
	, bForceUpdateEntitlementHistory(InForceUpdate)
	, EntitlementId(InEntitlementId)
	, LocalTargetUserNum(InLocalTargetUserNum)
{
}

void FOnlineAsyncTaskAccelByteGetUserEntitlementHistory::Initialize()
{
	TRY_PIN_SUBSYSTEM();

	FOnlineAsyncTaskAccelByte::Initialize();
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	if (EntitlementId.IsEmpty())
	{
		ErrorString = TEXT("Failed to get user entitlement history! Entitlement id is empty!");
		HttpStatus = static_cast<int32>(ErrorCodes::InvalidRequest);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get user entitlement history! Entitlement id is empty!"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

		return;
	}

	if (!IsRunningDedicatedServer())
	{
		ErrorString = TEXT("Failed to get user entitlement history! The endpoint only supports the dedicated server!");
		HttpStatus = static_cast<int32>(ErrorCodes::InvalidRequest);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get user entitlement history! The endpoint only supports the dedicated server!"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

		return;
	}

	const TSharedPtr<FOnlineIdentityAccelByte, ESPMode::ThreadSafe> IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(SubsystemPin->GetIdentityInterface());
	if (!IdentityInterface.IsValid())
	{
		ErrorString = TEXT("Failed to process the request! Identity interface is invalid!");
		HttpStatus = static_cast<int32>(ErrorCodes::ServiceUnavailableException);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to process the request! Identity interface is invalid!"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
	}

	if (!LocalTargetUserId.IsValid())
	{
		const FUniqueNetIdPtr TargetUserIdPtr = IdentityInterface->GetUniquePlayerId(LocalTargetUserNum);

		if (!TargetUserIdPtr.IsValid())
		{
			ErrorString = TEXT("Failed to get user entitlement history! User id is invalid!");
			HttpStatus = static_cast<int32>(ErrorCodes::InvalidRequest);
			AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get user entitlement history! User id is invalid!"));
			CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

			return;
		}

		LocalTargetUserId = FUniqueNetIdAccelByteUser::CastChecked(*TargetUserIdPtr.Get());
	}

	SuccessDelegate = TDelegateUtils<THandler<TArray<FAccelByteModelsUserEntitlementHistory>>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGetUserEntitlementHistory::OnGetUserEntitlementHistorySuccess);
	OnErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGetUserEntitlementHistory::OnGetUserEntitlementHistoryError);

	if (bForceUpdateEntitlementHistory)
	{
		if (LocalTargetUserId->IsValid())
		{
			SERVER_API_CLIENT_CHECK_GUARD(ErrorString);
			ServerApiClient->ServerEcommerce.GetUserEntitlementHistory(LocalTargetUserId->GetAccelByteId(), EntitlementId, SuccessDelegate, OnErrorDelegate);
		}
	}
	else
	{
		const FOnlineEntitlementsAccelBytePtr EntitlementInterface = StaticCastSharedPtr<FOnlineEntitlementsAccelByte>(SubsystemPin->GetEntitlementsInterface());

		if (!EntitlementInterface.IsValid())
		{
			ErrorString = TEXT("Failed to get user entitlement history! Entitlement interface is invalid!");
			HttpStatus = static_cast<int32>(ErrorCodes::ServiceUnavailableException);
			AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get user entitlement history! Entitlement interface is invalid!"));
		}

		EntitlementInterface->GetCachedUserEntitlementHistory(LocalTargetUserId.ToSharedRef(), EntitlementId, UserEntitlementHistoryResponse);

		CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetUserEntitlementHistory::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM();

	Super::TriggerDelegates();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(SubsystemPin->GetIdentityInterface());
	const FOnlineEntitlementsAccelBytePtr EntitlementInterface = StaticCastSharedPtr<FOnlineEntitlementsAccelByte>(SubsystemPin->GetEntitlementsInterface());

	if (bWasSuccessful)
	{
		if (!IdentityInterface.IsValid())
		{
			ErrorString = TEXT("Failed to process the request! Identity interface is invalid!");
			HttpStatus = static_cast<int32>(ErrorCodes::ServiceUnavailableException);
		}

		if (!EntitlementInterface.IsValid())
		{
			ErrorString = TEXT("Failed to process the request! Entitlement interface is invalid!");
			HttpStatus = static_cast<int32>(ErrorCodes::ServiceUnavailableException);
		}

		if (bForceUpdateEntitlementHistory)
		{
			EntitlementInterface->AddUserEntitlementHistoryToMap(LocalTargetUserId.ToSharedRef(), EntitlementId, UserEntitlementHistoryResponse);
		}
	}

	auto Error = bWasSuccessful ?
		ONLINE_ERROR(EOnlineErrorResult::Success) :
		ONLINE_ERROR(EOnlineErrorResult::RequestFailure
			, FString::Printf(TEXT("%d"), HttpStatus)
			, FText::FromString(ErrorString));

	EntitlementInterface->TriggerOnGetUserEntitlementHistoryCompleteDelegates(LocalTargetUserNum, bWasSuccessful, UserEntitlementHistoryResponse, Error);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetUserEntitlementHistory::OnGetUserEntitlementHistorySuccess(TArray<FAccelByteModelsUserEntitlementHistory> const& Result)
{	
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	HttpStatus = static_cast<int32>(ErrorCodes::StatusOk);

	if (Result.Num() != 0)
	{
		UserEntitlementHistoryResponse = Result;
	}
	else
	{
		UE_LOG_AB(Log, TEXT("[Warning] Success to get user entitlement history but the result is empty!"));
	}

	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetUserEntitlementHistory::OnGetUserEntitlementHistoryError(int32 ErrorCode, FString const& ErrorMessage)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	HttpStatus = ErrorCode;
	ErrorString = ErrorMessage;
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

#undef ONLINE_ERROR_NAMESPACE