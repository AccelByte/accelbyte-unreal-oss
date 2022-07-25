// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#if ENGINE_MAJOR_VERSION >= 5
#include "Online/CoreOnline.h"
#else
#include "UObject/CoreOnline.h"
#endif
#include "OnlineSubsystemAccelByte.h"
#include "OnlineUserInterfaceAccelByte.h"
#include "ExecTestBase.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * Test case for FOnlineUserAccelByte::QueryUserIdMapping.
 * 
 * Console command for running is as follows:
 * ONLINE TEST USER MAP "<DisplayNameOrEmail>"
 */
class FExecTestQueryUserIdMapping : public FExecTestBase, public TSharedFromThis<FExecTestQueryUserIdMapping>
{
public:

	/**
	 * Constructs an instance of the test case to query a user ID by a display name or email.
	 * 
	 * @param DisplayNameOrEmail User's display name or email address to query by, must be exact match
	 */
	FExecTestQueryUserIdMapping(UWorld* InWorld, const FName& InSubsystemName, FString InDisplayNameOrEmail);

	virtual bool Run() override;

private:

	/** Display name or email address that we want to query, must be exact match */
	FString InitialDisplayNameOrEmail;

	/** Delegate callback fired when the task the map a user display name or email is finished. */
	void OnQueryUserIdMappingComplete(bool bWasSuccessful, const FUniqueNetId& CallerUserId, const FString& DisplayNameOrEmail, const FUniqueNetId& FoundUserId, const FString& Error);

};

#endif