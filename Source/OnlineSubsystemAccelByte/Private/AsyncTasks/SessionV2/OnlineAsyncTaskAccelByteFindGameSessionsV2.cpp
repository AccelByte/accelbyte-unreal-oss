// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteFindGameSessionsV2.h"
#include "OnlineSessionInterfaceV2AccelByte.h"
#include "OnlineSessionSettingsAccelByte.h"
#include "OnlineSubsystemAccelByteSessionSettings.h"

using namespace AccelByte;

EAccelByteV2SessionQueryComparisonOp OSSComparisonOpToAccelByteOp(const EOnlineComparisonOp::Type& OssOp)
{
	switch (OssOp)
	{
	case EOnlineComparisonOp::Equals:
		return EAccelByteV2SessionQueryComparisonOp::EQUAL;
	case EOnlineComparisonOp::NotEquals:
		return EAccelByteV2SessionQueryComparisonOp::NOT_EQUAL;
	case EOnlineComparisonOp::GreaterThan:
		return EAccelByteV2SessionQueryComparisonOp::GREATER_THAN;
	case EOnlineComparisonOp::GreaterThanEquals:
		return EAccelByteV2SessionQueryComparisonOp::GREATER_THAN_EQUAL;
	case EOnlineComparisonOp::LessThan:
		return EAccelByteV2SessionQueryComparisonOp::LESS_THAN;
	case EOnlineComparisonOp::LessThanEquals:
		return EAccelByteV2SessionQueryComparisonOp::LESS_THAN_EQUAL;
	case EOnlineComparisonOp::In:
		return EAccelByteV2SessionQueryComparisonOp::CONTAINS;
	case EOnlineComparisonOp::NotIn:
		return EAccelByteV2SessionQueryComparisonOp::NOT_CONTAINS;
	case EOnlineComparisonOp::Near:
		UE_LOG_AB(Warning, TEXT("AccelByte V2 session query does not support Near. Defaulting to EQUAL"));
		return EAccelByteV2SessionQueryComparisonOp::EQUAL;
	}

	return EAccelByteV2SessionQueryComparisonOp::EQUAL;
}

FOnlineAsyncTaskAccelByteFindGameSessionsV2::FOnlineAsyncTaskAccelByteFindGameSessionsV2(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InSearchingPlayerId, const TSharedRef<FOnlineSessionSearch>& InSearchSettings)
	: FOnlineAsyncTaskAccelByte(InABInterface, true)
	, SearchSettings(InSearchSettings)
	, ResultsRemaining(SearchSettings->MaxSearchResults)
{
	// #TODO #SESSIONv2 Make this support the custom timeout value from the search settings handle eventually...
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InSearchingPlayerId);
}

void FOnlineAsyncTaskAccelByteFindGameSessionsV2::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("UserId: %s"), *UserId->ToDebugString());

	QueryStruct = FAccelByteModelsV2GameSessionQuery();
	for (const TPair<FName, FOnlineSessionSearchParam>& SearchParam : SearchSettings->QuerySettings.SearchParams)
	{
		// Do not include the session type in the query!
		if (SearchParam.Key == SETTING_SESSION_TYPE)
		{
			continue;
		}

		const EAccelByteV2SessionQueryComparisonOp QueryOperator = OSSComparisonOpToAccelByteOp(SearchParam.Value.ComparisonOp);
		const FString FieldName = SearchParam.Key.ToString().ToUpper();

		AddVariantDataToQuery(QueryStruct, FieldName, QueryOperator, SearchParam.Value.Data);
	}

	// Query first page of results to start
	QueryResultsPage(0);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteFindGameSessionsV2::Tick()
{
	Super::Tick();
}

void FOnlineAsyncTaskAccelByteFindGameSessionsV2::Finalize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	SearchSettings->SearchState = (bWasSuccessful) ? EOnlineAsyncTaskState::Done : EOnlineAsyncTaskState::Failed;

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteFindGameSessionsV2::TriggerDelegates()
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(SubsystemPin->GetSessionInterface());
	if (!ensure(SessionInterface.IsValid()))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to trigger delegates for finding game sessions as our session interface is invalid!"));
		return;
	}

	SessionInterface->TriggerOnFindSessionsCompleteDelegates(bWasSuccessful);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteFindGameSessionsV2::QueryResultsPage(int32 Offset)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("Offset: %d"), Offset);

	SetLastUpdateTimeToCurrentTime();

	// Mark that we are actively searching if we have not already
	if (SearchSettings->SearchState != EOnlineAsyncTaskState::InProgress)
	{
		SearchSettings->SearchState = EOnlineAsyncTaskState::InProgress;
	}

	// Make call to query game sessions from offset with user defined limit
	OnQueryGameSessionsSuccessDelegate = TDelegateUtils<THandler<FAccelByteModelsV2PaginatedGameSessionQueryResult>>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteFindGameSessionsV2::OnQueryGameSessionsSuccess, Offset);
	OnQueryGameSessionsErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteFindGameSessionsV2::OnQueryGameSessionsError);

	const int32 Limit = FMath::Min(ResultsRemaining, ResultsPerPage);
	API_FULL_CHECK_GUARD(Session);
	Session->QueryGameSessions(QueryStruct, OnQueryGameSessionsSuccessDelegate, OnQueryGameSessionsErrorDelegate, Offset, Limit);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteFindGameSessionsV2::OnQueryGameSessionsSuccess(const FAccelByteModelsV2PaginatedGameSessionQueryResult& Result, int32 LastOffset)
{
	TRY_PIN_SUBSYSTEM();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("SessionsFound: %d"), Result.Data.Num());

	SetLastUpdateTimeToCurrentTime();

	const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(SubsystemPin->GetSessionInterface());
	AB_ASYNC_TASK_VALIDATE(SessionInterface.IsValid(), "Failed to construct game session search results as our session interface is invalid!");

	ResultsRemaining -= Result.Data.Num();
	for (const FAccelByteModelsV2GameSession& Session : Result.Data)
	{
		FOnlineSessionSearchResult SearchResult;
		if (!SessionInterface->ConstructGameSessionFromBackendSessionModel(Session, SearchResult.Session))
		{
			UE_LOG_AB(Warning, TEXT("Failed to convert session with ID '%s' to a session search result during FindSessions! Skipping!"), *Session.ID);
			continue;
		}

		SearchSettings->SearchResults.Emplace(SearchResult);
	}

	if (ResultsRemaining > 0 && !Result.Paging.Next.IsEmpty())
	{
		// If we still have not hit our maximum search results, but we also have results to grab from backend, then make a request
		// to get more results past the ones that we have already queried!
		const int32 NewOffset = LastOffset + Result.Data.Num();
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Sending request to get more search results at offset %d"), NewOffset);
		QueryResultsPage(NewOffset);
	}
	else
	{
		// Otherwise, just complete the task so that we can return these results to the game
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Found all possible results for this search, completing task!"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	}
}

