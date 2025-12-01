// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteLoginQueueCancelTicket.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteHelpers.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineAsyncTaskAccelByteLoginQueueCancelTicket"

FOnlineAsyncTaskAccelByteLoginQueueCancelTicket::FOnlineAsyncTaskAccelByteLoginQueueCancelTicket(
	FOnlineSubsystemAccelByte* const InABInterface, int32 InLocalUserNum, const FString& InTicketId)
	: FOnlineAsyncTaskAccelByte(InABInterface, InLocalUserNum)
	, TicketId(InTicketId)
	, ErrorCode(0)
{
}

void FOnlineAsyncTaskAccelByteLoginQueueCancelTicket::Initialize()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("LocalUserNum: %d, ticketId: %s"), LocalUserNum, *TicketId);

	Super::Initialize();

	FAccelByteInstancePtr AccelByteInstance = GetAccelByteInstance().Pin();
	if(!AccelByteInstance.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Unable to cancel login queue, AccelByteInstance is invalid"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}
	
	FApiClientPtr LocalApiClient = SubsystemPin->GetApiClient(LocalUserNum);

	if (!LocalApiClient.IsValid())
	{
		LocalApiClient = AccelByteInstance->GetApiClient(FString::Printf(TEXT("%d"), LocalUserNum));
	}

	SetApiClient(LocalApiClient);
	
	API_FULL_CHECK_GUARD(LoginQueue, ErrorStr);

	OnCancelTicketSuccessHandler = TDelegateUtils<FVoidHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteLoginQueueCancelTicket::OnCancelTicketSuccess);
	OnCancelTicketErrorHandler = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteLoginQueueCancelTicket::OnCancelTicketError);

	LoginQueue->CancelTicket(TicketId, OnCancelTicketSuccessHandler, OnCancelTicketErrorHandler);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteLoginQueueCancelTicket::Finalize()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("LocalUserNum: %d, ticketId: %s, bWasSuccessful: %s"), LocalUserNum, *TicketId, bWasSuccessful? TEXT("True") : TEXT("False"));

	if(!bWasSuccessful)
	{
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Nothing to finalize as the request is unsuccessful"));
		return;
	}

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(SubsystemPin->GetIdentityInterface());
	if(!IdentityInterface.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Failed to finalize login queue cancel result, as our Identity Interface is invalid!"));
		return;
	}

	IdentityInterface->FinalizeLoginQueueCancel(LocalUserNum);
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteLoginQueueCancelTicket::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("LocalUserNum: %d, ticketId: %s, bWasSuccessful: %s"), LocalUserNum, *TicketId, bWasSuccessful? TEXT("True") : TEXT("False"));

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(SubsystemPin->GetIdentityInterface());
	if(!IdentityInterface.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Failed to broadcast login queue cancel result, as our Identity Interface is invalid!"));
		return;
	}

	IdentityInterface->TriggerAccelByteOnLoginQueueCancelCompleteDelegates(LocalUserNum, bWasSuccessful,
			ONLINE_ERROR_ACCELBYTE(ErrorCode, bWasSuccessful? EOnlineErrorResult::Success : EOnlineErrorResult::RequestFailure));

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteLoginQueueCancelTicket::OnCancelTicketSuccess()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("LocalUserNum: %d, ticketId: %s"), LocalUserNum, *TicketId);

	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteLoginQueueCancelTicket::OnCancelTicketError(int32 InErrorCode, const FString& InErrorMessage)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("LocalUserNum: %d, ticketId: %s, ErrorCode: %d, ErrorMessage %s"), LocalUserNum, *TicketId, InErrorCode, *InErrorMessage);

	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
	ErrorCode = InErrorCode;
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

#undef ONLINE_ERROR_NAMESPACE