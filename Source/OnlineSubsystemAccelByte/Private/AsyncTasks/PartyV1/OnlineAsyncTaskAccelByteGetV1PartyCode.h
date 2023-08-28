// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "Interfaces/OnlinePartyInterface.h"
#include "Models/AccelByteLobbyModels.h"

/**
 * Delegate fired once the FOnlineAsyncTaskAccelByteGetV1PartyCode task completes.
 *
 * @param 1 [bWasSuccessful] Boolean representing whether the task completed successfully or not
 * @param 2 [PartyCode] The obtained value
 * @param 3 [UserId] The user that request for PartyCode
 * @param 4 [PartyId] The requested partyId
 */
DECLARE_DELEGATE_FourParams(FOnPartyCodeGenerated, bool /*1*/, const FString& /*2*/, const TSharedRef<const FUniqueNetIdAccelByteUser>& /*3*/, const FString& /*4*/ );
 
/**
 * Fill out information about your async task here.
 */
class FOnlineAsyncTaskAccelByteGetV1PartyCode
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteGetV1PartyCode, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteGetV1PartyCode(FOnlineSubsystemAccelByte* const InABInterface, const TSharedRef<const FUniqueNetIdAccelByteUser>& InUserId, const FString& PartyId, const FOnPartyCodeGenerated& InDelegate);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteGetPartyCode");
	}

private:
	
	/** Requested PartyId that will be returned*/
	FString PartyId;

	/** Delegate fired once we get when the operation GetPartyCode complete */
	FOnPartyCodeGenerated Delegate;

	/** Delegate handler for when the response complete */
	void OnPartyGetCodeResponse(const FAccelByteModelsPartyGetCodeResponse& Result);

	/** Response of the requested PartyCode */
	FString ResponsePartyCode{};

};

