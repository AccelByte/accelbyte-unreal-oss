// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "Models/AccelByteAgreementModels.h"
#include "Models/AccelBytePredefinedEventModels.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "OnlineAgreementInterfaceAccelByte.h"

/**
 * Task for query user's eligibilities
 */
class FOnlineAsyncTaskAccelByteQueryEligibilities
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteQueryEligibilities, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteQueryEligibilities(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, bool bInNotAcceptedOnly, bool bInAlwaysRequestToService);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteQueryEligibilities");
	}

private:

	/**
	 * Delegate handler for when querying eligibilities succeeds
	 */
	void OnQueryEligibilitiesSuccess(const TArray<FAccelByteModelsRetrieveUserEligibilitiesResponse>& Result);
	THandler<TArray<FAccelByteModelsRetrieveUserEligibilitiesResponse>> OnQueryEligibilitiesSuccessDelegate;

	/**
	 * Delegate handler for when querying eligibilities fails
	 */
	void OnQueryEligibilitiesError(int32 ErrorCode, const FString& ErrorMessage);
	FErrorHandler OnQueryEligibilitiesErrorDelegate;

	/**
	 * The list of eligibilities retrieved from Agreement service
	 */
	TArray<FAccelByteModelsRetrieveUserEligibilitiesResponse> Eligibilities;

	/**
	 * String representing the error code that occurred
	 */
	FString ErrorStr;

	/**
	 * Digit code representing the error that occurred
	 */
	int32 ErrorCode;

	/**
	 * List of documents not yet accepted by user, will be send to the analytics
	 */
	FAccelByteModelsUserAgreementNotAcceptedPayload NotAcceptedAgreementPayload{};

	bool bNotAcceptedOnly;
	bool bAlwaysRequestToService;
};
