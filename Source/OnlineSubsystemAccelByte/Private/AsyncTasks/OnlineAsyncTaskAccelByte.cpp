// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncEpicTaskAccelByte.h"

void FOnlineAsyncTaskAccelByte::ExecuteCriticalSectionAction(FVoidHandler Action)
{
	if (!Action.IsBound())
	{
		return;
	}

	// If current task has an epic, we can pass it to the child
	// Else, if Subsystem already has an Epic, then we can proceed to CreateAndDispatch
	// Else we create a new Epic & register it to the subsystem

	if (this->Epic != nullptr)
	{
		FScopeLock TaskLock(Subsystem->GetUpcomingParentTaskLock());
		FScopeLock EpicLock(Subsystem->GetEpicTaskLock());
		Subsystem->SetUpcomingEpic(this->Epic);
		Subsystem->SetUpcomingParentTask(this);
		Action.Execute();
		Subsystem->ResetEpicHasBeenSet();
		Subsystem->ResetParentTaskHasBeenSet();
		return;
	}
	
	{
		FScopeLock EpicLock(Subsystem->GetEpicTaskLock());
		if (Subsystem->IsUpcomingEpicAlreadySet())
		{
			FScopeLock Lock(Subsystem->GetUpcomingParentTaskLock());
			Subsystem->SetUpcomingParentTask(this);
			Action.Execute();
			Subsystem->ResetParentTaskHasBeenSet();
			return;
		}
	}

	FScopeLock TaskLock(Subsystem->GetUpcomingParentTaskLock());
	FScopeLock EpicLock(Subsystem->GetEpicTaskLock());
	FVoidHandler EmptyDelegate = FVoidHandler::CreateLambda([]() {});
	FOnlineAsyncEpicTaskAccelByte* EpicPtr = Subsystem->CreateAndDispatchEpic(LocalUserNum, EmptyDelegate);
	Subsystem->SetUpcomingEpic(this->Epic);
	Subsystem->SetUpcomingParentTask(this);
	Action.Execute();
	Subsystem->ResetEpicHasBeenSet();
	Subsystem->ResetParentTaskHasBeenSet();
}

void FOnlineAsyncTaskAccelByte::ForcefullySetTimeoutState()
{
	CompleteTask(EAccelByteAsyncTaskCompleteState::TimedOut);
	OnTaskTimedOut();
	DeltaTickAccumulation += TaskTimeoutInSeconds;
	LastTaskUpdateInSeconds -= TaskTimeoutInSeconds;
}
