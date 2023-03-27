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

DECLARE_MULTICAST_DELEGATE_FourParams(FOnListUserByUserIdComplete, int32 /*LocalUserNum*/, bool /*bWasSuccessful*/, const FListUserDataResponse& /*Data*/, const FOnlineError  & /* OnlineError  */);
typedef FOnListUserByUserIdComplete::FDelegate FOnListUserByUserIdCompleteDelegate;

class ONLINESUBSYSTEMACCELBYTE_API FOnlineUserAccelByte : public IOnlineUser, public TSharedFromThis<FOnlineUserAccelByte, ESPMode::ThreadSafe>
{
public:
	FOnlineUserAccelByte(FOnlineSubsystemAccelByte* InSubsystem);
	virtual ~FOnlineUserAccelByte() override = default;

	/**
	 * Convenience method to get an instance of this interface from the subsystem associated with the world passed in.
	 *
	 * @param World World instance that we wish to get the interface from
	 * @param OutInterfaceInstance Instance of the interface that we got from the subsystem, or nullptr if not found
	 * @returns boolean that is true if we could get an instance of the interface, false otherwise
	 */
	static bool GetFromWorld(const UWorld* World, TSharedPtr<FOnlineUserAccelByte, ESPMode::ThreadSafe>& OutInterfaceInstance);

	/**
	 * Convenience method to get an instance of this interface from the subsystem passed in.
	 *
	 * @param Subsystem Subsystem instance that we wish to get this interface from
	 * @param OutInterfaceInstance Instance of the interface that we got from the subsystem, or nullptr if not found
	 * @returns boolean that is true if we could get an instance of the interface, false otherwise
	 */
	static bool GetFromSubsystem(const IOnlineSubsystem* Subsystem, TSharedPtr<FOnlineUserAccelByte, ESPMode::ThreadSafe>& OutInterfaceInstance);

	//~ Begin IOnlineUser overrides
	virtual bool QueryUserInfo(int32 LocalUserNum, const TArray<TSharedRef<const FUniqueNetId>>& UserIds) override;
	virtual bool GetAllUserInfo(int32 LocalUserNum, TArray<TSharedRef<class FOnlineUser>>& OutUsers) override;
	virtual TSharedPtr<FOnlineUser> GetUserInfo(int32 LocalUserNum, const class FUniqueNetId& UserId) override;
	virtual bool QueryUserIdMapping(const FUniqueNetId& UserId, const FString& DisplayNameOrEmail, const FOnQueryUserMappingComplete& Delegate = FOnQueryUserMappingComplete()) override;
	virtual bool QueryExternalIdMappings(const FUniqueNetId& UserId, const FExternalIdQueryOptions& QueryOptions, const TArray<FString>& ExternalIds, const FOnQueryExternalIdMappingsComplete& Delegate = FOnQueryExternalIdMappingsComplete()) override;
	virtual void GetExternalIdMappings(const FExternalIdQueryOptions& QueryOptions, const TArray<FString>& ExternalIds, TArray<TSharedPtr<const FUniqueNetId>>& OutIds) override;
	virtual TSharedPtr<const FUniqueNetId> GetExternalIdMapping(const FExternalIdQueryOptions& QueryOptions, const FString& ExternalId) override;
	//~ End IOnlineUser overrides

	/**
	 * Get bulk list user by UserId. Only for request by Game Server
	 * 
	 * @param LocalUserNum Index of user(server) that is attempting to create the stats
	 * @param UserIds User to create stats for 
	 */
	virtual void ListUserByUserId(const int32 LocalUserNum, const TArray<FString>& UserIds);
	
	DEFINE_ONLINE_PLAYER_DELEGATE_THREE_PARAM(MAX_LOCAL_PLAYERS, OnListUserByUserIdComplete, bool /*bWasSuccessful*/, const FListUserDataResponse& /* ListUser*/, const FOnlineError & /* OnlineError */);
 	
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
