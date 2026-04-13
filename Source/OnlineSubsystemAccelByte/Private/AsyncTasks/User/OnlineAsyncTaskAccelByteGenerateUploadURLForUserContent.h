// Copyright (c) 2026 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "Models/AccelByteUserProfileModels.h"
#include "OnlineUserInterfaceAccelByte.h"

class FOnlineAsyncTaskAccelByteGenerateUploadURLForUserContent
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteGenerateUploadURLForUserContent, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteGenerateUploadURLForUserContent(FOnlineSubsystemAccelByte* const InABSubsystem, int32 InLocalUserNum, const FString& InUserId, EAccelByteFileType InFileType, EAccelByteUploadCategory InCategory, const FOnGenerateUploadURLForUserContentComplete& InDelegate);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteGenerateUploadURLForUserContent");
	}

private:
	/**
	 * UserId for the content upload
	 */
	FString UserId;

	/**
	 * File type for the upload URL
	 */
	EAccelByteFileType FileType;

	/**
	 * Upload category (DEFAULT, REPORTING)
	 */
	EAccelByteUploadCategory Category;

	/**
	 * Upload URL result from the backend
	 */
	FAccelByteModelsUserProfileUploadURLResult UploadURLResult;

	/** String containing an error if one was encountered */
	FString ErrorString;

	/** Delegate to call when the generate upload URL for user content request completes */
	FOnGenerateUploadURLForUserContentComplete Delegate;

	/**
	 * Delegate handler for when generating upload URL for user content from the AccelByte SDK succeeds.
	 *
	 * @param Result Upload URL result containing URLs and access information
	 */
	void OnGenerateUploadURLForUserContentSuccess(const FAccelByteModelsUserProfileUploadURLResult& Result);
	THandler<FAccelByteModelsUserProfileUploadURLResult> OnGenerateUploadURLForUserContentSuccessDelegate;

	/**
	 * Delegate handler for when generating upload URL for user content fails.
	 *
	 * @param ErrorCode Code returned from the backend that represents the error encountered for the request
	 * @param ErrorMessage Message from the backend that describes the error encountered
	 */
	void OnGenerateUploadURLForUserContentError(int32 ErrorCode, const FString& ErrorMessage);
	FErrorHandler OnGenerateUploadURLForUserContentErrorDelegate;

	/**
	 * Method to generate upload URL for user content
	 */
	void GenerateUploadURLForUserContent();
};
