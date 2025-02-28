// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteReadUserFile.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineUserCloudInterfaceAccelByte.h"
#include "Api/AccelByteCloudStorageApi.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteReadUserFile::FOnlineAsyncTaskAccelByteReadUserFile(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InUserId, const FString& InFileName)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, FileName(InFileName)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InUserId);
}

void FOnlineAsyncTaskAccelByteReadUserFile::Initialize()
{
	TRY_PIN_SUBSYSTEM();

	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s; FileName: %s"), *UserId->ToDebugString(), *FileName);

	// Check the UserCloud cache for a SlotId that corresponds to the file name
	const TSharedPtr<FOnlineUserCloudAccelByte, ESPMode::ThreadSafe> UserCloudInterface = StaticCastSharedPtr<FOnlineUserCloudAccelByte>(SubsystemPin->GetUserCloudInterface());
	const FString SlotId = UserCloudInterface->GetSlotIdFromCache(UserId.ToSharedRef(), FileName);

	// If we do not have a corresponding cached slot ID, query all of the user's slots to see if a match is found
	if (SlotId.IsEmpty())
	{
		THandler<TArray<FAccelByteModelsSlot>> OnGetAllSlotsSuccessDelegate = TDelegateUtils<THandler<TArray<FAccelByteModelsSlot>>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteReadUserFile::OnGetAllSlotsSuccess);
		FErrorHandler OnGetAllSlotsErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteReadUserFile::OnGetAllSlotsError);
		API_FULL_CHECK_GUARD(CloudStorage);
		CloudStorage->GetAllSlots(OnGetAllSlotsSuccessDelegate, OnGetAllSlotsErrorDelegate);
	}
	// Otherwise, just get the slot contents from the cached ID
	else
	{
		RunGetSlot(SlotId);
	}
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteReadUserFile::Finalize()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful)
	{
		const TSharedPtr<FOnlineUserCloudAccelByte, ESPMode::ThreadSafe> UserCloudInterface = StaticCastSharedPtr<FOnlineUserCloudAccelByte>(SubsystemPin->GetUserCloudInterface());
		if (UserCloudInterface.IsValid())
		{
			UserCloudInterface->AddSlotIdToCache(UserId.ToSharedRef(), FileName, ResolvedSlotId);
			UserCloudInterface->AddFileContentsToReadCache(UserId.ToSharedRef(), FileName, MoveTemp(FileContents));
		}

		// maybe do file save routine here? need to figure out where to save files locally
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteReadUserFile::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const IOnlineUserCloudPtr UserCloudInterface = SubsystemPin->GetUserCloudInterface();
	if (UserCloudInterface.IsValid())
	{
		UserCloudInterface->TriggerOnReadUserFileCompleteDelegates(bWasSuccessful, UserId.ToSharedRef().Get(), FileName);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteReadUserFile::RunGetSlot(const FString& SlotId)
{
	THandler<TArray<uint8>> OnGetSlotSuccessDelegate = TDelegateUtils<THandler<TArray<uint8>>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteReadUserFile::OnGetSlotSuccess);
	FErrorHandler OnGetSlotErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteReadUserFile::OnGetSlotError);
	API_FULL_CHECK_GUARD(CloudStorage);
	CloudStorage->GetSlot(SlotId, OnGetSlotSuccessDelegate, OnGetSlotErrorDelegate);

	ResolvedSlotId = SlotId;
}

void FOnlineAsyncTaskAccelByteReadUserFile::OnGetAllSlotsSuccess(const TArray<FAccelByteModelsSlot>& Results)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Results amount: %d"), Results.Num());

	// Check each result to see if the slot's label matches the FileName that we passed to the task
	FString FoundSlotId;
	for (const FAccelByteModelsSlot& Slot : Results)
	{
		if (Slot.Label == FileName)
		{
			FoundSlotId = Slot.SlotId;
			break;
		}
	}

	// No match was found, error out
	if (FoundSlotId.IsEmpty())
	{
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get file (%s) from user's (%s) cloud storage slots!"), *FileName, *UserId->ToDebugString());
		return;
	}

	RunGetSlot(FoundSlotId);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteReadUserFile::OnGetAllSlotsError(int32 ErrorCode, const FString& ErrorMessage)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s; FileName: %s"), *UserId->ToDebugString(), *FileName);

	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

	AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get file data for '%s' as we could not query user's (%s) slots! Error code: %d; Error message: %s"), *FileName, *UserId->ToDebugString(), ErrorCode, *ErrorMessage);
}

void FOnlineAsyncTaskAccelByteReadUserFile::OnGetSlotSuccess(const TArray<uint8>& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s; FileName: %s; Result Size: %d"), *UserId->ToDebugString(), *FileName, Result.Num());

	FileContents = Result;
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Successfully retrieved data for file '%s' from backend!"), *FileName);
}

void FOnlineAsyncTaskAccelByteReadUserFile::OnGetSlotError(int32 ErrorCode, const FString& ErrorMessage)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s; FileName: %s"), *UserId->ToDebugString(), *FileName);

	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

	AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get file data for '%s'! Error code: %d; Error message: %s"), *FileName, ErrorCode, *ErrorMessage);
}