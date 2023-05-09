// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "Models/AccelByteStatisticModels.h"
#include "OnlineStatisticInterfaceAccelByte.h"

/**
 * Task for accept agreement policies
 */
class FOnlineAsyncTaskAccelByteListUserStatItems
	: public FOnlineAsyncTaskAccelByte
	, public TSelfPtr<FOnlineAsyncTaskAccelByteListUserStatItems, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteListUserStatItems(FOnlineSubsystemAccelByte *const InABInterface
		, int32 InLocalUserNum 
		, TArray<FString> const& StatCodes
		, TArray<FString> const& Tags
		, FString const& AdditionalKey
		, bool bAlwaysRequestToService);
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
