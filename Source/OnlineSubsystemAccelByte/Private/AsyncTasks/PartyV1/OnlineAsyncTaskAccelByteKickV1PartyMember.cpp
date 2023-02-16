// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteKickV1PartyMember.h"
#include "OnlinePartyInterfaceAccelByte.h"
#include "OnlineSessionInterfaceV1AccelByte.h"
#include "Core/AccelByteRegistry.h"
#include "Api/AccelByteLobbyApi.h"

FOnlineAsyncTaskAccelByteKickV1PartyMember::FOnlineAsyncTaskAccelByteKickV1PartyMember(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FOnlinePartyId& InPartyId, const FUniqueNetId& InTargetMemberId, const FOnKickPartyMemberComplete& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, PartyId(StaticCastSharedRef<const FOnlinePartyIdAccelByte>(InPartyId.AsShared()))
	, TargetMemberId(FUniqueNetIdAccelByteUser::CastChecked(InTargetMemberId))
	, Delegate(InDelegate)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InLocalUserId);
}

void FOnlineAsyncTaskAccelByteKickV1PartyMember::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s; PartyId: %s; TargetMemberId: %s"), *UserId->ToDebugString(), *PartyId->ToString(), *TargetMemberId->ToDebugString());

	// First, grab our party interface instance to do some checks before attempting to kick a member
	const TSharedPtr<FOnlinePartySystemAccelByte, ESPMode::ThreadSafe> PartyInterface = StaticCastSharedPtr<FOnlinePartySystemAccelByte>(Subsystem->GetPartyInterface());
	if (!PartyInterface.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to initiate kicking a member '%s' from this party '%s' as the party interface instance was not valid!"), *TargetMemberId->ToDebugString(), *PartyId->ToString());
		CompletionResult = EKickMemberCompletionResult::UnknownClientFailure;
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	// First, check if we are in this party in the first place
	if (!PartyInterface->IsPlayerInParty(UserId.ToSharedRef().Get(), PartyId.Get()))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to initiate kicking a member '%s' from this party '%s' as the calling user is not in this party!"), *TargetMemberId->ToDebugString(), *PartyId->ToString());
		CompletionResult = EKickMemberCompletionResult::LocalMemberNotMember;
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	// As a final quick check, we want to see if we are the leader of this party before attempting to kick anyone from it
	if (!PartyInterface->IsMemberLeader(UserId.ToSharedRef().Get(), PartyId.Get(), UserId.ToSharedRef().Get()))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to initiate kicking a member '%s' from this party '%s' as the player '%s' is not the party leader!"), *TargetMemberId->ToDebugString(), *PartyId->ToString(), *UserId->ToDebugString());
		CompletionResult = EKickMemberCompletionResult::LocalMemberNotLeader;
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	AccelByte::Api::Lobby::FPartyKickResponse OnKickPartyMemberResponseDelegate = TDelegateUtils<AccelByte::Api::Lobby::FPartyKickResponse>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteKickV1PartyMember::OnKickPartyMemberResponse);
	ApiClient->Lobby.SetInvitePartyKickMemberResponseDelegate(OnKickPartyMemberResponseDelegate);
	ApiClient->Lobby.SendKickPartyMemberRequest(TargetMemberId->GetAccelByteId());
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Sent request to kick a member from a party."));
}

void FOnlineAsyncTaskAccelByteKickV1PartyMember::Finalize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful)
	{
		// First, grab our party interface instance to grab our party instance
		const TSharedPtr<FOnlinePartySystemAccelByte, ESPMode::ThreadSafe> PartyInterface = StaticCastSharedPtr<FOnlinePartySystemAccelByte>(Subsystem->GetPartyInterface());
		if (!PartyInterface.IsValid())
		{
			AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to finalize kicking party member as the party interface was not valid!"));
			return;
		}

		// Now, attempt to grab the actual party instance from the party interface
		TSharedPtr<FOnlinePartyAccelByte> Party = PartyInterface->GetPartyForUser(UserId.ToSharedRef(), PartyId);
		if (!Party.IsValid())
		{
			AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to finalize kicking party member as the party specified was not valid!"));
			return;
		}

		// Finally, we can remove this member from our party
		Party->RemoveMember(UserId.ToSharedRef(), TargetMemberId, EMemberExitedReason::Kicked);

		// Remove the reflected party for the kicked member on our client side
		PartyInterface->RemovePartyFromInterface(TargetMemberId, Party.ToSharedRef());
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteKickV1PartyMember::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	Delegate.ExecuteIfBound(UserId.ToSharedRef().Get(), PartyId.Get(), TargetMemberId.Get(), CompletionResult);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteKickV1PartyMember::OnKickPartyMemberResponse(const FAccelByteModelsKickPartyMemberResponse& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Result: %s"), *Result.Code);

	if (Result.Code != TEXT("0"))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to kick member '%s' from party '%s' as the response failed on the backend! Response code: %s"), *TargetMemberId->ToDebugString(), *PartyId->ToString(), *Result.Code);
		CompletionResult = EKickMemberCompletionResult::UnknownInternalFailure;
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		return;
	}

	CompletionResult = EKickMemberCompletionResult::Succeeded;
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	UE_LOG(LogAccelByteOSSParty, Verbose, TEXT("Kicked user '%s' from party '%s'!"), *TargetMemberId->GetAccelByteId(), *PartyId->ToString());

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Successfully kicked member '%s' from party '%s'!"), *TargetMemberId->ToDebugString(), *PartyId->ToString());
}