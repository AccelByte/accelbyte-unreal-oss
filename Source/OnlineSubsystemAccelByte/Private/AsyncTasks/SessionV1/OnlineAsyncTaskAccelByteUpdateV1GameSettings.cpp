// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteUpdateV1GameSettings.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"

FOnlineAsyncTaskAccelByteUpdateV1GameSettings::FOnlineAsyncTaskAccelByteUpdateV1GameSettings(FOnlineSubsystemAccelByte* const InABInterface, FName InSessionName, FOnlineSessionSettings& InUpdatedSessionSettings, bool InBShouldRefreshOnlineData)
    : FOnlineAsyncTaskAccelByte(InABInterface, true)
    , SessionName(InSessionName)
    , UpdatedSessionSettings(InUpdatedSessionSettings)
    , bShouldRefreshOnlineData(InBShouldRefreshOnlineData)
{
    TRY_PIN_SUBSYSTEM_CONSTRUCTOR()

    LocalUserNum = SubsystemPin->GetLocalUserNumCached();
}

void FOnlineAsyncTaskAccelByteUpdateV1GameSettings::Initialize()
{
    TRY_PIN_SUBSYSTEM()

    Super::Initialize();

    AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("SessionName: %s"), *SessionName.ToString());

    const IOnlineSessionPtr SessionInterface = SubsystemPin->GetSessionInterface();
    check(SessionInterface != nullptr);

    FNamedOnlineSession* Session = SessionInterface->GetNamedSession(SessionName);
    if (Session == nullptr)
    {
        AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Could not update game settings as we failed to get session with name '%s'!"), *SessionName.ToString());
        CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
        return;
    }

    SessionId = Session->GetSessionIdStr();
    if (SessionId.IsEmpty())
    {
        AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Could not update game settings as we failed to get session ID for session with name '%s'!"), *SessionName.ToString());
        CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
        return;
    }

    auto SettingJson = MakeShared<FJsonObject>();
	for (const auto& Set : UpdatedSessionSettings.Settings)
	{
		auto const& Data = Set.Value.Data;
		auto JsonShared = Set.Value.Data.ToJson();
		SettingJson->SetField(Set.Key.ToString(), JsonShared->TryGetField(TEXT("value")));
	}

    THandler<FAccelByteModelsSessionBrowserData> OnUpdateGameSessionSuccessDelegate = THandler<FAccelByteModelsSessionBrowserData>::CreateLambda([this](const FAccelByteModelsSessionBrowserData& Result) {
        CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
        UE_LOG_AB(Warning, TEXT("Update game settings '%s' success"), *SessionName.ToString());
    });

    FErrorHandler OnUpdateGameSessionErrorDelegate = FErrorHandler::CreateLambda([this](int32 ErrorCode, const FString& ErrorMessage) {
        CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
        UE_LOG_AB(Error, TEXT("Failed to update game settings '%s' from backend! Error code: %d; Error message: %s"), *SessionName.ToString(), ErrorCode, *ErrorMessage);
    });

    API_CLIENT_CHECK_GUARD();
    ApiClient->SessionBrowser.UpdateGameSettings(SessionId, SettingJson, OnUpdateGameSessionSuccessDelegate, OnUpdateGameSessionErrorDelegate);

    AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Sent off task to update %s game settings!"), *SessionName.ToString());
}

void FOnlineAsyncTaskAccelByteUpdateV1GameSettings::TriggerDelegates()
{
    TRY_PIN_SUBSYSTEM()

    AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

    const IOnlineSessionPtr SessionInterface = SubsystemPin->GetSessionInterface();
    SessionInterface->TriggerOnUpdateSessionCompleteDelegates(SessionName, bWasSuccessful);

    AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}