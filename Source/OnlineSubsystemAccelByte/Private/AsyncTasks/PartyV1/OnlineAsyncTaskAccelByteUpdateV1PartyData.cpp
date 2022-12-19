// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteUpdateV1PartyData.h"
#include "OnlineSubsystemAccelByte.h"
#include "Core/AccelByteRegistry.h"
#include "Api/AccelByteLobbyApi.h"
#include "OnlinePartyInterfaceAccelByte.h"

FOnlineAsyncTaskAccelByteUpdateV1PartyData::FOnlineAsyncTaskAccelByteUpdateV1PartyData(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId, const FOnlinePartyId& InPartyId, const FName& InNamespace, const FOnlinePartyData& InPartyData)
	: FOnlineAsyncTaskAccelByte(InABInterface, true)
	, PartyId(StaticCastSharedRef<const FOnlinePartyIdAccelByte>(InPartyId.AsShared()))
	, Namespace(InNamespace)
	, PartyData(MakeShared<FOnlinePartyData>(InPartyData))
{
	UserId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(InLocalUserId.AsShared());
}

void FOnlineAsyncTaskAccelByteUpdateV1PartyData::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s; PartyId: %s"), *UserId->ToDebugString(), *PartyId->ToString());

	// Create function for writing new data to party storage
	TFunction<FJsonObjectWrapper(FJsonObjectWrapper)> PartyStorageWriterFunction = [PartyData = PartyData](FJsonObjectWrapper PartyStorageData) {
		// Before doing anything, we want to make sure that the JSON object is valid so that we can modify it
		if (!PartyStorageData.JsonObject.IsValid())
		{
			UE_LOG_AB(Warning, TEXT("Failed to update party storage as the JSON object provided was nullptr!"));
			return PartyStorageData;
		}

		// First we want to retrieve all dirty data that has been modified for this party data
		FOnlineKeyValuePairs<FString, FVariantData> DirtyData;
		TArray<FString> RemovedData;
		PartyData->GetDirtyKeyValAttrs(DirtyData, RemovedData);

		// Once we have done that, we want to iterate through the data that has changed, and update the JSON object
		for (const TPair<FString, FVariantData>& Data : DirtyData)
		{
			// We also don't want a type suffix for this data, so just pass false at the end
			// NOTE : Afif we do need the type suffix since we are retrieving the data by using FOnlinePartyData::FromJson
			Data.Value.AddToJsonObject(PartyStorageData.JsonObject.ToSharedRef(), Data.Key, true);
		}

		// Now, we want to remove the fields in the JSON object that have been removed from the party data
		for (const FString& Key : RemovedData)
		{
			PartyStorageData.JsonObject->RemoveField(Key);
		}

		// Finally, clear all data marked dirty from the party data object
		// NOTE : Damar, we want to retry this in case if process is fail.
		//PartyData->ClearDirty();

		return PartyStorageData;
	};

	THandler<FAccelByteModelsPartyDataNotif> OnWritePartyStorageSuccessDelegate = THandler<FAccelByteModelsPartyDataNotif>::CreateRaw(this, &FOnlineAsyncTaskAccelByteUpdateV1PartyData::OnWritePartyStorageSuccess);
	FErrorHandler OnWritePartyStorageErrorDelegate = FErrorHandler::CreateRaw(this, &FOnlineAsyncTaskAccelByteUpdateV1PartyData::OnWritePartyStorageError);
	ApiClient->Lobby.WritePartyStorage(PartyId->ToString(), PartyStorageWriterFunction, OnWritePartyStorageSuccessDelegate, OnWritePartyStorageErrorDelegate, 5);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Sent off request to update party storage!"));
}

void FOnlineAsyncTaskAccelByteUpdateV1PartyData::Finalize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));
	
	if (bWasSuccessful)
	{
		// If we successfully wrote new data for the party, then we want to update the party data on the party object with
		// with the updated data that we just sent off to the backend.
		const TSharedPtr<FOnlinePartySystemAccelByte, ESPMode::ThreadSafe> PartyInterface = StaticCastSharedPtr<FOnlinePartySystemAccelByte>(Subsystem->GetPartyInterface());
		if (PartyInterface.IsValid())
		{
			TSharedPtr<FOnlinePartyAccelByte> PartyObject = PartyInterface->GetPartyForUser(UserId.ToSharedRef(), PartyId);
			if (PartyObject.IsValid())
			{
				PartyObject->SetPartyData(PartyData);
			}
		}
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteUpdateV1PartyData::OnWritePartyStorageSuccess(const FAccelByteModelsPartyDataNotif& Result)
{
	// There's not really much we want to do here besides complete the task, we will pass the new party data to the party
	// object on Finalize
	SetLastUpdateTimeToCurrentTime();
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
}

void FOnlineAsyncTaskAccelByteUpdateV1PartyData::OnWritePartyStorageError(int32 ErrorCode, const FString& ErrorMessage)
{
	SetLastUpdateTimeToCurrentTime();
	UE_LOG_AB(Warning, TEXT("Failed to write party storage for party '%s'! Error code: %d; Error message: %s"), *PartyId->ToString(), ErrorCode, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}
