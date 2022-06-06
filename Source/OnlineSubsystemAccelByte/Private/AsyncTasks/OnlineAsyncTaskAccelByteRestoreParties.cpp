// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteRestoreParties.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlinePartyInterfaceAccelByte.h"
#include "Api/AccelByteLobbyApi.h"
#include "OnlineError.h"

#define ONLINE_ERROR_NAMESPACE "FOnlinePartySystemAccelByte"

FOnlineAsyncTaskAccelByteRestoreParties::FOnlineAsyncTaskAccelByteRestoreParties(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FOnRestorePartiesComplete& InCompletionDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface, true)
	, CompletionDelegate(InCompletionDelegate)
	, PartyData(MakeShared<FOnlinePartyData>())
{
	UserId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(InLocalUserId.AsShared());
}

void FOnlineAsyncTaskAccelByteRestoreParties::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s"), *UserId->ToDebugString());

	// Need to check whether we are already in a party on the interface side before moving on to trying to restore state
	const FOnlinePartySystemAccelBytePtr PartyInterface = StaticCastSharedPtr<FOnlinePartySystemAccelByte>(Subsystem->GetPartyInterface());
	if (!PartyInterface.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Could not restore parties as the party interface was invalid!"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	// First, check if we are already in a party in the cache, as we cannot join multiple parties
	if (PartyInterface->IsPlayerInAnyParty(UserId.ToSharedRef().Get()))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Could not restore parties as the user is already in a party!"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	// Get information about the current user's party, which then will give us a party to restore if we are in one
	AccelByte::Api::Lobby::FPartyInfoResponse OnGetPartyInfoResponseDelegate = AccelByte::Api::Lobby::FPartyInfoResponse::CreateRaw(this, &FOnlineAsyncTaskAccelByteRestoreParties::OnGetPartyInfoResponse);
	ApiClient->Lobby.SetInfoPartyResponseDelegate(OnGetPartyInfoResponseDelegate);
	ApiClient->Lobby.SendInfoPartyRequest();

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Sent off request to get user's party info on the backend."));
}

void FOnlineAsyncTaskAccelByteRestoreParties::Tick()
{
	Super::Tick();
}

