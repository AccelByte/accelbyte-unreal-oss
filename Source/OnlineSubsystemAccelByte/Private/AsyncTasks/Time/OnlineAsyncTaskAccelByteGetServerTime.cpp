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
	THandler<FTime> OnGetServerTimeSuccess =
		TDelegateUtils<THandler<FTime>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGetServerTime::HandleGetServerTimeSuccess);
	FErrorHandler OnError = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGetServerTime::HandleGetServerTimeError);
	FRegistry::TimeManager.GetServerTime(OnGetServerTimeSuccess, OnError, true);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetServerTime::TriggerDelegates() 
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const IOnlineTimePtr TimeInterface = Subsystem->GetTimeInterface();
	if (TimeInterface.IsValid()) 
	{
		TimeInterface->TriggerOnQueryServerUtcTimeCompleteDelegates(bWasSuccessful, LocalCachedServerTime->ToString(), ErrorMessage);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetServerTime::HandleGetServerTimeSuccess(FTime const& Result)
{
	const FOnlineTimeAccelBytePtr TimeInterface =  StaticCastSharedPtr<FOnlineTimeAccelByte>(Subsystem->GetTimeInterface());
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
