// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteWriteUserFile.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineUserCloudInterfaceAccelByte.h"
#include "Core/AccelByteRegistry.h"
#include "Api/AccelByteCloudStorageApi.h"

FOnlineAsyncTaskAccelByteWriteUserFile::FOnlineAsyncTaskAccelByteWriteUserFile(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InUserId, const FString& InFileName, const TArray<uint8>& InFileContents, bool InBCompressBeforeUpload)
	: FOnlineAsyncTaskAccelByte(InABInterface, true)
	, FileName(InFileName)
	, FileContents(InFileContents)
	, bCompressBeforeUpload(InBCompressBeforeUpload)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InUserId);
}

void FOnlineAsyncTaskAccelByteWriteUserFile::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s; FileName: %s; FileContent Size: %d"), *UserId->ToDebugString(), *FileName, FileContents.Num());

	// Check the UserCloud cache for a SlotId that corresponds to the file name
	const TSharedPtr<FOnlineUserCloudAccelByte, ESPMode::ThreadSafe> UserCloudInterface = StaticCastSharedPtr<FOnlineUserCloudAccelByte>(Subsystem->GetUserCloudInterface());
	const FString SlotId = UserCloudInterface->GetSlotIdFromCache(UserId.ToSharedRef(), FileName);
	if (SlotId.IsEmpty())
	{
		THandler<TArray<FAccelByteModelsSlot>> OnGetAllSlotsSuccessDelegate = TDelegateUtils<THandler<TArray<FAccelByteModelsSlot>>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteWriteUserFile::OnGetAllSlotsSuccess);
		FErrorHandler OnGetAllSlotsErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteWriteUserFile::OnGetAllSlotsError);
		ApiClient->CloudStorage.GetAllSlots(OnGetAllSlotsSuccessDelegate, OnGetAllSlotsErrorDelegate);
	}
	else
	{
		RunWriteSlot(SlotId);
	}
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteWriteUserFile::Finalize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));
	
	if (bWasSuccessful)
	{
		const TSharedPtr<FOnlineUserCloudAccelByte, ESPMode::ThreadSafe> UserCloudInterface = StaticCastSharedPtr<FOnlineUserCloudAccelByte>(Subsystem->GetUserCloudInterface());
		if (UserCloudInterface.IsValid())
		{
			UserCloudInterface->AddSlotIdToCache(UserId.ToSharedRef(), FileName, ResolvedSlotId);
		}
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteWriteUserFile::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const IOnlineUserCloudPtr UserCloudInterface = Subsystem->GetUserCloudInterface();
	if (UserCloudInterface.IsValid())
	{
		UserCloudInterface->TriggerOnWriteUserFileCompleteDelegates(bWasSuccessful, UserId.ToSharedRef().Get(), FileName);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteWriteUserFile::RunWriteSlot(const FString& SlotId)
{
	THandler<FAccelByteModelsSlot> OnCreateOrUpdateSlotSuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsSlot>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteWriteUserFile::OnCreateOrUpdateSlotSuccess);
	FErrorHandler OnCreateOrUpdateSlotErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteWriteUserFile::OnCreateOrUpdateSlotError);
	FHttpRequestProgressDelegate OnCreateOrUpdateSlotProgressDelegate = TDelegateUtils<FHttpRequestProgressDelegate>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteWriteUserFile::OnCreateOrUpdateSlotProgress);

	// If we have an empty slot ID passed in, we will treat this as meaning that we need to create a new slot
	if (SlotId.IsEmpty())
	{
		ApiClient->CloudStorage.CreateSlot(FileContents, FileName, TArray<FString>(), FileName, TEXT(""), OnCreateOrUpdateSlotSuccessDelegate, OnCreateOrUpdateSlotProgressDelegate, OnCreateOrUpdateSlotErrorDelegate);
	}
	// Otherwise, we just want to update the existing slot
	else
	{
		ApiClient->CloudStorage.UpdateSlot(SlotId, FileContents, FileName, TArray<FString>(), FileName, TEXT(""), OnCreateOrUpdateSlotSuccessDelegate, OnCreateOrUpdateSlotProgressDelegate, OnCreateOrUpdateSlotErrorDelegate);
	}

	ResolvedSlotId = SlotId;
	bHasUploadStarted = true;
}

void FOnlineAsyncTaskAccelByteWriteUserFile::OnGetAllSlotsSuccess(const TArray<FAccelByteModelsSlot>& Results)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Results amount: %d"), Results.Num());

	// Check to see if we already have a slot with the corresponding label of FileName
	FString FoundSlotId;
	for (const FAccelByteModelsSlot& Slot : Results)
	{
		if (Slot.Label == FileName)
		{
			FoundSlotId = Slot.SlotId;
			break;
		}
	}

	RunWriteSlot(FoundSlotId);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteWriteUserFile::OnGetAllSlotsError(int32 ErrorCode, const FString& ErrorMessage)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s; FileName: %s"), *UserId->ToDebugString(), *FileName);

	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

	AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to write contents for file (%s) as we could not query the user's (%s) slots! Error code: %d; Error message: %s"), *FileName, *UserId->ToString(), ErrorCode, *ErrorMessage);
}

void FOnlineAsyncTaskAccelByteWriteUserFile::OnCreateOrUpdateSlotSuccess(const FAccelByteModelsSlot& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("SlotId: %s"), *Result.SlotId);
	
	// For now, this will just notify the task as done, I don't believe that we need to add a file header or contents to
	// caches, as those should be done explicitly through ReadUserFile and EnumerateUserFiles?
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteWriteUserFile::OnCreateOrUpdateSlotProgress(FHttpRequestPtr Request, int32 BytesSent, int32 BytesReceived)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s; FileName: %s; BytesSent: %d; BytesRecieved: %d"), *UserId->ToDebugString(), *FileName, BytesSent, BytesReceived);

	SetLastUpdateTimeToCurrentTime();

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteWriteUserFile::OnCreateOrUpdateSlotError(int32 ErrorCode, const FString& ErrorMessage)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s; FileName: %s"), *UserId->ToDebugString(), *FileName);

	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

	AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to write contents for file (%s) for user (%s) as the call to the backend failed! Error code: %d; Error message: %s"), *FileName, *UserId->ToDebugString(), ErrorCode, *ErrorMessage);
}
