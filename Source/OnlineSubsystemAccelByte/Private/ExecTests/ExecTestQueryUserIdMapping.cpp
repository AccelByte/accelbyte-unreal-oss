// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#if WITH_DEV_AUTOMATION_TESTS

#include "ExecTestQueryUserIdMapping.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlineUserInterface.h"

FExecTestQueryUserIdMapping::FExecTestQueryUserIdMapping(UWorld* InWorld, const FName& InSubsystemName, FString InDisplayNameOrEmail)
	: FExecTestBase(InWorld, InSubsystemName)
	, InitialDisplayNameOrEmail(InDisplayNameOrEmail)
{
}

bool FExecTestQueryUserIdMapping::Run()
{
	const IOnlineSubsystem* Subsystem = ::Online::GetSubsystem(World, SubsystemName);
	if (Subsystem != nullptr)
	{
		const IOnlineIdentityPtr IdentityInterface = Subsystem->GetIdentityInterface();
		if (IdentityInterface != nullptr)
		{
			// Get the initial user's FUniqueNetId and then send off a request to map the DisplayNameOrEmail to a user ID
			TSharedPtr<const FUniqueNetId> UserId = IdentityInterface->GetUniquePlayerId(0);
			
			const IOnlineUserPtr UserInterface = Subsystem->GetUserInterface();
			if (UserInterface.IsValid())
			{
				IOnlineUser::FOnQueryUserMappingComplete UserMappingCompleteDelegate = IOnlineUser::FOnQueryUserMappingComplete::CreateSP(AsShared(), &FExecTestQueryUserIdMapping::OnQueryUserIdMappingComplete);
				UserInterface->QueryUserIdMapping(UserId.ToSharedRef().Get(), InitialDisplayNameOrEmail, UserMappingCompleteDelegate);

				return true;
			}
		}
	}

	return false;
}

void FExecTestQueryUserIdMapping::OnQueryUserIdMappingComplete(bool bWasSuccessful, const FUniqueNetId& CallerUserId, const FString& DisplayNameOrEmail, const FUniqueNetId& FoundUserId, const FString& Error)
{
	bIsComplete = true;

	if (!bWasSuccessful)
	{
		UE_LOG_AB(Error, TEXT("Request to query user ID from display name or email in FExecTestQueryUserIdMapping failed! Error: %s"), *Error);
		return;
	}

	if (!FoundUserId.IsValid())
	{
		UE_LOG_AB(Error, TEXT("User ID from query in FExecTestQueryUserIdMapping was invalid! ID: %s"), *FoundUserId.ToString());
		return;
	}

	UE_LOG_AB(Log, TEXT("Found user ID '%s' for '%s'"), *FoundUserId.ToString(), *DisplayNameOrEmail);
}

#endif
