// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteHelpers.h"
#include "AsyncTasks/OnlineAsyncEpicTaskAccelByte.h"
#include "OnlineSubsystemAccelByteLog.h"

FOnlineAsyncTaskAccelByte::FOnlineAsyncTaskAccelByte(FOnlineSubsystemAccelByte* const InABSubsystem
	, bool bInShouldUseTimeout)
	: FOnlineAsyncTaskAccelByte(InABSubsystem
		, INVALID_CONTROLLERID
		, (bInShouldUseTimeout) ? ASYNC_TASK_FLAG_BIT(EAccelByteAsyncTaskFlags::UseTimeout) : ASYNC_TASK_FLAG_BIT(EAccelByteAsyncTaskFlags::None))
{
}
	
/**
	* Breaking const placement here as parent class has the InSubsystem defined as 'T* const'. Trying to define as 'const T*' gives error C2664.
	* 
	* Child classes that use this constructor will also need to use this convention.
	*
	* @param InABSubsystem A pointer to AccelByte OnlineSubsystem instance
	* @param InLocalUserNum Local User Index
	* @param bInShouldUseTimeout Whether any child of this task will by default use a timeout mechanism on Tick
	*/
FOnlineAsyncTaskAccelByte::FOnlineAsyncTaskAccelByte(FOnlineSubsystemAccelByte* const InABSubsystem
	, int32 InLocalUserNum
	, bool bInShouldUseTimeout)
	: FOnlineAsyncTaskAccelByte(InABSubsystem
		, InLocalUserNum
		, (bInShouldUseTimeout) ? ASYNC_TASK_FLAG_BIT(EAccelByteAsyncTaskFlags::UseTimeout) : ASYNC_TASK_FLAG_BIT(EAccelByteAsyncTaskFlags::None))
{
}

/**
	* Breaking const placement here as parent class has the InSubsystem defined as 'T* const'. Trying to define as 'const T*' gives error C2664.
	* 
	* Child classes that use this constructor will also need to use this convention.
	*
	* @param InABSubsystem A pointer to AccelByte OnlineSubsystem instance
	* @param InLocalUserNum Local User Index
	* @param InFlags Flags whether any child of this task will by default use a timeout mechanism on Tick
	*/
FOnlineAsyncTaskAccelByte::FOnlineAsyncTaskAccelByte(FOnlineSubsystemAccelByte* const InABSubsystem
	, int32 InLocalUserNum
	, int32 InFlags)
	: FOnlineAsyncTaskAccelByte(InABSubsystem
		, InLocalUserNum
		, InFlags
		, nullptr)
{
}

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

void FOnlineAsyncTaskAccelByte::Initialize()
{
	CurrentState = EAccelByteAsyncTaskState::Initializing;

	// We only care about setting the last update time if we are using a timeout
	if (bShouldUseTimeout)
	{
		SetLastUpdateTimeToCurrentTime();
	}

	// Do not attempt to get API clients for server async tasks, as servers do not have API client support.
	// We also don't need to get the corresponding user ID for the server, as server's don't have user IDs.
	if (!BitFlags[static_cast<uint8>(EAccelByteAsyncTaskFlags::ServerTask)])
	{
		if (LocalUserNum != INVALID_CONTROLLERID)
		{
			GetApiClient(LocalUserNum);
		}
		else if (UserId.IsValid())
		{
			GetApiClient(UserId.ToSharedRef());
		}

		// Get corresponding UserId or LocalUserNum for player that started this task
		GetOtherUserIdentifiers();
	}
	else
	{
		TRY_PIN_SUBSYSTEM();
		if (LocalUserNum == INVALID_CONTROLLERID)
		{
			LocalUserNum = SubsystemPin->GetLocalUserNumCached();
		}
		UserId = nullptr;
	}
}

void FOnlineAsyncTaskAccelByte::Tick()
{
	if (HasTaskTimedOut())
	{
		UE_LOG(LogAccelByteOSS, Warning, TEXT("Task %s has been idle for longer than %f s"), *GetTaskName(), TaskTimeoutInSeconds);
		if (bShouldUseTimeout)
		{
			CompleteTask(EAccelByteAsyncTaskCompleteState::TimedOut);
			OnTaskTimedOut();
		}
	}

	// If we are not currently in the working state, then kick off the work we need to do for the task
	if (CurrentState != EAccelByteAsyncTaskState::Working)
	{
		CurrentState = EAccelByteAsyncTaskState::Working;
		OnTaskStartWorking();
	}
}

