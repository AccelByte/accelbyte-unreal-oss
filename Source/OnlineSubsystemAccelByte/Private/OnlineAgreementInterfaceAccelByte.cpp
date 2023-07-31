// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#include "OnlineAgreementInterfaceAccelByte.h"
#include "OnlineSubsystemAccelByte.h"
#include "Core/AccelByteMultiRegistry.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "OnlineSubsystemAccelByteInternalHelpers.h"
#include "AsyncTasks/Agreement/OnlineAsyncTaskAccelByteQueryEligibilities.h"
#include "AsyncTasks/Agreement/OnlineAsyncTaskAccelByteGetLocalizedPolicyContent.h"
#include "AsyncTasks/Agreement/OnlineAsyncTaskAccelByteAcceptAgreementPolicies.h"
#include "OnlineSubsystemUtils.h"

bool FOnlineAgreementAccelByte::GetFromSubsystem(const IOnlineSubsystem* Subsystem, FOnlineAgreementAccelBytePtr& OutInterfaceInstance)
{
	const FOnlineSubsystemAccelByte* ABSubsystem = static_cast<const FOnlineSubsystemAccelByte*>(Subsystem);
	if (ABSubsystem == nullptr)
	{
		OutInterfaceInstance = nullptr;
		return false;
	}

	OutInterfaceInstance = ABSubsystem->GetAgreementInterface();
	return OutInterfaceInstance.IsValid();
}

bool FOnlineAgreementAccelByte::GetFromWorld(const UWorld* World, FOnlineAgreementAccelBytePtr& OutInterfaceInstance)
{
	const IOnlineSubsystem* Subsystem = ::Online::GetSubsystem(World);
	if (Subsystem == nullptr)
	{
		OutInterfaceInstance = nullptr;
		return false;
	}

	return GetFromSubsystem(Subsystem, OutInterfaceInstance);
}

#define ONLINE_ERROR_NAMESPACE "FOnlineAccelByteQueryEligibilities"
bool FOnlineAgreementAccelByte::QueryEligibleAgreements(int32 LocalUserNum, bool bNotAcceptedOnly, bool bAlwaysRequestToService)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("LocalUserNum: %d"), LocalUserNum);

	const IOnlineIdentityPtr IdentityInterface = AccelByteSubsystem->GetIdentityInterface();
	if (IdentityInterface.IsValid())
	{
		// Check whether user is connected or not yet
		if (IdentityInterface->GetLoginStatus(LocalUserNum) == ELoginStatus::LoggedIn)
		{
			const TSharedPtr<const FUniqueNetId> UserIdPtr = IdentityInterface->GetUniquePlayerId(LocalUserNum);
			TSharedPtr<FUserOnlineAccount> UserAccount;
			if (UserIdPtr.IsValid())
			{
				AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteQueryEligibilities>(AccelByteSubsystem, *UserIdPtr.Get(), bNotAcceptedOnly, bAlwaysRequestToService);
				AB_OSS_INTERFACE_TRACE_END(TEXT("Dispatching async task to attempt to query user's eligible agreements!"));
				
				return true;
			}
			else
			{
				const FString ErrorStr = TEXT("query-eligibilities-failed-userid-invalid");
				AB_OSS_INTERFACE_TRACE_END(TEXT("UserId is not valid at user index '%d'!"), LocalUserNum);
				TriggerOnQueryEligibilitiesCompletedDelegates(LocalUserNum, false, TArray<FAccelByteModelsRetrieveUserEligibilitiesResponse>{}, ErrorStr);
				TriggerAccelByteOnQueryEligibilitiesCompletedDelegates(LocalUserNum, true, TArray<FAccelByteModelsRetrieveUserEligibilitiesResponse>{}, ONLINE_ERROR_ACCELBYTE(ErrorStr));

				return false;
			}
		}
	}

	const FString ErrorStr = TEXT("query-eligibilities-failed-not-logged-in");
	AB_OSS_INTERFACE_TRACE_END(TEXT("User not logged in at user index '%d'!"), LocalUserNum);
	TriggerOnQueryEligibilitiesCompletedDelegates(LocalUserNum, false, TArray<FAccelByteModelsRetrieveUserEligibilitiesResponse>{}, ErrorStr);
	TriggerAccelByteOnQueryEligibilitiesCompletedDelegates(LocalUserNum, true, TArray<FAccelByteModelsRetrieveUserEligibilitiesResponse>{}, ONLINE_ERROR_ACCELBYTE(ErrorStr));

	return false;
}
#undef ONLINE_ERROR_NAMESPACE

bool FOnlineAgreementAccelByte::GetEligibleAgreements(int32 LocalUserNum, TArray<TSharedRef<FAccelByteModelsRetrieveUserEligibilitiesResponse>>& OutEligibilites)
{
	const IOnlineIdentityPtr IdentityInterface = AccelByteSubsystem->GetIdentityInterface();
	TArray<TSharedRef<FAccelByteModelsRetrieveUserEligibilitiesResponse>> Eligibilities = EligibilitiesMap.FindRef(IdentityInterface->GetUniquePlayerId(LocalUserNum)->AsShared());
	if (Eligibilities.Num() > 0)
	{
		for (TSharedRef<FAccelByteModelsRetrieveUserEligibilitiesResponse> Eligibility : Eligibilities)
		{
			OutEligibilites.Add(Eligibility);
		}
		return true;
	}
	return false;
}

