// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSessionInterfaceV2AccelByte.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineErrorAccelByte.h"

class FOnlineAsyncTaskAccelBytePromoteV2GameSessionLeader
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelBytePromoteV2GameSessionLeader, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelBytePromoteV2GameSessionLeader(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FString& InSessionId, const FUniqueNetId& InTargetMemberId);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelBytePromoteV2GameSessionLeader");
	}

private:

	/** Game session ID the user is attempting to promote someone as leader of	*/
	FString SessionId;

	/** ID of the member that the user is attempting to promote. */
	TSharedRef<const FUniqueNetIdAccelByteUser> TargetMemberId;

	/** Online error information in case any error happen. */
	FOnlineErrorAccelByte OnlineError;

	/** Send promote game session leader request to backend. */
	bool PromoteGameSessionLeader();

	/** Delegate handler for when we get a successful response from the backend on promoting a member of a game session to leader */
	THandler<FAccelByteModelsV2GameSession> OnPromoteGameSessionLeaderSuccessDelegate;
	void OnPromoteGameSessionLeaderSuccess(const FAccelByteModelsV2GameSession& BackendSessionData);

	/** Delegate handler for when we get a failed response from the backend on promoting a member of a game session to leader */
	FErrorHandler OnPromoteGameSessionLeaderErrorDelegate;
	void OnPromoteGameSessionLeaderError(int32 ErrorCode, const FString& ErrorMessage);
};
