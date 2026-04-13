// Copyright (c) 2026 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "Models/AccelByteUserModels.h"

class FOnlineAsyncTaskAccelByteUpdateUserProfile
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteUpdateUserProfile, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteUpdateUserProfile(FOnlineSubsystemAccelByte* const InABSubsystem, int32 InLocalUserNum, const FUniqueNetId& InUserId, const FAccelByteModelsUserProfileUpdateRequest& InUpdateRequest);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteUpdateUserProfile");
	}

private:
	/**
	 * Update request containing the profile data to update.
	 * Fields like FirstName, LastName, AvatarUrl, Language, Timezone, etc.
	 * @see FAccelByteModelsUserProfileUpdateRequest for complete field specification
	 */
	FAccelByteModelsUserProfileUpdateRequest UpdateRequest;

	/**
	 * Online user account for the user requested the task
	 */
	TSharedPtr<FUserOnlineAccountAccelByte> Account;

	/** String containing an error if one was encountered */
	FString ErrorString;

	/**
	 * Delegate handler for when updating the current user's profile from the game namespace from the AccelByte SDK succeeds.
	 *
	 * IMPORTANT: This method caches more user attributes than CreateUserProfile or QueryUserProfile:
	 * - UpdateUserProfile caches: AvatarURL, AvatarSmallURL, AvatarLargeURL, UserLanguage, FirstName,
	 *   LastName, Timezone, DateOfBirth, ZipCode
	 * - CreateUserProfile caches: AvatarURL, AvatarSmallURL, AvatarLargeURL, UserLanguage only
	 * - QueryUserProfile caches: AvatarURL, AvatarSmallURL, AvatarLargeURL, UserLanguage only
	 *
	 * This means GetUserAttribute() calls for FirstName, LastName, Timezone, DateOfBirth, and ZipCode
	 * will return values after UpdateUserProfile but may return empty strings after CreateUserProfile
	 * or QueryUserProfile, depending on which was called last.
	 *
	 * @param Result Information about the user profile that was updated
	 */
	void OnUpdateUserProfileSuccess(const FAccelByteModelsUserProfileInfo& Result);
	THandler<FAccelByteModelsUserProfileInfo> OnUpdateProfileSuccessDelegate;

	/**
	 * Delegate handler for when updating a user profile on the game namespace fails.
	 *
	 * @param ErrorCode Code returned from the backend that represents the error encountered for the request
	 * @param ErrorMessage Message from the backend that describes the error encountered
	 */
	void OnUpdateUserProfileError(int32 ErrorCode, const FString& ErrorMessage);
	FErrorHandler OnUpdateProfileErrorDelegate;

	/** Payload to be sent to analytics */
	FJsonObjectWrapper UserProfileUpdatedFieldsPayload{};

	/**
	 * Method to update user profile for the user
	 */
	void UpdateUserProfile();
};
