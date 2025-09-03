// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#if 1 // MMv1 Deprecation

#include "OnlineAsyncTaskAccelByteQueryV1PartyInfo.h"
#include "OnlineSubsystemAccelByteUtils.h"
#include "OnlineUserInterfaceAccelByte.h"

#include "Core/AccelByteReport.h"

using namespace AccelByte;

FOnlineAsyncTaskAccelByteQueryV1PartyInfo::FOnlineAsyncTaskAccelByteQueryV1PartyInfo(FOnlineSubsystemAccelByte* const InABInterface, int32 InLocalUserNum, const FString& InPartyId, const TArray<FString>& InMembers, const FOnQueryPartyInfoComplete& InDelegate)
	: FOnlineAsyncTaskAccelByte(InABInterface, true)
	, PartyId(InPartyId)
	, Members(InMembers)
	, Delegate(InDelegate)
{
	FReport::LogDeprecated(FString(__FUNCTION__),
		TEXT("Party V1 functionality is deprecated and replaced by Party V2. For more information, see https://docs.accelbyte.io/gaming-services/services/play/party/"));
	LocalUserNum = InLocalUserNum;
}

FOnlineAsyncTaskAccelByteQueryV1PartyInfo::FOnlineAsyncTaskAccelByteQueryV1PartyInfo(FOnlineSubsystemAccelByte* const InABInterface, const FUniqueNetId& InUserId, const FString& InPartyId, const TArray<FString>& InMembers, const FOnQueryPartyInfoComplete& InDelegate)
    : FOnlineAsyncTaskAccelByte(InABInterface, true)
	, PartyId(InPartyId)
	, Members(InMembers)
	, Delegate(InDelegate)
{
	UserId = FUniqueNetIdAccelByteUser::CastChecked(InUserId);
}

void FOnlineAsyncTaskAccelByteQueryV1PartyInfo::Initialize()
{
	TRY_PIN_SUBSYSTEM();

    Super::Initialize();

    AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("PartyId: %s; Member count: %d"), *PartyId, Members.Num());

	// First, we want to send off a request to get basic information for each party member, such as display name
	FOnlineUserCacheAccelBytePtr UserStore = SubsystemPin->GetUserCache();
	if (!UserStore.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Could not query user information for all party members as our user store instance is invalid!"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	FOnQueryUsersComplete OnQueryPartyMembersCompleteDelegate = TDelegateUtils<FOnQueryUsersComplete>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteQueryV1PartyInfo::OnQueryPartyMembersComplete);
	UserStore->QueryUsersByAccelByteIds(LocalUserNum, Members, OnQueryPartyMembersCompleteDelegate, true);
	
	StatsQueriesRemaining.Set(Members.Num());
	CustomizationQueriesRemaining.Set(Members.Num());
	ProgressionQueriesRemaining.Set(Members.Num());
	DailyPlayStreakQueriesRemaining.Set(Members.Num());
	RanksQueriesRemaining.Set(Members.Num());

	Super::ExecuteCriticalSectionAction(FVoidHandler::CreateLambda([&]()
	{
		// Finally, we want to send a request to get party storage for this party, so that we can save it to party data
		const AccelByte::Api::Lobby::FPartyDataUpdateNotif OnGetPartyStorageSuccessDelegate = TDelegateUtils<AccelByte::Api::Lobby::FPartyDataUpdateNotif>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteQueryV1PartyInfo::OnGetPartyStorageSuccess);
		const FErrorHandler OnGetPartyStorageErrorDelegate = TDelegateUtils<FErrorHandler>::CreateThreadSafeSelfPtr(this, &FOnlineAsyncTaskAccelByteQueryV1PartyInfo::OnGetPartyStorageError);
		API_FULL_CHECK_GUARD(Lobby);
		Lobby->GetPartyStorage(PartyId, OnGetPartyStorageSuccessDelegate, OnGetPartyStorageErrorDelegate);
	}));

    AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryV1PartyInfo::Tick()
{
    Super::Tick();

	if (HasFinishedAsyncWork())
	{
		CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
	}
}

