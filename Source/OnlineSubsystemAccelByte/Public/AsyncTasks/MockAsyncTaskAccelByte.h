// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "AsyncTasks/OnlineAsyncEpicTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineUserInterfaceAccelByte.h"
#include "Models/AccelByteUserModels.h"
#include "OnlineUserCacheAccelByte.h"
#include "OnlineError.h"
#include "Core/AccelByteError.h"

class FMockAsyncTaskAccelByte;

#define FMockAsyncTaskDone AccelByte::THandler<FOnlineError>

/** Parameter used to create a Mock async task and 
  * Simplify the process to bind delegate
  * Simplify the way to retrieve the task pointer and epic pointer
 **/
struct MockAsyncTaskParameter
{
	// Called when the task complete
	AccelByte::THandler<FOnlineError> TaskCompleteDelegate;
	
	// Action will be done on Initialize(), to create child task that will be done through critical section
	AccelByte::FVoidHandler CreateChildDelegate;
	
	// Pointer to this task, will be set by the task itself
	FMockAsyncTaskAccelByte* TaskPtr = nullptr;
	
	// Pointer to this task's Epic
	FOnlineAsyncEpicTaskAccelByte* EpicPtr = nullptr;

	// Times taken to finish/set task complete
	double CompletionTime = 0.0f;

	// Time limit to consider this task time out
	double TimeoutLimitSeconds = 0.0f;

	// Task/Node name
	FString TaskName = {};

	/// The parent task will rely to this to compare it with ChildReportComplete count
	uint32 ChildCount = 0;
};

class ONLINESUBSYSTEMACCELBYTE_API FMockAsyncTaskAccelByte
	: public  FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FMockAsyncTaskAccelByte, ESPMode::ThreadSafe>
{
public:
	/// <summary>
	/// Constructor of this MockAsyncTask
	/// </summary>
	/// <param name="InABSubsystem"></param>
	/// <param name="InCompletionTime"></param>
	/// <param name="bInShouldUseTimeout"></param>
	FMockAsyncTaskAccelByte(
		FOnlineSubsystemAccelByte* const InABSubsystem,
		MockAsyncTaskParameter& InParameter,
		bool bInShouldUseTimeout = false);

	virtual void TriggerDelegates() override;

	virtual void Tick() override;
	
	virtual void Initialize() override;

	// Used to ensure the child task really complete before we can consider the current task completion
	void ChildReportComplete() { ChildCompleteReportedCount.Increment(); }

	FOnlineAsyncEpicTaskAccelByte* GetEpic() { return this->Epic; }

	FString& GetTaskNamePublic() { return Parameter.TaskName; }

protected:

	virtual const FString GetTaskName() const override
	{
		return Parameter.TaskName;
	}

	MockAsyncTaskParameter& Parameter;

	FString ErrorCode;
	FString ErrorMessage;

	FThreadSafeCounter ChildCompleteReportedCount;
};
