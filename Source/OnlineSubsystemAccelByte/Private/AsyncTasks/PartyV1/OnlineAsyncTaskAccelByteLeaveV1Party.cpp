// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteLeaveV1Party.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlinePartyInterfaceAccelByte.h"
#include "OnlineSessionInterfaceV1AccelByte.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteLeaveV1Party::FOnlineAsyncTaskAccelByteLeaveV1Party(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FOnlinePartyId& InPartyId, bool InBSynchronizeLeave, const FOnLeavePartyComplete& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, PartyId(StaticCastSharedRef<const FOnlinePartyIdAccelByte>(InPartyId.AsShared()))
	, bSynchronizeLeave(InBSynchronizeLeave)
	, Delegate(InDelegate)
	, CompletionResult(ELeavePartyCompletionResult::LeavePending)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InLocalUserId);
}

void FOnlineAsyncTaskAccelByteLeaveV1Party::Initialize()
{
	TRY_PIN_SUBSYSTEM()

	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s; PartyId: %s; Synchronize Leave: %s"), *UserId->ToDebugString(), *PartyId->ToString(), LOG_BOOL_FORMAT(bSynchronizeLeave));

	// First, check if the player is currently in a party with this ID, if we're not, then we shouldn't do this
	// 
	// NOTE(Maxwell): There might be a chance that if party state is not properly cleaned up manually by the developer,
	// "Auto Kick on Disconnect" is disabled, and RestoreParties has not been called at some point that this call will
	// fail. In this case, you should figure out whether you want to manually leave parties and have auto kick off, or
	// if you want to auto kick players if their lobby websocket disconnects, which may have side effects in the event
	// of an error with that socket.
	const TSharedPtr<FOnlinePartySystemAccelByte, ESPMode::ThreadSafe> PartyInterface = StaticCastSharedPtr<FOnlinePartySystemAccelByte>(SubsystemPin->GetPartyInterface());
	if (!PartyInterface.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Could not leave a party as the party interface instance was invalid!"));
		CompletionResult = ELeavePartyCompletionResult::UnknownParty;
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	const FOnlineSessionV1AccelBytePtr SessionInt = StaticCastSharedPtr<FOnlineSessionV1AccelByte>(SubsystemPin->GetSessionInterface());
	if (!SessionInt.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Could not leave a party as the session interface instance was invalid!"));
		CompletionResult = ELeavePartyCompletionResult::UnknownParty;
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	const bool bIsPlayerInThisParty = PartyInterface->IsPlayerInParty(UserId.ToSharedRef().Get(), PartyId.Get());
	if (!bIsPlayerInThisParty)
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Could not leave party '%s' as the player is not currently in this party!"), *PartyId->ToString());
		CompletionResult = ELeavePartyCompletionResult::UnknownParty;
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	// Finally, if we are actually in this party, then we can send off the request to leave the party on the backend
	// provided that synchronization with the backend was requested
	if (bSynchronizeLeave)
	{
		// Cancel matchmaking in the case that the leave party request is made while in matchmaking state
		SessionInt->CancelMatchmakingNotification();

		AccelByte::Api::Lobby::FPartyLeaveResponse OnLeavePartyResponseDelegate = TDelegateUtils<AccelByte::Api::Lobby::FPartyLeaveResponse>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteLeaveV1Party::OnLeavePartyResponse);
		API_CLIENT_CHECK_GUARD();
		ApiClient->Lobby.SetLeavePartyResponseDelegate(OnLeavePartyResponseDelegate);
		ApiClient->Lobby.SendLeavePartyRequest();
	}
	// If we aren't synchronizing the party leave to the backend, then we can consider this task successful as we just
	// have to clear the local cache of parties
	else
	{
		CompletionResult = ELeavePartyCompletionResult::Succeeded;
		CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteLeaveV1Party::Finalize()
{
	TRY_PIN_SUBSYSTEM()

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful)
	{
		// If we successfully completed this task, all that is left to do is remove the party from the local list
		const TSharedPtr<FOnlinePartySystemAccelByte, ESPMode::ThreadSafe> PartyInterface = StaticCastSharedPtr<FOnlinePartySystemAccelByte>(SubsystemPin->GetPartyInterface());
		if (PartyInterface.IsValid())
		{
			// Since we have left the party, we not only want to remove the party on our own local cache, but also remove any
			// cached party information that we have for any of the members in that party.
			TSharedPtr<FOnlinePartyAccelByte> Party = PartyInterface->GetPartyForUser(UserId.ToSharedRef(), PartyId);
			if (Party.IsValid())
			{
				TArray<FOnlinePartyMemberConstRef> Members = Party->GetAllMembers();
				for (const FOnlinePartyMemberConstRef& Member : Members)
				{
					TSharedRef<const FUniqueNetIdAccelByteUser> MemberId = FUniqueNetIdAccelByteUser::CastChecked(Member->GetUserId());
					PartyInterface->RemovePartyForUser(MemberId, PartyId);
				}

				PartyInterface->RemovePartyForUser(UserId.ToSharedRef(), PartyId);
			}
		}
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteLeaveV1Party::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM()

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful) 
	{
		const TSharedPtr<FOnlinePartySystemAccelByte, ESPMode::ThreadSafe> PartyInterface = StaticCastSharedPtr<FOnlinePartySystemAccelByte>(SubsystemPin->GetPartyInterface());
		if (PartyInterface.IsValid()) 
		{
			PartyInterface->TriggerOnPartyExitedDelegates(UserId.ToSharedRef().Get(), PartyId.Get());
		}
	}

	Delegate.ExecuteIfBound(UserId.ToSharedRef().Get(), PartyId.Get(), CompletionResult);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteLeaveV1Party::OnLeavePartyResponse(const FAccelByteModelsLeavePartyResponse& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Response code: %s"), *Result.Code);

	// First check if the result of the response was an error
	if (Result.Code != TEXT("0"))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to leave party '%s' on the backend! Response code: %s"), *PartyId->ToString(), *Result.Code);
		CompletionResult = ELeavePartyCompletionResult::UnknownInternalFailure;
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		return;
	}

	// Next, we can just complete the task successfully which will then remove the party from the local list on Finalize
	CompletionResult = ELeavePartyCompletionResult::Succeeded;
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	UE_LOG(LogAccelByteOSSParty, Verbose, TEXT("Left party '%s'!"), *PartyId->ToString());
}