void FOnlineAsyncTaskAccelByteQueryV1PartyInfo::Finalize()
{
    AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

    AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryV1PartyInfo::TriggerDelegates()
{
    AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	Delegate.ExecuteIfBound(bWasSuccessful, PartyInfo);

    AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

bool FOnlineAsyncTaskAccelByteQueryV1PartyInfo::HasFinishedAsyncWork()
{
	return bHasRetrievedPartyMemberInfo &&
		bHasRetrievedPartyStorage;
}

void FOnlineAsyncTaskAccelByteQueryV1PartyInfo::OnQueryPartyMembersComplete(bool bIsSuccessful, TArray<FAccelByteUserInfoRef> UsersQueried)
{
	SetLastUpdateTimeToCurrentTime();
	if (bIsSuccessful)
	{
		PartyInfo.MemberInfo = UsersQueried;
	}
	else
	{
		UE_LOG_AB(Warning, TEXT("Failed to query information about party members!"));
	}
	bHasRetrievedPartyMemberInfo = true;
}

void FOnlineAsyncTaskAccelByteQueryV1PartyInfo::OnGetPartyStorageSuccess(const FAccelByteModelsPartyDataNotif& Result)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("PartyId: %s"), *Result.PartyId);

	SetLastUpdateTimeToCurrentTime();

	// First, check if we have a valid JSON object in the first place
	if (!Result.Custom_attribute.JsonObject.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get party storage as the JSON object representing the custom storage attributes is not valid!"));
		bHasRetrievedPartyStorage = true;
		return;
	}

	// Now, we can use the JSON wrapper util to convert our response to a string
	FString JSONString;
	if (!Result.Custom_attribute.JsonObjectToString(JSONString))
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get party storage as we could not convert the JSON object to a string!"));
		bHasRetrievedPartyStorage = true;
		return;
	}

	// Add attrs field for the use of FOnlinePartyData::FromJson()
	TSharedRef<FJsonObject> MainJsonObj = MakeShared<FJsonObject>();
	TSharedRef<FJsonValueObject> JsonValue = MakeShareable(new FJsonValueObject(Result.Custom_attribute.JsonObject));
	MainJsonObj->SetField("Attrs", JsonValue);

	FString PartyDataStr;
	FJsonObjectWrapper PartyStorageData;
	PartyStorageData.JsonObject = MainJsonObj;
	PartyStorageData.JsonObjectToString(JSONString);

	// Finally, we can provide this JSON string to the FromJson method of our PartyData instance which will populate our values
	PartyInfo.PartyData->FromJson(JSONString);
	bHasRetrievedPartyStorage = true;

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteQueryV1PartyInfo::OnGetPartyStorageError(int32 ErrorCode, const FString& ErrorMessage)
{
	SetLastUpdateTimeToCurrentTime();
	UE_LOG_AB(Warning, TEXT("Failed to get party storage for party %s as the request to the backend failed! Error code: %d; Error message: %s"), *PartyId, ErrorCode, *ErrorMessage);
	bHasRetrievedPartyStorage = true;
}

void FOnlineAsyncTaskAccelByteQueryV1PartyInfo::OnTaskTimedOut()
{
	UE_LOG(LogAccelByteOSSParty, Verbose,
		TEXT("Query party info timeout: bHasRetrievedPartyMemberInfo %s bHasRetrievedPartyStorage %s CustomizationQueriesRemaining %d StatsQueriesRemaining %d ProgressionQueriesRemaining %d DailyPlayStreakQueriesRemaining %d RanksQueriesRemaining %d"),
		LOG_BOOL_FORMAT(bHasRetrievedPartyMemberInfo),
		LOG_BOOL_FORMAT(bHasRetrievedPartyStorage),
		CustomizationQueriesRemaining.GetValue(),
		StatsQueriesRemaining.GetValue(),
		ProgressionQueriesRemaining.GetValue(),
		DailyPlayStreakQueriesRemaining.GetValue(),
		RanksQueriesRemaining.GetValue()
	);
}
#endif