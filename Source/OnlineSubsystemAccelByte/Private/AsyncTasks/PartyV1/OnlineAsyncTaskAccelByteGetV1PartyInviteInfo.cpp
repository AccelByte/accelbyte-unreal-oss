// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteGetV1PartyInviteInfo.h"
#include "OnlinePartyInterfaceAccelByte.h"
#include "Core/AccelByteRegistry.h"
#include "Api/AccelByteUserApi.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteGetV1PartyInviteInfo::FOnlineAsyncTaskAccelByteGetV1PartyInviteInfo(FOnlineSubsystemAccelByte* const InABInterface, const TSharedRef<const FUniqueNetIdAccelByteUser>& InUserId, const FAccelByteModelsPartyGetInvitedNotice& InNotification)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, Notification(InNotification)
{
	UserId = InUserId;
}

void FOnlineAsyncTaskAccelByteGetV1PartyInviteInfo::Initialize()
{
	TRY_PIN_SUBSYSTEM()

	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s; Sender ID: %s"), *UserId->ToDebugString(), *Notification.From);

	FOnlineUserCacheAccelBytePtr UserStore = SubsystemPin->GetUserCache();
	if (!UserStore.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Could not get party invitation information from sender '%s' as our user store instance is invalid!"), *Notification.From);
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	Super::ExecuteCriticalSectionAction(FVoidHandler::CreateLambda([&]()
	{
		FOnQueryUsersComplete OnQueryNotificationSenderCompleteDelegate = TDelegateUtils<FOnQueryUsersComplete>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGetV1PartyInviteInfo::OnQueryNotificationSenderComplete);
		UserStore->QueryUsersByAccelByteIds(LocalUserNum, { Notification.From }, OnQueryNotificationSenderCompleteDelegate, true);
	}));

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetV1PartyInviteInfo::Finalize()
{
	TRY_PIN_SUBSYSTEM()

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful)
	{
		const TSharedPtr<FOnlinePartySystemAccelByte, ESPMode::ThreadSafe> PartyInterface = StaticCastSharedPtr<FOnlinePartySystemAccelByte>(SubsystemPin->GetPartyInterface());
		if (PartyInterface.IsValid())
		{
			// First, create a new shared ref for the party ID
			TSharedRef<const FOnlinePartyIdAccelByte> PartyId = MakeShared<const FOnlinePartyIdAccelByte>(Notification.PartyId);

			// Finally, construct the invite with our data and add it to our interface
			TSharedRef<FAccelBytePartyInvite> Invite = MakeShared<FAccelBytePartyInvite>(PartyId, NotificationSenderInfo->Id.ToSharedRef(), NotificationSenderInfo->DisplayName, Notification.InvitationToken);
			PartyInterface->AddPartyInvite(UserId.ToSharedRef(), Invite);
		}
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetV1PartyInviteInfo::OnQueryNotificationSenderComplete(bool bIsSuccessful, TArray<FAccelByteUserInfoRef> UsersQueried)
{
	if (bIsSuccessful && UsersQueried.IsValidIndex(0))
	{
		NotificationSenderInfo = UsersQueried[0];
		CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	}
	else
	{
		UE_LOG_AB(Warning, TEXT("Failed to get information about user '%s' that sent a party invite to user '%s'!"), *Notification.From, *UserId->ToDebugString());
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
	}
}
