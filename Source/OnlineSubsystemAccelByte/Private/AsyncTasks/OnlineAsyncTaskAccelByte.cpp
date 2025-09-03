// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncEpicTaskAccelByte.h"

FOnlineAsyncTaskAccelByte::FOnlineAsyncTaskAccelByte(FOnlineSubsystemAccelByte* const InABSubsystem
	, int32 InLocalUserNum
	, int32 InFlags
	, TSharedPtr<FAccelByteKey> InLockKey)
	: FOnlineAsyncTaskBasic(InABSubsystem)
#if ENGINE_MAJOR_VERSION >= 5
	, AccelByteSubsystem(InABSubsystem ? InABSubsystem->AsWeak() : FOnlineSubsystemAccelByteWPtr())
#else
	, AccelByteSubsystem(InABSubsystem ? InABSubsystem->AsShared() : FOnlineSubsystemAccelByteWPtr())
#endif
	, LocalUserNum(InLocalUserNum)
	, Flags(InFlags)
	, LockKey(InLockKey)
	, Subsystem(InABSubsystem)
{
	int32 AllFlagsCount = static_cast<int32>(EAccelByteAsyncTaskFlags::AllFlags);
	BitFlags.Init(false, AllFlagsCount);

	for (int FlagBit = 0; FlagBit < AllFlagsCount; FlagBit++)
	{
		BitFlags[FlagBit] = HasFlag(FlagBit);
	}

	if (InABSubsystem && InLocalUserNum != INVALID_CONTROLLERID)
	{
		TOptional<bool> IsDS = InABSubsystem->IsDedicatedServer(InLocalUserNum);
		if (IsDS.IsSet() && IsDS.GetValue())
		{
			BitFlags[static_cast<uint8>(EAccelByteAsyncTaskFlags::ServerTask)] = true;
		}
	}

	bShouldUseTimeout = BitFlags[static_cast<uint8>(EAccelByteAsyncTaskFlags::UseTimeout)];

	// NOTE(Maxwell, 7/8/2021): Due to a bug where we cannot cancel requests on the SDK side, as well as cancel the delegates
	// that are supposed to run with these requests, if a timeout is quicker than a request could be received from the backend
	// we may get a crash from that. To combat this for now, we want to set our default timeout to always be one second higher
	// than the SDK HTTP timeout to give the SDK a chance to fire off its delegates for a timeout.
	// Fix this once https://accelbyte.atlassian.net/browse/OSS-193 is implemented.
	TaskTimeoutInSeconds = static_cast<double>(AccelByte::FHttpRetryScheduler::TotalTimeout) + 1.0;
}

void FOnlineAsyncTaskAccelByte::ForcefullySetTimeoutState()
{
	CompleteTask(EAccelByteAsyncTaskCompleteState::TimedOut);
	OnTaskTimedOut();
	DeltaTickAccumulation += TaskTimeoutInSeconds;
	LastTaskUpdateInSeconds -= TaskTimeoutInSeconds;
}

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