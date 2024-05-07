// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#if WITH_DEV_AUTOMATION_TESTS

#include "ExecTestQueryExternalIds.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlineUserInterface.h"

FExecTestQueryExternalIds::FExecTestQueryExternalIds(UWorld* InWorld, const FName& InSubsystemName, const FString& InType, const TArray<FString>& InExternalIds)
	: FExecTestBase(InWorld, InSubsystemName)
	, Type(InType)
	, ExternalIds(InExternalIds)
{
}

bool FExecTestQueryExternalIds::Run()
{
	IOnlineSubsystem* Subsystem = ::Online::GetSubsystem(World, SubsystemName);

	if (Subsystem != nullptr)
	{
		const IOnlineIdentityPtr IdentityInterface = Subsystem->GetIdentityInterface();

		if (IdentityInterface.IsValid())
		{
			// Grab the first user's FUniqueNetId and then send off a request to map external IDs passed in
			FUniqueNetIdPtr UserId = IdentityInterface->GetUniquePlayerId(TEST_USER_INDEX);
			
			if (!UserId.IsValid())
			{
				return false;
			}
			
			IOnlineUserPtr UserInterface = Subsystem->GetUserInterface();
			if (UserInterface.IsValid())
			{
				IOnlineUser::FOnQueryExternalIdMappingsComplete OnQueryExternalIdMappingsCompleteDelegate = IOnlineUser::FOnQueryExternalIdMappingsComplete::CreateSP(AsShared(), &FExecTestQueryExternalIds::OnQueryExternalIdMappingsComplete);

				FExternalIdQueryOptions Options;
				Options.AuthType = Type;
				UserInterface->QueryExternalIdMappings(UserId.ToSharedRef().Get(), Options, ExternalIds, OnQueryExternalIdMappingsCompleteDelegate);

				return true;
			}
		}
	}

	return false;
}

void FExecTestQueryExternalIds::OnQueryExternalIdMappingsComplete(bool bWasSuccessful, const FUniqueNetId& UserId, const FExternalIdQueryOptions& QueryOptions, const TArray<FString>& FoundIds, const FString& Error)
{
	bIsComplete = true;

	if (!bWasSuccessful)
	{
		UE_LOG_AB(Error, TEXT("Call to QueryExternalIdMappings failed on FExecTestQueryExternalIds. Error: %s"), *Error);
		return;
	}

	const IOnlineSubsystem* Subsystem = ::Online::GetSubsystem(World, SubsystemName);
	if (Subsystem == nullptr)
	{
		UE_LOG_AB(Error, TEXT("Could not get IOnlineSubsystem interface for FExecTestQueryExternalIds"));
		return;
	}

	IOnlineUserPtr UserInterface = Subsystem->GetUserInterface();
	if (!UserInterface.IsValid())
	{
		UE_LOG_AB(Error, TEXT("Could not get IOnlineUser interface for FExecTestQueryExternalIds"));
		return;
	}

	// Log out each external ID that we were able to map, or log an warning if we failed to get a mapping
	for (const FString& ExternalId : FoundIds)
	{
		FUniqueNetIdPtr IdMapping = UserInterface->GetExternalIdMapping(QueryOptions, ExternalId);
		
		if (IdMapping.IsValid())
		{
			UE_LOG_AB(Log, TEXT("Successfully mapped external ID '%s' to AccelByte user ID '%s'!"), *ExternalId, *IdMapping->ToString());
		}
		else
		{
			UE_LOG_AB(Warning, TEXT("Failed to map external ID '%s' to AccelByte user ID!"), *ExternalId);
		}
	}
}

#endif
