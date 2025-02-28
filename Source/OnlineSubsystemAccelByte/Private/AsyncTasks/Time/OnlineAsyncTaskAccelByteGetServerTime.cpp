// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteGetServerTime.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineTimeInterfaceAccelByte.h"
#include "Models/AccelByteGeneralModels.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteGetServerTime::FOnlineAsyncTaskAccelByteGetServerTime(FOnlineSubsystemAccelByte* const InABInterface)
	: FOnlineAsyncTaskAccelByte(InABInterface, true)
	, LocalCachedServerTime(MakeShared<FDateTime>())
	, ErrorMessage(TEXT(""))
{
}

void FOnlineAsyncTaskAccelByteGetServerTime::Initialize() 
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(" "));
	
	FAccelByteInstancePtr AccelByteInstancePtr = GetAccelByteInstance().Pin();
	if(!AccelByteInstancePtr.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("AccelByteInstance is invalid"));
		ErrorMessage = TEXT("AccelByteInstance is invalid");
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}
	
	FAccelByteTimeManagerPtr TimeManagerPtr = AccelByteInstancePtr->GetTimeManager().Pin();
	if(!AccelByteInstancePtr.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("TimeManager is invalid"));
		ErrorMessage = TEXT("TimeManager is invalid");
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	THandler<FTime> OnGetServerTimeSuccess =
		TDelegateUtils<THandler<FTime>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGetServerTime::HandleGetServerTimeSuccess);
	FErrorHandler OnError = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGetServerTime::HandleGetServerTimeError);

	TimeManagerPtr->GetServerTime(OnGetServerTimeSuccess, OnError, true);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetServerTime::TriggerDelegates() 
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const IOnlineTimePtr TimeInterface = SubsystemPin->GetTimeInterface();
	if (TimeInterface.IsValid()) 
	{
		TimeInterface->TriggerOnQueryServerUtcTimeCompleteDelegates(bWasSuccessful, LocalCachedServerTime->ToString(), ErrorMessage);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetServerTime::HandleGetServerTimeSuccess(FTime const& Result)
{
	TRY_PIN_SUBSYSTEM();

	const FOnlineTimeAccelBytePtr TimeInterface =  StaticCastSharedPtr<FOnlineTimeAccelByte>(SubsystemPin->GetTimeInterface());
	if (TimeInterface.IsValid())
	{
		CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	}
	else
	{
		AB_OSS_ASYNC_TASK_TRACE_BEGIN_VERBOSITY(Warning, TEXT("Message: %s"), TEXT("Failed to request the task of get server time as our time interface is invalid!"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
	}
	LocalCachedServerTime = MakeShared<FDateTime>(Result.CurrentTime);
}

void FOnlineAsyncTaskAccelByteGetServerTime::HandleGetServerTimeError(int32 Code, FString const& ErrMsg)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN_VERBOSITY(Error, TEXT("Code: %d; Message: %s"), Code, *ErrMsg);

	ErrorMessage = ErrMsg;
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}
