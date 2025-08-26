// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSessionInterfaceV2AccelByte.h"

struct FAccelByteModelsGameSessionUpdateDSInformationRequest;

class FOnlineAsyncTaskAccelByteUpdateDSInformation
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteUpdateDSInformation, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteUpdateDSInformation(FOnlineSubsystemAccelByte* const InABInterface
		, FName InSessionName
		, FAccelByteModelsGameSessionUpdateDSInformationRequest const& InNewDSInformation
		, FOnUpdateDSInformationComplete const& InCompletionDelegate);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;

protected:
	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteUpdateDSInformation");
	}

private:
	/**
	 * @brief Local name of the session to update DS information for.
	 */
	FName SessionName{};

	/**
	 * @brief Information about the DS that we are updating on the session.
	 */
	FAccelByteModelsGameSessionUpdateDSInformationRequest NewDSInformation{};

	/**
	 * @brief Delegate triggered after request to update DS information completes.
	 */
	FOnUpdateDSInformationComplete CompletionDelegate{};

	/**
	 * @brief Error to return to the caller when request completes.
	 */
	FOnlineError OnlineError{};

	void OnUpdateDSInformationComplete();
	void OnUpdateDSInformationError(int32 ErrorCode, const FString& ErrorMessage);

};
