#include "OnlineAsyncTaskAccelByteGetCurrentUserEntitlementHistory.h"

#include "OnlineEntitlementsInterfaceAccelByte.h"
#include "Interfaces/OnlineEntitlementsInterface.h"
#include "Core/AccelByteApiBase.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineAsyncTaskAccelByteGetCurrentUserEntitlementHistory"

FOnlineAsyncTaskAccelByteGetCurrentUserEntitlementHistory::FOnlineAsyncTaskAccelByteGetCurrentUserEntitlementHistory(FOnlineSubsystemAccelByte* const InABSubsystem
	, FUniqueNetId const& InLocalUserId
	, bool InForceUpdate
	, FUniqueEntitlementId const& InEntitlementId
	, EAccelByteEntitlementClass const& InEntitlementClass
	, FDateTime InStartDate
	, FDateTime InEndDate
	, FPagedQuery const& InPage)
	: FOnlineAsyncTaskAccelByte(InABSubsystem)
	, bForceUpdateEntitlementHistory(InForceUpdate)
	, EntitlementId(InEntitlementId)
	, EntitlementClass(InEntitlementClass)
	, StartDate(InStartDate)
	, EndDate(InEndDate)
	, PagedQuery(InPage)
{
	if (!IsRunningDedicatedServer())
	{
		UserId = FUniqueNetIdAccelByteUser::CastChecked(InLocalUserId);
	}
}

FOnlineAsyncTaskAccelByteGetCurrentUserEntitlementHistory::FOnlineAsyncTaskAccelByteGetCurrentUserEntitlementHistory(FOnlineSubsystemAccelByte* const InABSubsystem
	, int32 InLocalUserNum
	, bool InForceUpdate
	, FUniqueEntitlementId const& InEntitlementId
	, EAccelByteEntitlementClass const& InEntitlementClass
	, FDateTime InStartDate
	, FDateTime InEndDate
	, FPagedQuery const& InPage)
	: FOnlineAsyncTaskAccelByte(InABSubsystem)
	, bForceUpdateEntitlementHistory(InForceUpdate)
	, EntitlementId(InEntitlementId)
	, EntitlementClass(InEntitlementClass)
	, StartDate(InStartDate)
	, EndDate(InEndDate)
	, PagedQuery(InPage)
{
	if (!IsRunningDedicatedServer())
	{
		LocalUserNum = InLocalUserNum;
	}
}

