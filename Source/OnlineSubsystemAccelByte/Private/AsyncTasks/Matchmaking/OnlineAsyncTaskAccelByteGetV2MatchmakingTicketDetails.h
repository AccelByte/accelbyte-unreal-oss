// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Models/AccelByteMatchmakingModels.h"
#include "OnlineSessionInterfaceV2AccelByte.h"
#include "OnlineErrorAccelByte.h"

class FOnlineAsyncTaskAccelByteGetV2MatchmakingTicketDetails
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteGetV2MatchmakingTicketDetails, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteGetV2MatchmakingTicketDetails(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InPlayerId, const FString& InTicketId);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteGetV2MatchmakingTicketDetails");
	}

private:
	
	/** Ticket ID we want to check */
	FString TicketId;

	/** match ticket details result */
	FAccelByteModelsV2MatchmakingGetTicketDetailsResponse MatchTicketDetailResponse;

	/** Online error information in case any error happen. */
	FOnlineError OnlineError;

	THandler<FAccelByteModelsV2MatchmakingGetTicketDetailsResponse> OnGetMatchTicketDetailSuccessDelegate;
	void OnGetMyMatchTicketSuccess(const FAccelByteModelsV2MatchmakingGetTicketDetailsResponse& Result);

	FErrorHandler OnGetMatchTicketDetailErrorDelegate;
	void OnGetMyMatchTicketError(int32 ErrorCode, const FString& ErrorMessage);

	void SendGetMyTicketRequest();
};
