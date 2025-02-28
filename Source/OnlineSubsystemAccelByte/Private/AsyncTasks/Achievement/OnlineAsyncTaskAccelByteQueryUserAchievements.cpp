// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteQueryUserAchievements.h"

#include "OnlineAchievementsInterfaceAccelByte.h"
#include "Interfaces/OnlineAchievementsInterface.h"
#include "OnlinePredefinedEventInterfaceAccelByte.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteQueryUserAchievements::FOnlineAsyncTaskAccelByteQueryUserAchievements(
	FOnlineSubsystemAccelByte* const InABSubsystem,
	FUniqueNetId const& InPlayerId,
	FOnQueryAchievementsCompleteDelegate const& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABSubsystem),
	Delegate(InDelegate)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InPlayerId);
}

FOnlineAsyncTaskAccelByteQueryUserAchievements::FOnlineAsyncTaskAccelByteQueryUserAchievements(
	FOnlineSubsystemAccelByte* const InABSubsystem,
	FUniqueNetId const& InPlayerId,
	FAccelByteQueryAchievementsParameters const& InRequestParameters,
	FOnQueryAchievementsCompleteDelegate const& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABSubsystem),
	Delegate(InDelegate),
	RequestParameters(InRequestParameters)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InPlayerId);
}

void FOnlineAsyncTaskAccelByteQueryUserAchievements::Initialize()
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

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryUserAchievements::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s, bWasSuccessful: %s"), *UserId->ToDebugString() ,LOG_BOOL_FORMAT(bWasSuccessful));

	FOnlineAsyncTaskAccelByte::TriggerDelegates();
	Delegate.ExecuteIfBound(*UserId,bWasSuccessful);
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));

}

void FOnlineAsyncTaskAccelByteQueryUserAchievements::QueryAchievement(int32 Offset, int32 Limit)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Starting query achievement..."));
	
	const THandler<FAccelByteModelsPaginatedUserAchievement> OnQueryAchievementSuccess =
		TDelegateUtils<THandler<FAccelByteModelsPaginatedUserAchievement>>::CreateThreadSafeSelfPtr(this,
			&FOnlineAsyncTaskAccelByteQueryUserAchievements::HandleQueryAchievementSuccess);
	const FErrorHandler OnError = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this,
		&FOnlineAsyncTaskAccelByteQueryUserAchievements::HandleQueryAchievementError);

	API_FULL_CHECK_GUARD(Achievement);
	Achievement->QueryUserAchievements(
		RequestParameters.SortBy,
		OnQueryAchievementSuccess,
		OnError,
		Offset,
		Limit,
		RequestParameters.bUnlocked,
		RequestParameters.Tag);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryUserAchievements::HandleQueryAchievementSuccess(
	FAccelByteModelsPaginatedUserAchievement const& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""))

	if (Result.Data.Num() != 0)
	{
		UserAchievements.Append(Result.Data);

		if (RequestParameters.Page.Count != -1)
		{
			RequestParameters.Page.Count -= 1;

			if (RequestParameters.Page.Count == 0)
			{
				CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
			}
		}

		// Check if 
		if (!Result.Paging.Next.IsEmpty())
		{
			FString UrlOut;
			FString Params;
			Result.Paging.Next.Split(TEXT("?"), &UrlOut, &Params);
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
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""))
}

void FOnlineAsyncTaskAccelByteQueryUserAchievements::HandleQueryAchievementError(
	int32 ErrorCode,
	FString const& ErrorMessage)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN_VERBOSITY(Error, TEXT("Code: %d; Message: %s"), ErrorCode, *ErrorMessage);

	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}


void FOnlineAsyncTaskAccelByteQueryUserAchievements::Finalize()
{
	TRY_PIN_SUBSYSTEM();

	const TSharedPtr<FOnlineAchievementsAccelByte, ESPMode::ThreadSafe> AchievementInterface =
		StaticCastSharedPtr<FOnlineAchievementsAccelByte>(SubsystemPin->GetAchievementsInterface());

	for(FAccelByteModelsUserAchievement const& UserAchievement : UserAchievements)
	{
		TSharedRef<FOnlineAchievement> Achievement = MakeShared<FOnlineAchievement>();
		Achievement->Id = UserAchievement.AchievementCode;
		Achievement->Progress = UserAchievement.LatestValue;

		AchievementInterface->AddUserAchievementToMap(UserId.ToSharedRef(), Achievement);
	}

	const FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface = SubsystemPin->GetPredefinedEventInterface();
	if (PredefinedEventInterface.IsValid())
	{
		FAccelByteModelsAchievementsGetAllPayload AchievementsGetAllPayload{};
		AchievementsGetAllPayload.UserId = UserId->GetAccelByteId();

		PredefinedEventInterface->SendEvent(LocalUserNum, MakeShared<FAccelByteModelsAchievementsGetAllPayload>(AchievementsGetAllPayload));
	}
}



