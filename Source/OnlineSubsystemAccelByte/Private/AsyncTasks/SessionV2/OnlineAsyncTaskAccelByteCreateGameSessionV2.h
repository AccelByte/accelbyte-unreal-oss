// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "OnlineSessionSettings.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSessionSettings.h"
#include "Models/AccelByteSessionModels.h"

class FOnlineAsyncTaskAccelByteCreateGameSessionV2
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteCreateGameSessionV2, ESPMode::ThreadSafe>
{
public:

	/** Constructor to setup the RegisterPlayers task */
	FOnlineAsyncTaskAccelByteCreateGameSessionV2(FOnlineSubsystemAccelByte* const InABInterface
		, const FUniqueNetId& InHostingPlayerId
		, const FName& InSessionName
		, const FOnlineSessionSettings& InNewSessionSettings
		, bool IsDedicatedServer = false);

	FOnlineAsyncTaskAccelByteCreateGameSessionV2(FOnlineSubsystemAccelByte* const InABInterface
		, int32 InLocalUserNum
		, const FName& InSessionName
		, const FOnlineSessionSettings& InNewSessionSettings
		, bool IsDedicatedServer = false);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteCreateGameSessionV2");
	}

private:

	/** Name that we wish to store this new session under */
	FName SessionName;

	/** Settings for the session that we wish to create */
	FOnlineSessionSettings NewSessionSettings;

	/** Structure representing the game session object that was just created */
	FAccelByteModelsV2GameSession CreatedGameSession;

	THandler<FAccelByteModelsV2GameSession> OnCreateGameSessionSuccessDelegate;
	void OnCreateGameSessionSuccess(const FAccelByteModelsV2GameSession& Result);

	FErrorHandler OnCreateGameSessionErrorDelegate;
	void OnCreateGameSessionError(int32 ErrorCode, const FString& ErrorMessage);

};

