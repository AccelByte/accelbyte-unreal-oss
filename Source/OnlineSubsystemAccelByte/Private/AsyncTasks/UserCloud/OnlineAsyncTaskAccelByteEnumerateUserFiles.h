// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineUserCloudInterfaceAccelByte.h"
#include "Models/AccelByteCloudStorageModels.h"

/**
 * Async task to get information about all files that a user has in their cloud store
 */
class FOnlineAsyncTaskAccelByteEnumerateUserFiles
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteEnumerateUserFiles, ESPMode::ThreadSafe>
{
public:

	/** Constructor to setup the EnumerateUserFiles task */
	FOnlineAsyncTaskAccelByteEnumerateUserFiles(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InUserId);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteEnumerateUserFiles");
	}

private:

	/** Map containing each file header instance constructed from the cloud storage slots */
	TMap<FString, FCloudFileHeader> FileNameToFileHeaderMap;

	/** Delegate handler for when the GetAllSlots call succeeds */
	void OnGetAllSlotsSuccess(const TArray<FAccelByteModelsSlot>& Results);

	/** Delegate handler for when the GetAllSlots call fails */
	void OnGetAllSlotsError(int32 ErrorCode, const FString& ErrorMessage);

};
