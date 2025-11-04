// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "AsyncTasks/OnlineAsyncEpicTaskAccelByte.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameEngine.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteHelpers.h"

double FOnlineAsyncEpicTaskAccelByte::GetWorldDelta()
{
	double Output = 0.066f;
	if (UGameEngine* GameEngine = CastChecked<UGameEngine>(GEngine))
	{
		auto World = GameEngine->GetGameWorld();
		Output = UGameplayStatics::GetWorldDeltaSeconds(World);
	}
	return Output;
}

void FOnlineAsyncEpicTaskAccelByte::Initialize()
{
	FOnlineAsyncTaskAccelByte::Initialize();
}

void FOnlineAsyncEpicTaskAccelByte::Finalize()
{

}
void FOnlineAsyncEpicTaskAccelByte::TriggerDelegates()
{
	Delegate.ExecuteIfBound();
}

void FOnlineAsyncEpicTaskAccelByte::Tick()
{
	TRY_PIN_SUBSYSTEM();

	// If we are not currently in the working state, then kick off the work we need to do for the task
	if (CurrentState != EAccelByteAsyncTaskState::Working)
	{
		CurrentState = EAccelByteAsyncTaskState::Working;
		FOnlineAsyncTaskAccelByte::OnTaskStartWorking();
	}

	if (TaskContainer.Num() == 0)
	{
		//nothing to do, set as complete
		this->CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
		return;
	}
	
	const double HardLimitSeconds = 0.15f;
	auto CurrentTime = FPlatformTime::Seconds();
	auto Delta = CurrentTime - FOnlineAsyncTaskAccelByte::LastTaskUpdateInSeconds;
#if !UE_BUILD_SHIPPING
	// To ignore large delta (breakpoints, alt-tab, etc)
	Delta = FMath::Min<double>(Delta, HardLimitSeconds);
#endif
	TDoubleLinkedList<TArray<FOnlineAsyncTaskAccelByte*>>::TDoubleLinkedListNode* Node = TaskContainer.GetHead();
	if (Node == nullptr)
	{
		//nothing to do with the task container because it is empty
		return;
	}

	//Cleanup empty queue, iteration stops if it finds a non-empty Array
	while (Node->GetValue().Num() == 0)
	{
		auto NextNode = Node->GetNextNode();
		TaskContainer.RemoveNode(Node);
		if (NextNode == nullptr)
		{
			return;
		}
		Node = NextNode;
	}

	TArray<FOnlineAsyncTaskAccelByte*>& CurrentQueue = Node->GetValue();
	TArray<FOnlineAsyncTaskAccelByte*> CopyOfQueue = {};
	bool bIsTimeout = false;

	for (int i = 0; i < CurrentQueue.Num(); i++)
	{
		FOnlineAsyncTaskAccelByte* CurrentTask = CurrentQueue[i];
		if (CurrentTask == nullptr)
		{
			continue;
		}

		if (CurrentTask->GetCurrentState() == EAccelByteAsyncTaskState::Uninitialized)
		{
			if (!SubsystemPin.IsValid())
			{
				continue;
			}
			//Send to AsyncTaskManager
			SubsystemPin->EnqueueTaskForInitialize(CurrentTask);
		}
		else if (!bIsTimeout)
		{
			CurrentTask->Tick(Delta);
			if (CurrentTask->IsDone())
			{
				if (!SubsystemPin.IsValid())
				{
					continue;
				}
				SubsystemPin->AddTaskToOutQueue(CurrentTask);
				continue;
			}
		}
		CopyOfQueue.Add(CurrentTask);
		if (CurrentTask->HasTaskTimedOut())
		{
			bIsTimeout = true;
			continue;
		}
	}

	if (CopyOfQueue.Num() == 0)
	{
		TaskContainer.RemoveNode(Node);
	}
	else
	{
		Node->GetValue() = CopyOfQueue;
	}

	LastTaskUpdateInSeconds = CurrentTime;

	if (bIsTimeout)
	{
		this->Timeout();
	}
}


void FOnlineAsyncEpicTaskAccelByte::Timeout()
{
	TRY_PIN_SUBSYSTEM();

	TDoubleLinkedList<TArray<FOnlineAsyncTaskAccelByte*>>::TDoubleLinkedListNode* Node = TaskContainer.GetHead();

	while (Node != nullptr)
	{
		TArray<FOnlineAsyncTaskAccelByte*>& CurrentQueue = Node->GetValue();

		for (int i = 0; i < CurrentQueue.Num(); i++)
		{
			FOnlineAsyncTaskAccelByte* CurrentTask = CurrentQueue[i];
			if (CurrentTask == nullptr)
			{
				continue;
			}
			CurrentTask->ForcefullySetTimeoutState();
			SubsystemPin->AddTaskToOutQueue(CurrentTask);
		}

		Node = Node->GetNextNode();
	}

	this->CompleteTask(EAccelByteAsyncTaskCompleteState::TimedOut);
	this->OnTaskTimedOut();
}

bool FOnlineAsyncEpicTaskAccelByte::bIsTopStackRunning()
{
	if (TaskContainer.Num() == 0 || TaskContainer.GetHead() == nullptr)
	{
		return false;
	}
	
	if (TaskContainer.GetHead() || TaskContainer.GetHead()->GetValue().Num() == 0)
	{
		return false;
	}

	auto AsyncTaskAB = TaskContainer.GetHead()->GetValue();
	if (AsyncTaskAB.Num() == 0)
	{
		return false;
	}

	auto TopStackState = (AsyncTaskAB[0])->GetCurrentState();
	return TopStackState != EAccelByteAsyncTaskState::Uninitialized;
}

void FOnlineAsyncEpicTaskAccelByte::Enqueue(ETypeOfOnlineAsyncTask TaskType, FOnlineAsyncTaskAccelByte* ChildTask)
{
	auto Stack = TArray<FOnlineAsyncTaskAccelByte*>();
	Stack.Add(ChildTask);

	if (bIsTopStackRunning() || TaskContainer.Num() == 0)
	{
		TaskContainer.AddHead(Stack);
		return;
	}

	switch (TaskType)
	{
	case ETypeOfOnlineAsyncTask::Parallel:
		TaskContainer.GetHead()->GetValue().Add(ChildTask);
		break;
	case ETypeOfOnlineAsyncTask::Serial:
	{
		auto NodePtr = TaskContainer.GetHead();
		while (NodePtr != nullptr)
		{
			if (NodePtr->GetValue().Num() == 0)
			{
				(void)NodePtr->GetNextNode();
				continue;
			}

			// If running, insert above the running task
			if (NodePtr->GetValue()[0]->GetCurrentState() != EAccelByteAsyncTaskState::Uninitialized)
			{
				TaskContainer.InsertNode(Stack, NodePtr);
				return;
			}
			NodePtr = NodePtr->GetNextNode();
		}
		TaskContainer.AddTail(Stack);
	}
		break;
	default:
		break;
	}
}
