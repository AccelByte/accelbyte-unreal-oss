// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteGetV1PartyCode.h"
#include "OnlineSubsystemAccelByteUtils.h"
#include "OnlineUserInterfaceAccelByte.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteGetV1PartyCode::FOnlineAsyncTaskAccelByteGetV1PartyCode(FOnlineSubsystemAccelByte* const InABInterface, const TSharedRef<const FUniqueNetIdAccelByteUser>& InUserId, const FString& InPartyId, const FOnPartyCodeGenerated& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface, true),
	PartyId(InPartyId),
	Delegate(InDelegate)
{
	UserId = InUserId;
};

void FOnlineAsyncTaskAccelByteGetV1PartyCode::Initialize()
{
	Super::Initialize();
	AccelByte::Api::Lobby::FPartyGetCodeResponse OnPartyGetCodeResponseDelegate = TDelegateUtils<AccelByte::Api::Lobby::FPartyGetCodeResponse>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteGetV1PartyCode::OnPartyGetCodeResponse);

	API_FULL_CHECK_GUARD(Lobby);
	Lobby->SetPartyGetCodeResponseDelegate(OnPartyGetCodeResponseDelegate);
	Lobby->SendPartyGetCodeRequest();
}

void FOnlineAsyncTaskAccelByteGetV1PartyCode::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	Delegate.ExecuteIfBound(bWasSuccessful, ResponsePartyCode, UserId.ToSharedRef(), PartyId);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetV1PartyCode::OnPartyGetCodeResponse(const FAccelByteModelsPartyGetCodeResponse& Result)
{
	bool bWasResponseOkay = Result.Code == TEXT("0");
	ResponsePartyCode.Empty();

	if (bWasResponseOkay && !Result.PartyCode.IsEmpty())
	{
		ResponsePartyCode = Result.PartyCode;
		CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	}
	else
	{
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
	}
}