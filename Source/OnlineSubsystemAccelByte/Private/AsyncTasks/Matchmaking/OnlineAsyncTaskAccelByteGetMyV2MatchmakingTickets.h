// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Models/AccelByteMatchmakingModels.h"
#include "OnlineSessionInterfaceV2AccelByte.h"

class FOnlineAsyncTaskAccelByteGetMyV2MatchmakingTickets
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteGetMyV2MatchmakingTickets, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteGetMyV2MatchmakingTickets(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InPlayerId, const FName InSessionName, const FString& InMatchPool);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteGetMyV2MatchamakingTickets");
	}

private:

	/** Name of the session we are checking active tickets*/
	FName SessionName;
	
	/** Match pool that we wish for matchmaking to search through */
	FString MatchPool;

	/* Get my tickets result*/
	TArray<FAccelByteModelsV2MatchmakingTicketStatuses> MyTicketResults;

	/* Pagination limit for request*/
	int32 Limit = 50;

	/* Pagination offset for sending request and tracking current offset */
	int32 Offset = 0;

	/* Cached unmatched ticket from all the ticket in response */
	FString ActiveTicketId;


	THandler<FAccelByteModelsV2MatchmakingTicketStatuses> OnGetMyMatchTicketSuccessDelegate;
	void OnGetMyMatchTicketSuccess(const FAccelByteModelsV2MatchmakingTicketStatuses& Result);

	FErrorHandler OnGetMyMatchTicketErrorDelegate;
	void OnGetMyMatchTicketError(int32 ErrorCode, const FString& ErrorMessage);

	void SendGetMyTicketRequest();
};
