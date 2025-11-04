// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteGetV2MatchmakingTicketDetails.h"

#include "OnlineAsyncTaskAccelByteStartV2Matchmaking.h"
#include "OnlineSessionInterfaceV2AccelByte.h"
#include "OnlineSubsystemAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteHelpers.h"

#define ONLINE_ERROR_NAMESPACE ("FOnlineAsyncTaskAccelByteGetV2MatchmakingTicketDetails")

FOnlineAsyncTaskAccelByteGetV2MatchmakingTicketDetails::FOnlineAsyncTaskAccelByteGetV2MatchmakingTicketDetails(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InPlayerId, const FString& InTicketId)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, TicketId(InTicketId)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InPlayerId);
}

void FOnlineAsyncTaskAccelByteGetV2MatchmakingTicketDetails::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("LocalPlayerId: %s; TicketId: %s"), *UserId->ToDebugString(), *TicketId);

	SendGetMyTicketRequest();
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetV2MatchmakingTicketDetails::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM();

	Super::TriggerDelegates();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("TicketId: %s"), *TicketId);

	const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(SubsystemPin->GetSessionInterface());
	if (!ensure(SessionInterface.IsValid()))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to trigger delegates for starting matchmaking as our session interface is invalid!"));
		return;
	}

	SessionInterface->TriggerOnGetMatchTicketDetailsCompleteDelegates(MatchTicketDetailResponse, OnlineError);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetV2MatchmakingTicketDetails::OnGetMyMatchTicketSuccess(
	const FAccelByteModelsV2MatchmakingGetTicketDetailsResponse& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("TicketId: %s"), *TicketId);

	MatchTicketDetailResponse = Result;
	OnlineError = FOnlineErrorAccelByte::CreateError(ONLINE_ERROR_NAMESPACE, FString(), EOnlineErrorResult::Success);
	
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetV2MatchmakingTicketDetails::OnGetMyMatchTicketError(const int32 ErrorCode,
	const FString& ErrorMessage)
{
	UE_LOG_AB(Warning, TEXT("Failed to get match ticket details! Error code: %d; Error message: %s"), ErrorCode, *ErrorMessage);
	OnlineError = FOnlineErrorAccelByte::CreateError(ONLINE_ERROR_NAMESPACE, ErrorCode, EOnlineErrorResult::RequestFailure);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}

void FOnlineAsyncTaskAccelByteGetV2MatchmakingTicketDetails::SendGetMyTicketRequest()
{
	OnGetMatchTicketDetailSuccessDelegate = AccelByte::TDelegateUtils<THandler<FAccelByteModelsV2MatchmakingGetTicketDetailsResponse>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGetV2MatchmakingTicketDetails::OnGetMyMatchTicketSuccess);
	OnGetMatchTicketDetailErrorDelegate = AccelByte::TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGetV2MatchmakingTicketDetails::OnGetMyMatchTicketError);;

	API_FULL_CHECK_GUARD(MatchmakingV2, OnlineError);
	MatchmakingV2->GetMatchTicketDetails(TicketId, OnGetMatchTicketDetailSuccessDelegate, OnGetMatchTicketDetailErrorDelegate);
}

#undef ONLINE_ERROR_NAMESPACE