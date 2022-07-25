// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "OnlineAsyncTaskManager.h"
#include "OnlineSubsystemAccelByteModule.h"
#include "OnlineSubsystemAccelByteInternalHelpers.h"
#include <Core/AccelByteMultiRegistry.h>
#include <OnlineSubsystemAccelByteTypes.h>
#include <OnlineIdentityInterfaceAccelByte.h>
#include <OnlineSubsystemAccelByte.h>

#define AB_OSS_ASYNC_TASK_TRACE_BEGIN_VERBOSITY(Verbosity, Format, ...) UE_LOG_AB(Verbosity, TEXT(">>> %s::%s (AsyncTask method) was called. Args: ") Format, *GetTaskName(), *FString(__func__), ##__VA_ARGS__)
#define AB_OSS_ASYNC_TASK_TRACE_BEGIN(Format, ...) AB_OSS_ASYNC_TASK_TRACE_BEGIN_VERBOSITY(Verbose, Format, ##__VA_ARGS__)
#define AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Verbosity, Format, ...) UE_LOG_AB(Verbosity, TEXT("<<< %s::%s (AsyncTask method) has finished execution. ") Format, *GetTaskName(), *FString(__func__), ##__VA_ARGS__)
#define AB_OSS_ASYNC_TASK_TRACE_END(Format, ...) AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Verbose, Format, ##__VA_ARGS__)

/**
 * Declares two delegates for use in an SDK call wrapped in an async task. Success delegate will have the name
 * On{Verb}SuccessDelegate, and be bound to the On{Verb}Success method of the class. Error delegate will have
 * the name On{Verb}ErrorDelegate, and be bound to the On{Verb}Error method of the class.
 * 
 * @param AsyncTaskClass Name of the class that we are binding delegate methods to
 * @param Verb Name of the action that is being handled by the two delegates, effects the name of the final delegates
 * @param SuccessType Delegate type for the success delegate
 */
#define AB_ASYNC_TASK_DECLARE_SDK_DELEGATES(AsyncTaskClass, Verb, SuccessType) \
	const SuccessType On##Verb##SuccessDelegate = SuccessType::CreateRaw(this, &AsyncTaskClass::On##Verb##Success); \
	const FErrorHandler On##Verb##ErrorDelegate = FErrorHandler::CreateRaw(this, &AsyncTaskClass::On##Verb##Error);

/**
 * Enum to describe what state an async task is in currently
 */
enum class EAccelByteAsyncTaskState : uint8
{
	Initializing = 0,
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
	UseTimeout = 1, // Whether to track the task with a timeout that will automaticlaly end once the time limit is reached
	ServerTask = 2 // Whether this is a server async task, meaning that we won't have API clients to retrieve
};

#define ASYNC_TASK_FLAG_BIT(Flag) static_cast<uint8>(Flag)

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
	 * @param InBShouldUseTimeout Whether any child of this task will by default use a timeout mechanism on Tick
	 */
	explicit FOnlineAsyncTaskAccelByte(FOnlineSubsystemAccelByte* const InABSubsystem, bool InBShouldUseTimeout=false)
		: FOnlineAsyncTaskAccelByte(InABSubsystem, static_cast<uint8>(EAccelByteAsyncTaskFlags::UseTimeout))
	{
	}

	/**
	 * Breaking const placement here as parent class has the InSubsystem defined as 'T* const'. Trying to define as 'const T*' gives error C2664.
	 *
	 * Child classes that use this constructor will also need to use this convention.
	 *
	 * @param InBShouldUseTimeout Whether any child of this task will by default use a timeout mechanism on Tick
	 */
	explicit FOnlineAsyncTaskAccelByte(FOnlineSubsystemAccelByte* const InABSubsystem, uint8 InFlags)
		: FOnlineAsyncTaskBasic(InABSubsystem)
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
		if (bShouldUseTimeout && HasTaskTimedOut())
		{
			CompleteTask(EAccelByteAsyncTaskCompleteState::TimedOut);
			OnTaskTimedOut();
		}

		// If we are not currently in the working state, then kick off the work we need to do for the task
		if (CurrentState != EAccelByteAsyncTaskState::Working)
		{
			CurrentState = EAccelByteAsyncTaskState::Working;
			OnTaskStartWorking();
		}
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

	virtual FString ToString() const override
	{
		const FString CompleteStateString = AsyncTaskCompleteStateToString(CompleteState);
		return FString::Printf(TEXT("%s (bWasSuccessful: %s; CompleteState: %s)"), *GetTaskName(), LOG_BOOL_FORMAT(bWasSuccessful), *CompleteStateString);
	}

protected:

	/** Adding this type definition here to easily signify when we want to call a super method, like other UE4 constructs */
	using Super = FOnlineAsyncTaskAccelByte;

	/** Enum representing the current state of a task as a whole */
	EAccelByteAsyncTaskState CurrentState = EAccelByteAsyncTaskState::Initializing;

	/** Enum representing the state that a task has finished in */
	EAccelByteAsyncTaskCompleteState CompleteState = EAccelByteAsyncTaskCompleteState::Incomplete;

	/** Whether this task requires a timeout to be used, will be set up through the constructor for the task */
	bool bShouldUseTimeout = false;

	/** Time in seconds since the last time an async portion of a task has updated its timeout */
	double LastTaskUpdateInSeconds = 0.0;

	/** Time in seconds that we should timeout this request, set to 30 seconds by default */
	double TaskTimeoutInSeconds = 30.0;

	/** Critical section for locking the last update time in seconds */
	FCriticalSection TimeoutLock;

	/**
	 * Index of the user that we want to perform actions with, can be blank in favor of a user ID. Will be set to
	 * INVALID_CONTROLLERID unless a task uses a user index.
	 */
	int32 LocalUserNum = INVALID_CONTROLLERID;

	/*** ID of the user that we want to perform actions with, can also be nullptr in favor of a user index. */
	TSharedPtr<const FUniqueNetIdAccelByteUser> UserId = nullptr;

	/** API client that should be used for this task, use GetApiClient to get a valid instance */
	AccelByte::FApiClientPtr ApiClient;

	/** Flags associated with this async task */
	uint8 Flags = 0;

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
	 * Method for checking in tick whether we should consider this task as timed out, will handle locking mechanisms
	 */
	virtual bool HasTaskTimedOut()
	{
		FScopeLock ScopeLock(&TimeoutLock);
		const double CurrentTimeInSeconds = FPlatformTime::Seconds();
		return (CurrentTimeInSeconds - LastTaskUpdateInSeconds >= TaskTimeoutInSeconds);
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
	virtual AccelByte::FApiClientPtr GetApiClient(const TSharedRef<const FUniqueNetIdAccelByteUser>& InUserId)
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
		const TSharedPtr<FOnlineIdentityAccelByte, ESPMode::ThreadSafe> IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(Subsystem->GetIdentityInterface());
		if (!IdentityInterface.IsValid())
		{
			return;
		}

		// If we have a local user num, then we want to get a user ID from that user num
		if (LocalUserNum != INVALID_CONTROLLERID)
		{
			TSharedPtr<const FUniqueNetId> PlayerId = IdentityInterface->GetUniquePlayerId(LocalUserNum);
			if (PlayerId.IsValid())
			{
				UserId = StaticCastSharedPtr<const FUniqueNetIdAccelByteUser>(PlayerId);
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

};
