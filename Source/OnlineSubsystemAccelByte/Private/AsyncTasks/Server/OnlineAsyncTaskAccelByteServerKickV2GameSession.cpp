#include "OnlineAsyncTaskAccelByteServerKickV2GameSession.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteServerKickV2GameSession::FOnlineAsyncTaskAccelByteServerKickV2GameSession(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FName& InSessionName, const FUniqueNetId& InPlayerIdToKick, const FOnKickPlayerComplete& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, SessionName(InSessionName)
	, PlayerIdToKick(FUniqueNetIdAccelByteUser::CastChecked(InPlayerIdToKick))
	, Delegate(InDelegate)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InLocalUserId);
}

void FOnlineAsyncTaskAccelByteServerKickV2GameSession::Initialize()
{
	TRY_PIN_SUBSYSTEM();

	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("LocalUserId: %s; SessionName: %s; PlayerIdToKick: %s"), *UserId->ToDebugString(), *SessionName.ToString(), *PlayerIdToKick->ToDebugString());

	const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(SubsystemPin->GetSessionInterface());
	if (!SessionInterface.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to kick player from game session as our session interface is invalid!"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	const FNamedOnlineSession* Session = SessionInterface->GetNamedSession(SessionName);
	if (Session == nullptr)
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to kick player from game session as the session can't be found."), *SessionName.ToString());
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	GameSessionID = Session->GetSessionIdStr();

	const auto OnSuccessDelegate = TDelegateUtils<FVoidHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteServerKickV2GameSession::OnKickUserSuccess);
	const auto OnErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteServerKickV2GameSession::OnKickUserError);

	SERVER_API_CLIENT_CHECK_GUARD()
	ServerApiClient->ServerSession.KickUserFromGameSession(GameSessionID, PlayerIdToKick->GetAccelByteId(), OnSuccessDelegate, OnErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteServerKickV2GameSession::Finalize()
{
	Super::Finalize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful)
	{
		TRY_PIN_SUBSYSTEM();

		const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(SubsystemPin->GetSessionInterface());
		if (!ensure(SessionInterface.IsValid()))
		{
			AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to finalize kicking a player from game session as our session interface is invalid!"));
			return;
		}

		SessionInterface->UnregisterPlayer(SessionName, PlayerIdToKick.Get());
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteServerKickV2GameSession::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful)
	{
		TRY_PIN_SUBSYSTEM();

		const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(SubsystemPin->GetSessionInterface());
		if (!ensure(SessionInterface.IsValid()))
		{
			AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to trigger delegates for kicking a player from game session as our session interface is invalid!"));
			return;
		}

		// OnSessionParticipantLeft added in Unreal 5.2 to replace OnSessionParticipantRemoved
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 2
		SessionInterface->TriggerOnSessionParticipantLeftDelegates(SessionName, PlayerIdToKick.Get(), EOnSessionParticipantLeftReason::Kicked);
#endif

		// Both OnSessionParticipantsChange and OnSessionParticipantRemoved are deprecated in 5.2 and removed in 5.5
#if !(ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 5)
		SessionInterface->TriggerOnSessionParticipantsChangeDelegates(SessionName, PlayerIdToKick.Get(), false);

#if ENGINE_MAJOR_VERSION >= 5 || (ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION >= 27)
		SessionInterface->TriggerOnSessionParticipantRemovedDelegates(SessionName, PlayerIdToKick.Get());
#endif
#endif // !(ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 5)

	}

	Delegate.ExecuteIfBound(bWasSuccessful, PlayerIdToKick.Get());

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteServerKickV2GameSession::OnKickUserSuccess()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteServerKickV2GameSession::OnKickUserError(int32 ErrorCode, const FString& ErrorMessage)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN_VERBOSITY(Warning, TEXT("Failed to kick user %s from game session %s. Error code: %d; Error message: %s"), *PlayerIdToKick->GetAccelByteId(), *GameSessionID, ErrorCode, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
	AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT(""));
}
