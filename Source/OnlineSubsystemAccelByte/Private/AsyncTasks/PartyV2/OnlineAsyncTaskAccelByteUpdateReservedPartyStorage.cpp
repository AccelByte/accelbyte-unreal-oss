// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteUpdateReservedPartyStorage.h"
#include "OnlineSessionInterfaceV2AccelByte.h"
#include "OnlineSubsystemAccelByteSessionSettings.h"
#include "OnlineAsyncTaskAccelByteRefreshV2PartySession.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteUpdateReservedPartyStorage::FOnlineAsyncTaskAccelByteUpdateReservedPartyStorage(
	FOnlineSubsystemAccelByte* const InABInterface, 
	FUniqueNetIdAccelByteUserPtr UserUniqueNetId, 
	FAccelByteModelsV2PartySessionStorageReservedData ReservedStorage)
	: FOnlineAsyncTaskAccelByte(InABInterface, false)
	, UserDataToStoreOnReservedStorage(ReservedStorage)
{
	TRY_PIN_SUBSYSTEM_CONSTRUCTOR()
	
	UserId = UserUniqueNetId;
}

void FOnlineAsyncTaskAccelByteUpdateReservedPartyStorage::Initialize()
{
	Super::Initialize();

	TRY_PIN_SUBSYSTEM();

	const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(SubsystemPin->GetSessionInterface());
	AB_ASYNC_TASK_VALIDATE(SessionInterface.IsValid(), "Failed to update party session as our session interface is invalid!");

	auto PartySession = SessionInterface->GetPartySession();
	AB_ASYNC_TASK_VALIDATE(PartySession != nullptr, "The user does not have a party, unable to set party attribute");
	AB_ASYNC_TASK_VALIDATE(PartySession->GetSessionIdStr().Len() > 0, "The user party session ID is invalid");

	OnUpdatePartyReservedStorageSuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsV2PartySessionStorageReservedData>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteUpdateReservedPartyStorage::OnUpdatePartySessionSuccess);
	OnUpdatePartyReservedStorageErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteUpdateReservedPartyStorage::OnUpdatePartySessionError);

	API_FULL_CHECK_GUARD(Session);
	Session->StorePersonalDataToReservedPartySessionStorage
		( PartySession->GetSessionIdStr()
		, UserDataToStoreOnReservedStorage
		, OnUpdatePartyReservedStorageSuccessDelegate
		, OnUpdatePartyReservedStorageErrorDelegate);
}

void FOnlineAsyncTaskAccelByteUpdateReservedPartyStorage::Finalize()
{
	// Do nothing, intentional. Internal OSS functionality.
}

void FOnlineAsyncTaskAccelByteUpdateReservedPartyStorage::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM();
	Super::TriggerDelegates();
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(SubsystemPin->GetSessionInterface());
	if (!ensure(SessionInterface.IsValid()))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to trigger delegates from update reserved party storage as our session interface is invalid!"));
		return;
	}

	SessionInterface->TriggerOnUpdatePlayerReservedPartySessionStorageCompleteDelegates(UserReservedDataResult, bWasSuccessful, OnlineError);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

#define ONLINE_ERROR_NAMESPACE "FOnlineAsyncTaskAccelByteUpdateReservedPartyStorage"

void FOnlineAsyncTaskAccelByteUpdateReservedPartyStorage::OnUpdatePartySessionSuccess(const FAccelByteModelsV2PartySessionStorageReservedData& BackendSessionData)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	UserReservedDataResult = BackendSessionData;
	OnlineError = ONLINE_ERROR(EOnlineErrorResult::Success, FString(), FText());
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteUpdateReservedPartyStorage::OnUpdatePartySessionError(int32 ErrorCode, const FString& ErrorMessage)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	OnlineError = ONLINE_ERROR(EOnlineErrorResult::RequestFailure, FString::FromInt(ErrorCode), FText::FromString(ErrorMessage));
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}
#undef ONLINE_ERROR_NAMESPACE
