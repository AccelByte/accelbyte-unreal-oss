// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "Containers/List.h"
#include "OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineAsyncTaskManager.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "Models/AccelByteUserModels.h"
#include "OnlineError.h"
#include "OnlineSubsystemTypes.h"

class FOnlineAsyncEpicTaskAccelByte : public FOnlineAsyncTaskAccelByte, public TSelfPtr<FOnlineAsyncEpicTaskAccelByte, ESPMode::ThreadSafe>
{
	uint32 EpicID = 0;
	TDoubleLinkedList<TArray<FOnlineAsyncTaskAccelByte*>> TaskContainer;

public:
	FOnlineAsyncEpicTaskAccelByte(FOnlineSubsystemAccelByte* const InABSubsystem, int32 InLocalUserNum, const FVoidHandler& InDelegate) 
		: FOnlineAsyncTaskAccelByte(InABSubsystem, false /*should not use timeout*/)
		, Delegate(InDelegate)
	{
		LocalUserNum = InLocalUserNum;
	}

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;
	virtual void Tick() override;

	void SetEpicID(uint32 ID) { EpicID = ID; }
	uint32 GetEpicID() { return EpicID; }

	/** Set the current Task/Epic as timeout along with all the task in the TaskContainer */
	void Timeout();
	
	uint32 GetCurrentStackCapacity() { return TaskContainer.Num(); }

	//TODO cyclic checking 
	//recursively check GetParentTask(...) , collect it, and detect it etc.
	void Enqueue(ETypeOfOnlineAsyncTask TaskType, FOnlineAsyncTaskAccelByte* ChildTask);

	/** For test assertion purpose */
	const TDoubleLinkedList<TArray<FOnlineAsyncTaskAccelByte*>>& GetTaskContainer() { return TaskContainer; }

protected:

	virtual const FString GetTaskName() const override
	{
		FString Name = FString::Printf(TEXT("FOnlineAsyncEpicTaskAccelByte_%d"), EpicID);
		return Name;
	}

private:
	bool bIsTopStackRunning();
	
	double GetWorldDelta();

	/** Delegate fired on Epic Task completion */
	FVoidHandler Delegate;

};