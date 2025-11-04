// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#if 1 // MMv1 Deprecation

#include "OnlineAsyncTaskAccelByteGetV1PartyCode.h"
#include "OnlineSubsystemAccelByteUtils.h"
#include "OnlineUserInterfaceAccelByte.h"

#include "Core/AccelByteReport.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteHelpers.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteGetV1PartyCode::FOnlineAsyncTaskAccelByteGetV1PartyCode(FOnlineSubsystemAccelByte* const InABInterface, const TSharedRef<const FUniqueNetIdAccelByteUser>& InUserId, const FString& InPartyId, const FOnPartyCodeGenerated& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface, true),
	PartyId(InPartyId),
	Delegate(InDelegate)
{
	FReport::LogDeprecated(FString(__FUNCTION__),
		TEXT("Party V1 functionality is deprecated and replaced by Party V2. For more information, see https://docs.accelbyte.io/gaming-services/services/play/party/"));
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
#endif