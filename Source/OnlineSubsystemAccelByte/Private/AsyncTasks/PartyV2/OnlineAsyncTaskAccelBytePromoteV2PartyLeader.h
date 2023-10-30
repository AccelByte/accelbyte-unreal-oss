// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSessionInterfaceV2AccelByte.h"
#include "OnlineSubsystemAccelByteTypes.h"

/**
 * Async task to promote a member of a party to leader on the backend
 */
class FOnlineAsyncTaskAccelBytePromoteV2PartyLeader
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelBytePromoteV2PartyLeader, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelBytePromoteV2PartyLeader(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FName& InSessionName, const FString& InSessionId, const FUniqueNetId& InTargetMemberId, const FOnPromotePartySessionLeaderComplete& InCompletionDelegate);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelBytePromoteV2PartyLeader");
	}

private:

	/** 
	* Local name of the party session that we are trying to promote the leader of
	*/
	FName SessionName{};
	
	/** 
	* Party session ID the user is attempting to promote someone as leader of
	*/
	FString SessionId; 

	/** 
	* ID of the member that the user is attempting to promote 
	*/
	TSharedRef<const FUniqueNetIdAccelByteUser> TargetMemberId;

	/** 
	* Delegate fired when we finish promotion
	*/
	FOnPromotePartySessionLeaderComplete CompletionDelegate{};

	/**
	 * Delegate handler for when we get a successful response from the backend on promoting a member of a party to leader
	 */
	THandler<FAccelByteModelsV2PartySession> OnPromotePartyLeaderSuccessDelegate;
	void OnPromotePartyLeaderSuccess(const FAccelByteModelsV2PartySession& BackendSessionData);

	/**
	 * Delegate handler for when we get a failed response from the backend on promoting a member of a party to leader
	 */
	FErrorHandler OnPromotePartyLeaderErrorDelegate;
	void OnPromotePartyLeaderError(int32 ErrorCode, const FString& ErrorMessage);

};

