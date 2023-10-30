// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "Models/AccelByteSessionModels.h"
#include "OnlineSessionInterfaceV2AccelByte.h"

/**
 * Queries all invites for a player, includes both game session and party session invites. Developer then can filter those invites locally.
 */
class FOnlineAsyncTaskAccelByteQueryAllV2SessionInvites
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteQueryAllV2SessionInvites, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteQueryAllV2SessionInvites(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InPlayerId);

    virtual void Initialize() override;
    virtual void Tick() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteQueryAllV2SessionInvites");
	}

private:
	/** Whether or not we have received a response to get all game session invites */
	FThreadSafeBool bHasReceivedGameSessionInviteResponse{false};

	/** Whether or not we have received a response to get all party session invites */
	FThreadSafeBool bHasReceivedPartySessionInviteResponse{false};

	/** Array of constructed invites, will be passed back to session interface */
	TArray<FOnlineSessionInviteAccelByte> Invites;

	THandler<FAccelByteModelsV2PaginatedGameSessionQueryResult> OnGetGameSessionInvitesSuccessDelegate;
	void OnGetGameSessionInvitesSuccess(const FAccelByteModelsV2PaginatedGameSessionQueryResult& Result);

	FErrorHandler OnGetGameSessionInvitesErrorDelegate;
	void OnGetGameSessionInvitesError(int32 ErrorCode, const FString& ErrorMessage);

	THandler<FAccelByteModelsV2PaginatedPartyQueryResult> OnGetPartySessionInvitesSuccessDelegate;
	void OnGetPartySessionInvitesSuccess(const FAccelByteModelsV2PaginatedPartyQueryResult& Result);

	FErrorHandler OnGetPartySessionInvitesErrorDelegate;
	void OnGetPartySessionInvitesError(int32 ErrorCode, const FString& ErrorMessage);

};