void FOnlineAsyncTaskAccelByteFindGameSessionsV2::OnQueryGameSessionsError(int32 ErrorCode, const FString& ErrorMessage)
{
	SetLastUpdateTimeToCurrentTime();

	UE_LOG_AB(Warning, TEXT("Failed to query game sessions from backend! Error code: %d; Error message: %s"), ErrorCode, *ErrorMessage);
	CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
}

bool FOnlineAsyncTaskAccelByteFindGameSessionsV2::AddVariantDataToQuery(FAccelByteModelsV2GameSessionQuery& Query, const FString& FieldName, const EAccelByteV2SessionQueryComparisonOp& Comparison, const FVariantData& Data) const
{
	#define ADD_NUMBER_VALUE_TO_QUERY(Type) Type Type##Value;                    \
		Data.GetValue(Type##Value);                                              \
		Query.AddParam(FieldName, Comparison, static_cast<double>(Type##Value)); \

	switch (Data.GetType())
	{
	case EOnlineKeyValuePairDataType::Double:
	{
		ADD_NUMBER_VALUE_TO_QUERY(double);
		return true;
	}
	case EOnlineKeyValuePairDataType::Float:
	{
		ADD_NUMBER_VALUE_TO_QUERY(float);
		return true;
	}
	case EOnlineKeyValuePairDataType::String:
	{
		FString StringValue;
		Data.GetValue(StringValue);
		Query.AddParam(FieldName, Comparison, StringValue);
		return true;
	}
	case EOnlineKeyValuePairDataType::Int32:
	{
		ADD_NUMBER_VALUE_TO_QUERY(int32);
		return true;
	}
	case EOnlineKeyValuePairDataType::Int64:
	{
		ADD_NUMBER_VALUE_TO_QUERY(int64);
		return true;
	}
	case EOnlineKeyValuePairDataType::UInt32:
	{
		ADD_NUMBER_VALUE_TO_QUERY(uint32);
		return true;
	}
	case EOnlineKeyValuePairDataType::UInt64:
	{
		ADD_NUMBER_VALUE_TO_QUERY(uint64);
		return true;
	}
	case EOnlineKeyValuePairDataType::Blob:
	{
		AddArrayParameterToQuery(Query, FieldName, Comparison, Data);
		return true;
	}
	default:
	{
		UE_LOG_AB(Warning, TEXT("AccelByte V2 session queries only support number and string types!"));
		return false;
	}
	}

	#undef ADD_VALUE_TO_QUERY
}

bool FOnlineAsyncTaskAccelByteFindGameSessionsV2::AddArrayParameterToQuery(FAccelByteModelsV2GameSessionQuery& Query, const FString& FieldName, const EAccelByteV2SessionQueryComparisonOp& Comparison, const FVariantData& Data) const
{
	const auto ArrayFieldType = FOnlineSearchSettingsAccelByte::GetArrayFieldType(Data);

	if(ArrayFieldType == ESessionSettingsAccelByteArrayFieldType::INVALID)
	{
		return false;
	}

	if(ArrayFieldType == ESessionSettingsAccelByteArrayFieldType::STRINGS)
	{
		TArray<FString> ArrayValue;
		if(!FOnlineSearchSettingsAccelByte::Get(Data, ArrayValue))
		{
			return false;
		}

		Query.AddParam(FieldName, Comparison, ArrayValue);
	}
	else if(ArrayFieldType == ESessionSettingsAccelByteArrayFieldType::DOUBLES)
	{
		TArray<double> ArrayValue;
		if(!FOnlineSearchSettingsAccelByte::Get(Data, ArrayValue))
		{
			return false;
		}

		Query.AddParam(FieldName, Comparison, ArrayValue);
	}

	return true;
}
