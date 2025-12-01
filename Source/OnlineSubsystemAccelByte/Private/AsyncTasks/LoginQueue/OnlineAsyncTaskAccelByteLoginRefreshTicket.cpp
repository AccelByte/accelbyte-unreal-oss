// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteLoginRefreshTicket.h"
#include "OnlinePredefinedEventInterfaceAccelByte.h"
#include "Core/AccelByteError.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteHelpers.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineAsyncTaskAccelByteLoginRefreshTicket"

FOnlineAsyncTaskAccelByteLoginRefreshTicket::FOnlineAsyncTaskAccelByteLoginRefreshTicket(
	FOnlineSubsystemAccelByte* const InABInterface, int32 InLocalUserNum, const FString& InTicketId, const FOnRefreshTicketCompleteDelegate InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface, InLocalUserNum)
	, TicketId(InTicketId)
	, Delegate(InDelegate)
	, ErrorCode(0)
{
}

void FOnlineAsyncTaskAccelByteLoginRefreshTicket::Initialize()
{
	TRY_PIN_SUBSYSTEM();

	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("LocalUserNum: %d"), LocalUserNum);

	FAccelByteInstancePtr AccelByteInstance = GetAccelByteInstance().Pin();
	if(!AccelByteInstance.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Unable to refresh login queue ticket, AccelByteInstance is invalid"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}
	
	FApiClientPtr LocalApiClient = SubsystemPin->GetApiClient(LocalUserNum);

	if (!LocalApiClient.IsValid())
	{
		LocalApiClient = AccelByteInstance->GetApiClient(FString::Printf(TEXT("%d"), LocalUserNum));
	}

	SetApiClient(LocalApiClient);
	
	OnRefreshTicketSuccessHandler = TDelegateUtils<THandler<FAccelByteModelsLoginQueueTicketInfo>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteLoginRefreshTicket::OnRefreshTicketSuccess);
	OnRefreshTicketErrorHandler = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteLoginRefreshTicket::OnRefreshTicketError);
	API_FULL_CHECK_GUARD(LoginQueue, ErrorStr);
	LoginQueue->RefreshTicket(TicketId, OnRefreshTicketSuccessHandler, OnRefreshTicketErrorHandler);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));

}

void FOnlineAsyncTaskAccelByteLoginRefreshTicket::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("TicketId: %s, bWasSuccessfull %s"), *Result.Ticket, bWasSuccessful? TEXT("TRUE") : TEXT("FALSE"));
	
	if(bWasSuccessful)
	{
		Delegate.ExecuteIfBound(bWasSuccessful, Result, ONLINE_ERROR_ACCELBYTE(TEXT(""), EOnlineErrorResult::Success));
	}
	else
	{
		Delegate.ExecuteIfBound(bWasSuccessful, Result, ONLINE_ERROR_ACCELBYTE(FOnlineErrorAccelByte::PublicGetErrorKey(ErrorCode, ErrorStr)));
	}
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteLoginRefreshTicket::OnRefreshTicketSuccess(
	const FAccelByteModelsLoginQueueTicketInfo& TicketInfo)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("LocalUserNum: %d, ticketId: %s, position: %d"), LocalUserNum, *TicketInfo.Ticket, TicketInfo.Position);

	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	Result = TicketInfo;
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteLoginRefreshTicket::OnRefreshTicketError(int32 InErrorCode, const FString& InErrorMessage)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("LocalUserNum: %d, ticketId: %s, Error code %d, Message %s"), LocalUserNum, *TicketId, ErrorCode, *InErrorMessage);

	ErrorCode = InErrorCode;
	ErrorStr = InErrorMessage;
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

#undef ONLINE_ERROR_NAMESPACE