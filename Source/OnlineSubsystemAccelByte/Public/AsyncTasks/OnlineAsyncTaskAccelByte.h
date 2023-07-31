// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "OnlineAsyncTaskManager.h"
#include "OnlineSubsystemAccelByteModule.h"
#include "OnlineSubsystemAccelByteInternalHelpers.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "Engine/LocalPlayer.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "OnlineSubsystemAccelByte.h"
#include "Core/AccelByteMultiRegistry.h"

#define AB_OSS_ASYNC_TASK_TRACE_BEGIN_VERBOSITY(Verbosity, Format, ...) UE_LOG_AB(Verbosity, TEXT(">>> %s::%s (AsyncTask method) was called. Args: ") Format, *GetTaskName(), *FString(__func__), ##__VA_ARGS__)
#define AB_OSS_ASYNC_TASK_TRACE_BEGIN(Format, ...) AB_OSS_ASYNC_TASK_TRACE_BEGIN_VERBOSITY(Verbose, Format, ##__VA_ARGS__)
#define AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Verbosity, Format, ...) UE_LOG_AB(Verbosity, TEXT("<<< %s::%s (AsyncTask method) has finished execution. ") Format, *GetTaskName(), *FString(__func__), ##__VA_ARGS__)
#define AB_OSS_ASYNC_TASK_TRACE_END(Format, ...) AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Verbose, Format, ##__VA_ARGS__)

/**
 * Declares two methods for SDK delegate handlers. Use in a header file to define these for the task, and then use
 * AB_ASYNC_TASK_DEFINE_SDK_DELEGATES to define the bindings to these delegates in the async task logic.
 *
 * @param Verb Action that this delegate is handling
 */
#define AB_ASYNC_TASK_DECLARE_SDK_DELEGATES(Verb) void On##Verb##Success(); \
	void On##Verb##Error(int32 ErrorCode, const FString& ErrorMessage);

/**
 * Declares two methods for SDK delegate handlers. Use in a header file to define these for the task, and then use
 * AB_ASYNC_TASK_DEFINE_SDK_DELEGATES to define the bindings to these delegates in the async task logic.
 * 
 * @param Verb Action that this delegate is handling
 * @param SuccessType Type that the success delegate will recieve
 */
#define AB_ASYNC_TASK_DECLARE_SDK_DELEGATES_WITH_RESULT(Verb, SuccessType) void On##Verb##Success(const SuccessType& Result); \
	void On##Verb##Error(int32 ErrorCode, const FString& ErrorMessage);

/**
 * Defines two delegates for use in an SDK call wrapped in an async task. Success delegate will have the name
 * On{Verb}SuccessDelegate, and be bound to the On{Verb}Success method of the class. Error delegate will have
 * the name On{Verb}ErrorDelegate, and be bound to the On{Verb}Error method of the class.
 * 
 * @param AsyncTaskClass Name of the class that we are binding delegate methods to
 * @param Verb Name of the action that is being handled by the two delegates, effects the name of the final delegates
 * @param SuccessType Delegate type for the success delegate
 */
#define AB_ASYNC_TASK_DEFINE_SDK_DELEGATES(AsyncTaskClass, Verb, SuccessType) \
	const SuccessType On##Verb##SuccessDelegate = SuccessType::CreateRaw(this, &AsyncTaskClass::On##Verb##Success); \
	const FErrorHandler On##Verb##ErrorDelegate = FErrorHandler::CreateRaw(this, &AsyncTaskClass::On##Verb##Error);

/**
 * Convenience macro for async tasks to ensure that a expression evaluates to true, otherwise throwing an InvalidState error in the task.
 * This will also stop execution of the task by returning out.
 * 
 * @param Expression Expression that you want to evaluate to true, or fail the task
 * @param Message Message to log in a trace if the expression is false. This will automatically be wrapped in TEXT macro, so no need to do that manually.
 */
#define AB_ASYNC_TASK_ENSURE(Expression, Message, ...) if (!ensure(Expression))   \
{                                                                                 \
	AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT(Message), ##__VA_ARGS__); \
	CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);                 \
	return;                                                                       \
}

/**
 * Convenience macro to log a request error and fail a task.
 * 
 * @param Message Message to log for the request that failed
 * @param ErrorCode Error code for the request that failed
 * @param ErrorMessage Error message for the request that failed
 */
#define AB_ASYNC_TASK_REQUEST_FAILED(Message, ErrorCode, ErrorMessage, ...) const FString LogString = FString::Printf(TEXT(Message), ##__VA_ARGS__); \
	UE_LOG_AB(Warning, TEXT("%s. Error code: %d; Error message: %s"), *LogString, ErrorCode, *ErrorMessage); \
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

