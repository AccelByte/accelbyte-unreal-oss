// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteCreateV1Party.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlinePartyInterfaceAccelByte.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Core/AccelByteRegistry.h"
#include "Api/AccelByteLobbyApi.h"

FOnlineAsyncTaskAccelByteCreateV1Party::FOnlineAsyncTaskAccelByteCreateV1Party(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FOnlinePartyTypeId InPartyTypeId, const FPartyConfiguration& InPartyConfig, const FOnCreatePartyComplete& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, PartyTypeId(InPartyTypeId)
	, PartyConfig(InPartyConfig)
	, Delegate(InDelegate)
{
	UserId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(InLocalUserId.AsShared());
}

void FOnlineAsyncTaskAccelByteCreateV1Party::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s"), *UserId->ToDebugString());

	// First, check if we are in a party locally, if so then we should fail and say to call leave party
	const FOnlinePartySystemAccelBytePtr PartyInterface = StaticCastSharedPtr<FOnlinePartySystemAccelByte>(Subsystem->GetPartyInterface());
	if (PartyInterface->IsPlayerInAnyParty(UserId.ToSharedRef().Get()))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Player '%s' was already in a party when CreateParty was called. Call LeaveParty before attempting to create another party!"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	// Next, we want to send off a request to check on the backend if we are in a party. This way we can validate in case
	// we're in one, but we haven't restored our state. This will tell the developer to call RestoreParties to restore
	// that previous state and act accordingly.
	AccelByte::Api::Lobby::FPartyInfoResponse OnGetPartyInfoResponseDelegate = AccelByte::Api::Lobby::FPartyInfoResponse::CreateRaw(this, &FOnlineAsyncTaskAccelByteCreateV1Party::OnGetPartyInfoResponse);
	ApiClient->Lobby.SetInfoPartyResponseDelegate(OnGetPartyInfoResponseDelegate);
	ApiClient->Lobby.SendInfoPartyRequest();

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Sent request to get info about current party before creating party!"));
}

void FOnlineAsyncTaskAccelByteCreateV1Party::Finalize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful)
	{
		const TSharedPtr<FOnlinePartySystemAccelByte, ESPMode::ThreadSafe> PartyInterface = StaticCastSharedPtr<FOnlinePartySystemAccelByte>(Subsystem->GetPartyInterface());
		if (!PartyInterface.IsValid())
		{
			UE_LOG_AB(Warning, TEXT("Failed to create party as the party interface was not valid!"));
			return;
		}

		TSharedRef<FOnlinePartyAccelByte> Party = MakeShared<FOnlinePartyAccelByte>(PartyInterface.ToSharedRef(), PartyInfo.PartyId, PartyInfo.InvitationToken, PartyConfig, UserId.ToSharedRef());
		Party->SetPartyCode(PartyCode);
		Party->AddPlayerCrossplayPreferenceAndPlatform(UserId.ToSharedRef());

		// Since we should be the only member of the party right now, just use our ID and display name to fill out the
		// member information for us. This may have to change if we ever support creating a party with players already
		// filled, but for now we only support creating one with just ourselves.
		const IOnlineIdentityPtr IdentityInterface = Subsystem->GetIdentityInterface();
		if (IdentityInterface.IsValid())
		{
			TSharedRef<FOnlinePartyMemberAccelByte> PartyMember = MakeShared<FOnlinePartyMemberAccelByte>(UserId.ToSharedRef(), IdentityInterface->GetPlayerNickname(UserId.ToSharedRef().Get()));
			Party->AddMember(UserId.ToSharedRef(), PartyMember);
		}

		PartyInterface->AddPartyToInterface(UserId.ToSharedRef(), Party);
		PartyId = Party->PartyId;
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteCreateV1Party::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const TSharedPtr<FOnlinePartySystemAccelByte, ESPMode::ThreadSafe> PartyInterface = StaticCastSharedPtr<FOnlinePartySystemAccelByte>(Subsystem->GetPartyInterface());
	Delegate.ExecuteIfBound(UserId.ToSharedRef().Get(), PartyId, CompletionResult);

	if (PartyId.IsValid())
	{
		PartyInterface->TriggerOnPartyJoinedDelegates(UserId.ToSharedRef().Get(), PartyId.ToSharedRef().Get());
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteCreateV1Party::OnGetPartyInfoResponse(const FAccelByteModelsInfoPartyResponse& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s; Get Party Info Response Code: %d"), *UserId->ToDebugString(), *Result.Code);

	// If we successfully get party info (code 0 is success), then we have to fail the task since we are already in a party
	if (Result.Code == TEXT("0"))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to create party for user (%s) as they are already in a party on the backend! Call RestoreParties then call LeaveParty before trying to create a new one!"), *UserId->ToDebugString());
		CompletionResult = ECreatePartyCompletionResult::AlreadyInParty;
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}
	
	// Finally, since we are not in a party, we can send the request to create one
	AccelByte::Api::Lobby::FPartyCreateResponse OnCreatePartyResponseDelegate = AccelByte::Api::Lobby::FPartyCreateResponse::CreateRaw(this, &FOnlineAsyncTaskAccelByteCreateV1Party::OnCreatePartyResponse);
	ApiClient->Lobby.SetCreatePartyResponseDelegate(OnCreatePartyResponseDelegate);
	ApiClient->Lobby.SendCreatePartyRequest();

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Sent off request to create party for user (%s)!"), *UserId->ToDebugString());
}

void FOnlineAsyncTaskAccelByteCreateV1Party::OnCreatePartyResponse(const FAccelByteModelsCreatePartyResponse& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s"), *UserId->ToDebugString(), *Result.Code);

	if (Result.Code != TEXT("0"))
	{
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Failed to create party! Response from backend had code '%s'!"), *Result.Code);
		CompletionResult = ECreatePartyCompletionResult::UnknownInternalFailure;
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
	}
	else
	{
		// CreateParty response also has information about the party, so just copy the struct to the member so that we
		// can construct the party object in the Finalize method
		PartyInfo = Result;

		// Send a request to get party code for the current party
		AccelByte::Api::Lobby::FPartyGetCodeResponse OnPartyGetCodeResponseDelegate = AccelByte::Api::Lobby::FPartyGetCodeResponse::CreateRaw(this, &FOnlineAsyncTaskAccelByteCreateV1Party::OnPartyGetCodeResponse);
		ApiClient->Lobby.SetPartyGetCodeResponseDelegate(OnPartyGetCodeResponseDelegate);
		ApiClient->Lobby.SendPartyGetCodeRequest();
	}
}

void FOnlineAsyncTaskAccelByteCreateV1Party::OnPartyGetCodeResponse(const FAccelByteModelsPartyGetCodeResponse& Result)
{
	if (Result.Code != TEXT("0"))
	{
		UE_LOG_AB(Warning, TEXT("Failed to create party as we could not get the party code! Response from backend had error code '%s'!"), *Result.Code);
		CompletionResult = ECreatePartyCompletionResult::UnknownInternalFailure;
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
	}
	else
	{
		PartyCode = Result.PartyCode;

		CompletionResult = ECreatePartyCompletionResult::Succeeded;
		CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

		UE_LOG(LogAccelByteOSSParty, Verbose, TEXT("Party created for user '%s' with ID '%s'!"), *UserId->ToDebugString(), *PartyInfo.PartyId);
	}

	// Reset the party code delegate
	ApiClient->Lobby.SetPartyGetCodeResponseDelegate(AccelByte::Api::Lobby::FPartyGetCodeResponse());
}
