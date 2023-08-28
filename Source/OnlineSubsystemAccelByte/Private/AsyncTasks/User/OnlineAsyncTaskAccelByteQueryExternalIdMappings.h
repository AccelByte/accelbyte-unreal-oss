// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineUserInterfaceAccelByte.h"
#include "Models/AccelByteUserModels.h"

/**
 * Async task fired by FOnlineUserAccelByte::QueryExternalIdMappings to query the backend to map a platform user ID to
 * an AccelByte user ID.
 */
class FOnlineAsyncTaskAccelByteQueryExternalIdMappings
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteQueryExternalIdMappings, ESPMode::ThreadSafe>
{
public:

	/**
	 * Constructor to set up the async task with data necessary for mapping external IDs.
	 * 
	 * @param InABSubsystem Instance of the AccelByte OnlineSubsystem that this task is attached to
	 * @param InUserId Id of the user that made the call to query these mappings
	 * @param InQueryOptions Options used to determine how the external ID query should be treated
	 * @param InExternalIds Array of strings corresponding to platform user IDs
	 * @param InDelegate Delegate to fire once the async task is complete and we have data back
	 */
	FOnlineAsyncTaskAccelByteQueryExternalIdMappings(FOnlineSubsystemAccelByte* const InABSubsystem, const FUniqueNetId& InUserId, const FExternalIdQueryOptions& InQueryOptions, const TArray<FString>& InExternalIds, const IOnlineUser::FOnQueryExternalIdMappingsComplete& InDelegate);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteQueryExternalIdMappings");
	}

private:

	/** Query options that were passed in when the request was initiated */
	FExternalIdQueryOptions QueryOptions;

	/** Array of external IDs that a mapping was requested for */
	TArray<FString> InitialExternalIds;

	/** Map of external IDs to AccelByte user IDs */
	TMap<FString, TSharedRef<const FUniqueNetId>> ExternalIdToFoundAccelByteIdMap;

	/** Delegate to fire after external ID mappings were found */
	IOnlineUser::FOnQueryExternalIdMappingsComplete Delegate;

	/**
	 * String used to identify the error that occurred to the delegate
	 */
	FString ErrorStr;

	/**
	 * Delegate handler for when querying a bulk number of platform IDs for their corresponding AccelByte IDs is successful
	 */
	void OnBulkGetUserByOtherPlatformIdsSuccess(const FBulkPlatformUserIdResponse& Result);

	/**
	 * Delegate handler for when querying a bulk number of platform IDs for their corresponding AccelByte IDs fails
	 */
	void OnBulkGetUserByOtherPlatformIdsError(int32 ErrorCode, const FString& ErrorMessage);

};