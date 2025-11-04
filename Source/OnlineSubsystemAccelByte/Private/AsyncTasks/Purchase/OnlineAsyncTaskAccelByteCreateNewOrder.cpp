// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteCreateNewOrder.h"
#include "OnlinePredefinedEventInterfaceAccelByte.h"
#include "OnlinePurchaseInterfaceAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteHelpers.h"

#define ONLINE_ERROR_NAMESPACE "FOnlinePurchaseSystemAccelByte"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteCreateNewOrder::FOnlineAsyncTaskAccelByteCreateNewOrder(
	FOnlineSubsystemAccelByte* const InABSubsystem, const FUniqueNetId& InUserId, const FAccelByteModelsOrderCreate& InOrderCreate)
	: FOnlineAsyncTaskAccelByte(InABSubsystem)
	, OrderCreate(InOrderCreate)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Construct"));

	UserId = FUniqueNetIdAccelByteUser::CastChecked(InUserId); 
	OrderInfo = FAccelByteModelsOrderInfo{}; 
	ErrorCode = 0;
	ErrorMessage = TEXT("");
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));	
}

void FOnlineAsyncTaskAccelByteCreateNewOrder::Initialize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Initialized"));
	Super::Initialize();
	
	const THandler<FAccelByteModelsOrderInfo>& OnSuccess = TDelegateUtils<THandler<FAccelByteModelsOrderInfo>>::CreateThreadSafeSelfPtr
		(this, &FOnlineAsyncTaskAccelByteCreateNewOrder::HandleSuccess);
	const FErrorHandler& OnError = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr
		(this, &FOnlineAsyncTaskAccelByteCreateNewOrder::HandleError);
	API_FULL_CHECK_GUARD(Order, ErrorMessage);
	Order->CreateNewOrder(OrderCreate, OnSuccess, OnError);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteCreateNewOrder::Finalize()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Finalized"));
	
	Super::Finalize();

	const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = SubsystemPin->GetPredefinedEventInterface();
	if (PredefinedEventInterface.IsValid())
	{
		if (bWasSuccessful)
		{
			FAccelByteModelsPaymentSuccededPayload Payload{};
			Payload.UserId = OrderInfo.UserId;
			Payload.ItemId = OrderInfo.ItemId;
			Payload.Price = OrderInfo.Price;
			Payload.OrderNo = OrderInfo.OrderNo;
			Payload.PaymentOrderNo = OrderInfo.PaymentOrderNo;
			Payload.Status = FAccelByteUtilities::GetUEnumValueAsString(OrderInfo.Status);
			PredefinedEventInterface->SendEvent(LocalUserNum, MakeShared<FAccelByteModelsPaymentSuccededPayload>(Payload));
		}
		else
		{
			FAccelByteModelsPaymentFailedPayload Payload{};
			Payload.UserId = UserId->GetAccelByteId();
			Payload.ItemId = OrderCreate.ItemId;
			Payload.Price = OrderCreate.Price;
			PredefinedEventInterface->SendEvent(LocalUserNum, MakeShared<FAccelByteModelsPaymentFailedPayload>(Payload));
		}
	}
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteCreateNewOrder::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Trigger Delegates"));
	
	Super::TriggerDelegates();
	const FOnlinePurchaseAccelBytePtr PurchaseInterface = StaticCastSharedPtr<FOnlinePurchaseAccelByte>(SubsystemPin->GetPurchaseInterface());
	if (PurchaseInterface.IsValid())
	{
		const FOnlineErrorAccelByte OnlineError = bWasSuccessful ? ONLINE_ERROR_ACCELBYTE(TEXT(""), EOnlineErrorResult::Success) :
			ONLINE_ERROR_ACCELBYTE(FOnlineErrorAccelByte::PublicGetErrorKey(ErrorCode, ErrorMessage));
		PurchaseInterface->TriggerOnCreateNewOrderCompleteDelegates(bWasSuccessful, OrderInfo, OnlineError);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteCreateNewOrder::HandleSuccess(const FAccelByteModelsOrderInfo& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("HandleSuccess"));

	OrderInfo = Result; 
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteCreateNewOrder::HandleError(int32 Code, const FString& Message)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("HandleError"));

	ErrorCode = Code;
	ErrorMessage = Message; 
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

#undef ONLINE_ERROR_NAMESPACE