// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "OnlineSessionSettings.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Models/AccelByteMatchmakingModels.h"
#include "OnlineSessionInterfaceV2AccelByte.h"

class FOnlineAsyncTaskAccelByteStartV2Matchmaking
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteStartV2Matchmaking, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteStartV2Matchmaking(FOnlineSubsystemAccelByte* const InABInterface, const TSharedRef<FOnlineSessionSearchAccelByte>& InSearchHandle, const FName& InSessionName, const FString& InMatchPool, const FOnStartMatchmakingComplete& InDelegate);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteStartV2Matchmaking");
	}

private:
	/** Handle for the current matchmaking search, updated by us based on updates to matchmaking */
	TSharedRef<FOnlineSessionSearchAccelByte> SearchHandle;

	/** Name of the session that we are trying to matchmake for */
	FName SessionName;

	/** Match pool that we wish for matchmaking to search through */
	FString MatchPool;

	/** Delegate fired when we finish the call to start matchmaking */
	FOnStartMatchmakingComplete Delegate;

	/** Container for create match ticket response */
	FAccelByteModelsV2MatchmakingCreateTicketResponse CreateMatchTicketResponse;

	TSharedPtr<FJsonObject> AttributesJsonObject;

	void OnGetLatenciesSuccess(const TArray<TPair<FString, float>>& Latencies);
	void OnGetLatenciesError(int32 ErrorCode, const FString& ErrorMessage);

	/** Create the actual matchmaking ticket on the backend. Will be done if we have latencies cached, or after we query for new latencies. */
	void CreateMatchTicket();

	/** Determine what session ID we should attach to the new match ticket, if any at all */
	FString GetTicketSessionId() const;

	void OnStartMatchmakingSuccess(const FAccelByteModelsV2MatchmakingCreateTicketResponse& Result);
	void OnStartMatchmakingError(int32 ErrorCode, const FString& ErrorMessage, const FErrorCreateMatchmakingTicketV2& CreateTicketErrorInfo);

};
