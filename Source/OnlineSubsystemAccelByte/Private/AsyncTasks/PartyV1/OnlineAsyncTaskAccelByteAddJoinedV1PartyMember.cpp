// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteAddJoinedV1PartyMember.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlinePartyInterfaceAccelByte.h"
#include "OnlineUserCacheAccelByte.h"

FOnlineAsyncTaskAccelByteAddJoinedV1PartyMember::FOnlineAsyncTaskAccelByteAddJoinedV1PartyMember(FOnlineSubsystemAccelByte* const InABInterface, const TSharedRef<const FUniqueNetIdAccelByteUser>& InLocalUserId, const TSharedRef<FOnlinePartyAccelByte>& InParty, const FString& InJoinedAccelByteId)
	: FOnlineAsyncTaskAccelByte(InABInterface, true)
	, Party(InParty)
	, JoinedAccelByteId(InJoinedAccelByteId)
{
	UserId = InLocalUserId;
}

void FOnlineAsyncTaskAccelByteAddJoinedV1PartyMember::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s; PartyId: %s; JoinedAccelByteId: %s"), *UserId->ToDebugString(), *Party->PartyId->ToString(), *JoinedAccelByteId);

	FOnlineUserCacheAccelBytePtr UserStore = Subsystem->GetUserCache();
	if (!UserStore.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Could not add user '%s' to party member list as our user store instance is invalid!"), *JoinedAccelByteId);
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	FOnQueryUsersComplete OnQueryJoinedPartyMemberCompleteDelegate = TDelegateUtils<FOnQueryUsersComplete>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteAddJoinedV1PartyMember::OnQueryJoinedPartyMemberComplete);
	UserStore->QueryUsersByAccelByteIds(LocalUserNum, { JoinedAccelByteId }, OnQueryJoinedPartyMemberCompleteDelegate, true);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Sent request to get further data on joined party member!"));
}

void FOnlineAsyncTaskAccelByteAddJoinedV1PartyMember::Tick()
{
	Super::Tick();

	if (HasFinishedAsyncWork())
	{
		CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	}
}

void FOnlineAsyncTaskAccelByteAddJoinedV1PartyMember::Finalize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful)
	{
		TSharedRef<FOnlinePartyMemberAccelByte> NewMember = MakeShared<FOnlinePartyMemberAccelByte>(JoinedPartyMember->Id.ToSharedRef(), JoinedPartyMember->DisplayName);
		NewMember->SetMemberConnectionStatus(EMemberConnectionStatus::Connected);
		Party->AddMember(UserId.ToSharedRef(), NewMember);
		Party->RemoveInvite(UserId.ToSharedRef(), JoinedPartyMember->Id.ToSharedRef(), EPartyInvitationRemovedReason::Accepted);

		const TSharedPtr<FOnlinePartySystemAccelByte, ESPMode::ThreadSafe> PartyInterface = StaticCastSharedPtr<FOnlinePartySystemAccelByte>(Subsystem->GetPartyInterface());
		PartyInterface->AddPartyToInterface(JoinedPartyMember->Id.ToSharedRef(), Party);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

bool FOnlineAsyncTaskAccelByteAddJoinedV1PartyMember::HasFinishedAsyncWork()
{
	return bHasRetrievedMemberInfo /*&& bHasRetrievedMemberStats && bHasRetrievedMemberCustomizations && bHasRetrievedMemberProgression && bHasRetrievedMemberDailyPlayStreak && bHasRetrievedMemberRanks*/;
}

void FOnlineAsyncTaskAccelByteAddJoinedV1PartyMember::OnQueryJoinedPartyMemberComplete(bool bIsSuccessful, TArray<TSharedRef<FAccelByteUserInfo>> UsersQueried)
{
	SetLastUpdateTimeToCurrentTime();

	if (bIsSuccessful && UsersQueried.IsValidIndex(0))
	{
		JoinedPartyMember = UsersQueried[0];
	}
	else
	{
		UE_LOG_AB(Warning, TEXT("Failed to get information about user '%s' that joined party '%s'!"), *JoinedAccelByteId, *Party->PartyId->ToString());
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
	}

	bHasRetrievedMemberInfo = true;
}
