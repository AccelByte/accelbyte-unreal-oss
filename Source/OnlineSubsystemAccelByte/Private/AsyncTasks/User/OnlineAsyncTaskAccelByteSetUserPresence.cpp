// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteSetUserPresence.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlinePresenceInterfaceAccelByte.h"
#include "OnlinePredefinedEventInterfaceAccelByte.h"

#include "Api/AccelByteLobbyApi.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteHelpers.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteSetUserPresence::FOnlineAsyncTaskAccelByteSetUserPresence(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FOnlineUserPresenceStatus& InStatus, const IOnlinePresence::FOnPresenceTaskCompleteDelegate& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface, true)
	, LocalCachedPresenceStatus(MakeShared<FOnlineUserPresenceStatus>(InStatus))
	, Delegate(InDelegate)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InLocalUserId);
}

void FOnlineAsyncTaskAccelByteSetUserPresence::Initialize() 
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(" "));

	switch (LocalCachedPresenceStatus->State)
	{
	case EOnlinePresenceState::Online: PresenceStatus = EAvailability::Online; break;
	case EOnlinePresenceState::DoNotDisturb: PresenceStatus = EAvailability::Busy; break;
	case EOnlinePresenceState::Chat: PresenceStatus = EAvailability::Invisible; break;
	case EOnlinePresenceState::Away: PresenceStatus = EAvailability::Away; break;
	case EOnlinePresenceState::Offline:
	default: PresenceStatus = EAvailability::Offline; break;
	}

	// Send off the actual request to set user presence
	AccelByte::Api::Lobby::FSetUserPresenceResponse OnSetUserPresenceResponseDelegate = TDelegateUtils<AccelByte::Api::Lobby::FSetUserPresenceResponse>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteSetUserPresence::OnSetUserPresenceResponse);
	API_FULL_CHECK_GUARD(Lobby);
	Lobby->SetUserPresenceResponseDelegate(OnSetUserPresenceResponseDelegate);
	Lobby->SendSetPresenceStatus(PresenceStatus, LocalCachedPresenceStatus->StatusStr);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteSetUserPresence::Finalize()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful)
	{
		FOnlinePresenceAccelBytePtr PresenceInterface = StaticCastSharedPtr<FOnlinePresenceAccelByte>(SubsystemPin->GetPresenceInterface());
		auto LocalCachedPresence = PresenceInterface->FindOrCreatePresence(UserId.ToSharedRef());

		LocalCachedPresence->Status.StatusStr = LocalCachedPresenceStatus->StatusStr;
		LocalCachedPresence->Status.State = LocalCachedPresenceStatus->State;
		LocalCachedPresence->bIsOnline = LocalCachedPresenceStatus->State == EOnlinePresenceState::Online ? true : false;
		LocalCachedPresence->bIsPlayingThisGame = LocalCachedPresenceStatus->State == EOnlinePresenceState::Online ? true : false;

		const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = SubsystemPin->GetPredefinedEventInterface();
		if (PredefinedEventInterface.IsValid())
		{
			FAccelByteModelsUserPresenceStatusUpdatedPayload UserPresenceStatusUpdatedPayload{};
			UserPresenceStatusUpdatedPayload.UserId = UserId->GetAccelByteId();
			UserPresenceStatusUpdatedPayload.Status = FAccelByteUtilities::GetUEnumValueAsString(PresenceStatus);
			PredefinedEventInterface->SendEvent(LocalUserNum, MakeShared<FAccelByteModelsUserPresenceStatusUpdatedPayload>(UserPresenceStatusUpdatedPayload));
		}
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteSetUserPresence::TriggerDelegates() 
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const IOnlinePresencePtr PresenceInterface = SubsystemPin->GetPresenceInterface();
	if (PresenceInterface.IsValid()) 
	{
		Delegate.ExecuteIfBound(*UserId.Get(), bWasSuccessful);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteSetUserPresence::OnSetUserPresenceResponse(const FAccelByteModelsSetOnlineUsersResponse& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Result: %s"), *Result.Code);

	if (Result.Code != TEXT("0"))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to Set presence status for user %s as the response failed on the backend! Response code: %s"), *UserId->ToDebugString(), *Result.Code);
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		return;
	}

	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Successfully Set presence status for user %s"), *UserId->ToDebugString());
}