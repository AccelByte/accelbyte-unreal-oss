// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"

class FOnlineAsyncTaskAccelByteVerifyLoginMfa
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteVerifyLoginMfa, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteVerifyLoginMfa(FOnlineSubsystemAccelByte* const InABSubsystem, int32 InLocalUserNum
		, const EAccelByteLoginAuthFactorType InFactorType, const FString& InCode, const FString& InMfaToken
		, const bool bInRememberDevice = false);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;
	virtual void OnTaskTimedOut() override;
	virtual void Tick() override;
	
protected:
	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteVerifyLoginMfa");
	}

private:
	/** Variables used for user mfa verification. */
	EAccelByteLoginAuthFactorType FactorType{EAccelByteLoginAuthFactorType::None};
	FString Code{};
	FString MfaToken{};

	/** Option to remember this device after mfa authenticated. */
	bool bRememberDevice{false};

	/** Online error information in case any error happen. */
	FOnlineError OnlineError;

	/** Handlers on verify login completed. */
	void OnVerifyLoginSuccess(const FAccelByteModelsLoginQueueTicketInfo& Response);
	void OnVerifyLoginError(const int32 ResponseErrorCode, const FString& ErrorMessage, const FErrorOAuthInfo& ErrorObject);

	/** Handler on login queue canceled. */
	FDelegateHandle OnLoginQueueCancelledDelegateHandle;
	FAccelByteOnLoginQueueCanceledByUserDelegate OnLoginQueueCancelledDelegate;
	void OnLoginQueueCancelled(int32 InLocalUserNum);

	/** Handler on login queue ticket claimed. */
	FDelegateHandle OnLoginQueueClaimTicketCompleteDelegateHandle;
	FAccelByteOnLoginQueueClaimTicketCompleteDelegate OnLoginQueueClaimTicketCompleteDelegate;
	void OnLoginQueueTicketClaimed(int32 InLocalUserNum, bool bWasClaimSuccessful, const FErrorOAuthInfo& ErrorObject);

	/** Flag indicating if login is in queue. */
	bool bLoginInQueue{false};

	/** Handler on login success. */
	void OnLoginSuccess();

	/** Online user account for the user that we were able to verify the login. */
	TSharedPtr<FUserOnlineAccountAccelByte> Account;

	/** String representing the error code that occurred. */
	FString ErrorStr{};

	/** Object representing the error code that occurred. */
	FErrorOAuthInfo ErrorOAuthObject{};

	/** Digit code representing the error that occurred. */
	int32 ErrorCode {-1};
};
