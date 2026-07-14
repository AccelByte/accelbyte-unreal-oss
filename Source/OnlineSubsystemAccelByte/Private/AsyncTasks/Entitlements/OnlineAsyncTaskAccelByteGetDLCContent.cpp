// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteGetDLCContent.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineSubsystemAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteLog.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteHelpers.h"

#define ONLINE_ERROR_NAMESPACE "FOnlineAsyncTaskAccelByteGetDLCContent"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteGetDLCContent::FOnlineAsyncTaskAccelByteGetDLCContent(FOnlineSubsystemAccelByte* const InABInterface
	, const FUniqueNetId& InUserId
	, EAccelByteDLCType InDLCType
	, bool bInIncludeAllNamespaces)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, DLCType(InDLCType)
	, bIncludeAllNamespaces(bInIncludeAllNamespaces)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InUserId);
}

void FOnlineAsyncTaskAccelByteGetDLCContent::Initialize()
{
	TRY_PIN_SUBSYSTEM();

	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	const THandler<FAccelByteModelsSimpleUserDLCRewardContentsResponse> OnSuccessDelegate =
		TDelegateUtils<THandler<FAccelByteModelsSimpleUserDLCRewardContentsResponse>>::CreateThreadSafeSelfPtr(
			this, &FOnlineAsyncTaskAccelByteGetDLCContent::OnGetDLCContentSuccess);

	const FErrorHandler OnErrorDelegate =
		TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(
			this, &FOnlineAsyncTaskAccelByteGetDLCContent::OnGetDLCContentFailed);

	API_FULL_CHECK_GUARD(Entitlement, OnlineError);

	Entitlement->GetDLCContent(DLCType, bIncludeAllNamespaces, OnSuccessDelegate, OnErrorDelegate);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetDLCContent::TriggerDelegates()
{
	Super::TriggerDelegates();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	TRY_PIN_SUBSYSTEM();

	const FOnlineEntitlementsAccelBytePtr EntitlementsInt =
		StaticCastSharedPtr<FOnlineEntitlementsAccelByte>(SubsystemPin->GetEntitlementsInterface());

	if (EntitlementsInt.IsValid())
	{
		EntitlementsInt->TriggerOnGetDLCContentCompleteDelegates(LocalUserNum, bWasSuccessful, Contents, OnlineError);
	}
	else
	{
		UE_LOG_AB(Warning, TEXT("GetDLCContent: EntitlementsInterface is invalid; delegate will not be triggered."));
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteGetDLCContent::OnGetDLCContentSuccess(const FAccelByteModelsSimpleUserDLCRewardContentsResponse& Response)
{
	UE_LOG_AB(Log, TEXT("Successfully retrieved DLC content"));
	Contents = Response.Data;
	OnlineError = ONLINE_ERROR(EOnlineErrorResult::Success);
	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
}

void FOnlineAsyncTaskAccelByteGetDLCContent::OnGetDLCContentFailed(int32 ErrorCode, const FString& ErrorMessage)
{
	UE_LOG_AB(Warning, TEXT("Failed to retrieve DLC content. Error %d: %s"), ErrorCode, *ErrorMessage);
	OnlineError = ONLINE_ERROR(EOnlineErrorResult::RequestFailure, FString::FromInt(ErrorCode), FText::FromString(ErrorMessage));
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}

#undef ONLINE_ERROR_NAMESPACE
