// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelBytePromoteV2PartyLeader.h"
#include "OnlinePredefinedEventInterfaceAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteHelpers.h"

#define ONLINE_ERROR_NAMESPACE "FOnlineAsyncTaskAccelBytePromoteV2PartyLeader"

FOnlineAsyncTaskAccelBytePromoteV2PartyLeader::FOnlineAsyncTaskAccelBytePromoteV2PartyLeader(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FName& InSessionName, const FString& InSessionId, const FUniqueNetId& InTargetMemberId, const FOnPromotePartySessionLeaderComplete& InCompletionDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, SessionName(InSessionName)
	, SessionId(InSessionId)
	, TargetMemberId(FUniqueNetIdAccelByteUser::CastChecked(InTargetMemberId))
	, CompletionDelegate(InCompletionDelegate)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InLocalUserId);
}

void FOnlineAsyncTaskAccelBytePromoteV2PartyLeader::Initialize()
{
	TRY_PIN_SUBSYSTEM();

	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s; SessionId: %s; TargetMemberId: %s"), *UserId->GetAccelByteId(), *SessionId, *TargetMemberId->GetAccelByteId());

	const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(SubsystemPin->GetSessionInterface());
	AB_ASYNC_TASK_VALIDATE(SessionInterface.IsValid(), "Failed to promote player to leader of party session as our session interface is invalid!");

	FNamedOnlineSession* OnlineSession = SessionInterface->GetPartySession();
	AB_ASYNC_TASK_VALIDATE(OnlineSession != nullptr, "Failed to promote player to leader of party session as the local session instance is invalid!");
	
	OnPromotePartyLeaderSuccessDelegate = AccelByte::TDelegateUtils<THandler<FAccelByteModelsV2PartySession>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelBytePromoteV2PartyLeader::OnPromotePartyLeaderSuccess);
	OnPromotePartyLeaderErrorDelegate = AccelByte::TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelBytePromoteV2PartyLeader::OnPromotePartyLeaderError);;
	API_FULL_CHECK_GUARD(Session);
	Session->PromotePartyLeader(SessionId, TargetMemberId->GetAccelByteId(), OnPromotePartyLeaderSuccessDelegate, OnPromotePartyLeaderErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Sent request to promote a member of this party to leader."));
}

void FOnlineAsyncTaskAccelBytePromoteV2PartyLeader::Finalize()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = SubsystemPin->GetPredefinedEventInterface();
	if (bWasSuccessful && PredefinedEventInterface.IsValid())
	{
		FAccelByteModelsMPV2PartySessionLeaderPromotedPayload PartySessionLeaderPromotedPayload{};
		PartySessionLeaderPromotedPayload.PromotedUserId = TargetMemberId->GetAccelByteId();
		PartySessionLeaderPromotedPayload.PartySessionId = SessionId;
		PredefinedEventInterface->SendEvent(LocalUserNum, MakeShared<FAccelByteModelsMPV2PartySessionLeaderPromotedPayload>(PartySessionLeaderPromotedPayload));
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelBytePromoteV2PartyLeader::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	EOnlineErrorResult Result = ((bWasSuccessful) ? EOnlineErrorResult::Success : EOnlineErrorResult::RequestFailure);
	CompletionDelegate.ExecuteIfBound(TargetMemberId.Get(), ONLINE_ERROR(Result));

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelBytePromoteV2PartyLeader::OnPromotePartyLeaderSuccess(const FAccelByteModelsV2PartySession& BackendSessionData)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Successfully promoted member of party to leader!"));
}

void FOnlineAsyncTaskAccelBytePromoteV2PartyLeader::OnPromotePartyLeaderError(int32 ErrorCode, const FString& ErrorMessage)
{
	AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to promote party member '%s' to leader of party session '%s' as the call to the backend failed! Error code: %d; Error message: %s"), *TargetMemberId->GetAccelByteId(), *SessionId, ErrorCode, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
	return;
}

#undef ONLINE_ERROR_NAMESPACE