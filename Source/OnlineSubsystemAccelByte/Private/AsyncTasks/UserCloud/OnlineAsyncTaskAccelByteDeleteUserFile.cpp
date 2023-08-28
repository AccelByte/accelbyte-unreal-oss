// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteDeleteUserFile.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineUserCloudInterfaceAccelByte.h"
#include "Core/AccelByteRegistry.h"
#include "Api/AccelByteCloudStorageApi.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteDeleteUserFile::FOnlineAsyncTaskAccelByteDeleteUserFile(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InUserId, const FString& InFileName, bool InBShouldCloudDelete, bool InBShouldLocallyDelete)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, FileName(InFileName)
	, bShouldCloudDelete(InBShouldCloudDelete)
	, bShouldLocallyDelete(InBShouldLocallyDelete)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InUserId);
}

void FOnlineAsyncTaskAccelByteDeleteUserFile::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s; FileName: %s; bShouldCloudDelete: %s; bShouldLocallyDelete: %s"), *UserId->ToDebugString(), *FileName, LOG_BOOL_FORMAT(bShouldCloudDelete), LOG_BOOL_FORMAT(bShouldLocallyDelete));

	if (bShouldCloudDelete)
	{
		// Check the UserCloud cache for a SlotId that corresponds to the file name
		const TSharedPtr<FOnlineUserCloudAccelByte, ESPMode::ThreadSafe> UserCloudInterface = StaticCastSharedPtr<FOnlineUserCloudAccelByte>(Subsystem->GetUserCloudInterface());
		const FString SlotId = UserCloudInterface->GetSlotIdFromCache(UserId.ToSharedRef(), FileName);

		// If we could not find a cached slot ID, then we need to query all of the users slots to find a match
		if (SlotId.IsEmpty())
		{
			THandler<TArray<FAccelByteModelsSlot>> OnGetAllSlotsSuccessDelegate = TDelegateUtils<THandler<TArray<FAccelByteModelsSlot>>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteDeleteUserFile::OnGetAllSlotsSuccess);
			FErrorHandler OnGetAllSlotsErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteDeleteUserFile::OnGetAllSlotsError);
			ApiClient->CloudStorage.GetAllSlots(OnGetAllSlotsSuccessDelegate, OnGetAllSlotsErrorDelegate);
		}
		else
		{
			RunDeleteSlot(SlotId);
		}
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteDeleteUserFile::Finalize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful)
	{
		const TSharedPtr<FOnlineUserCloudAccelByte, ESPMode::ThreadSafe> UserCloudInterface = StaticCastSharedPtr<FOnlineUserCloudAccelByte>(Subsystem->GetUserCloudInterface());
		if (UserCloudInterface.IsValid())
		{
			UserCloudInterface->RemoveSlotIdFromCache(UserId.ToSharedRef(), FileName);
		}
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteDeleteUserFile::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const IOnlineUserCloudPtr UserCloudInterface = Subsystem->GetUserCloudInterface();
	if (UserCloudInterface.IsValid())
	{
		UserCloudInterface->TriggerOnDeleteUserFileCompleteDelegates(bWasSuccessful, UserId.ToSharedRef().Get(), FileName);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteDeleteUserFile::RunDeleteSlot(const FString& SlotId)
{
	FVoidHandler OnDeleteSlotSuccessDelegate = TDelegateUtils<FVoidHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteDeleteUserFile::OnDeleteSlotSuccess);
	FErrorHandler OnDeleteSlotErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteDeleteUserFile::OnDeleteSlotError);
	ApiClient->CloudStorage.DeleteSlot(SlotId, OnDeleteSlotSuccessDelegate, OnDeleteSlotErrorDelegate);
}

void FOnlineAsyncTaskAccelByteDeleteUserFile::OnGetAllSlotsSuccess(const TArray<FAccelByteModelsSlot>& Results)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Results amount: %d"), Results.Num());

	FString FoundSlotId;
	for (const FAccelByteModelsSlot& Slot : Results)
	{
		if (Slot.Label == FileName)
		{
			FoundSlotId = Slot.SlotId;
			break;
		}
	}

	if (FoundSlotId.IsEmpty())
	{
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Failed to delete cloud file! Could not find file (%s) for user (%s)!"), *FileName, *UserId->ToDebugString());
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		return;
	}

	RunDeleteSlot(FoundSlotId);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteDeleteUserFile::OnGetAllSlotsError(int32 ErrorCode, const FString& ErrorMessage)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s; FileName: %s"), *UserId->ToDebugString(), *FileName);

	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

	AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to delete cloud file for '%s' as we could not query user's (%s) slots! Error code: %d; Error message: %s"), *FileName, *UserId->ToDebugString(), ErrorCode, *ErrorMessage);
}

void FOnlineAsyncTaskAccelByteDeleteUserFile::OnDeleteSlotSuccess()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s; FileName: %s"), *UserId->ToDebugString(), *FileName);

	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Successfully deleted file (%s) for user (%s) from CloudStorage!"), *FileName, *UserId->ToDebugString());
}

void FOnlineAsyncTaskAccelByteDeleteUserFile::OnDeleteSlotError(int32 ErrorCode, const FString& ErrorMessage)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s; FileName: %s"), *UserId->ToDebugString(), *FileName);

	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

	AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to delete cloud file for '%s' for user '%s' as the delete call failed on the backend! Error code: %d; Error message: %s"), *FileName, *UserId->ToDebugString(), ErrorCode, *ErrorMessage);
}