void FOnlineAsyncTaskAccelByteGetCurrentUserEntitlementHistory::Initialize()
{
	TRY_PIN_SUBSYSTEM();

	FOnlineAsyncTaskAccelByte::Initialize();
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	if (IsRunningDedicatedServer())
	{
		ErrorString = TEXT("Failed to get user entitlement history! The endpoint supports only for player!");
		HttpStatus = static_cast<int32>(ErrorCodes::InvalidRequest);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get user entitlement history! The endpoint supports only for player!"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

		return;
	}

	const TSharedPtr<FOnlineIdentityAccelByte, ESPMode::ThreadSafe> IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(SubsystemPin->GetIdentityInterface());

	if (!UserId.IsValid())
	{
		const FUniqueNetIdPtr UserIdPtr = IdentityInterface->GetUniquePlayerId(LocalUserNum);

		if (!UserIdPtr.IsValid())
		{
			ErrorString = TEXT("Failed to get user entitlement history! User id is invalid!");
			HttpStatus = static_cast<int32>(ErrorCodes::InvalidRequest);
			AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get user entitlement history! User id is invalid!"));
			CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

			return;
		}

		UserId = FUniqueNetIdAccelByteUser::CastChecked(*UserIdPtr.Get());
	}

	if (bForceUpdateEntitlementHistory)
	{
		if (PagedQuery.Count == -1 || PagedQuery.Count >= 100)
		{
			GetCurrentUserEntitlementHistory(PagedQuery.Start, 100);
		}
		else
		{
			GetCurrentUserEntitlementHistory(PagedQuery.Start, PagedQuery.Count);
		}
	}
	else
	{
		if (EntitlementId.IsEmpty())
		{
			ErrorString = TEXT("Failed to get user entitlement history! Entitlement id is empty!");
			HttpStatus = static_cast<int32>(ErrorCodes::InvalidRequest);
			AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get user entitlement history! Entitlement id is empty!"));
			CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

			return;
		}
		
		if (!FAccelByteIdValidator::IsAccelByteIdValid(EntitlementId, EAccelByteIdHypensRule::NO_HYPENS))
		{
			ErrorString = TEXT("Failed to get user entitlement history! Entitlement id is invalid!");
			HttpStatus = static_cast<int32>(ErrorCodes::InvalidRequest);
			AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get user entitlement history! Entitlement id is invalid!"));
			CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

			return;
		}

		const FOnlineEntitlementsAccelBytePtr EntitlementInterface = StaticCastSharedPtr<FOnlineEntitlementsAccelByte>(SubsystemPin->GetEntitlementsInterface());

		if (!EntitlementInterface.IsValid())
		{
			ErrorString = TEXT("Failed to get user entitlement history! Entitlement interface is invalid!");
			HttpStatus = static_cast<int32>(ErrorCodes::ServiceUnavailableException);
			AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get user entitlement history! Entitlement interface is invalid!"));
		}

		EntitlementInterface->GetCachedCurrentUserEntitlementHistory(UserId.ToSharedRef(), EntitlementId, CurrentUserEntitlementHistoryResponse);

		CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetCurrentUserEntitlementHistory::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM();

	Super::TriggerDelegates();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	const FOnlineEntitlementsAccelBytePtr EntitlementInterface = StaticCastSharedPtr<FOnlineEntitlementsAccelByte>(SubsystemPin->GetEntitlementsInterface());

	if (bWasSuccessful)
	{
		if (!EntitlementInterface.IsValid())
		{
			ErrorString = TEXT("Failed to process the request! Entitlement interface is invalid!");
			HttpStatus = static_cast<int32>(ErrorCodes::ServiceUnavailableException);
		}

		if (bForceUpdateEntitlementHistory)
		{
			EntitlementInterface->AddCurrentUserEntitlementHistoryToMap(UserId.ToSharedRef(), CurrentUserEntitlementHistoryResponse);
		}
	}

	auto Error = bWasSuccessful ?
		ONLINE_ERROR(EOnlineErrorResult::Success) :
		ONLINE_ERROR(EOnlineErrorResult::RequestFailure
			, FString::Printf(TEXT("%d"), HttpStatus)
			, FText::FromString(ErrorString));

	EntitlementInterface->TriggerOnGetCurrentUserEntitlementHistoryCompleteDelegates(LocalUserNum, bWasSuccessful, CurrentUserEntitlementHistoryResponse, Error);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetCurrentUserEntitlementHistory::GetCurrentUserEntitlementHistory(int32 Offset, int32 Limit)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Starting get current user entitlement history, Offset: %d, Limit: %d"), Offset, Limit);

	SuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsUserEntitlementHistoryPagingResult>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGetCurrentUserEntitlementHistory::OnGetUserEntitlementHistorySuccess);
	OnErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGetCurrentUserEntitlementHistory::OnGetUserEntitlementHistoryError);

	API_FULL_CHECK_GUARD(Entitlement, ErrorString);
	Entitlement->GetCurrentUserEntitlementHistory(SuccessDelegate, OnErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetCurrentUserEntitlementHistory::OnGetUserEntitlementHistorySuccess(FAccelByteModelsUserEntitlementHistoryPagingResult const& Result)
{	
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	HttpStatus = static_cast<int32>(ErrorCodes::StatusOk);

	if (Result.Data.Num() == 0)
	{
		UE_LOG_AB(Log, TEXT("[Warning] Success to get user entitlement history but the result is empty!"));

		CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

		return;
	}
	for (const auto& TempEntitlementHistory : Result.Data)
	{
		CurrentUserEntitlementHistoryResponse.Add(TempEntitlementHistory);
	}

	if (PagedQuery.Count != -1)
	{
		PagedQuery.Count -= 1;

		if (PagedQuery.Count == 0)
		{
			CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
		}
	}

	if (!Result.Paging.Next.IsEmpty())
	{
		FString UrlOut;
		FString Params;
		Result.Paging.Next.Split(TEXT("?"), &UrlOut, &Params);
		if (!Params.IsEmpty())
		{
			TArray<FString> ParamsArray;
			Params.ParseIntoArray(ParamsArray, TEXT("&"));
			int32 Offset = -1;
			int32 Limit = -1;
			for (const FString& Param : ParamsArray)
			{
				FString Key;
				FString Value;
				Param.Split(TEXT("="), &Key, &Value);
				if (Key.Equals(TEXT("offset")) && Value.IsNumeric())
				{
					Offset = FCString::Atoi(*Value);
				}
				else if (Key.Equals(TEXT("limit")) && Value.IsNumeric())
				{
					Limit = FCString::Atoi(*Value);
				}

				if (Offset != -1 && Limit != -1)
				{
					GetCurrentUserEntitlementHistory(Offset, Limit);
					return;
				}
			}
		}
	}

	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetCurrentUserEntitlementHistory::OnGetUserEntitlementHistoryError(int32 ErrorCode, FString const& ErrorMessage)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	HttpStatus = ErrorCode;
	ErrorString = ErrorMessage;
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

#undef ONLINE_ERROR_NAMESPACE