/**
 * Enum to describe what state an async task is in currently
 */
enum class EAccelByteAsyncTaskState : uint8
{
	Uninitialized = 0,
	Initializing,
	Working,
	Completed
};

/**
 * Enum to describe what state an async task finished in beyond just a success and fail state.
 */
enum class EAccelByteAsyncTaskCompleteState : uint8
{
	Success = 0,
	TimedOut,
	RequestFailed,
	InvalidState,
	Incomplete
};

const static inline FString AsyncTaskCompleteStateToString(const EAccelByteAsyncTaskCompleteState& CompleteState)
{
	switch (CompleteState)
	{
	case EAccelByteAsyncTaskCompleteState::Success:
		return TEXT("Success");
	case EAccelByteAsyncTaskCompleteState::TimedOut:
		return TEXT("Timed out");
	case EAccelByteAsyncTaskCompleteState::RequestFailed:
		return TEXT("Request failed");
	case EAccelByteAsyncTaskCompleteState::InvalidState:
		return TEXT("Invalid state");
	case EAccelByteAsyncTaskCompleteState::Incomplete:
		return TEXT("Incomplete");
	}
	return TEXT("Unknown");
}

enum class EAccelByteAsyncTaskFlags : uint8
{
	None = 0, // Special state for having no flags set on the task
	UseTimeout = 1, // Whether to track the task with a timeout that will automatically end once the time limit is reached
	ServerTask = 2 // Whether this is a server async task, meaning that we won't have API clients to retrieve
};

#define ASYNC_TASK_FLAG_BIT(Flag) static_cast<uint8>(Flag)

class FOnlineAsyncEpicTaskAccelByte;
/**
 * Base class for any async tasks created by the AccelByte OSS.
 * 
 * Here's a couple of guidelines to follow for creating new async tasks to cut down on bugs and to keep a consistent style:
 * 1. Always call the Super::Initialize or Super::Tick for either of those methods! This is especially important if the
 * task uses timeouts, as functionality for timeout update and timeout checking is included in those two super class methods.
 * 2. If creating a copy of an FUniqueNetId, always create it as a TSharedRef<FUniqueNetIdAccelByteUser>, as when it gets passed
 * back to interface methods, those methods may try and call AsShared on the unique ID, which will fail if it wasn't created
 * as a shared pointer.
 */
class FOnlineAsyncTaskAccelByte : public FOnlineAsyncTaskBasic<FOnlineSubsystemAccelByte>
{
public:
	
	/**
	 * Breaking const placement here as parent class has the InSubsystem defined as 'T* const'. Trying to define as 'const T*' gives error C2664.
	 * 
	 * Child classes that use this constructor will also need to use this convention.
	 * 
	 * @param InABSubsystem A pointer to AccelByte OnlineSubsystem instance
	 * @param bInShouldUseTimeout Whether any child of this task will by default use a timeout mechanism on Tick
	 */
	explicit FOnlineAsyncTaskAccelByte(FOnlineSubsystemAccelByte *const InABSubsystem
		, bool bInShouldUseTimeout = true)
		: FOnlineAsyncTaskAccelByte(InABSubsystem, INVALID_CONTROLLERID, (bInShouldUseTimeout) ? ASYNC_TASK_FLAG_BIT(EAccelByteAsyncTaskFlags::UseTimeout) : ASYNC_TASK_FLAG_BIT(EAccelByteAsyncTaskFlags::None))
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
	explicit FOnlineAsyncTaskAccelByte(FOnlineSubsystemAccelByte *const InABSubsystem
		, int32 InLocalUserNum
		, bool bInShouldUseTimeout = true)
		: FOnlineAsyncTaskAccelByte(InABSubsystem, InLocalUserNum, (bInShouldUseTimeout) ? ASYNC_TASK_FLAG_BIT(EAccelByteAsyncTaskFlags::UseTimeout) : ASYNC_TASK_FLAG_BIT(EAccelByteAsyncTaskFlags::None))
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
	explicit FOnlineAsyncTaskAccelByte(FOnlineSubsystemAccelByte *const InABSubsystem
		, int32 InLocalUserNum
		, uint8 InFlags)
		: FOnlineAsyncTaskBasic(InABSubsystem)
		, LocalUserNum(InLocalUserNum)
		, Flags(InFlags)
	{
		bShouldUseTimeout = HasFlag(EAccelByteAsyncTaskFlags::UseTimeout);

		// NOTE(Maxwell, 7/8/2021): Due to a bug where we cannot cancel requests on the SDK side, as well as cancel the delegates
		// that are supposed to run with these requests, if a timeout is quicker than a request could be received from the backend
		// we may get a crash from that. To combat this for now, we want to set our default timeout to always be one second higher
		// than the SDK HTTP timeout to give the SDK a chance to fire off its delegates for a timeout.
		// Fix this once https://accelbyte.atlassian.net/browse/OSS-193 is implemented.
		TaskTimeoutInSeconds = static_cast<double>(AccelByte::FHttpRetryScheduler::TotalTimeout) + 1.0;
	}

