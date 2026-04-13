// Copyright (c) 2026 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "Models/AccelByteUserProfileModels.h"
#include "OnlineUserInterfaceAccelByte.h"

class FOnlineAsyncTaskAccelByteGetPublicUserProfileInfo
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteGetPublicUserProfileInfo, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteGetPublicUserProfileInfo(FOnlineSubsystemAccelByte* const InABSubsystem, int32 InLocalUserNum, const FString& InUserId, const FOnGetPublicUserProfileInfoComplete& InDelegate);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteGetPublicUserProfileInfo");
	}

private:
	/**
	 * UserId to look up the user profile
	 */
	FString UserId;

	/**
	 * Public user profile information from the backend
	 */
	FAccelByteModelsPublicUserProfileInfo PublicUserProfileInfo;

	/** String containing an error if one was encountered */
	FString ErrorString;

	/** Delegate to call when the get public user profile info request completes */
	FOnGetPublicUserProfileInfoComplete Delegate;

	/**
	 * Delegate handler for when getting a user's public profile by UserId from the AccelByte SDK succeeds.
	 *
	 * @param Result Public user profile information
	 */
	void OnGetPublicUserProfileInfoSuccess(const FAccelByteModelsPublicUserProfileInfo& Result);
	THandler<FAccelByteModelsPublicUserProfileInfo> OnGetPublicUserProfileInfoSuccessDelegate;

	/**
	 * Delegate handler for when getting a user's public profile by UserId fails.
	 *
	 * @param ErrorCode Code returned from the backend that represents the error encountered for the request
	 * @param ErrorMessage Message from the backend that describes the error encountered
	 */
	void OnGetPublicUserProfileInfoError(int32 ErrorCode, const FString& ErrorMessage);
	FErrorHandler OnGetPublicUserProfileInfoErrorDelegate;

	/**
	 * Method to get public user profile by UserId
	 */
	void GetPublicUserProfileInfo();
};
