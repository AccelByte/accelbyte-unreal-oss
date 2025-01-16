// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteQueryAchievement.h"

#include "OnlineAchievementsInterfaceAccelByte.h"
#include "OnlinePredefinedEventInterfaceAccelByte.h"

using namespace AccelByte;

using QueryAchievementSuccessHandler = THandler<FAccelByteModelsPaginatedPublicAchievement>;

FOnlineAsyncTaskAccelByteQueryAchievement::FOnlineAsyncTaskAccelByteQueryAchievement(
	FOnlineSubsystemAccelByte* const Subsystem,
	FUniqueNetId const& InPlayerId,
	FOnQueryAchievementsCompleteDelegate const& InDelegate)
	: FOnlineAsyncTaskAccelByte(Subsystem)
	, Delegate(InDelegate)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InPlayerId);
}

FOnlineAsyncTaskAccelByteQueryAchievement::FOnlineAsyncTaskAccelByteQueryAchievement(
	FOnlineSubsystemAccelByte* const Subsystem,
	FUniqueNetId const& InPlayerId,
	FAccelByteQueryAchievementDescriptionParameters const& InRequestParameters,
	FOnQueryAchievementsCompleteDelegate const& InDelegate)
	: FOnlineAsyncTaskAccelByte(Subsystem)
	, Delegate(InDelegate)
	, RequestParameters(InRequestParameters)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InPlayerId);
}

void FOnlineAsyncTaskAccelByteQueryAchievement::Initialize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	FOnlineAsyncTaskAccelByte::Initialize();

	if (RequestParameters.Page.Count == -1 || RequestParameters.Page.Count >= 100)
	{
		QueryAchievement(RequestParameters.Page.Start, 100);
	}
	else
	{
		QueryAchievement(RequestParameters.Page.Start, RequestParameters.Page.Count);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryAchievement::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"),LOG_BOOL_FORMAT(bWasSuccessful));
	
	FOnlineAsyncTask::TriggerDelegates();
	Delegate.ExecuteIfBound(*UserId,bWasSuccessful);
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryAchievement::QueryAchievement(int32 Offset, int32 Limit)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Starting query achievement"));

	const QueryAchievementSuccessHandler OnQueryAchievementSuccess = TDelegateUtils<QueryAchievementSuccessHandler>::CreateThreadSafeSelfPtr(
		this,&FOnlineAsyncTaskAccelByteQueryAchievement::HandleQueryAchievementSuccess);
	const FErrorHandler OnError = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(
		this, &FOnlineAsyncTaskAccelByteQueryAchievement::HandleQueryAchievementError);

	API_CLIENT_CHECK_GUARD();
	ApiClient->Achievement.QueryAchievements(
		RequestParameters.Language,
		RequestParameters.SortBy,
		OnQueryAchievementSuccess,
		OnError,
		Offset,
		Limit,
		RequestParameters.Tag,
		RequestParameters.bGlobalAchievement);
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryAchievement::HandleQueryAchievementSuccess(
	FAccelByteModelsPaginatedPublicAchievement const& Response)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	if (Response.Data.Num() != 0)
	{
		PublicAchievements.Append(Response.Data);

		if (RequestParameters.Page.Count != -1)
		{
			RequestParameters.Page.Count -= 1;

			if (RequestParameters.Page.Count == 0)
			{
				CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
			}
		}

		// Check if 
		if (!Response.Paging.Next.IsEmpty())
		{
			FString UrlOut;
			FString Params;
			Response.Paging.Next.Split(TEXT("?"), &UrlOut, &Params);
			if (!Params.IsEmpty())
			{
				TArray<FString> ParamsArray;
				Params.ParseIntoArray(ParamsArray, TEXT("&"));
				int32 Offset = -1;
				int32 Limit = -1;
				for (const FString& Param : ParamsArray)
				{
					FString Key;
					FString Value;
					Param.Split(TEXT("="), &Key, &Value);
					if (Key.Equals(TEXT("offset")) && Value.IsNumeric())
					{
						Offset = FCString::Atoi(*Value);
					}
					else if (Key.Equals(TEXT("limit")) && Value.IsNumeric())
					{
						Limit = FCString::Atoi(*Value);
					}

					if (Offset != -1 && Limit != -1)
					{
						QueryAchievement(Offset, Limit);
						return;
					}
				}
			}
		}
	}
	else
	{
		UE_LOG_AB(Log, TEXT("[Warning] Success to get achievement items but the result is empty!"));
	}

	CompleteTask(EAccelByteAsyncTaskCompleteState::Success);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryAchievement::HandleQueryAchievementError(
	int32 ErrorCode,
	FString const& ErrorMessage)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN_VERBOSITY(Error, TEXT("Code: %d; Message: %s"), ErrorCode, *ErrorMessage);

	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryAchievement::Finalize()
{
	TRY_PIN_SUBSYSTEM()

	const TSharedPtr<FOnlineAchievementsAccelByte, ESPMode::ThreadSafe> AchievementInterface =
		StaticCastSharedPtr<FOnlineAchievementsAccelByte>(SubsystemPin->GetAchievementsInterface());
	
	for(auto& Data : PublicAchievements)
	{
		const TSharedRef<FOnlineAchievementDesc> Achievement = MakeShared<FOnlineAchievementDesc>();
		Achievement->Title = FText::FromString(Data.Name);
		Achievement->LockedDesc = FText::FromString(Data.Description);
		Achievement->UnlockedDesc = FText::FromString(Data.Description);
		Achievement->bIsHidden = Data.Hidden;
	
		AchievementInterface->AddPublicAchievementToMap(Data.AchievementCode,Achievement);
	}

	const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = SubsystemPin->GetPredefinedEventInterface();
	if (PredefinedEventInterface.IsValid())
	{
		FAccelByteModelsAchievementGetUsersPayload AchievementGetUsersPayload{};
		AchievementGetUsersPayload.UserId = UserId->GetAccelByteId();

		PredefinedEventInterface->SendEvent(LocalUserNum, MakeShared<FAccelByteModelsAchievementGetUsersPayload>(AchievementGetUsersPayload));
	}
}


