// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "Models/AccelByteCloudStorageModels.h"

/**
 * Async task to delete a user file from a CloudStorage slot.
 */
class FOnlineAsyncTaskAccelByteDeleteUserFile : public FOnlineAsyncTaskAccelByte
{
public:

	FOnlineAsyncTaskAccelByteDeleteUserFile(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InUserId, const FString& InFileName, bool InBShouldCloudDelete, bool InBShouldLocallyDelete);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteDeleteUserFile");
	}

private:

	/** Name of the slot that we wish to delete */
	FString FileName;

	/** Whether or not we should delete the file on the CloudStorage backend */
	bool bShouldCloudDelete;

	/**
	 * Whether we should delete the local copy of this file on the user's machine.
	 * 
	 * NOTE(Maxwell): This is ignored for now, as I do not really know where we would store these files locally.
	 */
	bool bShouldLocallyDelete;

	/** Method to run the API call to delete a slot by ID */
	void RunDeleteSlot(const FString& SlotId);

	/** Delegate handler for when the GetAllSlots call succeeds */
	void OnGetAllSlotsSuccess(const TArray<FAccelByteModelsSlot>& Results);

	/** Delegate handler for when the GetAllSlots call fails */
	void OnGetAllSlotsError(int32 ErrorCode, const FString& ErrorMessage);

	/** Delegate handler for when the DeleteSlot call succeeds */
	void OnDeleteSlotSuccess();

	/** Delegate handler for when the DeleteSlot call fails */
	void OnDeleteSlotError(int32 ErrorCode, const FString& ErrorMessage);

};
