// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "Runtime/Launch/Resources/Version.h"
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
 * Test case for FOnlineUserAccelByte::QueryExternalIdMappings.
 * 
 * Console command for running is as follows:
 * ONLINE TEST USER EXTERNAL <space separated external IDs)
 */
class FExecTestQueryExternalIds : public FExecTestBase, public TSharedFromThis<FExecTestQueryExternalIds>
{
public:

	/**
	 * Constructs an instance of the query external ID test case.
	 * 
	 * @param Type Type of user ID that we are trying to query, eg. live
	 * @param ExternalIds Array of External ID strings that will be passed to the query call
	 */
	FExecTestQueryExternalIds(UWorld* InWorld, const FName& InSubsystemName, const FString& InType, const TArray<FString>& InExternalIds);

	virtual bool Run() override;

private:

	/**
	 * Type of external IDs that we are trying to query, eg. live
	 *
	 * External ID types can be found in the AccelByte SDK in EAccelBytePlatformType.
	 */
	FString Type;

	/** Array of strings representing external IDs to query */
	TArray<FString> ExternalIds;

	/** Delegate callback for when the OSS task to query external IDs completes */
	void OnQueryExternalIdMappingsComplete(bool bWasSuccessful, const FUniqueNetId& UserId, const FExternalIdQueryOptions& QueryOptions, const TArray<FString>& FoundIds, const FString& Error);

};

#endif