// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteGetPartySessionStorage.h"
#include "OnlineSubsystemAccelByteSessionSettings.h"
#include "OnlineAsyncTaskAccelByteRefreshV2PartySession.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteGetPartySessionStorage::FOnlineAsyncTaskAccelByteGetPartySessionStorage(
	FOnlineSubsystemAccelByte* const InABInterface,
	FUniqueNetIdAccelByteUserPtr UserUniqueNetId,
	const FOnGetPartySessionStorageComplete& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface, false),
	Delegate(InDelegate)
{
	TRY_PIN_SUBSYSTEM_CONSTRUCTOR();
	UserId = UserUniqueNetId;
}

void FOnlineAsyncTaskAccelByteGetPartySessionStorage::Initialize()
{
	Super::Initialize();

	TRY_PIN_SUBSYSTEM();

	const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(SubsystemPin->GetSessionInterface());
	AB_ASYNC_TASK_VALIDATE(SessionInterface.IsValid(), "Failed to update party session as our session interface is invalid!");

	AB_ASYNC_TASK_VALIDATE(SessionInterface->IsCurrentUserEligiblePartyStorageAction(UserId), "Current user is not eligible to do party related action.");

	auto PartySession = SessionInterface->GetPartySession();
	AB_ASYNC_TASK_VALIDATE(PartySession != nullptr, "The user does not have a party, unable to set party attribute");
	AB_ASYNC_TASK_VALIDATE(PartySession->GetSessionIdStr().Len() > 0, "The user party session ID is invalid");

	OnGetPartySessionStorageSuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsV2PartySessionStorage>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGetPartySessionStorage::OnGetPartySessionStorageSuccess);
	OnGetPartySessionStorageErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGetPartySessionStorage::OnGetPartySessionStorageError);

	API_CLIENT_CHECK_GUARD();
	FAccelByteModelsV2PartySessionStorageReservedData ReservedData{};
	ApiClient->Session.GetPartySessionStorage(
		  PartySession->GetSessionIdStr()
		, OnGetPartySessionStorageSuccessDelegate
		, OnGetPartySessionStorageErrorDelegate);
}

void FOnlineAsyncTaskAccelByteGetPartySessionStorage::Finalize()
{}

void FOnlineAsyncTaskAccelByteGetPartySessionStorage::TriggerDelegates()
{
	Super::TriggerDelegates();
	Delegate.ExecuteIfBound(PartySessionStorageResult, bWasSuccessful);
}

void FOnlineAsyncTaskAccelByteGetPartySessionStorage::OnGetPartySessionStorageSuccess(const FAccelByteModelsV2PartySessionStorage& BackendSessionData)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	PartySessionStorageResult = BackendSessionData;
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetPartySessionStorage::OnGetPartySessionStorageError(int32 ErrorCode, const FString& ErrorMessage)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}
