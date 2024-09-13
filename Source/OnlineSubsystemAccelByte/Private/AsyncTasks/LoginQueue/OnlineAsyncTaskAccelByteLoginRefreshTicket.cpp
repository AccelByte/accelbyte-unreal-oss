// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteLoginRefreshTicket.h"
#include "OnlinePredefinedEventInterfaceAccelByte.h"
#include "Core/AccelByteError.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineAsyncTaskAccelByteLoginRefreshTicket"

FOnlineAsyncTaskAccelByteLoginRefreshTicket::FOnlineAsyncTaskAccelByteLoginRefreshTicket(
	FOnlineSubsystemAccelByte* const InABInterface, int32 InLoginUserNum, const FString& InTicketId, const FOnRefreshTicketCompleteDelegate InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, LoginUserNum(InLoginUserNum)
	, TicketId(InTicketId)
	, Delegate(InDelegate)
	, ErrorCode(0)
{
	LocalUserNum = INVALID_CONTROLLERID;
}

void FOnlineAsyncTaskAccelByteLoginRefreshTicket::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("LoginUserNum: %d"), LoginUserNum);
	InitApiClientForLogin(LoginUserNum);
	
	OnRefreshTicketSuccessHandler = TDelegateUtils<THandler<FAccelByteModelsLoginQueueTicketInfo>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteLoginRefreshTicket::OnRefreshTicketSuccess);
	OnRefreshTicketErrorHandler = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteLoginRefreshTicket::OnRefreshTicketError);
	API_CLIENT_CHECK_GUARD(ErrorStr);
	ApiClient->LoginQueue.RefreshTicket(TicketId, OnRefreshTicketSuccessHandler, OnRefreshTicketErrorHandler);

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
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("LoginUserNum: %d, ticketId: %s, position: %d"), LoginUserNum, *TicketInfo.Ticket, TicketInfo.Position);

	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	Result = TicketInfo;
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteLoginRefreshTicket::OnRefreshTicketError(int32 InErrorCode, const FString& InErrorMessage)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("LoginUserNum: %d, ticketId: %s, Error code %d, Message %s"), LoginUserNum, *TicketId, ErrorCode, *InErrorMessage);

	ErrorCode = InErrorCode;
	ErrorStr = InErrorMessage;
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

#undef ONLINE_ERROR_NAMESPACE