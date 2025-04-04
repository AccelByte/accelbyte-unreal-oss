// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystemTypes.h"
#include "OnlineDelegateMacros.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineUserCacheAccelByte.h"
#include "Models/AccelByteAgreementModels.h"
#include "OnlineErrorAccelByte.h"
#include "OnlineSubsystemAccelBytePackage.h"

DECLARE_MULTICAST_DELEGATE(FOnUserNotComplied);
typedef FOnUserNotComplied::FDelegate FOnUserNotCompliedDelegate;

DECLARE_MULTICAST_DELEGATE_FourParams(FOnQueryEligibilitiesCompleted, int32 /*LocalUserNum*/, bool /*bWasSuccessful*/, const TArray<FAccelByteModelsRetrieveUserEligibilitiesResponse>& /*Response*/, const FString& /*Error*/);
typedef FOnQueryEligibilitiesCompleted::FDelegate FOnQueryEligibilitiesCompletedDelegate;

DECLARE_MULTICAST_DELEGATE_FourParams(FOnGetLocalizedPolicyContentCompleted, int32 /*LocalUserNum*/, bool /*bWasSuccessful*/, const FString& /*Response*/, const FString& /*Error*/);
typedef FOnGetLocalizedPolicyContentCompleted::FDelegate FOnGetLocalizedPolicyContentCompletedDelegate;

DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnAcceptAgreementPoliciesCompleted, int32 /*LocalUserNum*/, bool /*bWasSuccessful*/, const FString& /*Error*/);
typedef FOnAcceptAgreementPoliciesCompleted::FDelegate FOnAcceptAgreementPoliciesCompletedDelegate;

DECLARE_MULTICAST_DELEGATE_FourParams(FAccelByteOnQueryEligibilitiesCompleted, int32 /*LocalUserNum*/, bool /*bWasSuccessful*/, const TArray<FAccelByteModelsRetrieveUserEligibilitiesResponse>& /*Response*/, const FOnlineErrorAccelByte& /*Error*/);
typedef FAccelByteOnQueryEligibilitiesCompleted::FDelegate FAccelByteOnQueryEligibilitiesCompletedDelegate;

DECLARE_MULTICAST_DELEGATE_FourParams(FAccelByteOnGetLocalizedPolicyContentCompleted, int32 /*LocalUserNum*/, bool /*bWasSuccessful*/, const FString& /*Response*/, const FOnlineErrorAccelByte& /*Error*/);
typedef FAccelByteOnGetLocalizedPolicyContentCompleted::FDelegate FAccelByteOnGetLocalizedPolicyContentCompletedDelegate;

DECLARE_MULTICAST_DELEGATE_ThreeParams(FAccelByteOnAcceptAgreementPoliciesCompleted, int32 /*LocalUserNum*/, bool /*bWasSuccessful*/, const FOnlineErrorAccelByte& /*Error*/);
typedef FAccelByteOnAcceptAgreementPoliciesCompleted::FDelegate FAccelByteOnAcceptAgreementPoliciesCompletedDelegate;

struct ONLINESUBSYSTEMACCELBYTE_API FABLocalizedPolicyContent
{
	FString Content{};
	FString LocaleCode{};
};

struct ONLINESUBSYSTEMACCELBYTE_API FABAcceptAgreementPoliciesRequest
{
	FString BasePolicyId{};
	FString LocaleCode{};
};

/**
 * Implementation of Agreement service from AccelByte services
 */
class ONLINESUBSYSTEMACCELBYTE_API FOnlineAgreementAccelByte : public TSharedFromThis<FOnlineAgreementAccelByte, ESPMode::ThreadSafe>
{
PACKAGE_SCOPE:
	/*Map of Eligibilities of each user*/
	TUniqueNetIdMap<TArray<TSharedRef<FAccelByteModelsRetrieveUserEligibilitiesResponse>>> EligibilitiesMap;

	TMap<FString, TArray<FABLocalizedPolicyContent>> LocalizedContentMap;

	/** Constructor that is invoked by the Subsystem instance to create a user cloud instance */
	FOnlineAgreementAccelByte(FOnlineSubsystemAccelByte* InSubsystem)
#if ENGINE_MAJOR_VERSION >= 5
		: AccelByteSubsystem(InSubsystem->AsWeak())
#else
		: AccelByteSubsystem(InSubsystem->AsShared())
#endif
	{}

public:
	virtual ~FOnlineAgreementAccelByte() {};
	
