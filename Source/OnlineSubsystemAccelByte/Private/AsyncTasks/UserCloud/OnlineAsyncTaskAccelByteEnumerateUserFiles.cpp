// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteEnumerateUserFiles.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineUserCloudInterfaceAccelByte.h"

#include "Api/AccelByteCloudStorageApi.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteEnumerateUserFiles::FOnlineAsyncTaskAccelByteEnumerateUserFiles(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InUserId)
	: FOnlineAsyncTaskAccelByte(InABInterface)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InUserId);
}

void FOnlineAsyncTaskAccelByteEnumerateUserFiles::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s"), *UserId->ToDebugString());

	THandler<TArray<FAccelByteModelsSlot>> OnGetAllSlotsSuccessDelegate = TDelegateUtils<THandler<TArray<FAccelByteModelsSlot>>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteEnumerateUserFiles::OnGetAllSlotsSuccess);
	FErrorHandler OnGetAllSlotsErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteEnumerateUserFiles::OnGetAllSlotsError);
	API_FULL_CHECK_GUARD(CloudStorage);
	CloudStorage->GetAllSlots(OnGetAllSlotsSuccessDelegate, OnGetAllSlotsErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteEnumerateUserFiles::Finalize()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s; Header Amount: %d"), LOG_BOOL_FORMAT(bWasSuccessful), FileNameToFileHeaderMap.Num());

	if (bWasSuccessful)
	{
		// Pass the new cloud headers that were created to the UserCloud interface
		TSharedPtr<FOnlineUserCloudAccelByte, ESPMode::ThreadSafe> UserCloudInterface = StaticCastSharedPtr<FOnlineUserCloudAccelByte>(SubsystemPin->GetUserCloudInterface());
		if (UserCloudInterface.IsValid())
		{
			UserCloudInterface->AddCloudHeaders(UserId.ToSharedRef(), FileNameToFileHeaderMap);
		}
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteEnumerateUserFiles::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	TSharedPtr<FOnlineUserCloudAccelByte, ESPMode::ThreadSafe> UserCloudInterface = StaticCastSharedPtr<FOnlineUserCloudAccelByte>(SubsystemPin->GetUserCloudInterface());
	if (UserCloudInterface.IsValid())
	{
		UserCloudInterface->TriggerOnEnumerateUserFilesCompleteDelegates(bWasSuccessful, UserId.ToSharedRef().Get());
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteEnumerateUserFiles::OnGetAllSlotsSuccess(const TArray<FAccelByteModelsSlot>& Results)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Slot amount: %d"), Results.Num());

	for (const FAccelByteModelsSlot& Slot : Results)
	{
		FCloudFileHeader Header;
		Header.FileName = Slot.StoredName;
		Header.FileSize = Slot.Size;
		Header.DLName = Slot.OriginalName;

		FileNameToFileHeaderMap.Add(Header.FileName, Header);
	}

	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteEnumerateUserFiles::OnGetAllSlotsError(int32 ErrorCode, const FString& ErrorMessage)
{
	UE_LOG_AB(Error, TEXT("Failed to get all user file slots from cloud storage! Error code: %d; Error message: %s"), ErrorCode, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}