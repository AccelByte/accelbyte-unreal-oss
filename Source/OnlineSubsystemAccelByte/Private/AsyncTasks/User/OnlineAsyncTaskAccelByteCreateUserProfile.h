// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineUserInterfaceAccelByte.h"
#include "Models/AccelByteUserModels.h"

class FOnlineAsyncTaskAccelByteCreateUserProfile : public FOnlineAsyncTaskAccelByte, public TSelfPtr<FOnlineAsyncTaskAccelByteCreateUserProfile, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteCreateUserProfile(FOnlineSubsystemAccelByte* const InABSubsystem, const FUniqueNetId& InUserId);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteCreateUserProfile");
	}

private:
	/**
	 * Online user account for the user requested the task
	 */
	TSharedPtr<FUserOnlineAccountAccelByte> Account;

	/** String containing an error if one was encountered */
	FString ErrorString;

	/**
	 * Method to create a blank user profile for the user when they first login and if they don't already have a profile
	 * Should only be called in the circumstance that the user already does not have a profile set up on the game namespace.
	 */
	void CreateUserProfile();

	/**
	 * Delegate handler for when getting the current user's profile from the game namespace from the AccelByte SDK succeeds.
	 *
	 * @param Result Information about the user profile that was received or created
	 * @param bWasCreated Whether or not this user profile was retrieved or freshly created
	 */
	void OnCreateUserProfileSuccess(const FAccelByteModelsUserProfileInfo& Result);
	THandler<FAccelByteModelsUserProfileInfo> OnCreateProfileSuccessDelegate;

	/**
	 * Delegate handler for when creating a new user profile on the game namespace fails.
	 *
	 * @param ErrorCode Code returned from the backend that represents the error encountered for the request
	 * @param ErrorMessage Message from the backend that describes the error encountered
	 */
	void OnCreateUserProfileError(int32 ErrorCode, const FString& ErrorMessage);
	FErrorHandler OnCreateProfileErrorDelegate;

	bool bWasCreated;
};