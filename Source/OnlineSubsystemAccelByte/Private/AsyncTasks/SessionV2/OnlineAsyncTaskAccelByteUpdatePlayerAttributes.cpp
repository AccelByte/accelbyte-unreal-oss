// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#include "OnlineAsyncTaskAccelByteUpdatePlayerAttributes.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteUpdatePlayerAttributes::FOnlineAsyncTaskAccelByteUpdatePlayerAttributes(FOnlineSubsystemAccelByte* const InABInterface, 
	const FUniqueNetId& InLocalUserId,
	const FOnlineSessionV2AccelBytePlayerAttributes& InAttributesToUpdate,
	const FOnUpdatePlayerAttributesComplete& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface, false)
	, AttributesToUpdate(InAttributesToUpdate)
	, Delegate(InDelegate)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InLocalUserId);
}

void FOnlineAsyncTaskAccelByteUpdatePlayerAttributes::Initialize()
{
	TRY_PIN_SUBSYSTEM()

	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("LocalUserId: %s"), *UserId->ToDebugString());

	// To begin, grab a copy of the current internal attributes model for this player from the session interface. By
	// this point attributes should be populated as it is done at login.
	FOnlineSessionV2AccelBytePtr SessionInterface{};
	AB_ASYNC_TASK_ENSURE(FOnlineSessionV2AccelByte::GetFromSubsystem(SubsystemPin.Get(), SessionInterface), "Failed to get session interface instance from subsystem")

	FAccelByteModelsV2PlayerAttributes* FoundAttributes = SessionInterface->GetInternalPlayerAttributes(UserId.ToSharedRef().Get());
	AB_ASYNC_TASK_ENSURE(FoundAttributes != nullptr, "Failed to get internal player attributes from the session interface")

	Attributes = *FoundAttributes;

	// Then, we need to check if the player is allowed to play crossplay at all on their local platform, so query the
	// identity interface for that
	FOnlineIdentityAccelBytePtr IdentityInterface{};
	AB_ASYNC_TASK_ENSURE(FOnlineIdentityAccelByte::GetFromSubsystem(SubsystemPin.Get(), IdentityInterface), "Failed to get identity interface instance from subsystem")

	IOnlineIdentity::FOnGetUserPrivilegeCompleteDelegate OnGetPlayerCrossplayPrivilegeDelegate = TDelegateUtils<IOnlineIdentity::FOnGetUserPrivilegeCompleteDelegate>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteUpdatePlayerAttributes::OnGetPlayerCrossplayPrivilege);
	IdentityInterface->GetUserPrivilege(UserId.ToSharedRef().Get(), EUserPrivileges::CanUserCrossPlay, OnGetPlayerCrossplayPrivilegeDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteUpdatePlayerAttributes::Finalize()
{
	Super::Finalize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful)
	{
		TRY_PIN_SUBSYSTEM()

		FOnlineSessionV2AccelBytePtr SessionInterface{};
		if (!ensureAlwaysMsgf(FOnlineSessionV2AccelByte::GetFromSubsystem(SubsystemPin.Get(), SessionInterface), TEXT("Failed to get session interface instance from subsystem")))
		{
			return;
		}

		SessionInterface->StorePlayerAttributes(UserId.ToSharedRef().Get(), MoveTempIfPossible(Attributes));
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteUpdatePlayerAttributes::TriggerDelegates()
{
	Super::TriggerDelegates();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	Delegate.ExecuteIfBound(UserId.ToSharedRef().Get(), bWasSuccessful);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteUpdatePlayerAttributes::OnGetPlayerCrossplayPrivilege(const FUniqueNetId& LocalUserId, EUserPrivileges::Type Privilege, uint32 PrivilegeResult)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("LocalUserId: %s; Result: %d"), *LocalUserId.ToDebugString(), PrivilegeResult);

	// Construct the update request model from both the previous model and the update passed into this call
	const bool bIsCrossplayAllowed = (PrivilegeResult == 0 && AttributesToUpdate.bEnableCrossplay == true);
	FAccelByteModelsV2StorePlayerAttributesRequest Request{};
	Request.CurrentPlatform = Attributes.CurrentPlatform;
	Request.Platforms = (AttributesToUpdate.Platforms.Num() != 0) ? AttributesToUpdate.Platforms : Attributes.Platforms;
	Request.CrossplayEnabled = (bIsCrossplayAllowed) ? AttributesToUpdate.bEnableCrossplay : false;
	Request.Data.JsonObject = AttributesToUpdate.Data;
	Request.Roles = AttributesToUpdate.Roles;

	if (Request.Data.JsonObject.IsValid())
	{
		FString OutDataStr{};
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutDataStr);
		AB_ASYNC_TASK_ENSURE(FJsonSerializer::Serialize(Request.Data.JsonObject.ToSharedRef(), Writer, true), "Failed to serialize update Data JSON object to a string!");

		Request.Data.JsonString = OutDataStr;
	}

	OnStorePlayerAttributesSuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsV2PlayerAttributes>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteUpdatePlayerAttributes::OnStorePlayerAttributesSuccess);
	OnStorePlayerAttributesErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteUpdatePlayerAttributes::OnStorePlayerAttributesError);;
	API_CLIENT_CHECK_GUARD();
	ApiClient->Session.StorePlayerAttributes(Request, OnStorePlayerAttributesSuccessDelegate, OnStorePlayerAttributesErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteUpdatePlayerAttributes::OnStorePlayerAttributesSuccess(const FAccelByteModelsV2PlayerAttributes& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	Attributes = Result;
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteUpdatePlayerAttributes::OnStorePlayerAttributesError(int32 ErrorCode, const FString& ErrorMessage)
{
	AB_ASYNC_TASK_REQUEST_FAILED("Failed to update or create player attributes in the session service!", ErrorCode, ErrorMessage);
}