	/**
	 * Simple tick override to check if we are using timeouts, and if so check the task timeout and complete the task unsuccessfully if it's over its timeout
	 */
	virtual void Tick() override
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

	/* To be ticked by Epic, Delta in seconds */
	virtual void Tick(double Delta)
	{
		Tick();
		DeltaTickAccumulation += Delta;
	}

	/**
	 * Basic initialize override to check if we are using timeouts, and if so update the last task update time to the
	 * current time.
	 */
	virtual void Initialize() override
	{
		CurrentState = EAccelByteAsyncTaskState::Initializing;

		// We only care about setting the last update time if we are using a timeout
		if (bShouldUseTimeout)
		{
			SetLastUpdateTimeToCurrentTime();
		}

		// Do not attempt to get API clients for server async tasks, as servers do not have API client support.
		// We also don't need to get the corresponding user ID for the server, as server's don't have user IDs.
		if (!HasFlag(EAccelByteAsyncTaskFlags::ServerTask))
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
	}

	/**
	 * Method for checking in tick whether we should consider this task as timed out, will handle locking mechanisms
	 */
	virtual bool HasTaskTimedOut()
	{
		// As a child task, it has different mechanism to check whether timeout or not
		if (Epic != nullptr)
		{
			return DeltaTickAccumulation >= TaskTimeoutInSeconds;
		}

		FScopeLock ScopeLock(&TimeoutLock);
		const double CurrentTimeInSeconds = FPlatformTime::Seconds();
		return (CurrentTimeInSeconds - LastTaskUpdateInSeconds >= TaskTimeoutInSeconds);
	}

	/** Intended to be used by Epic against child Task */
	void ForcefullySetTimeoutState();

	virtual FString ToString() const override
	{
		const FString CompleteStateString = AsyncTaskCompleteStateToString(CompleteState);
		return FString::Printf(TEXT("%s (bWasSuccessful: %s; CompleteState: %s)"), *GetTaskName(), LOG_BOOL_FORMAT(bWasSuccessful), *CompleteStateString);
	}

	EAccelByteAsyncTaskState GetCurrentState() { return CurrentState; }
	
	int32 GetLocalUserNum() { return LocalUserNum; }

	/** To set the current task's parent. To determine is this a nested task or not. */
	virtual void SetParentTask(FOnlineAsyncTaskAccelByte* Task) { ParentTask = Task; }

	/** To get the current task's parent. Able to pass the parent'task to its child. */
	virtual FOnlineAsyncTaskAccelByte* GetParentTask() { return ParentTask; }

	/** Determine whether the current task is a nested or not. */
	virtual bool HasParent() { return ParentTask != nullptr; }

	/** If this task is a part of Epic, we need to set this information */
	virtual void SetEpicForThisTask(FOnlineAsyncEpicTaskAccelByte* AssignedEpic)
	{
		Epic = AssignedEpic;
	}

protected:

	/** Adding this type definition here to easily signify when we want to call a super method, like other UE4 constructs */
	using Super = FOnlineAsyncTaskAccelByte;

	/** Enum representing the current state of a task as a whole */
	EAccelByteAsyncTaskState CurrentState = EAccelByteAsyncTaskState::Uninitialized;

	/** Enum representing the state that a task has finished in */
	EAccelByteAsyncTaskCompleteState CompleteState = EAccelByteAsyncTaskCompleteState::Incomplete;

	/** Whether this task requires a timeout to be used, will be set up through the constructor for the task */
	bool bShouldUseTimeout = false;

	/** Time in seconds since the last time an async portion of a task has updated its timeout */
	double LastTaskUpdateInSeconds = FPlatformTime::Seconds();

	/** Time in seconds that we should timeout this request, set to 30 seconds by default */
	double TaskTimeoutInSeconds = 30.0;

	/** Time that we will use to deteremine whether should we timeout this request. Unit in Seconds */
	double DeltaTickAccumulation = 0.0;

	/** Critical section for locking the last update time in seconds */
	FCriticalSection TimeoutLock;

