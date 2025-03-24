// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelBytePromoteV1PartyLeader.h"

#include "Api/AccelByteLobbyApi.h"
#include "OnlinePartyInterfaceAccelByte.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelBytePromoteV1PartyLeader::FOnlineAsyncTaskAccelBytePromoteV1PartyLeader(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FOnlinePartyId& InPartyId, const FUniqueNetId& InTargetMemberId, const FOnPromotePartyMemberComplete& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, PartyId(StaticCastSharedRef<const FOnlinePartyIdAccelByte>(InPartyId.AsShared()))
	, TargetMemberId(FUniqueNetIdAccelByteUser::CastChecked(InTargetMemberId))
	, Delegate(InDelegate)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InLocalUserId);
}

void FOnlineAsyncTaskAccelBytePromoteV1PartyLeader::Initialize()
{
	TRY_PIN_SUBSYSTEM();

	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s; PartyId: %s; TargetMemberId: %s"), *UserId->ToDebugString(), *PartyId->ToString(), *TargetMemberId->ToDebugString());

	// First, grab our party interface instance to do some checks before attempting to promote a member
	const TSharedPtr<FOnlinePartySystemAccelByte, ESPMode::ThreadSafe> PartyInterface = StaticCastSharedPtr<FOnlinePartySystemAccelByte>(SubsystemPin->GetPartyInterface());
	if (!PartyInterface.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to initiate promoting a member '%s' as leader of party '%s' as the party interface instance was not valid!"), *TargetMemberId->ToDebugString(), *PartyId->ToString());
		CompletionResult = EPromoteMemberCompletionResult::UnknownClientFailure;
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	// First, check if we are in this party in the first place
	if (!PartyInterface->IsPlayerInParty(UserId.ToSharedRef().Get(), PartyId.Get()))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to initiate promoting a member '%s' as leader of party '%s' as the calling user is not in this party!"), *TargetMemberId->ToDebugString(), *PartyId->ToString());
		CompletionResult = EPromoteMemberCompletionResult::LocalMemberNotMember;
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	// As a final quick check, we want to see if we are the leader of this party before attempting to promote anyone to take our place
	if (!PartyInterface->IsMemberLeader(UserId.ToSharedRef().Get(), PartyId.Get(), UserId.ToSharedRef().Get()))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to initiate promoting a member '%s' as leader of party '%s' as the player '%s' is not the current party leader!"), *TargetMemberId->ToDebugString(), *PartyId->ToString(), *UserId->ToDebugString());
		CompletionResult = EPromoteMemberCompletionResult::LocalMemberNotLeader;
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	AccelByte::Api::Lobby::FPartyPromoteLeaderResponse OnPromotePartyMemberResponseDelegate = TDelegateUtils<AccelByte::Api::Lobby::FPartyPromoteLeaderResponse>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelBytePromoteV1PartyLeader::OnPromotePartyMemberResponse);
	API_FULL_CHECK_GUARD(Lobby);
	Lobby->SetPartyPromoteLeaderResponseDelegate(OnPromotePartyMemberResponseDelegate);
	Lobby->SendPartyPromoteLeaderRequest(TargetMemberId->GetAccelByteId());

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Sent request to promote a member of this party to leader."));
}

void FOnlineAsyncTaskAccelBytePromoteV1PartyLeader::Finalize()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful)
	{
		// First, grab our party interface instance to grab our party instance
		const TSharedPtr<FOnlinePartySystemAccelByte, ESPMode::ThreadSafe> PartyInterface = StaticCastSharedPtr<FOnlinePartySystemAccelByte>(SubsystemPin->GetPartyInterface());
		if (!PartyInterface.IsValid())
		{
			AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to finalize promoting party member as the party interface was not valid!"));
			return;
		}

		// Now, attempt to grab the actual party instance from the party interface
		TSharedPtr<FOnlinePartyAccelByte> Party = PartyInterface->GetPartyForUser(UserId.ToSharedRef(), PartyId);
		if (!Party.IsValid())
		{
			AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to finalize promoting party member as the party specified was not valid!"));
			return;
		}

		// Finally, we can set the leader ID for the party to the new leader
		Party->LeaderId = TargetMemberId;
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelBytePromoteV1PartyLeader::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	Delegate.ExecuteIfBound(UserId.ToSharedRef().Get(), PartyId.Get(), TargetMemberId.Get(), CompletionResult);

	if (bWasSuccessful)
	{
		const TSharedPtr<FOnlinePartySystemAccelByte, ESPMode::ThreadSafe> PartyInterface = StaticCastSharedPtr<FOnlinePartySystemAccelByte>(SubsystemPin->GetPartyInterface());
		if (PartyInterface.IsValid())
		{
			PartyInterface->TriggerOnPartyMemberPromotedDelegates(UserId.ToSharedRef().Get(), PartyId.Get(), TargetMemberId.Get());
		}
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelBytePromoteV1PartyLeader::OnPromotePartyMemberResponse(const FAccelByteModelsPartyPromoteLeaderResponse& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	if (Result.Code != TEXT("0"))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to promote party member '%s' to leader of party '%s' as the request to the backend failed! Response code: %s"), *TargetMemberId->ToDebugString(), *PartyId->ToString(), *Result.Code);
		CompletionResult = EPromoteMemberCompletionResult::UnknownInternalFailure;
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		return;
	}

	CompletionResult = EPromoteMemberCompletionResult::Succeeded;
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	UE_LOG(LogAccelByteOSSParty, Verbose, TEXT("Promoted user '%s' to leader of party '%s'!"), *TargetMemberId->GetAccelByteId(), *PartyId->ToString());

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Successfully promoted member of party to leader!"));
}
