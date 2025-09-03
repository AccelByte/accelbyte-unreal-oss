// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#if 1 // MMv1 Deprecation

#include "OnlineAsyncTaskAccelByteJoinV1Party.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlinePartyInterfaceAccelByte.h"

#include "Api/AccelByteLobbyApi.h"
#include "OnlineAsyncTaskAccelByteQueryV1PartyInfo.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineUserInterfaceAccelByte.h"

#include "Core/AccelByteReport.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteJoinV1Party::FOnlineAsyncTaskAccelByteJoinV1Party(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const IOnlinePartyJoinInfo& InOnlinePartyJoinInfo, const FOnJoinPartyComplete& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface, true)
	, OnlinePartyJoinInfo(InOnlinePartyJoinInfo)
	, Delegate(InDelegate)
	, PartyData(MakeShared<FOnlinePartyData>())
{
	FReport::LogDeprecated(FString(__FUNCTION__),
		TEXT("Party V1 functionality is deprecated and replaced by Party V2. For more information, see https://docs.accelbyte.io/gaming-services/services/play/party/"));
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InLocalUserId);
}

FOnlineAsyncTaskAccelByteJoinV1Party::FOnlineAsyncTaskAccelByteJoinV1Party(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FString& InPartyCode, const FOnJoinPartyComplete& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface, true)
	, PartyCode(InPartyCode)
	, Delegate(InDelegate)
	, PartyData(MakeShared<FOnlinePartyData>())
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InLocalUserId);
}

