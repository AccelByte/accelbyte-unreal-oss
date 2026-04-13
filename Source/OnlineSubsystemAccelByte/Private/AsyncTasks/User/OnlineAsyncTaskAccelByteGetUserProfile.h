// Copyright (c) 2026 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineUserInterfaceAccelByte.h"
#include "Models/AccelByteUserProfileModels.h"

class FOnlineAsyncTaskAccelByteGetUserProfile
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteGetUserProfile, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteGetUserProfile(FOnlineSubsystemAccelByte* const InABSubsystem,
		int32 InLocalUserNum,
		const FString& InUserId,
		const FOnGetUserProfileComplete& InDelegate);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteGetUserProfile");
	}

private:

	/**
	 * UserId to get the complete profile for
	 */
	FString UserId;

	/**
	 * Delegate fired when we get a response back from the backend
	 */
	FOnGetUserProfileComplete Delegate;

	/**
	 * String representing the error code that occurred
	 */
	FString ErrorString;

	/**
	 * Profile information retrieved from backend
	 */
	FAccelByteModelsUserProfileInfo UserProfileInfo;

	/**
	 * Delegate handler for when getting the user profile from the AccelByte SDK succeeds.
	 *
	 * @param Result Complete information about the user profile including private data
	 */
	void OnGetUserProfileSuccess(const FAccelByteModelsUserProfileInfo& Result);
	THandler<FAccelByteModelsUserProfileInfo> OnGetUserProfileSuccessDelegate;

	/**
	 * Delegate handler for when getting the user profile fails.
	 *
	 * @param ErrorCode Code returned from the backend that represents the error encountered for the request
	 * @param ErrorMessage Message from the backend that describes the error encountered
	 */
	void OnGetUserProfileError(int32 ErrorCode, const FString& ErrorMessage);
	FErrorHandler OnGetUserProfileErrorDelegate;

	/**
	 * Method to get the specific user's complete profile by UserId
	 */
	void GetUserProfile();
};