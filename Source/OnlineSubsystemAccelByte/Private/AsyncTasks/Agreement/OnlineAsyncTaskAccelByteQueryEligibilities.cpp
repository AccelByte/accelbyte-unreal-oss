// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteQueryEligibilities.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineIdentityInterfaceAccelByte.h"

FOnlineAsyncTaskAccelByteQueryEligibilities::FOnlineAsyncTaskAccelByteQueryEligibilities(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, bool bInNotAcceptedOnly, bool bInAlwaysRequestToService)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, bNotAcceptedOnly(bInNotAcceptedOnly)
	, bAlwaysRequestToService(bInAlwaysRequestToService)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InLocalUserId);
}

void FOnlineAsyncTaskAccelByteQueryEligibilities::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s"), *UserId->ToDebugString());

	const FOnlineAgreementAccelBytePtr AgreementInterface = Subsystem->GetAgreementInterface();
	if (AgreementInterface.IsValid())
	{
		TArray<TSharedRef<FAccelByteModelsRetrieveUserEligibilitiesResponse>> EligibilitiesRef;
		if (AgreementInterface->GetEligibleAgreements(LocalUserNum, EligibilitiesRef) && !bAlwaysRequestToService)
		{
			TArray<FAccelByteModelsRetrieveUserEligibilitiesResponse> EligibilitiesReturn;
			for (const auto& Eligibility : EligibilitiesRef)
			{
				if (!bNotAcceptedOnly || !Eligibility->IsAccepted)
				{
					EligibilitiesReturn.Add(Eligibility.Get());
				}
			}

			Eligibilities = EligibilitiesReturn;
			CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
			AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Eligible agreements found at user index '%d'!"), LocalUserNum);
		}
		else
		{
			// Create delegates for successfully as well as unsuccessfully querying the user's eligibilities
			OnQueryEligibilitiesSuccessDelegate = TDelegateUtils<THandler<TArray<FAccelByteModelsRetrieveUserEligibilitiesResponse>>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteQueryEligibilities::OnQueryEligibilitiesSuccess);
			OnQueryEligibilitiesErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteQueryEligibilities::OnQueryEligibilitiesError);

			// Send off a request to query eligibilities, as well as connect our delegates for doing so
			ApiClient->Agreement.QueryLegalEligibilities(ApiClient->CredentialsRef->GetNamespace(), OnQueryEligibilitiesSuccessDelegate, OnQueryEligibilitiesErrorDelegate);
		}
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryEligibilities::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const FOnlineAgreementAccelBytePtr AgreementInterface = Subsystem->GetAgreementInterface();
	if (AgreementInterface.IsValid())
	{
		if (bWasSuccessful)
		{
			TArray<FAccelByteModelsRetrieveUserEligibilitiesResponse> EligibilitiesReturn;
			for (const auto& Eligibility : Eligibilities)
			{
				if (!bNotAcceptedOnly || !Eligibility.IsAccepted)
				{
					EligibilitiesReturn.Add(Eligibility);
				}
			}

			AgreementInterface->TriggerOnQueryEligibilitiesCompletedDelegates(LocalUserNum, true, EligibilitiesReturn, TEXT(""));
		}
		else
		{
			AgreementInterface->TriggerOnQueryEligibilitiesCompletedDelegates(LocalUserNum, false, TArray<FAccelByteModelsRetrieveUserEligibilitiesResponse>{}, ErrorStr);
		}
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryEligibilities::OnQueryEligibilitiesSuccess(const TArray<FAccelByteModelsRetrieveUserEligibilitiesResponse>& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	const FOnlineAgreementAccelBytePtr AgreementInterface = Subsystem->GetAgreementInterface();	
	Eligibilities = Result;
	
	if (AgreementInterface.IsValid())
	{
		TArray<TSharedRef<FAccelByteModelsRetrieveUserEligibilitiesResponse>> EligibilitiesRef;
		for (const auto& Eligibility : Result)
		{
			EligibilitiesRef.Add(MakeShared<FAccelByteModelsRetrieveUserEligibilitiesResponse>(Eligibility));
		}
		AgreementInterface->EligibilitiesMap.Emplace(UserId.ToSharedRef(), EligibilitiesRef);
	}

	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Sending request to query AccelByte eligible agreements for user '%s'!"), *UserId->ToDebugString());
}

void FOnlineAsyncTaskAccelByteQueryEligibilities::OnQueryEligibilitiesError(int32 ErrorCode, const FString& ErrorMessage)
{
	ErrorStr = TEXT("request-failed-query-eligibilities-error");
	UE_LOG_AB(Warning, TEXT("Failed to query user's eligible agreements! Error Code: %d; Error Message: %s"), ErrorCode, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}