// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "Models/AccelByteStatisticModels.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "OnlineStatisticInterfaceAccelByte.h"

/**
 * Task for accept agreement policies
 */
class FOnlineAsyncTaskAccelByteListUserStatItems : public FOnlineAsyncTaskAccelByte, public TSelfPtr<FOnlineAsyncTaskAccelByteListUserStatItems, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteListUserStatItems(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, 
		const TArray<FString>& StatCodes, const TArray<FString>& Tags, const FString& AdditionalKey, bool bAlwaysRequestToService);

	FOnlineAsyncTaskAccelByteListUserStatItems(FOnlineSubsystemAccelByte* const InABInterface, const TSharedRef<const FUniqueNetId> StatsUser,
		const FOnlineStatsQueryUserStatsComplete& Delegate);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteListUserStatItems");
	}

private:

	/**
	 * Delegate handler for when accept users succeed
	 */
	void OnListUserStatItemsSuccess(const TArray<FAccelByteModelsFetchUser>& Result);
	THandler<TArray<FAccelByteModelsFetchUser>> OnListUserStatItemsSuccessDelegate;

	/**
	 * Delegate handler for when accept users fail
	 */
	void OnListUserStatItemsError(int32 ErrorCode, const FString& ErrorMessage);
	FErrorHandler OnListUserStatItemsErrorDelegate;

	/**
	 * The list of users retrieved from Statistic service
	 */
	TArray<FAccelByteModelsFetchUser> Users;
	
	/**
	 * String representing the error code that occurred
	 */
	FString ErrorStr;

	TArray<FString> StatCodes;

	TArray<FString> Tags;

	FString AdditionalKey;

	bool bRequestResult;
	bool bAlwaysRequestToService;
};
