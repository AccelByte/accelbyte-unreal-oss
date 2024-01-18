// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "../OnlineAsyncTaskAccelByteLogin.h"

namespace AccelByte { class FApiClient; }

enum class ESimultaneousLoginAsyncTaskState : uint8
{
	Initialized = 0,
	NativePlatformLoginDone,
	SecondaryPlatformLoginDone,
	Success
};

/**
 * Async task to authenticate a user with the AccelByte backend, either using a native platform account, or a user specified account
 */
class FOnlineAsyncTaskAccelByteSimultaneousLogin
	: public FOnlineAsyncTaskAccelByteLogin
{
public:

	FOnlineAsyncTaskAccelByteSimultaneousLogin(FOnlineSubsystemAccelByte* const InABSubsystem
		, int32 InLocalUserNum
		, const FOnlineAccountCredentials& InAccountCredentials
		, bool bInCreateHeadlessAccount = true);

protected:
	virtual void Initialize() override;
	virtual void TriggerDelegates() override;
	virtual void PerformLogin(const FOnlineAccountCredentials& Credentials) override;

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteSimultaneousLogin");
	}

private:
	FString NativePlatformTicket{};
	FString SecondaryPlatformTicket{};
	FString SecondaryPlatformName{};
	ESimultaneousLoginAsyncTaskState CurrentAsyncTaskState{};

	bool IsInitializeAllowed();
	void PostProcessTriggerDelegates();
};