#define ONLINE_ERROR_NAMESPACE "FOnlineAccelByteGetLocalizedPolicyContent"
bool FOnlineAgreementAccelByte::GetLocalizedPolicyContent(int32 LocalUserNum, const FString& BasePolicyId, const FString& LocaleCode, bool bAlwaysRequestToService)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("Get Localized content, BasePolicy: %s, LocalUserNum: %d"), *BasePolicyId, LocalUserNum);

	const IOnlineIdentityPtr IdentityInterface = AccelByteSubsystem->GetIdentityInterface();
	if (IdentityInterface.IsValid())
	{
		// Check whether user is connected or not yet
		if (IdentityInterface->GetLoginStatus(LocalUserNum) == ELoginStatus::LoggedIn)
		{
			const TSharedPtr<const FUniqueNetId> UserIdPtr = IdentityInterface->GetUniquePlayerId(LocalUserNum);
			TSharedPtr<FUserOnlineAccount> UserAccount;
			if (UserIdPtr.IsValid())
			{
				AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGetLocalizedPolicyContent>(AccelByteSubsystem, *UserIdPtr.Get(), BasePolicyId, LocaleCode, bAlwaysRequestToService);
				AB_OSS_INTERFACE_TRACE_END(TEXT("Dispatching async task to attempt to get localized policy content!"));

				return true;
			}
			else
			{
				const FString ErrorStr = TEXT("get-localized-content-failed-userid-invalid");
				AB_OSS_INTERFACE_TRACE_END(TEXT("UserId is not valid at user index '%d'!"), LocalUserNum);
				TriggerOnGetLocalizedPolicyContentCompletedDelegates(LocalUserNum, false, TEXT(""), ErrorStr);
				TriggerAccelByteOnGetLocalizedPolicyContentCompletedDelegates(LocalUserNum, false, TEXT(""), ONLINE_ERROR_ACCELBYTE(ErrorStr));

				return false;
			}
		}
	}

	const FString ErrorStr = TEXT("get-localized-content-failed-not-logged-in");
	AB_OSS_INTERFACE_TRACE_END(TEXT("User not logged in at user index '%d'!"), LocalUserNum);
	TriggerOnGetLocalizedPolicyContentCompletedDelegates(LocalUserNum, false, TEXT(""), ErrorStr);
	TriggerAccelByteOnGetLocalizedPolicyContentCompletedDelegates(LocalUserNum, false, TEXT(""), ONLINE_ERROR_ACCELBYTE(ErrorStr));

	return false;
	
}
#undef ONLINE_ERROR_NAMESPACE

bool FOnlineAgreementAccelByte::GetLocalizedPolicyContentFromCache(const FString& BasePolicyId, const FString& LocaleCode, FString& OutLocalizedPolicyContent)
{
	if (LocalizedContentMap.Contains(BasePolicyId))
	{
		auto LocalizedContents = LocalizedContentMap.FindRef(BasePolicyId);
		for (const auto& LocalizedContent : LocalizedContents)
		{
			if (LocalizedContent.LocaleCode == LocaleCode)
			{
				OutLocalizedPolicyContent = LocalizedContent.Content;
				return true;
			}
		}
	}
	return false;
}

#define ONLINE_ERROR_NAMESPACE "FOnlineAccelByteAcceptAgreementPolicies"
bool FOnlineAgreementAccelByte::AcceptAgreementPolicies(int32 LocalUserNum, const TArray<FABAcceptAgreementPoliciesRequest>& DocumentsToAccept)
{
	AB_OSS_INTERFACE_TRACE_BEGIN(TEXT("Accept Agreement Policies, LocalUserNum: %d"), LocalUserNum);

	const IOnlineIdentityPtr IdentityInterface = AccelByteSubsystem->GetIdentityInterface();
	if (IdentityInterface.IsValid())
	{
		// Check whether user is connected or not yet
		if (IdentityInterface->GetLoginStatus(LocalUserNum) == ELoginStatus::LoggedIn)
		{
			const TSharedPtr<const FUniqueNetId> UserIdPtr = IdentityInterface->GetUniquePlayerId(LocalUserNum);
			TSharedPtr<FUserOnlineAccount> UserAccount;
			if (UserIdPtr.IsValid())
			{
				AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteAcceptAgreementPolicies>(AccelByteSubsystem, *UserIdPtr.Get(), DocumentsToAccept);
				AB_OSS_INTERFACE_TRACE_END(TEXT("Dispatching async task to attempt to accept agreement policies!"));

				return true;
			}
			else
			{
				const FString ErrorStr = TEXT("accept-policies-failed-userid-invalid");
				AB_OSS_INTERFACE_TRACE_END(TEXT("UserId is not valid at user index '%d'!"), LocalUserNum);
				TriggerOnAcceptAgreementPoliciesCompletedDelegates(LocalUserNum, false, ErrorStr);
				TriggerAccelByteOnAcceptAgreementPoliciesCompletedDelegates(LocalUserNum, false, ONLINE_ERROR_ACCELBYTE(ErrorStr));

				return false;
			}
		}
	}

	const FString ErrorStr = TEXT("accept-policies-failed-not-logged-in");
	AB_OSS_INTERFACE_TRACE_END(TEXT("User not logged in at user index '%d'!"), LocalUserNum);
	TriggerOnAcceptAgreementPoliciesCompletedDelegates(LocalUserNum, false, ErrorStr);
	TriggerAccelByteOnAcceptAgreementPoliciesCompletedDelegates(LocalUserNum, false, ONLINE_ERROR_ACCELBYTE(ErrorStr));

	return false;
}
#undef ONLINE_ERROR_NAMESPACE