void FOnlineAsyncTaskAccelByteJoinV1Party::Initialize()
{
	TRY_PIN_SUBSYSTEM();

	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s; %s"), *UserId->ToDebugString(), *GetJoinInfoString());

	// Attempt to get party invite token before sending off a request to join a party
	const FOnlinePartySystemAccelBytePtr PartyInterface = StaticCastSharedPtr<FOnlinePartySystemAccelByte>(SubsystemPin->GetPartyInterface());
	if (!PartyInterface.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Could not join party '%s' as the party interface was invalid!"), *GetPartyInfoForError());
		CompletionResult = EJoinPartyCompletionResult::UnknownClientFailure;
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	// First, check if we are already in a party in the cache, as we cannot join multiple parties
	if (PartyInterface->IsPlayerInAnyParty(UserId.ToSharedRef().Get()))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Could not join party '%s' as the user is already in a party!"), *GetPartyInfoForError());
		CompletionResult = EJoinPartyCompletionResult::AlreadyInParty;
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	// If we have no valid join info or valid party code and we have no PartyCode for join, then bail out
	if (!OnlinePartyJoinInfo.IsValid() && PartyCode.IsEmpty())
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Could not join a party as join info or PartyCode is invalid!"));
		CompletionResult = EJoinPartyCompletionResult::JoinInfoInvalid;
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	// Default NotApprovedReason to 0
	NotApprovedReason = 0;

	// Now, we want to send a request to query whether we are in a party currently or not, this way if we are in a party
	// on the backend, but not in one on the client, we can tell the developer that they need to call RestoreParties first
	AccelByte::Api::Lobby::FPartyInfoResponse OnGetPartyInfoResponseDelegate = TDelegateUtils<AccelByte::Api::Lobby::FPartyInfoResponse>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteJoinV1Party::OnGetPartyInfoResponse);
	API_FULL_CHECK_GUARD(Lobby);
	Lobby->SetInfoPartyResponseDelegate(OnGetPartyInfoResponseDelegate);
	Lobby->SendInfoPartyRequest();

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteJoinV1Party::Tick()
{
	Super::Tick();
}

void FOnlineAsyncTaskAccelByteJoinV1Party::Finalize()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const TSharedPtr<FOnlinePartySystemAccelByte, ESPMode::ThreadSafe> PartyInterface = StaticCastSharedPtr<FOnlinePartySystemAccelByte>(SubsystemPin->GetPartyInterface());
	if (!PartyInterface.IsValid())
	{
		UE_LOG_AB(Warning, TEXT("Failed to join party as the party interface instance was invalid!"));
		return;
	}
	
	if (bWasSuccessful)
	{
		// Construct the party instance and add it to the party interface
		TSharedRef<FOnlinePartyAccelByte> Party = FOnlinePartyAccelByte::CreatePartyFromPartyInfo(UserId.ToSharedRef(), PartyInterface.ToSharedRef(), PartyInfo, PartyMemberInfo, PartyData, TEXT(""), MemberStatuses);
		PartyInterface->AddPartyToInterface(UserId.ToSharedRef(), Party);
		JoinedPartyId = StaticCastSharedRef<const FOnlinePartyIdAccelByte>(Party->PartyId);

		// Also delete the invite instance from the interface so that it won't show up as pending
		PartyInterface->RemoveInviteForParty(UserId.ToSharedRef(), JoinedPartyId.ToSharedRef(), EPartyInvitationRemovedReason::Accepted);

		// Add party information for all members in the party locally so that we can query for them if needed
		for (const FAccelByteUserInfoRef& Member : PartyMemberInfo)
		{
			PartyInterface->AddPartyToInterface(Member->Id.ToSharedRef(), Party);
		}
	}
	else if (OnlinePartyJoinInfo.IsValid())
	{
		const TSharedRef<const FOnlinePartyIdAccelByte> PartyIdAccelByte = StaticCastSharedRef<const FOnlinePartyIdAccelByte>(OnlinePartyJoinInfo.GetPartyId());
		// Delete the invite instance from the interface so that it won't show up as pending
		PartyInterface->RemoveInviteForParty(UserId.ToSharedRef(), PartyIdAccelByte, EPartyInvitationRemovedReason::Invalidated);
	}
	else
	{
		UE_LOG_AB(Warning, TEXT("Unable to remove party invite OnlinePartyJoinInfo was invalid!"));
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteJoinV1Party::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful)
	{
		Delegate.ExecuteIfBound(UserId.ToSharedRef().Get(), JoinedPartyId.ToSharedRef().Get(), CompletionResult, NotApprovedReason);

		const TSharedPtr<FOnlinePartySystemAccelByte, ESPMode::ThreadSafe> PartyInterface = StaticCastSharedPtr<FOnlinePartySystemAccelByte>(SubsystemPin->GetPartyInterface());
		if (PartyInterface.IsValid())
		{
			PartyInterface->TriggerOnPartyJoinedDelegates(UserId.ToSharedRef().Get(), JoinedPartyId.ToSharedRef().Get());
		}
	}
	else
	{
		Delegate.ExecuteIfBound(UserId.ToSharedRef().Get(), FOnlinePartyIdAccelByte(), CompletionResult, NotApprovedReason);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteJoinV1Party::OnGetPartyInfoResponse(const FAccelByteModelsInfoPartyResponse& Result)
{
	TRY_PIN_SUBSYSTEM();

	SetLastUpdateTimeToCurrentTime();

	// Check whether we successfully got information on a party, if so, this means that we are already in a party on the backend
	// and should leave that party before joining a new party
	if (Result.Code == TEXT("0"))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Could not join party %s as the user is already in a party on the backend! Call RestoreParties first, and then leave that party before accepting this invite!"), *GetPartyInfoForError());
		CompletionResult = EJoinPartyCompletionResult::AlreadyInParty;
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	// Attempt to get party invite token before sending off a request to join a party
	const FOnlinePartySystemAccelBytePtr PartyInterface = StaticCastSharedPtr<FOnlinePartySystemAccelByte>(SubsystemPin->GetPartyInterface());
	if (!PartyInterface.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Could not join party %s as the party interface was invalid!"), *GetPartyInfoForError());
		CompletionResult = EJoinPartyCompletionResult::UnknownClientFailure;
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	API_FULL_CHECK_GUARD(Lobby);
	// Now, if we are joining via an invite, which we will be if join info is not invalid, then we will get the invite and accept it
	// otherwise, we will try and join via party code
	if (OnlinePartyJoinInfo.IsValid())
	{
		// Now, send the actual request to join a party via PartyId
		const AccelByte::Api::Lobby::FPartyJoinResponse OnJoinPartyResponseDelegate = TDelegateUtils<AccelByte::Api::Lobby::FPartyJoinResponse>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteJoinV1Party::OnJoinPartyResponse);
		Lobby->SetInvitePartyJoinResponseDelegate(OnJoinPartyResponseDelegate);
		const TSharedPtr<const FAccelBytePartyInvite> PartyInvite = PartyInterface->GetInviteForParty(UserId.ToSharedRef() ,StaticCastSharedRef<const FOnlinePartyIdAccelByte>(OnlinePartyJoinInfo.GetPartyId()));
		Lobby->SendAcceptInvitationRequest(OnlinePartyJoinInfo.GetPartyId()->ToString(), PartyInvite->InviteToken);
	}
	else
	{
		// We will want to leave the current party if in one before sending the request to join a party with code
		// Otherwise partyJoinViaCodeResponse will return error code 115704 (codename JoinViaPartyCodeUserHasParty) for users that accept an invitation from Steam app when game is not running
		AccelByte::Api::Lobby::FPartyLeaveResponse OnLeavePartyResponseDelegate = TDelegateUtils<AccelByte::Api::Lobby::FPartyLeaveResponse>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteJoinV1Party::OnLeavePartyResponse);
		Lobby->SetLeavePartyResponseDelegate(OnLeavePartyResponseDelegate);
		Lobby->SendLeavePartyRequest();
	}
}

void FOnlineAsyncTaskAccelByteJoinV1Party::OnJoinPartyResponse(const FAccelByteModelsPartyJoinResponse& Result)
{
	SetLastUpdateTimeToCurrentTime();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Result code: %s"), *Result.Code);

	if (Result.Code != TEXT("0"))
	{
		AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Failed to join party as there was an issue on the backend! Error code: %s"), *Result.Code);
		NotApprovedReason = FCString::Atoi(*Result.Code);
		CompletionResult = EJoinPartyCompletionResult::UnknownInternalFailure;
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		return;
	}

	// unbind this function
	if (OnlinePartyJoinInfo.IsValid())
	{
		API_FULL_CHECK_GUARD(Lobby);
		Lobby->SetInvitePartyJoinResponseDelegate(AccelByte::Api::Lobby::FPartyJoinResponse());
	}
	else
	{
		API_FULL_CHECK_GUARD(Lobby);
		Lobby->SetPartyJoinViaCodeResponseDelegate(AccelByte::Api::Lobby::FPartyJoinResponse());
	}

	// Set the party info member to be that of the result that we got from the backend
	PartyInfo = Result;

	Super::ExecuteCriticalSectionAction(FVoidHandler::CreateLambda([&]()
		{
			TRY_PIN_SUBSYSTEM();

		FOnQueryPartyInfoComplete OnQueryPartyInfoCompleteDelegate = TDelegateUtils<FOnQueryPartyInfoComplete>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteJoinV1Party::OnQueryPartyInfoComplete);
		SubsystemPin->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteQueryV1PartyInfo>(SubsystemPin.Get(), UserId.ToSharedRef().Get(), Result.PartyId, Result.Members, OnQueryPartyInfoCompleteDelegate);
		}));

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteJoinV1Party::OnQueryPartyInfoComplete(bool bIsSuccessful, const FAccelBytePartyInfo& Result)
{
	SetLastUpdateTimeToCurrentTime();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Result code: %s"), LOG_BOOL_FORMAT(bWasSuccessful));
	
	if (bIsSuccessful)
	{
		PartyMemberInfo = Result.MemberInfo;
		PartyData = Result.PartyData;
		TArray<FString> PartyMemberUid;
		for(FAccelByteUserInfoRef& Member : PartyMemberInfo)
		{
			PartyMemberUid.Add(Member->Id->GetAccelByteId());
		}

		API_FULL_CHECK_GUARD(Lobby);
		Lobby->BulkGetUserPresenceV2(PartyMemberUid,
			TDelegateUtils<THandler<FAccelByteModelsBulkUserStatusNotif>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteJoinV1Party::OnGetUserPresenceComplete),
			FErrorHandler::CreateLambda([this](int32 Code, FString const& ErrMsg)
			{
				AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Could not query party member presence"));
				CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
			})
		);

		UE_LOG(LogAccelByteOSSParty, Verbose, TEXT("Joined party '%s' through %s!"), *PartyInfo.PartyId, (OnlinePartyJoinInfo.IsValid()) ? TEXT("invite") : TEXT("code"));
	}
	else
	{
		UE_LOG_AB(Warning, TEXT("Failed to query information about joined party from backend!"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
	}
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteJoinV1Party::OnLeavePartyResponse(const FAccelByteModelsLeavePartyResponse& Result)
{
	// Now, send the actual request to join a party via PartyId
	const AccelByte::Api::Lobby::FPartyJoinResponse OnJoinPartyResponseDelegate = TDelegateUtils<AccelByte::Api::Lobby::FPartyJoinResponse>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteJoinV1Party::OnJoinPartyResponse);
	API_FULL_CHECK_GUARD(Lobby);
	Lobby->SetPartyJoinViaCodeResponseDelegate(OnJoinPartyResponseDelegate);
	Lobby->SendPartyJoinViaCodeRequest(PartyCode);
}

FString FOnlineAsyncTaskAccelByteJoinV1Party::GetJoinInfoString()
{
	if (OnlinePartyJoinInfo.IsValid())
	{
		return FString::Printf(TEXT("Party ID: %s"), *OnlinePartyJoinInfo.GetPartyId()->ToString());
	}
	else if (!PartyCode.IsEmpty())
	{
		return FString::Printf(TEXT("Party Code: %s"), *PartyCode);
	}

	return TEXT("");
}

FString FOnlineAsyncTaskAccelByteJoinV1Party::GetPartyInfoForError()
{
	if (OnlinePartyJoinInfo.IsValid())
	{
		return FString::Printf(TEXT("'%s'"), *OnlinePartyJoinInfo.GetPartyId()->ToString());
	}
	else if (!PartyCode.IsEmpty())
	{
		return FString::Printf(TEXT("with code '%s'"), *PartyCode);
	}

	return TEXT("");
}

void FOnlineAsyncTaskAccelByteJoinV1Party::OnGetUserPresenceComplete(const FAccelByteModelsBulkUserStatusNotif& Statuses)
{
	MemberStatuses.Data.Append(Statuses.Data);
	MemberStatuses.Away += Statuses.Away;
	MemberStatuses.Busy += Statuses.Busy;
	MemberStatuses.Invisible += Statuses.Invisible;
	MemberStatuses.Online += Statuses.Online;
	MemberStatuses.Offline += Statuses.Offline;

	if (Statuses.NotProcessed.Num() > 0)
	{
		SetLastUpdateTimeToCurrentTime();
		API_FULL_CHECK_GUARD(Lobby);
		Lobby->BulkGetUserPresenceV2(Statuses.NotProcessed,
			TDelegateUtils<THandler<FAccelByteModelsBulkUserStatusNotif>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteJoinV1Party::OnGetUserPresenceComplete),
			FErrorHandler::CreateLambda([this](int32 Code, FString const& ErrMsg)
				{
					AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Could not query party member presence"));
					CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
				})
		);
	}
	else
	{
		CompletionResult = EJoinPartyCompletionResult::Succeeded;
		CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	}
}
#endif