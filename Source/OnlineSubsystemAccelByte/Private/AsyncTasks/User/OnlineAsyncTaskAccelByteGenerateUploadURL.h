// Copyright (c) 2026 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "Models/AccelByteUserProfileModels.h"
#include "Models/AccelByteGeneralModels.h"
#include "OnlineUserInterfaceAccelByte.h"

class FOnlineAsyncTaskAccelByteGenerateUploadURL
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteGenerateUploadURL, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteGenerateUploadURL(FOnlineSubsystemAccelByte* const InABSubsystem, int32 InLocalUserNum, const FString& InFolder, EAccelByteFileType InFileType, const FOnGenerateUploadURLComplete& InDelegate);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteGenerateUploadURL");
	}

private:
	FString Folder;
	EAccelByteFileType FileType;
	FAccelByteModelsUserProfileUploadURLResult UploadURLResult;
	FString ErrorString;
	FOnGenerateUploadURLComplete Delegate;

	void OnGenerateUploadURLSuccess(const FAccelByteModelsUserProfileUploadURLResult& Result);
	THandler<FAccelByteModelsUserProfileUploadURLResult> OnGenerateUploadURLSuccessDelegate;

	void OnGenerateUploadURLError(int32 ErrorCode, const FString& ErrorMessage);
	FErrorHandler OnGenerateUploadURLErrorDelegate;

	void GenerateUploadURL();
};
