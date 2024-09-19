// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteAcceptAgreementPolicies.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "OnlineAgreementInterfaceAccelByte.h"
#include "OnlinePredefinedEventInterfaceAccelByte.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineAccelByteAcceptAgreementPolicies"
FOnlineAsyncTaskAccelByteAcceptAgreementPolicies::FOnlineAsyncTaskAccelByteAcceptAgreementPolicies(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const TArray<FABAcceptAgreementPoliciesRequest>& InDocumentsToAccept)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, DocumentsToAccept(InDocumentsToAccept)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InLocalUserId);
	bIsMandatory = false;
}

void FOnlineAsyncTaskAccelByteAcceptAgreementPolicies::Initialize()
{
	TRY_PIN_SUBSYSTEM()

	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Accepting agreement policies, UserId: %s"), *UserId->ToDebugString());

	const FOnlineAgreementAccelBytePtr AgreementInterface = SubsystemPin->GetAgreementInterface();
	if (AgreementInterface.IsValid())
	{
		TArray<TSharedRef<FAccelByteModelsRetrieveUserEligibilitiesResponse>> EligibilitiesRef;
		if (AgreementInterface->GetEligibleAgreements(LocalUserNum, EligibilitiesRef))
		{
			TArray<FAccelByteModelsAcceptAgreementRequest> RequestedDocuments;
			for (const auto& Document : DocumentsToAccept)
			{
				for (const auto& Eligibility : EligibilitiesRef)
				{
					bool bIsLocalFound = false;
					if (Eligibility->PolicyId == Document.BasePolicyId)
					{
						for (const auto& Version : Eligibility->PolicyVersions)
						{
							if (Version.IsInEffect)
							{
								FString DefaultLocalId;
								for (const auto& LocalizedPolicy : Version.LocalizedPolicyVersions)
								{
									if (LocalizedPolicy.LocaleCode == Document.LocaleCode)
									{
										RequestedDocuments.Add(FAccelByteModelsAcceptAgreementRequest{ LocalizedPolicy.Id, Version.Id, Eligibility->PolicyId, true });
										AcceptedAgreementPayload.AgreementDocuments.Add(FAccelByteModelsAgreementDocument{ LocalizedPolicy.Id, Version.Id, Eligibility->PolicyId });
										bIsLocalFound = true;
										if (Eligibility->IsMandatory)
										{
											bIsMandatory = true;
										}
										break;
									}
									if (LocalizedPolicy.IsDefaultSelection)
									{
										DefaultLocalId = LocalizedPolicy.Id;
									}
								}
								if (!bIsLocalFound)
								{
									RequestedDocuments.Add(FAccelByteModelsAcceptAgreementRequest{ DefaultLocalId, Version.Id, Eligibility->PolicyId, true });
									AcceptedAgreementPayload.AgreementDocuments.Add(FAccelByteModelsAgreementDocument{ DefaultLocalId, Version.Id, Eligibility->PolicyId });
									bIsLocalFound = true;
									if (Eligibility->IsMandatory)
									{
										bIsMandatory = true;
									}
								}
								break;
							}
						}
					}
					if (bIsLocalFound)
					{
						break;
					}
				}
			}

			if (RequestedDocuments.Num() > 0)
			{
				// Create delegates for successfully as well as unsuccessfully requesting to accept agreement policies
				OnAcceptAgreementPoliciesSuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsAcceptAgreementResponse>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteAcceptAgreementPolicies::OnAcceptAgreementPoliciesSuccess);
				OnAcceptAgreementPoliciesErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteAcceptAgreementPolicies::OnAcceptAgreementPoliciesError);

				// Send off a request to accept agreement policies, as well as connect our delegates for doing so
				API_CLIENT_CHECK_GUARD(ErrorStr);
				ApiClient->Agreement.BulkAcceptPolicyVersions(RequestedDocuments, OnAcceptAgreementPoliciesSuccessDelegate, OnAcceptAgreementPoliciesErrorDelegate);
			}
			else
			{
				ErrorStr = TEXT("request-failed-document-not-found-error");
				UE_LOG_AB(Warning, TEXT("Failed to accept agreement policies! Document to accept not found!"));
				CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
			}
		}
		else
		{
			ErrorStr = TEXT("request-failed-eligible-agreement-not-found-error");
			UE_LOG_AB(Warning, TEXT("Failed to accept agreement policies! Eligible Agreements Not Found, please query eligible agreements first"));
			CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		}
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteAcceptAgreementPolicies::Finalize()
{
	TRY_PIN_SUBSYSTEM()

	if (bWasSuccessful)
	{
		const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = SubsystemPin->GetPredefinedEventInterface();
		PredefinedEventInterface->SendEvent(LocalUserNum, MakeShared<FAccelByteModelsUserAgreementAcceptedPayload>(AcceptedAgreementPayload));
	}
}

void FOnlineAsyncTaskAccelByteAcceptAgreementPolicies::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM()

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const FOnlineAgreementAccelBytePtr AgreementInterface = SubsystemPin->GetAgreementInterface();
	if (AgreementInterface.IsValid())
	{
		if (bWasSuccessful)
		{
			if (bIsMandatory)
			{
				const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(SubsystemPin->GetIdentityInterface());
				API_CLIENT_CHECK_GUARD(ErrorStr);
				if (IdentityInterface.IsValid())
				{
					EAccelByteLoginType Type = EAccelByteLoginType::RefreshToken;
					FString ID = "";
					FString Password = ApiClient->CredentialsRef->GetRefreshToken();

					IdentityInterface->Login(LocalUserNum, FOnlineAccelByteAccountCredentials{ Type , ID, Password });
				}
			}
			else
			{
				AgreementInterface->TriggerOnAcceptAgreementPoliciesCompletedDelegates(LocalUserNum, bRequestResult, TEXT(""));
				AgreementInterface->TriggerAccelByteOnAcceptAgreementPoliciesCompletedDelegates(LocalUserNum, bRequestResult, ONLINE_ERROR_ACCELBYTE(TEXT(""), bRequestResult? EOnlineErrorResult::Success : EOnlineErrorResult::RequestFailure));
			}
		}
		else
		{
			AgreementInterface->TriggerOnAcceptAgreementPoliciesCompletedDelegates(LocalUserNum, false, ErrorStr);
			AgreementInterface->TriggerAccelByteOnAcceptAgreementPoliciesCompletedDelegates(LocalUserNum, false, ONLINE_ERROR_ACCELBYTE(FOnlineErrorAccelByte::PublicGetErrorKey(ErrorCode, ErrorStr)));
		}
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteAcceptAgreementPolicies::OnAcceptAgreementPoliciesSuccess(const FAccelByteModelsAcceptAgreementResponse& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	bRequestResult = Result.Proceed;

	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Request to accept agreement policies for user '%s' Success!"), *UserId->ToDebugString());
}

void FOnlineAsyncTaskAccelByteAcceptAgreementPolicies::OnAcceptAgreementPoliciesError(int32 InErrorCode, const FString& ErrorMessage)
{
	ErrorCode = InErrorCode;
	ErrorStr = TEXT("request-failed-accept-agreement-policies-error");
	UE_LOG_AB(Warning, TEXT("Failed to accept agreement policies! Error Code: %d; Error Message: %s"), ErrorCode, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}
#undef ONLINE_ERROR_NAMESPACE