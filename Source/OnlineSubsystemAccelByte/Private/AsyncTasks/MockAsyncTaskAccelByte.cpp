// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "AsyncTasks/MockAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncEpicTaskAccelByte.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineSubsystemAccelByteDefines.h"
#include "Core/AccelByteError.h"

using namespace AccelByte;

#define ONLINE_ERROR_NAMESPACE "FOnlineMockAsyncTaskTestAccelByte"

FMockAsyncTaskAccelByte::FMockAsyncTaskAccelByte(
	FOnlineSubsystemAccelByte* const InABSubsystem,
	MockAsyncTaskParameter& InParameter,
	bool bInShouldUseTimeout)
	: FOnlineAsyncTaskAccelByte(InABSubsystem, bInShouldUseTimeout)
	, Parameter(InParameter.AsShared())
	, ErrorCode(TEXT(""))
	, ErrorMessage(TEXT(""))
{
	Super::TaskTimeoutInSeconds = Parameter->TimeoutLimitSeconds;
}

void FMockAsyncTaskAccelByte::TriggerDelegates()
{
	Super::TriggerDelegates();

	if (!Parameter.IsValid())
	{
		return;
	}

	EOnlineErrorResult Result = ((bWasSuccessful) ? EOnlineErrorResult::Success : EOnlineErrorResult::RequestFailure);

	if (CompleteState == EAccelByteAsyncTaskCompleteState::TimedOut)
	{
		ErrorCode = FString::Printf(TEXT("%d"), (int32)ErrorCodes::StatusRequestTimeout);
		ErrorMessage = TEXT("Task Timed Out");
	}

	if (this->ParentTask && bWasSuccessful)
	{
		auto MockParentPtr = static_cast<FMockAsyncTaskAccelByte*>(this->ParentTask);
		MockParentPtr->ChildReportComplete();
	}

	auto ReturnedDelegateValue = ONLINE_ERROR(Result, ErrorCode, FText::FromString(ErrorMessage));
	Parameter->TaskCompleteDelegate.ExecuteIfBound(ReturnedDelegateValue);
}

void FMockAsyncTaskAccelByte::Tick()
{
	if (!Parameter.IsValid())
	{
		return;
	}

	if (this->Epic == nullptr && (FPlatformTime::Seconds() - this->LastTaskUpdateInSeconds) >= Parameter->CompletionTime)
	{
		CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
		return;
	}
	if (Super::HasTaskTimedOut())
	{
		CompleteTask(EAccelByteAsyncTaskCompleteState::TimedOut);
		return;
	}
	if (Parameter->ChildCount == 0 && this->DeltaTickAccumulation >= Parameter->CompletionTime)
	{
		CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
		return;
	}
	if (Parameter->ChildCount == ChildCompleteReportedCount.GetValue()
		&& Parameter->ChildCount > 0
		&& this->DeltaTickAccumulation >= Parameter->CompletionTime)
	{
		CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
		return;
	}
}

void FMockAsyncTaskAccelByte::Initialize()
{
	FOnlineAsyncTaskAccelByte::Initialize();
	
	if (Parameter.IsValid())
	{
		Parameter->EpicPtr = this->Epic;
		FOnlineAsyncTaskAccelByte::ExecuteCriticalSectionAction(Parameter->CreateChildDelegate);
	}
};

#undef ONLINE_ERROR_NAMESPACE 