void FOnlineAsyncTaskAccelByteRestoreParties::Finalize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful && bUserHasPartyToRestore)
	{
		const TSharedPtr<FOnlinePartySystemAccelByte, ESPMode::ThreadSafe> PartyInterface = StaticCastSharedPtr<FOnlinePartySystemAccelByte>(Subsystem->GetPartyInterface());
		if (!PartyInterface.IsValid())
		{
			AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to restore party as the party interface was invalid!"));
			return;
		}

		// Construct the party instance and add it to the party interface
		TSharedRef<FOnlinePartyAccelByte> Party = FOnlinePartyAccelByte::CreatePartyFromPartyInfo(UserId.ToSharedRef(), PartyInterface.ToSharedRef(), PartyInfo, PartyMemberInfo, PartyData, PartyCode);
		PartyId = StaticCastSharedRef<const FOnlinePartyIdAccelByte>(Party->PartyId);
		PartyInterface->AddPartyToInterface(UserId.ToSharedRef(), Party);
		
		// Also delete the invite instance from the interface so that it won't show up as pending
		PartyInterface->RemoveInviteForParty(UserId.ToSharedRef(), StaticCastSharedRef<const FOnlinePartyIdAccelByte>(Party->PartyId), EPartyInvitationRemovedReason::Accepted);

		// Add party information for all members in the party locally so that we can query for them if needed
		for (const TSharedRef<FAccelByteUserInfo>& Member : PartyMemberInfo)
		{
			PartyInterface->AddPartyToInterface(Member->Id.ToSharedRef(), Party);
		}

		// Get party code request for later stored in party storage
		ApiClient->Lobby.SendPartyGetCodeRequest();
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRestoreParties::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	EOnlineErrorResult Result = ((bWasSuccessful) ? EOnlineErrorResult::Success : EOnlineErrorResult::RequestFailure);
	CompletionDelegate.ExecuteIfBound(UserId.ToSharedRef().Get(), ONLINE_ERROR(Result));

	// If we've restored a party, then trigger a delegate that we've joined a party
	if (bUserHasPartyToRestore && PartyId.IsValid())
	{
		const TSharedPtr<FOnlinePartySystemAccelByte, ESPMode::ThreadSafe> PartyInterface = StaticCastSharedPtr<FOnlinePartySystemAccelByte>(Subsystem->GetPartyInterface());
		PartyInterface->TriggerOnPartyJoinedDelegates(UserId.ToSharedRef().Get(), PartyId.ToSharedRef().Get());
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRestoreParties::OnGetPartyInfoResponse(const FAccelByteModelsInfoPartyResponse& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	SetLastUpdateTimeToCurrentTime();

	// Response code 11223 is EIDPartyInfoSuccessGetUserPartyInfoEmpty, meaning that the call succeeded but the user is not in a party
	if (Result.Code == TEXT("11223"))
	{
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("User is not in a party and therefore no party could be restored!"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	}
	// Any other response code besides zero is a failure
	else if (Result.Code != TEXT("0"))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to restore party for user '%s' as the call to the backend failed! Response code: %d"), *UserId->ToDebugString(), *Result.Code);
		CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	}
	// We were able to successfully get party info, restore it
	else
	{
		// Copy the party info to a local member so that we can populate on finalize
		PartyInfo = Result;
		bUserHasPartyToRestore = true;

		FOnQueryPartyInfoComplete OnQueryPartyInfoCompleteDelegate = FOnQueryPartyInfoComplete::CreateRaw(this, &FOnlineAsyncTaskAccelByteRestoreParties::OnQueryPartyInfoComplete);
		Subsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteQueryPartyInfo>(Subsystem, UserId.ToSharedRef().Get(), Result.PartyId, Result.Members, OnQueryPartyInfoCompleteDelegate);
	}
}

void FOnlineAsyncTaskAccelByteRestoreParties::OnQueryPartyInfoComplete(bool bIsSuccessful, const FAccelBytePartyInfo& Result)
{
	SetLastUpdateTimeToCurrentTime();
	if (bIsSuccessful)
	{
		PartyMemberInfo = Result.MemberInfo;
		PartyData = Result.PartyData;
		
		if (UserId->GetAccelByteId() == PartyInfo.LeaderId)
		{
			// Send a request to get party code for the current party for party leader
			const AccelByte::Api::Lobby::FPartyGetCodeResponse OnPartyGetCodeResponseDelegate = AccelByte::Api::Lobby::FPartyGetCodeResponse::CreateRaw(this, &FOnlineAsyncTaskAccelByteRestoreParties::OnPartyGetCodeResponse);
			ApiClient->Lobby.SetPartyGetCodeResponseDelegate(OnPartyGetCodeResponseDelegate);
			ApiClient->Lobby.SendPartyGetCodeRequest();
		}
		else
		{
			// Send a request to get party code for the current party for party member
			const THandler<FAccelByteModelsPartyData> OnGetPartyDataSuccessDelegate = THandler<FAccelByteModelsPartyData>::CreateRaw(this, &FOnlineAsyncTaskAccelByteRestoreParties::OnGetPartyDataSuccess);
			const FErrorHandler OnGetPartyDataErrorDelegate = FErrorHandler::CreateRaw(this, &FOnlineAsyncTaskAccelByteRestoreParties::OnGetPartyDataError);
			ApiClient->Lobby.GetPartyData(PartyInfo.PartyId, OnGetPartyDataSuccessDelegate, OnGetPartyDataErrorDelegate);
		}
	}
	else
	{
		UE_LOG_AB(Warning, TEXT("Failed to query information about restored party from backend!"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
	}
}

void FOnlineAsyncTaskAccelByteRestoreParties::OnPartyGetCodeResponse(const FAccelByteModelsPartyGetCodeResponse& Result)
{
	if (Result.Code != TEXT("0"))
	{
		UE_LOG_AB(Warning, TEXT("Failed to restore party as we could not get the party code! Response from backend had error code '%s', party code will be empty"), *Result.Code);
		CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	}
	else
	{
		PartyCode = Result.PartyCode;

		CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

		UE_LOG(LogAccelByteOSSParty, Verbose, TEXT("Party restored for user '%s' with ID '%s'!"), *UserId->ToDebugString(), *PartyInfo.PartyId);
	}

	// Reset the party code delegate
	ApiClient->Lobby.SetPartyGetCodeResponseDelegate(AccelByte::Api::Lobby::FPartyGetCodeResponse());
}

void FOnlineAsyncTaskAccelByteRestoreParties::OnGetPartyDataSuccess(const FAccelByteModelsPartyData& InPartyData)
{
	UE_LOG_AB(Warning, TEXT("Failed to get party code from party data, party code will be empty"));
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
}

void FOnlineAsyncTaskAccelByteRestoreParties::OnGetPartyDataError(int32 ErrorCode, const FString& ErrorMessage)
{
	UE_LOG_AB(Warning, TEXT("Failed to get party data from restored party from backend, party code will be empty"));
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
}

#undef ONLINE_ERROR_NAMESPACE