	/**
	 * Convenience method to get an instance of this interface from the subsystem passed in.
	 *
	 * @param Subsystem Subsystem instance that we wish to get this interface from
	 * @param OutInterfaceInstance Instance of the interface that we got from the subsystem, or nullptr if not found
	 * @returns boolean that is true if we could get an instance of the interface, false otherwise
	 */
	static bool GetFromSubsystem(const IOnlineSubsystem* Subsystem, TSharedPtr<FOnlineAgreementAccelByte, ESPMode::ThreadSafe>& OutInterfaceInstance);

	/**
	 * Convenience method to get an instance of this interface from the subsystem associated with the world passed in.
	 *
	 * @param World World instance that we wish to get the interface from
	 * @param OutInterfaceInstance Instance of the interface that we got from the subsystem, or nullptr if not found
	 * @returns boolean that is true if we could get an instance of the interface, false otherwise
	 */
	static bool GetFromWorld(const UWorld* World, TSharedPtr<FOnlineAgreementAccelByte, ESPMode::ThreadSafe>& OutInterfaceInstance);

	DEFINE_ONLINE_PLAYER_DELEGATE(MAX_LOCAL_PLAYERS, OnUserNotComplied);
	DEFINE_ONLINE_PLAYER_DELEGATE_THREE_PARAM(MAX_LOCAL_PLAYERS, OnQueryEligibilitiesCompleted, bool, const TArray<FAccelByteModelsRetrieveUserEligibilitiesResponse>&, const FString&);
	DEFINE_ONLINE_PLAYER_DELEGATE_THREE_PARAM(MAX_LOCAL_PLAYERS, OnGetLocalizedPolicyContentCompleted, bool, const FString&, const FString&);
	DEFINE_ONLINE_PLAYER_DELEGATE_TWO_PARAM(MAX_LOCAL_PLAYERS, OnAcceptAgreementPoliciesCompleted, bool, const FString&);

	DEFINE_ONLINE_PLAYER_DELEGATE_THREE_PARAM(MAX_LOCAL_PLAYERS, AccelByteOnQueryEligibilitiesCompleted, bool, const TArray<FAccelByteModelsRetrieveUserEligibilitiesResponse>&, const FOnlineErrorAccelByte&);
	DEFINE_ONLINE_PLAYER_DELEGATE_THREE_PARAM(MAX_LOCAL_PLAYERS, AccelByteOnGetLocalizedPolicyContentCompleted, bool, const FString&, const FOnlineErrorAccelByte&);
	DEFINE_ONLINE_PLAYER_DELEGATE_TWO_PARAM(MAX_LOCAL_PLAYERS, AccelByteOnAcceptAgreementPoliciesCompleted, bool, const FOnlineErrorAccelByte&);

	/**
	 * @brief Query user's eligibilities
	 * 
	 * @param bNotAcceptedOnly true to only get the eligibilities that not complied yet.
	 * @param bAlwaysRequestToService true to force create request to query the eligibilities from agreement service.
	 */
	bool QueryEligibleAgreements(int32 LocalUserNum, bool bNotAcceptedOnly = false, bool bAlwaysRequestToService = false);

	/*Get the stored user's eligibilities*/
	bool GetEligibleAgreements(int32 LocalUserNum, TArray<TSharedRef<FAccelByteModelsRetrieveUserEligibilitiesResponse>>& OutEligibilites);

	/**
	 * @brief Get the localized policy content
	 *
	 * @param bAlwaysRequestToService true to force create request to query the content from agreement service.
	 */
	bool GetLocalizedPolicyContent(int32 LocalUserNum, const FString& BasePolicyId, const FString& LocaleCode, bool bAlwaysRequestToService = false);

	/*Get the stored localized policy content*/
	bool GetLocalizedPolicyContentFromCache(const FString& BasePolicyId, const FString& LocaleCode, FString& OutLocalizedPolicyContent);

	bool AcceptAgreementPolicies(int32 LocalUserNum, const TArray<FABAcceptAgreementPoliciesRequest>& DocumentsToAccept);

protected:
	/** Hidden default constructor, the constructor that takes in a subsystem instance should be used instead. */
	FOnlineAgreementAccelByte()
		: AccelByteSubsystem(nullptr)
	{}

	/** Instance of the subsystem that created this interface */
	FOnlineSubsystemAccelByteWPtr AccelByteSubsystem = nullptr;

};