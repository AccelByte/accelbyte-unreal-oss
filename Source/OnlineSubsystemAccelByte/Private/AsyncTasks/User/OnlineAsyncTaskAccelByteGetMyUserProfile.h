// Copyright (c) 2026 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "Models/AccelByteUserProfileModels.h"
#include "OnlineUserInterfaceAccelByte.h"

class FOnlineAsyncTaskAccelByteGetMyUserProfile
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteGetMyUserProfile, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteGetMyUserProfile(FOnlineSubsystemAccelByte* const InABSubsystem, int32 InLocalUserNum, const FOnGetMyUserProfileComplete& InDelegate);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteGetMyUserProfile");
	}

private:
	/**
	 * Complete user profile information from the backend including private data
	 */
	FAccelByteModelsUserProfileInfo UserProfileInfo;

	/**
	 * Online user account for the user requested the task
	 */
	TSharedPtr<FUserOnlineAccountAccelByte> Account;

	/** String containing an error if one was encountered */
	FString ErrorString;

	/** Delegate to call when the get my user profile request completes */
	FOnGetMyUserProfileComplete Delegate;

	/**
	 * Delegate handler for when getting the current user's complete profile from the AccelByte SDK succeeds.
	 *
	 * @param Result Complete information about the user profile including private data
	 */
	void OnGetMyUserProfileSuccess(const FAccelByteModelsUserProfileInfo& Result);
	THandler<FAccelByteModelsUserProfileInfo> OnGetMyUserProfileSuccessDelegate;

	/**
	 * Delegate handler for when getting the current user's profile fails.
	 *
	 * @param ErrorCode Code returned from the backend that represents the error encountered for the request
	 * @param ErrorMessage Message from the backend that describes the error encountered
	 */
	void OnGetMyUserProfileError(int32 ErrorCode, const FString& ErrorMessage);
	FErrorHandler OnGetMyUserProfileErrorDelegate;

	/**
	 * Method to get the authenticated user's own complete profile
	 */
	void GetMyUserProfile();
};