void FOnlineAsyncTaskAccelByte::CompleteTask(const EAccelByteAsyncTaskCompleteState& InCompleteState)
{
	// We're already marked as complete, do not change this state!
	if (bIsComplete)
	{
		UE_LOG_AB(Warning, TEXT("Tried to complete async task while state was already complete! Current complete state: %s; Requested complete state: %s"), *AsyncTaskCompleteStateToString(CompleteState), *AsyncTaskCompleteStateToString(InCompleteState));
		return;
	}

	CurrentState = EAccelByteAsyncTaskState::Completed;
	CompleteState = InCompleteState;
	bWasSuccessful = (CompleteState == EAccelByteAsyncTaskCompleteState::Success);
	bIsComplete = true;
}

void FOnlineAsyncTaskAccelByte::RaiseGenericError()
{
	TaskOnlineError = EOnlineErrorResult::RequestFailure;
	TaskErrorStr = TEXT("request-to-obtain-valid-apiclient");
	UE_LOG_AB(Warning, TEXT("%s"), *TaskErrorStr);
	CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
}

void FOnlineAsyncTaskAccelByte::RaiseGenericServerError()
{
	TaskOnlineError = EOnlineErrorResult::RequestFailure;
	TaskErrorStr = TEXT("request-to-obtain-valid-serverapiclient");
	UE_LOG_AB(Warning, TEXT("%s"), *TaskErrorStr);
	CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
}

AccelByte::FApiClientPtr FOnlineAsyncTaskAccelByte::GetApiClient(int32 InLocalUserNum)
{
	TRY_PIN_SUBSYSTEM(nullptr);

	AccelByte::FApiClientPtr ApiClient = GetApiClientInternal();
	if (!ApiClient.IsValid())
	{
		ApiClient = SubsystemPin->GetApiClient(InLocalUserNum);
		ApiClientInternal = ApiClient;
	}
	return ApiClient;
}

AccelByte::FApiClientPtr FOnlineAsyncTaskAccelByte::GetApiClient(FUniqueNetIdAccelByteUserRef const& InUserId)
{
	TRY_PIN_SUBSYSTEM(nullptr);

	AccelByte::FApiClientPtr ApiClient = GetApiClientInternal();
	if (!ApiClient.IsValid())
	{
		ApiClient = SubsystemPin->GetApiClient(InUserId.Get());
		ApiClientInternal = ApiClient;
	}
	return ApiClient;
}

FAccelByteInstanceWPtr FOnlineAsyncTaskAccelByte::GetAccelByteInstance()
{
	TRY_PIN_SUBSYSTEM(nullptr);

	return SubsystemPin->GetAccelByteInstance();
}

void FOnlineAsyncTaskAccelByte::GetOtherUserIdentifiers()
{
	TRY_PIN_SUBSYSTEM();

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(SubsystemPin->GetIdentityInterface());
	if (!IdentityInterface.IsValid())
	{
		return;
	}

	// If we have a local user num, then we want to get a user ID from that user num
	if (LocalUserNum != INVALID_CONTROLLERID)
	{
		FUniqueNetIdPtr PlayerId = IdentityInterface->GetUniquePlayerId(LocalUserNum);
		if (PlayerId.IsValid())
		{
//#ifdef ONLINESUBSYSTEMACCELBYTE_PACKAGE
			UserId = FUniqueNetIdAccelByteUser::CastChecked(PlayerId.ToSharedRef());
//#else
			//UserId = StaticCastSharedPtr<const FUniqueNetIdAccelByteUser>(PlayerId);
//#endif
		}
	}
	// Otherwise, if we have a user ID, we want to get their local user num
	else if (UserId.IsValid())
	{
		int32 FoundLocalUserNum;
		if (IdentityInterface->GetLocalUserNum(UserId.ToSharedRef().Get(), FoundLocalUserNum))
		{
			LocalUserNum = FoundLocalUserNum;
		}
	}
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