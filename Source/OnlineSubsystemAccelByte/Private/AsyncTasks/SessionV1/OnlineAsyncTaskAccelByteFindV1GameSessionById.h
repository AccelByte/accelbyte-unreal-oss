// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#if 1 // MMv1 Deprecation
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"

class FOnlineAsyncTaskAccelByteFindV1GameSessionById
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteFindV1GameSessionById, ESPMode::ThreadSafe>
{
public:

	FOnlineAsyncTaskAccelByteFindV1GameSessionById(FOnlineSubsystemAccelByte* const InABInterface
		, const FUniqueNetId& InSearchingPlayerId
		, const FUniqueNetId& InSessionId
		, const FOnSingleSessionResultCompleteDelegate& InDelegate
		, bool IsDedidcatedServer = false);

	FOnlineAsyncTaskAccelByteFindV1GameSessionById(FOnlineSubsystemAccelByte* const InABInterface
		, int32 InLocalUserNum
		, const FUniqueNetId& InSessionId
		, const FOnSingleSessionResultCompleteDelegate& InDelegate
		, bool IsDedidcatedServer = false);

	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void TriggerDelegates() override;

protected:

	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteFindV1SessionById");
	}

private:
	/**
	 * ID of the session that we are trying to find
	 */
	TSharedRef<const FUniqueNetIdAccelByteResource> SessionId;
	
	/**
	 * Delegate fired when we finish our attempt to find this single session
	 */
	FOnSingleSessionResultCompleteDelegate Delegate;

	/**
	 * Game session instance that we found on the backend from the session ID
	 */
	FAccelByteModelsSessionBrowserData FoundGameSession;

	/**
	 * Session search result constructed from our found game session
	 */
	FOnlineSessionSearchResult FoundSessionResult;

	THandler<FAccelByteModelsSessionBrowserData> OnGetGameSessionDetailsSuccessDelegate;
	void OnGetGameSessionDetailsSuccess(const FAccelByteModelsSessionBrowserData& InFoundGameSession);

	FErrorHandler OnGetGameSessionDetailsErrorDelegate;
	void OnGetGameSessionDetailsError(int32 ErrorCode, const FString& ErrorMessage);	
};
#endif