// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineUserCloudInterfaceAccelByte.h"

/**
 * Async task to read file contents from a slot in the CloudStorage API.
 */
class FOnlineAsyncTaskAccelByteReadUserFile : public FOnlineAsyncTaskAccelByte
{
public:

	/** Constructor to setup the ReadUserFile task */
	FOnlineAsyncTaskAccelByteReadUserFile(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InUserId, const FString& InFileName);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteReadUserFile");
	}

private:

	/** String identifier for the file that we want to read, will correspond to SlotName */
	FString FileName;

	/** Array of bytes that correspond to the file data read */
	TArray<uint8> FileContents;

	/** Slot ID that the read operation was ultimately performed on */
	FString ResolvedSlotId;

	void RunGetSlot(const FString& SlotId);

	/** Delegate handler for when the GetAllSlots call succeeds */
	void OnGetAllSlotsSuccess(const TArray<FAccelByteModelsSlot>& Results);

	/** Delegate handler for when the GetAllSlots call fails */
	void OnGetAllSlotsError(int32 ErrorCode, const FString& ErrorMessage);

	/**
	 * Delegate handler for when we get a slot back from the backend.
	 * 
	 * @param Result Array of bytes that the slot has stored
	 */
	void OnGetSlotSuccess(const TArray<uint8>& Result);

	/** Delegate handler for when we fail to get a slot back from the backend */
	void OnGetSlotError(int32 ErrorCode, const FString& ErrorMessage);

};

