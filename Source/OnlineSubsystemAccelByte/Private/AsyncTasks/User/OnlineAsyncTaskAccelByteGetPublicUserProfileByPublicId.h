// Copyright (c) 2026 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "Models/AccelByteUserProfileModels.h"
#include "OnlineUserInterfaceAccelByte.h"

class FOnlineAsyncTaskAccelByteGetPublicUserProfileByPublicId
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteGetPublicUserProfileByPublicId, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteGetPublicUserProfileByPublicId(FOnlineSubsystemAccelByte* const InABSubsystem, int32 InLocalUserNum, const FString& InPublicId, const FOnGetPublicUserProfileByPublicIdComplete& InDelegate);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteGetPublicUserProfileByPublicId");
	}

private:
	/**
	 * PublicId (friend code) to look up the user profile
	 */
	FString PublicId;

	/**
	 * Public user profile information from the backend
	 */
	FAccelByteModelsPublicUserProfileInfo PublicUserProfileInfo;

	/** String containing an error if one was encountered */
	FString ErrorString;

	/** Delegate to call when the get public user profile by PublicId request completes */
	FOnGetPublicUserProfileByPublicIdComplete Delegate;

	/**
	 * Delegate handler for when getting a user's public profile by PublicId from the AccelByte SDK succeeds.
	 *
	 * @param Result Public user profile information
	 */
	void OnGetPublicUserProfileByPublicIdSuccess(const FAccelByteModelsPublicUserProfileInfo& Result);
	THandler<FAccelByteModelsPublicUserProfileInfo> OnGetPublicUserProfileByPublicIdSuccessDelegate;

	/**
	 * Delegate handler for when getting a user's public profile by PublicId fails.
	 *
	 * @param ErrorCode Code returned from the backend that represents the error encountered for the request
	 * @param ErrorMessage Message from the backend that describes the error encountered
	 * @param JsonObject Additional error information as JSON object
	 */
	void OnGetPublicUserProfileByPublicIdError(int32 ErrorCode, const FString& ErrorMessage, const FJsonObject& JsonObject);
	AccelByte::FCustomErrorHandler OnGetPublicUserProfileByPublicIdErrorDelegate;

	/**
	 * Method to get public user profile by PublicId
	 */
	void GetPublicUserProfileByPublicId();
};
