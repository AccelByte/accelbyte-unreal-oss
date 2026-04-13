// Copyright (c) 2026 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineUserInterfaceAccelByte.h"
#include "Models/AccelByteUserProfileModels.h"

class FOnlineAsyncTaskAccelByteBulkGetPublicUserProfileInfosV2
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteBulkGetPublicUserProfileInfosV2, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteBulkGetPublicUserProfileInfosV2(FOnlineSubsystemAccelByte* const InABSubsystem,
		int32 InLocalUserNum,
		const TArray<FString>& InUserIds,
		const FOnBulkGetPublicUserProfileInfosV2Complete& InDelegate);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteBulkGetPublicUserProfileInfosV2");
	}

private:

	/**
	 * UserIds to get the public profile infos for
	 */
	TArray<FString> UserIds;

	/**
	 * Delegate fired when we get a response back from the backend
	 */
	FOnBulkGetPublicUserProfileInfosV2Complete Delegate;

	/**
	 * String representing the error code that occurred
	 */
	FString ErrorString;

	/**
	 * Profile information retrieved from backend
	 */
	FAccelByteModelsPublicUserProfileInfoV2 PublicUserProfileInfoV2;

	/**
	 * Delegate handler for when getting the bulk public user profile infos from the AccelByte SDK succeeds.
	 *
	 * @param Result Bulk public user profile information with NotProcessed array
	 */
	void OnBulkGetPublicUserProfileInfosV2Success(const FAccelByteModelsPublicUserProfileInfoV2& Result);
	THandler<FAccelByteModelsPublicUserProfileInfoV2> OnBulkGetPublicUserProfileInfosV2SuccessDelegate;

	/**
	 * Delegate handler for when getting the bulk public user profile infos fails.
	 *
	 * @param ErrorCode Code returned from the backend that represents the error encountered for the request
	 * @param ErrorMessage Message from the backend that describes the error encountered
	 */
	void OnBulkGetPublicUserProfileInfosV2Error(int32 ErrorCode, const FString& ErrorMessage);
	FErrorHandler OnBulkGetPublicUserProfileInfosV2ErrorDelegate;

	/**
	 * Method to get bulk public user profile infos with V2 endpoint
	 */
	void BulkGetPublicUserProfileInfosV2();

};