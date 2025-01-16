// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteLoginQueueCancelTicket.h"
#include "OnlineIdentityInterfaceAccelByte.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineAsyncTaskAccelByteLoginQueueCancelTicket"

FOnlineAsyncTaskAccelByteLoginQueueCancelTicket::FOnlineAsyncTaskAccelByteLoginQueueCancelTicket(
	FOnlineSubsystemAccelByte* const InABInterface, int32 InLoginUserNum, const FString& InTicketId)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, LoginUserNum(InLoginUserNum)
	, TicketId(InTicketId)
	, ErrorCode(0)
{
	LocalUserNum = INVALID_CONTROLLERID;
}

void FOnlineAsyncTaskAccelByteLoginQueueCancelTicket::Initialize()
{
	TRY_PIN_SUBSYSTEM()

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("LoginUserNum: %d, ticketId: %s"), LoginUserNum, *TicketId);

	Super::Initialize();

	FAccelByteInstancePtr AccelByteInstance = GetAccelByteInstance().Pin();
	if(!AccelByteInstance.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Unable to cancel login queue, AccelByteInstance is invalid"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}
	
	if (SubsystemPin->IsMultipleLocalUsersEnabled())
	{
		SetApiClient(AccelByteInstance->GetApiClient(FString::Printf(TEXT("%d"), LoginUserNum)));
	}
	else
	{
		SetApiClient(AccelByteInstance->GetApiClient());
	}
	
	API_CLIENT_CHECK_GUARD(ErrorStr);

	OnCancelTicketSuccessHandler = TDelegateUtils<FVoidHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteLoginQueueCancelTicket::OnCancelTicketSuccess);
	OnCancelTicketErrorHandler = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteLoginQueueCancelTicket::OnCancelTicketError);

	ApiClient->LoginQueue.CancelTicket(TicketId, OnCancelTicketSuccessHandler, OnCancelTicketErrorHandler);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteLoginQueueCancelTicket::Finalize()
{
	TRY_PIN_SUBSYSTEM()

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("LoginUserNum: %d, ticketId: %s, bWasSuccessful: %s"), LoginUserNum, *TicketId, bWasSuccessful? TEXT("True") : TEXT("False"));

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

	IdentityInterface->FinalizeLoginQueueCancel(LoginUserNum);
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteLoginQueueCancelTicket::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM()

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("LoginUserNum: %d, ticketId: %s, bWasSuccessful: %s"), LoginUserNum, *TicketId, bWasSuccessful? TEXT("True") : TEXT("False"));

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(SubsystemPin->GetIdentityInterface());
	if(!IdentityInterface.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Failed to broadcast login queue cancel result, as our Identity Interface is invalid!"));
		return;
	}

	IdentityInterface->TriggerAccelByteOnLoginQueueCancelCompleteDelegates(LoginUserNum, bWasSuccessful,
			ONLINE_ERROR_ACCELBYTE(ErrorCode, bWasSuccessful? EOnlineErrorResult::Success : EOnlineErrorResult::RequestFailure));

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteLoginQueueCancelTicket::OnCancelTicketSuccess()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("LoginUserNum: %d, ticketId: %s"), LoginUserNum, *TicketId);

	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteLoginQueueCancelTicket::OnCancelTicketError(int32 InErrorCode, const FString& InErrorMessage)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("LoginUserNum: %d, ticketId: %s, ErrorCode: %d, ErrorMessage %s"), LoginUserNum, *TicketId, InErrorCode, *InErrorMessage);

	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
	ErrorCode = InErrorCode;
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

#undef ONLINE_ERROR_NAMESPACE