	/**
	 * Index of the user that we want to perform actions with, can be blank in favor of a user ID. Will be set to
	 * INVALID_CONTROLLERID unless a task uses a user index.
	 */
	int32 LocalUserNum = INVALID_CONTROLLERID;

	/*** ID of the user that we want to perform actions with, can also be nullptr in favor of a user index. */
	FUniqueNetIdAccelByteUserPtr UserId = nullptr;

	/** API client that should be used for this task, use GetApiClient to get a valid instance */
	AccelByte::FApiClientPtr ApiClient;

	/** Flags associated with this async task */
	uint8 Flags = 0;

	/** The address of the parent task if the current task is a nested async call  */
	FOnlineAsyncTaskAccelByte* ParentTask = nullptr;

	/** Epic for this task */
	FOnlineAsyncEpicTaskAccelByte* Epic = nullptr;

	/**
	 * Basic method to get the current name of the task, used for ToString on tasks as well as trace logs.
	 *
	 * Should be overridden by any async task that extends from this base class.
	 */
	virtual const FString GetTaskName() const
	{
		return TEXT("");
	}

	/**
	 * Method used to complete an async task and set its completion state. This should be used instead of directly setting
	 * the bWasSuccessful and bIsComplete flags, as it has additional checks to make sure that these won't change once
	 * we have already marked the task as completed, as well as provides context as to what may have occurred if the task
	 * was unsuccessful.
	 */
	void CompleteTask(const EAccelByteAsyncTaskCompleteState& InCompleteState)
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

	/**
	 * Method for updating a timeout value with the current time in seconds, handles locking mechanisms.
	 *
	 * This should be called for any task that utilizes a timeout either when getting a response back from an async request
	 * or after kicking off async requests (ex. at the end of your Initialize method).
	 */
	virtual void SetLastUpdateTimeToCurrentTime()
	{
		FScopeLock ScopeLock(&TimeoutLock);
		LastTaskUpdateInSeconds = FPlatformTime::Seconds();
	}

	/**
	 * Method called when this async task has timed out. Use to add custom timeout functionality.
	 */
	virtual void OnTaskTimedOut() {}

	/**
	 * Gets an API client instance for a user specified by either index or ID.
	 */
	virtual AccelByte::FApiClientPtr GetApiClient(int32 InLocalUserNum)
	{
		if (ApiClient.IsValid())
		{
			return ApiClient;
		}

		if (Subsystem == nullptr)
		{
			return nullptr;
		}

		const TSharedPtr<FOnlineIdentityAccelByte, ESPMode::ThreadSafe> IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(Subsystem->GetIdentityInterface());
		if (!IdentityInterface.IsValid())
		{
			return nullptr;
		}

		ApiClient = IdentityInterface->GetApiClient(InLocalUserNum);
		return ApiClient;
	}

	/**
	 * Gets an API client instance for a user specified by either index or ID.
	 */
	virtual AccelByte::FApiClientPtr GetApiClient(FUniqueNetIdAccelByteUserRef const& InUserId)
	{
		if (ApiClient.IsValid())
		{
			return ApiClient;
		}

		const TSharedPtr<FOnlineIdentityAccelByte, ESPMode::ThreadSafe> IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(Subsystem->GetIdentityInterface());
		if (!IdentityInterface.IsValid())
		{
			return nullptr;
		}

		ApiClient = IdentityInterface->GetApiClient(InUserId.Get());
		return ApiClient;
	}

	/**
	 * Get corresponding local user num or user ID for user that is performing this task
	 */
	void GetOtherUserIdentifiers()
	{
		const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(Subsystem->GetIdentityInterface());
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
#ifdef ONLINESUBSYSTEMACCELBYTE_PACKAGE
				UserId = FUniqueNetIdAccelByteUser::CastChecked(PlayerId.ToSharedRef());
#else
				UserId = StaticCastSharedPtr<const FUniqueNetIdAccelByteUser>(PlayerId);
#endif
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

	/**
	 * Handler for when the async task has officially kicked off work (i.e. when we have moved off the game thread)
	 */
	virtual void OnTaskStartWorking() {}

	/**
	 * Whether or not this task has the flag specified.
	 */
	bool HasFlag(const EAccelByteAsyncTaskFlags& Flag) const
	{
		uint8 FlagBit = static_cast<uint8>(Flag);
		return (Flags & FlagBit) == FlagBit;
	}

	/** 
	* To prevent deadlock, please call use this function from Initialize():
	*     IF there's a need to call an OSS interface function
	*     IF there's a need to CreateAndDispatch....Task<>
	*/
	void ExecuteCriticalSectionAction(FVoidHandler Action);

};
