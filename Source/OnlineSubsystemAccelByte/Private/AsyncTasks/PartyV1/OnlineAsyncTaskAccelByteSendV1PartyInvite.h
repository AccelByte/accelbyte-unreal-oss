// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "../OnlineAsyncTaskAccelByte.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlinePartyInterfaceAccelByte.h"

/**
 * Task for sending an invite to a user to join a party
 */
class FOnlineAsyncTaskAccelByteSendV1PartyInvite : public FOnlineAsyncTaskAccelByte
{
public:

	FOnlineAsyncTaskAccelByteSendV1PartyInvite(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FOnlinePartyId& InPartyId, const FPartyInvitationRecipient& InRecipient, const FOnSendPartyInvitationComplete& InDelegate);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteSendPartyInvite");
	}

private:

	/** ID of the party that the user is sending an invitation for */
	TSharedRef<const FOnlinePartyIdAccelByte> PartyId;
	
	/** Structure representing a recipient of the party invite we are sending */
	FPartyInvitationRecipient Recipient;
	
	/** Delegate that is fired after sending the party invite */
	FOnSendPartyInvitationComplete Delegate;

	/** Result of the invite player to party call on the backend, sent to delegate */
	ESendPartyInvitationCompletionResult CompletionResult;

	/** ID of the user that will receive the invite as an AccelByte ID */
	TSharedRef<const FUniqueNetIdAccelByteUser> RecipientId;
	
	/** Delegate handler for when the request to send a party invite has received a response */
	void OnPartyInviteResponse(const FAccelByteModelsPartyInviteResponse& Result);

};

