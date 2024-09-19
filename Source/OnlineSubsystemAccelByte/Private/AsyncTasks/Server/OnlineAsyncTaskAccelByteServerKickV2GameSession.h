// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineSessionInterfaceV2AccelByte.h"

class FOnlineAsyncTaskAccelByteServerKickV2GameSession
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteServerKickV2GameSession, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteServerKickV2GameSession(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FName& InSessionName, const FUniqueNetId& InPlayerIdToKick, const FOnKickPlayerComplete& InDelegate);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:
	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteServerKickV2GameSession");
	}

private:
	/** Local name of the game session that we are trying to kick a player from. */
	FName SessionName{};

	/** The game session id associated with the SessionName. */
	FString GameSessionID{};
	
	/** ID of the player that we are trying to kick from the game session. */
	TSharedRef<const FUniqueNetIdAccelByteUser> PlayerIdToKick;

	/** Delegate fired when we finish kicking a player from the game session. */
	FOnKickPlayerComplete Delegate{};

	/** Handler for on kick user completed. */
	void OnKickUserSuccess();
	void OnKickUserError(int32 ErrorCode, const FString& ErrorMessage);
};
