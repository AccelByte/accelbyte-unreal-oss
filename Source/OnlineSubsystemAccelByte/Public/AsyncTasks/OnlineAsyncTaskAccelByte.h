// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "OnlineAsyncTaskManager.h"
#include "OnlineSubsystemAccelByteModule.h"
#include "OnlineSubsystemAccelByteInternalHelpers.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineError.h"
#include "Engine/LocalPlayer.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "OnlineSubsystemAccelByte.h"

#include "Core/AccelByteError.h"

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
	ServerTask = 2, // Whether this is a server async task, meaning that we won't have API clients to retrieve
	AllFlags
};

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
class ONLINESUBSYSTEMACCELBYTE_API FOnlineAsyncTaskAccelByte : public FOnlineAsyncTaskBasic<FOnlineSubsystemAccelByte>
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
	explicit FOnlineAsyncTaskAccelByte(FOnlineSubsystemAccelByte* const InABSubsystem
		, bool bInShouldUseTimeout = true);
	
	/**
	 * Breaking const placement here as parent class has the InSubsystem defined as 'T* const'. Trying to define as 'const T*' gives error C2664.
	 * 
	 * Child classes that use this constructor will also need to use this convention.
	 *
	 * @param InABSubsystem A pointer to AccelByte OnlineSubsystem instance
	 * @param InLocalUserNum Local User Index
	 * @param bInShouldUseTimeout Whether any child of this task will by default use a timeout mechanism on Tick
	 */
	explicit FOnlineAsyncTaskAccelByte(FOnlineSubsystemAccelByte* const InABSubsystem
		, int32 InLocalUserNum
		, bool bInShouldUseTimeout = true);

	/**
	 * Breaking const placement here as parent class has the InSubsystem defined as 'T* const'. Trying to define as 'const T*' gives error C2664.
	 * 
	 * Child classes that use this constructor will also need to use this convention.
	 *
	 * @param InABSubsystem A pointer to AccelByte OnlineSubsystem instance
	 * @param InLocalUserNum Local User Index
	 * @param InFlags Flags whether any child of this task will by default use a timeout mechanism on Tick
	 */
	explicit FOnlineAsyncTaskAccelByte(FOnlineSubsystemAccelByte* const InABSubsystem
		, int32 InLocalUserNum
		, int32 InFlags);

	/**
	 * Breaking const placement here as parent class has the InSubsystem defined as 'T* const'. Trying to define as 'const T*' gives error C2664.
	 *
	 * Child classes that use this constructor will also need to use this convention.
	 *
	 * @param InABSubsystem A pointer to AccelByte OnlineSubsystem instance
	 * @param InLocalUserNum Local User Index
	 * @param InFlags Flags whether any child of this task will by default use a timeout mechanism on Tick
	 * @param InLockKey Key lock to hold while this async task is alive
	 */
	explicit FOnlineAsyncTaskAccelByte(FOnlineSubsystemAccelByte* const InABSubsystem
		, int32 InLocalUserNum
		, int32 InFlags
		, TSharedPtr<FAccelByteKey> InLockKey);

	/**
	 * Simple tick override to check if we are using timeouts, and if so check the task timeout and complete the task unsuccessfully if it's over its timeout
	 */
	virtual void Tick() override;

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
	virtual void Initialize() override;

	/**
	 * Method for checking in tick whether we should consider this task as timed out, will handle locking mechanisms
	 */
	virtual bool HasTaskTimedOut()
	{
		if (!bShouldUseTimeout)
		{
			return false;
		}

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

	bool SetLocalUserNum(int32 InLocalUserNum)
	{
		if (LocalUserNum >= INVALID_CONTROLLERID
			&& LocalUserNum < MAX_LOCAL_PLAYERS)
		{
			LocalUserNum = InLocalUserNum;
			return true;
		}
		return false;
	}

protected:

	/** Adding this type definition here to easily signify when we want to call a super method, like other UE4 constructs */
	using Super = FOnlineAsyncTaskAccelByte;

	/** Need to use this instead of using parent's member FOnlineAsyncTaskBasic::Subsystem T* raw pointer */
	FOnlineSubsystemAccelByteWPtr AccelByteSubsystem;

	/** Enum representing the current state of a task as a whole */
	EAccelByteAsyncTaskState CurrentState = EAccelByteAsyncTaskState::Uninitialized;

	/** Enum representing the state that a task has finished in */
	EAccelByteAsyncTaskCompleteState CompleteState = EAccelByteAsyncTaskCompleteState::Incomplete;

	/** Enum representing the online error result of a task */
	EOnlineErrorResult TaskOnlineError = EOnlineErrorResult::Unknown;

	/** String representing the error code that occurred */
	FString TaskErrorCode{};

	/** String representing the error message that occurred */
	FString TaskErrorStr{};

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

	/** Flags associated with this async task */
	int32 Flags = 0;

	TBitArray<FDefaultBitArrayAllocator> BitFlags;

	/** The address of the parent task if the current task is a nested async call  */
	FOnlineAsyncTaskAccelByte* ParentTask = nullptr;

	/** Epic for this task */
	FOnlineAsyncEpicTaskAccelByte* Epic = nullptr;

	/** lock key to keep alive while async task is active */
	TSharedPtr<FAccelByteKey> LockKey;

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
	void CompleteTask(const EAccelByteAsyncTaskCompleteState& InCompleteState);

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
		
	template<typename T>
	void RaiseGenericError(T Args)
	{}

	void RaiseGenericError();

	template<>
	void RaiseGenericError<FString&>(FString& InErrorStrMember)
	{
		InErrorStrMember = TEXT("request-to-obtain-valid-apiclient");
		RaiseGenericError();
	}

	template<>
	void RaiseGenericError<FOnlineError&>(FOnlineError& InOnlineError)
	{
		InOnlineError.CreateError("AccelByteApiClientError", EOnlineErrorResult::RequestFailure);
		RaiseGenericError();
	}
	
	template<>
	void RaiseGenericError<FOnlineErrorAccelByte&>(FOnlineErrorAccelByte& InOnlineError)
	{
		InOnlineError.CreateError("AccelByteApiClientError", static_cast<int32>(AccelByte::ErrorCodes::InvalidRequest), EOnlineErrorResult::RequestFailure);
		RaiseGenericError();
	}

	template<typename T>
	void RaiseGenericServerError(T Args)
	{}

	void RaiseGenericServerError();

	template<>
	void RaiseGenericServerError<FString&>(FString& InErrorStrMember)
	{
		InErrorStrMember = TEXT("request-to-obtain-valid-serverapiclient");
		RaiseGenericError();
	}

	template<>
	void RaiseGenericServerError<FOnlineError&>(FOnlineError& InOnlineError)
	{
		InOnlineError.CreateError("AccelByteServerApiClientError", EOnlineErrorResult::RequestFailure);
		RaiseGenericError();
	}
	
	template<>
	void RaiseGenericServerError<FOnlineErrorAccelByte&>(FOnlineErrorAccelByte& InOnlineError)
	{
		InOnlineError.CreateError("AccelByteServerApiClientError", static_cast<int32>(AccelByte::ErrorCodes::InvalidRequest), EOnlineErrorResult::RequestFailure);
		RaiseGenericError();
	}

	/**
	 * Method called when this async task has timed out. Use to add custom timeout functionality.
	 */
	virtual void OnTaskTimedOut() { };

	/**
	 * Gets an API client instance for a user specified by either index or ID.
	 */
	virtual AccelByte::FApiClientPtr GetApiClient(int32 InLocalUserNum);

	/**
	 * Gets an API client instance for a user specified by either index or ID.
	 */
	virtual AccelByte::FApiClientPtr GetApiClient(FUniqueNetIdAccelByteUserRef const& InUserId);

	virtual FAccelByteInstanceWPtr GetAccelByteInstance();

	/**
	 * SHOULD NOTE BE ACCESSED MANUALLY
	 * Gets current API client member, please use the macro API_CLIENT_CHECK_GUARD(); 
	 * to include checker and create local apiClient
	 */
	AccelByte::FApiClientPtr GetApiClientInternal()
	{
		return ApiClientInternal.Pin();
	}

	/**
	 * Sets current API client member (expected to be used by Login async tasks)
	 */
	void SetApiClient(AccelByte::FApiClientPtr Input)
	{
		ApiClientInternal = Input;
	}

	/**
	 * Check the validity of the ApiClient member 
	 */
	bool IsApiClientValid()
	{
		AccelByte::FApiClientPtr ApiClient = GetApiClientInternal();
		return ApiClient.IsValid();
	}

	/**
	 * Get corresponding local user num or user ID for user that is performing this task
	 */
	void GetOtherUserIdentifiers();

	/**
	 * Handler for when the async task has officially kicked off work (i.e. when we have moved off the game thread)
	 */
	virtual void OnTaskStartWorking() {}

	/**
	 * Whether or not this task has the flag specified.
	 */
	bool HasFlag(const EAccelByteAsyncTaskFlags& Flag) const
	{
		return HasFlag(static_cast<uint8>(Flag));
	}

	bool HasFlag(uint8 FlagBit) const
	{
		int32 SelectedFlag = 1 << FlagBit;
		return (Flags & SelectedFlag) == SelectedFlag;
	}

	/** 
	* To prevent deadlock, please call use this function from Initialize():
	*     IF there's a need to call an OSS interface function
	*     IF there's a need to CreateAndDispatch....Task<>
	*/
	void ExecuteCriticalSectionAction(FVoidHandler Action);

private:
	/** API client that should be used for this task, use API_CLIENT_CHECK_GUARD() to get a valid instance */
	AccelByte::FApiClientWPtr ApiClientInternal;

	/** Forcefully redefintion of Subsystem to prevent the child task accessing parent's public FOnlineAsyncTaskBasic::Subsystem T* raw pointer */
	FOnlineSubsystemAccelByte* Subsystem;
};
