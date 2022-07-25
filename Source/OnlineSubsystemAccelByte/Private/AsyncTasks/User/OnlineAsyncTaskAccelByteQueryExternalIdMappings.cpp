// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteQueryExternalIdMappings.h"
#include "Core/AccelByteRegistry.h"
#include "Core/AccelByteError.h"
#include "Api/AccelByteUserApi.h"
#include "OnlineSubsystemAccelByte.h"
#include "Models/AccelByteUserModels.h"

FOnlineAsyncTaskAccelByteQueryExternalIdMappings::FOnlineAsyncTaskAccelByteQueryExternalIdMappings
	( FOnlineSubsystemAccelByte* const InABSubsystem
	, const FUniqueNetId& InUserId
	, const FExternalIdQueryOptions& InQueryOptions
	, const TArray<FString>& InExternalIds
	, const IOnlineUser::FOnQueryExternalIdMappingsComplete& InDelegate )
	: FOnlineAsyncTaskAccelByte(InABSubsystem)
	, QueryOptions(InQueryOptions)
	, InitialExternalIds(InExternalIds)
	, Delegate(InDelegate)
{
	UserId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(InUserId.AsShared());
}

void FOnlineAsyncTaskAccelByteQueryExternalIdMappings::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	// First, try and remove all empty strings from the array
	InitialExternalIds.RemoveAll([](const FString& Element) { return Element.IsEmpty(); });

	// Now, check if the array is empty and bail out if it is
	if (InitialExternalIds.Num() <= 0)
	{
		ErrorStr = TEXT("external-ids-empty");
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to query external ID mappings as the array of IDs passed in was empty!"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}
	
	const UEnum* PlatformTypeEnum = StaticEnum<EAccelBytePlatformType>();
	int64 Result = PlatformTypeEnum->GetValueByNameString(QueryOptions.AuthType);
	if (Result == INDEX_NONE)
	{
		ErrorStr = TEXT("external-id-type-invalid");
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Auth type unsupported by OSS! Check EAccelBytePlatformType definition for supported auth types. Query Type: %s"), *QueryOptions.AuthType);
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	const THandler<FBulkPlatformUserIdResponse> OnBulkGetUserByOtherPlatformIdsSuccessDelegate = THandler<FBulkPlatformUserIdResponse>::CreateRaw(this, &FOnlineAsyncTaskAccelByteQueryExternalIdMappings::OnBulkGetUserByOtherPlatformIdsSuccess);
	const FErrorHandler OnBulkGetUserByOtherPlatformIdsErrorDelegate = FErrorHandler::CreateRaw(this, &FOnlineAsyncTaskAccelByteQueryExternalIdMappings::OnBulkGetUserByOtherPlatformIdsError);
	ApiClient->User.BulkGetUserByOtherPlatformUserIds(static_cast<EAccelBytePlatformType>(Result), InitialExternalIds, OnBulkGetUserByOtherPlatformIdsSuccessDelegate, OnBulkGetUserByOtherPlatformIdsErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryExternalIdMappings::Finalize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	TSharedPtr<FOnlineUserAccelByte, ESPMode::ThreadSafe> UserInterface = StaticCastSharedPtr<FOnlineUserAccelByte>(Subsystem->GetUserInterface());
	if (UserInterface != nullptr)
	{
		UserInterface->AddExternalIdMappings(ExternalIdToFoundAccelByteIdMap);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryExternalIdMappings::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Found ID Amount: %d; Is Delegate Bound: %s; bWasSuccessful: %s"), ExternalIdToFoundAccelByteIdMap.Num(), LOG_BOOL_FORMAT(Delegate.IsBound()), LOG_BOOL_FORMAT(bWasSuccessful));

	TArray<FString> FoundExternalIds;
	FoundExternalIds.Empty(ExternalIdToFoundAccelByteIdMap.Num());
	ExternalIdToFoundAccelByteIdMap.GetKeys(FoundExternalIds);

	Delegate.ExecuteIfBound(bWasSuccessful, UserId.ToSharedRef().Get(), QueryOptions, FoundExternalIds, TEXT(""));

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryExternalIdMappings::OnBulkGetUserByOtherPlatformIdsSuccess(const FBulkPlatformUserIdResponse& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Result Amount: %d"), Result.UserIdPlatforms.Num());

	for (const FPlatformUserIdMap& Mapping : Result.UserIdPlatforms)
	{
		FAccelByteUniqueIdComposite CompositeId;
		CompositeId.Id = Mapping.UserId;
		
		// Also add platform type from native subsystem and native ID, as that's usually what we'll query
		// #NOTE (Maxwell): This will break if a different platform is being queried
		IOnlineSubsystem* NativeSubsystem = IOnlineSubsystem::GetByPlatform();
		if (NativeSubsystem != nullptr)
		{
			CompositeId.PlatformType = NativeSubsystem->GetSubsystemName().ToString();
			CompositeId.PlatformId = Mapping.PlatformUserId;
		}

		TSharedRef<const FUniqueNetIdAccelByteUser> AccelByteID = FUniqueNetIdAccelByteUser::Create(CompositeId).ToSharedRef();
		ExternalIdToFoundAccelByteIdMap.Add(Mapping.PlatformUserId, AccelByteID);
	}

	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryExternalIdMappings::OnBulkGetUserByOtherPlatformIdsError(int32 ErrorCode, const FString& ErrorMessage)
{
	ErrorStr = TEXT("external-id-query-request-failed");
	UE_LOG_AB(Warning, TEXT("Failed to query bulk user IDs from platform user IDs! Error code: %d; Error message: %s"), ErrorCode, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}
