// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "OnlineAsyncTaskAccelByte.h"
#include "OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "Models/AccelByteAgreementModels.h"
#include <OnlineIdentityInterfaceAccelByte.h>
#include <OnlineAgreementInterfaceAccelByte.h>

/**
 * Task for get localized policy content
 */
class FOnlineAsyncTaskAccelByteGetLocalizedPolicyContent : public FOnlineAsyncTaskAccelByte, public TSelfPtr<FOnlineAsyncTaskAccelByteGetLocalizedPolicyContent, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteGetLocalizedPolicyContent(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FString& InBasePolicyId, const FString& InLocaleCode, bool bInAlwaysRequestToService);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteGetLocalizedPolicyContent");
	}

private:

	/**
	 * Delegate handler for when get localized policy content succeeds
	 */
	void OnGetLocalizedPolicyContentSuccess(const FString& Result);
	THandler<FString> OnGetLocalizedPolicyContentSuccessDelegate;

	/**
	 * Delegate handler for when get localized policy content fails
	 */
	void OnGetLocalizedPolicyContentError(int32 ErrorCode, const FString& ErrorMessage);
	FErrorHandler OnGetLocalizedPolicyContentErrorDelegate;

	/**
	 * The Localized Policy content retrieved from Agreement service
	 */
	FString LocalizedPolicyContent;

	/**
	 * String representing the error code that occurred
	 */
	FString ErrorStr;

	FString BasePolicyId;
	FString LocaleCode;
	FString BaseUrl;
	FString AttachmentLocation;
	bool bAlwaysRequestToService;
};
