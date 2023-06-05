// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#include "OnlineAsyncTaskAccelByteInitializePlayerAttributes.h"
#include "OnlineSessionInterfaceV2AccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineIdentityInterfaceAccelByte.h"

FOnlineAsyncTaskAccelByteInitializePlayerAttributes::FOnlineAsyncTaskAccelByteInitializePlayerAttributes(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InLocalUserId)
	: FOnlineAsyncTaskAccelByte(InABInterface, ASYNC_TASK_FLAG_BIT(EAccelByteAsyncTaskFlags::None))
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InLocalUserId);
}

void FOnlineAsyncTaskAccelByteInitializePlayerAttributes::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("LocalUserId: %s"), *UserId->ToDebugString());

	// First, we need to check if the player is allowed to play crossplay at all on their local platform, so query the
	// identity interface for that
	FOnlineIdentityAccelBytePtr IdentityInterface{};
	AB_ASYNC_TASK_ENSURE(FOnlineIdentityAccelByte::GetFromSubsystem(Subsystem, IdentityInterface), "Failed to get identity interface instance from subsystem")

	IOnlineIdentity::FOnGetUserPrivilegeCompleteDelegate OnGetPlayerCrossplayPrivilegeDelegate = TDelegateUtils<IOnlineIdentity::FOnGetUserPrivilegeCompleteDelegate>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteInitializePlayerAttributes::OnGetPlayerCrossplayPrivilege);
	IdentityInterface->GetUserPrivilege(UserId.ToSharedRef().Get(), EUserPrivileges::CanUserCrossPlay, OnGetPlayerCrossplayPrivilegeDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteInitializePlayerAttributes::Finalize()
{
	Super::Finalize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful)
	{
		FOnlineSessionV2AccelBytePtr SessionInterface{};
		if (!ensureAlwaysMsgf(FOnlineSessionV2AccelByte::GetFromSubsystem(Subsystem, SessionInterface), TEXT("Failed to get session interface instance from subsystem")))
		{
			return;
		}

		SessionInterface->StorePlayerAttributes(UserId.ToSharedRef().Get(), MoveTempIfPossible(Attributes));
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteInitializePlayerAttributes::TriggerDelegates()
{
	Super::TriggerDelegates();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	FOnlineSessionV2AccelBytePtr SessionInterface{};
	if (!ensureAlwaysMsgf(FOnlineSessionV2AccelByte::GetFromSubsystem(Subsystem, SessionInterface), TEXT("Failed to get session interface instance from subsystem")))
	{
		return;
	}

	SessionInterface->TriggerOnPlayerAttributesInitializedDelegates(UserId.ToSharedRef().Get(), bWasSuccessful);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteInitializePlayerAttributes::OnGetPlayerCrossplayPrivilege(const FUniqueNetId& LocalUserId, EUserPrivileges::Type Privilege, uint32 PrivilegeResult)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("LocalUserId: %s; Result: %d"), *LocalUserId.ToDebugString(), PrivilegeResult);

	// Player is allowed to crossplay if the result integer is equal to zero
	bIsCrossplayAllowed = (PrivilegeResult == 0);

	// Send off a request to get attributes from the session service
	AB_ASYNC_TASK_DEFINE_SDK_DELEGATES(FOnlineAsyncTaskAccelByteInitializePlayerAttributes, GetPlayerAttributes, THandler<FAccelByteModelsV2PlayerAttributes>);
	ApiClient->Session.GetPlayerAttributes(OnGetPlayerAttributesSuccessDelegate, OnGetPlayerAttributesErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteInitializePlayerAttributes::OnGetPlayerAttributesSuccess(const FAccelByteModelsV2PlayerAttributes& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	Attributes = Result;

	// Check if the platform information stored in the session services matches our current platform, as well as if the
	// currently stored crossplay preference makes sense (player cannot have crossplay set to true if crossplay is
	// disallowed on their local platform). If so, then we should just complete the task as there is nothing to update.
	// Otherwise, construct a request to update the platform information.
	const FString CurrentPlatformString = Subsystem->GetNativePlatformNameString();
	const bool bCurrentPlatformMatches = Result.CurrentPlatform.Equals(CurrentPlatformString, ESearchCase::IgnoreCase);
	const bool bCrossplayStateValid = !(Result.CrossplayEnabled && !bIsCrossplayAllowed);
	const bool bStoredPlatformInfoMatches = Result.Platforms.ContainsByPredicate([&CurrentPlatformString, UserIdRef = UserId.ToSharedRef()](const FAccelByteModelsV2PlayerAttributesPlatform& Platform) {
		return Platform.Name.Equals(CurrentPlatformString, ESearchCase::IgnoreCase) && Platform.UserID.Equals(UserIdRef->GetPlatformId());
	});
	if (bCurrentPlatformMatches && bStoredPlatformInfoMatches && bCrossplayStateValid)
	{
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Retrieved player attributes from session service and stored current platform matches local current platform. Finishing task!"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
		return;
	}

	SendAttributeUpdateRequest(Result);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteInitializePlayerAttributes::OnGetPlayerAttributesError(int32 ErrorCode, const FString& ErrorMessage)
{
	// If we could not find stored attributes for this player, then we should just create new ones
	if (ErrorCode == static_cast<int32>(AccelByte::ErrorCodes::SessionPlayerAttributesNotFound))
	{
		SendAttributeUpdateRequest({});
		return;
	}

	AB_ASYNC_TASK_REQUEST_FAILED("Failed to get player attributes from session service!", ErrorCode, ErrorMessage);
}

void FOnlineAsyncTaskAccelByteInitializePlayerAttributes::SendAttributeUpdateRequest(const FAccelByteModelsV2PlayerAttributes& PreviousAttributes)
{
	// Start by copying over data from the previous attribute structure, while also setting current platform string to the
	// platform type of the player's ID
	FAccelByteModelsV2StorePlayerAttributesRequest Request{};
	Request.CrossplayEnabled = (bIsCrossplayAllowed) ? PreviousAttributes.CrossplayEnabled : false;
	Request.CurrentPlatform = UserId->GetPlatformType();
	Request.Data = PreviousAttributes.Data;
	Request.Platforms = PreviousAttributes.Platforms;

	// Check if the current platform for the player is already in the platforms array. If so, just update the ID of the
	// user stored in the array. Otherwise, add a new element.
	FAccelByteModelsV2PlayerAttributesPlatform* FoundPlatform = Request.Platforms.FindByPredicate([UserIdRef = UserId.ToSharedRef()](const FAccelByteModelsV2PlayerAttributesPlatform& Platform) {
		return Platform.Name.Equals(UserIdRef->GetPlatformType());
	});
	if (FoundPlatform != nullptr)
	{
		(*FoundPlatform).UserID = UserId->GetPlatformId();
	}
	else
	{
		FAccelByteModelsV2PlayerAttributesPlatform& NewPlatform = Request.Platforms.AddDefaulted_GetRef();
		NewPlatform.Name = UserId->GetPlatformType();
		NewPlatform.UserID = UserId->GetPlatformId();
	}

	AB_ASYNC_TASK_DEFINE_SDK_DELEGATES(FOnlineAsyncTaskAccelByteInitializePlayerAttributes, StorePlayerAttributes, THandler<FAccelByteModelsV2PlayerAttributes>);
	ApiClient->Session.StorePlayerAttributes(Request, OnStorePlayerAttributesSuccessDelegate, OnStorePlayerAttributesErrorDelegate);
}

void FOnlineAsyncTaskAccelByteInitializePlayerAttributes::OnStorePlayerAttributesSuccess(const FAccelByteModelsV2PlayerAttributes& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	Attributes = Result;
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteInitializePlayerAttributes::OnStorePlayerAttributesError(int32 ErrorCode, const FString& ErrorMessage)
{
	AB_ASYNC_TASK_REQUEST_FAILED("Failed to update or create player attributes in the session service!", ErrorCode, ErrorMessage);
}
