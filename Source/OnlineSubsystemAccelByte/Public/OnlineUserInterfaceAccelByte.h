// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#if ENGINE_MAJOR_VERSION >= 5
#include "Online/CoreOnline.h"
#else
#include "UObject/CoreOnline.h"
#endif

#include "Interfaces/OnlineUserInterface.h"
#include "OnlineSubsystemTypes.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineSubsystemAccelByteTypes.h"

/**
 * Delegate that denotes when a user report has completed.
 *
 * @param bWasSuccessful true if the report was sent successfully, false otherwise.
 */
DECLARE_DELEGATE_OneParam(FOnReportUserComplete, bool /*bWasSuccessful*/);

class ONLINESUBSYSTEMACCELBYTE_API FOnlineUserAccelByte : public IOnlineUser, public TSharedFromThis<FOnlineUserAccelByte, ESPMode::ThreadSafe>
{
public:
	FOnlineUserAccelByte(FOnlineSubsystemAccelByte* InSubsystem);

	virtual ~FOnlineUserAccelByte() override = default;

	//~ Begin IOnlineUser overrides
	virtual bool QueryUserInfo(int32 LocalUserNum, const TArray<TSharedRef<const FUniqueNetId>>& UserIds) override;
	virtual bool GetAllUserInfo(int32 LocalUserNum, TArray<TSharedRef<class FOnlineUser>>& OutUsers) override;
	virtual TSharedPtr<FOnlineUser> GetUserInfo(int32 LocalUserNum, const class FUniqueNetId& UserId) override;
	virtual bool QueryUserIdMapping(const FUniqueNetId& UserId, const FString& DisplayNameOrEmail, const FOnQueryUserMappingComplete& Delegate = FOnQueryUserMappingComplete()) override;
	virtual bool QueryExternalIdMappings(const FUniqueNetId& UserId, const FExternalIdQueryOptions& QueryOptions, const TArray<FString>& ExternalIds, const FOnQueryExternalIdMappingsComplete& Delegate = FOnQueryExternalIdMappingsComplete()) override;
	virtual void GetExternalIdMappings(const FExternalIdQueryOptions& QueryOptions, const TArray<FString>& ExternalIds, TArray<TSharedPtr<const FUniqueNetId>>& OutIds) override;
	virtual TSharedPtr<const FUniqueNetId> GetExternalIdMapping(const FExternalIdQueryOptions& QueryOptions, const FString& ExternalId) override;
	//~ End IOnlineUser overrides

PACKAGE_SCOPE:

#if WITH_DEV_AUTOMATION_TESTS
	/**
	 * Internal method for handling extra exec tests for this interface.
	 */
	bool TestExec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar);
#endif

	/**
	 * Internal method used by FOnlineAsyncTaskAccelByteQueryUserInfo to move user info instances back to this interface.
	 */
	void AddUserInfo(const TSharedRef<const FUniqueNetId>& UserId, const TSharedRef<FUserOnlineAccountAccelByte>& UserInfo)
	{
		IDToUserInfoMap.Add(UserId, UserInfo);
	}

	/**
	 * Internal method used by FOnlineAsyncTaskAccelByteQueryExternalIdMappings to move external ID to AccelByte ID references back to this interface.
	 */
	void AddExternalIdMappings(const TMap<FString, TSharedRef<const FUniqueNetId>>& InMap)
	{
		ExternalIDToAccelByteIDMap.Append(InMap);
	}
private:
	/** Pointer to the AccelByte OSS instance that instantiated this online user interface. */
	FOnlineSubsystemAccelByte* AccelByteSubsystem = nullptr;

	/** Map of AccelByte IDs to AccelByte accounts */
	TUniqueNetIdMap<TSharedRef<FUserOnlineAccountAccelByte>> IDToUserInfoMap;

	/** Map of external IDs (external platform user IDs or display names) to AccelByte IDs */
	TMap<FString, TSharedRef<const FUniqueNetId>> ExternalIDToAccelByteIDMap;
};
