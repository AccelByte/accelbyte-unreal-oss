// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "OnlineAsyncTaskAccelByte.h"
#include "OnlineSubsystemAccelByteTypes.h"

/**
 * Async task to write a file to a slot using the CloudStorage API.
 */
class FOnlineAsyncTaskAccelByteWriteUserFile : public FOnlineAsyncTaskAccelByte
{
public:

	FOnlineAsyncTaskAccelByteWriteUserFile(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InUserId, const FString& InFileName, const TArray<uint8>& InFileContents, bool InBCompressBeforeUpload);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteWriteUserFile");
	}

private:

	/** Name of the file that we wish to write to the cloud storage slot */
	FString FileName;

	/** Array of bytes corresponding to the data that we wish to write to cloud storage */
	TArray<uint8> FileContents;

	/** Whether we should compress the file before uploading it to the backend */
	bool bCompressBeforeUpload;

	/** Whether we have initiated the create or update call */
	FThreadSafeBool bHasUploadStarted = false;

	/** Slot ID that the write operation was ultimately performed on */
	FString ResolvedSlotId;

	/**
	 * Makes the API call to create or update the slot
	 *
	 * @param SlotId the ID of the slot that we wish to write to, will create a new slot if blank
	 */
	void RunWriteSlot(const FString& SlotId);

	/** Delegate handler for when the GetAllSlots call succeeds */
	void OnGetAllSlotsSuccess(const TArray<FAccelByteModelsSlot>& Results);

	/** Delegate handler for when the GetAllSlots call fails */
	void OnGetAllSlotsError(int32 ErrorCode, const FString& ErrorMessage);

	/** Delegate handler for when the CreateSlot or UpdateSlot call succeeds */
	void OnCreateOrUpdateSlotSuccess(const FAccelByteModelsSlot& Result);

	/** Delegate handler for when request progress is updated for a CreateSlot or UpdateSlot call */
	void OnCreateOrUpdateSlotProgress(FHttpRequestPtr Request, int32 BytesSent, int32 BytesReceived);

	/** Delegate handler for when the CreateSlot or UpdateSlot call fails */
	void OnCreateOrUpdateSlotError(int32 ErrorCode, const FString& ErrorMessage);

};

