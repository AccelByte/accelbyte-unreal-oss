// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteSendV1PartyInvite.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlinePartyInterfaceAccelByte.h"
#include "Api/AccelByteLobbyApi.h"
#include "Core/AccelByteRegistry.h"

FOnlineAsyncTaskAccelByteSendV1PartyInvite::FOnlineAsyncTaskAccelByteSendV1PartyInvite(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FOnlinePartyId& InPartyId, const FPartyInvitationRecipient& InRecipient, const FOnSendPartyInvitationComplete& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, PartyId(StaticCastSharedRef<const FOnlinePartyIdAccelByte>(InPartyId.AsShared()))
	, Recipient(InRecipient)
	, Delegate(InDelegate)
	, RecipientId(FUniqueNetIdAccelByteUser::Invalid())
{
	UserId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(InLocalUserId.AsShared());
}

void FOnlineAsyncTaskAccelByteSendV1PartyInvite::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s; PartyId: %s; Recipient ID: %s"), *UserId->ToString(), *PartyId->ToString(), *Recipient.Id->ToDebugString());

	// First, we want to check if the player is in this party before attempting to send an invite
	const TSharedPtr<FOnlinePartySystemAccelByte, ESPMode::ThreadSafe> PartyInterface = StaticCastSharedPtr<FOnlinePartySystemAccelByte>(Subsystem->GetPartyInterface());
	if (!PartyInterface.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Could not invite player '%s' to party '%s' as the party interface instance is invalid!"), *Recipient.Id->ToString(), *PartyId->ToString());
		CompletionResult = ESendPartyInvitationCompletionResult::UnknownInternalFailure;
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	if (!PartyInterface->IsPlayerInParty(UserId.ToSharedRef().Get(), PartyId.Get()))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Could not invite player '%s' to party '%s' as the requesting player '%s' is not in the party!"), *Recipient.Id->ToString(), *PartyId->ToString(), *UserId->ToString());
		CompletionResult = ESendPartyInvitationCompletionResult::UnknownInternalFailure;
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	RecipientId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(Recipient.Id);

	// Now, once we know we are in this party, we want to send a request to invite the player to the party
	AccelByte::Api::Lobby::FPartyInviteResponse OnPartyInviteResponseDelegate = AccelByte::Api::Lobby::FPartyInviteResponse::CreateRaw(this, &FOnlineAsyncTaskAccelByteSendV1PartyInvite::OnPartyInviteResponse);
	ApiClient->Lobby.SetInvitePartyResponseDelegate(OnPartyInviteResponseDelegate);
	ApiClient->Lobby.SendInviteToPartyRequest(RecipientId->GetAccelByteId());

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteSendV1PartyInvite::Finalize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful)
	{
		// Get our party interface instance, then get our party object for the user specified, and then add the player ID
		// to the array of player IDs that we have invited to this party
		const TSharedPtr<FOnlinePartySystemAccelByte, ESPMode::ThreadSafe> PartyInterface = StaticCastSharedPtr<FOnlinePartySystemAccelByte>(Subsystem->GetPartyInterface());
		if (PartyInterface.IsValid())
		{
			TSharedPtr<FOnlinePartyAccelByte> PartyObject = PartyInterface->GetPartyForUser(UserId.ToSharedRef(), PartyId);
			if (PartyObject.IsValid())
			{
				PartyObject->AddUserToInvitedPlayers(UserId.ToSharedRef(), UserId.ToSharedRef(), RecipientId);
			}
		}
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteSendV1PartyInvite::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	Delegate.ExecuteIfBound(UserId.ToSharedRef().Get(), PartyId.Get(), Recipient.Id.Get(), CompletionResult);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteSendV1PartyInvite::OnPartyInviteResponse(const FAccelByteModelsPartyInviteResponse& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Response code: %s"), *Result.Code);

	// Check if the request to invite this user failed on the backend first, if so, complete the task with a fail state
	if (Result.Code != TEXT("0"))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to invite user '%s' to party '%s' as the request failed on the backend! Response code: %s"), *RecipientId->ToDebugString(), *PartyId->ToString(), *Result.Code);
		CompletionResult = ESendPartyInvitationCompletionResult::UnknownInternalFailure;
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		return;
	}

	// If the request succeeded, then we can just complete this task and move this user to the pending invites on the interface
	CompletionResult = ESendPartyInvitationCompletionResult::Succeeded;
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	UE_LOG(LogAccelByteOSSParty, Verbose, TEXT("Sent party invite to user '%s' for party with ID '%s'!"), *RecipientId->GetAccelByteId(), *PartyId->ToString());

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}
