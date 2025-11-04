// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteQueryUserOrders.h"

#include "OnlinePurchaseInterfaceAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteHelpers.h"

#define ONLINE_ERROR_NAMESPACE "FOnlinePurchaseSystemAccelByte"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteQueryUserOrders::FOnlineAsyncTaskAccelByteQueryUserOrders(
	FOnlineSubsystemAccelByte* const InABSubsystem, const FUniqueNetId& InUserId, const FAccelByteModelsUserOrdersRequest& InUserOrderRequest)
	: FOnlineAsyncTaskAccelByte(InABSubsystem)
	, UserOrderRequest(InUserOrderRequest) 
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Construct"));
	
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InUserId);
	PagedOrderInfo = FAccelByteModelsPagedOrderInfo{}; 
	ErrorCode = 0;
	ErrorMessage = TEXT("");
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));	
}

void FOnlineAsyncTaskAccelByteQueryUserOrders::Initialize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Initialized"));
	Super::Initialize();
	
	const THandler<FAccelByteModelsPagedOrderInfo>& OnSuccess = TDelegateUtils<THandler<FAccelByteModelsPagedOrderInfo>>::CreateThreadSafeSelfPtr
		(this, &FOnlineAsyncTaskAccelByteQueryUserOrders::HandleSuccess);
	const FErrorHandler& OnError = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr
		(this, &FOnlineAsyncTaskAccelByteQueryUserOrders::HandleError);
	API_FULL_CHECK_GUARD(Order, ErrorMessage);
	Order->QueryUserOrders(UserOrderRequest, OnSuccess, OnError);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryUserOrders::Finalize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Finalized"));
	
	Super::Finalize();
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryUserOrders::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Trigger Delegates"));
	
	Super::TriggerDelegates();
	const FOnlinePurchaseAccelBytePtr PurchaseInterface = StaticCastSharedPtr<FOnlinePurchaseAccelByte>(SubsystemPin->GetPurchaseInterface());
	if (PurchaseInterface.IsValid())
	{
		const FOnlineErrorAccelByte OnlineError = bWasSuccessful ? ONLINE_ERROR_ACCELBYTE(TEXT(""), EOnlineErrorResult::Success) :
			ONLINE_ERROR_ACCELBYTE(FOnlineErrorAccelByte::PublicGetErrorKey(ErrorCode, ErrorMessage));
		PurchaseInterface->TriggerOnQueryUserOrdersCompleteDelegates(bWasSuccessful, PagedOrderInfo, OnlineError);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryUserOrders::HandleSuccess(const FAccelByteModelsPagedOrderInfo& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("HandleSuccess"));

	PagedOrderInfo = Result; 
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryUserOrders::HandleError(int32 Code, const FString& Message)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("HandleError"));

	ErrorCode = Code;
	ErrorMessage = Message; 
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

#undef ONLINE_ERROR_NAMESPACE