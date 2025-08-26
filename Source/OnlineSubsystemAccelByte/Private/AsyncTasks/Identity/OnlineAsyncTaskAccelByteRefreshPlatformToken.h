// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineUserInterfaceAccelByte.h"

class FOnlineAsyncTaskAccelByteRefreshPlatformToken
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteRefreshPlatformToken, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteRefreshPlatformToken(FOnlineSubsystemAccelByte* const InABSubsystem, int32 LocalUserNum, FName SubsystemName);

	virtual void Initialize() override; 
	virtual void TriggerDelegates() override;

protected:
	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteRefreshPlatformTokens");
	}

private:
	/*
	 * Helper function to get and check current native interface. Return the result success or not.
	 */
	bool CheckAndGetOnlineIdentityInterface(IOnlineIdentityPtr& OutIdentityInterface, EAccelBytePlatformType& OutPlatformType);

	// Constructor member
	FName RefreshSubsystemName{};

	// Endpoint Handlers
	FOnLoginCompleteDelegate NativeLoginComplete{};
	FDelegateHandle NativeLoginCompleteHandle{};
	void OnNativeAutoLoginResponse(int32 NativeLocalUserNum, bool bWasNativeLoginSuccessful, const FUniqueNetId& NativeUserId, const FString& NativeError);
	void ApiClientRefreshToBackend(EAccelBytePlatformType PlatformTypeForIAM, const FString& PlatformToken);
	void HandleSuccess(const FPlatformTokenRefreshResponse& Response);
	void HandleError(int32 ErrorCode, const FString& ErrorMessage, const FErrorOAuthInfo& ErrorInfo);

	// Endpoint Result
	FPlatformTokenRefreshResponse Result{};
	int32 ErrorCode = 0;
	FString ErrorMessage{};
	FErrorOAuthInfo ErrorInfo{};
};
