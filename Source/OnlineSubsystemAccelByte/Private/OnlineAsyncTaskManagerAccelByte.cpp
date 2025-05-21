// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskManagerAccelByte.h"
#include "OnlineSubsystemAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"

FOnlineAsyncTaskManagerAccelByte::FOnlineAsyncTaskManagerAccelByte(FOnlineSubsystemAccelByte* ParentSubsystem)
#if ENGINE_MAJOR_VERSION >= 5
	: AccelByteSubsystem(ParentSubsystem->AsWeak())
#else
	: AccelByteSubsystem(ParentSubsystem->AsShared())
#endif
{
}

void FOnlineAsyncTaskManagerAccelByte::OnlineTick()
{
	check(AccelByteSubsystem.Pin().IsValid());
	check(FPlatformTLS::GetCurrentThreadId() == OnlineThreadId);
}

void FOnlineAsyncTaskManagerAccelByte::CheckMaxParallelTasks()
{
#if !UE_BUILD_SHIPPING && (ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION >= 27)
	if (MaxParallelTasks == ParallelTasks.Num())
	{
		UE_LOG(LogAccelByteOSS, Warning, TEXT("The number of Parallel Tasks has reached it cap: %d, Please put some delay between each tasks."), MaxParallelTasks);
	}
#endif
}

void FOnlineAsyncTaskManagerAccelByte::GameThreadCustomTick()
{
	//Dequeueing task for intializing, need to be done from game thread
	FOnlineAsyncTask* DequeuedItem = nullptr;
	while (AsyncTaskToBeInitialized.Dequeue(DequeuedItem))
	{
		if (DequeuedItem == nullptr)
		{
			continue;
		}
		FOnlineAsyncTaskAccelByte* UpcastedItem = static_cast<FOnlineAsyncTaskAccelByte*>(DequeuedItem);
		if (UpcastedItem == nullptr)
		{
			continue;
		}
		if (UpcastedItem->GetCurrentState() == EAccelByteAsyncTaskState::Uninitialized)
		{
			UpcastedItem->Initialize();
		}
	}
}

void FOnlineAsyncTaskManagerAccelByte::EnqueueTaskForInitialize(FOnlineAsyncTask* TaskPtr)
{
	if (TaskPtr == nullptr)
	{
		return;
	}

	AsyncTaskToBeInitialized.Enqueue(TaskPtr);
}