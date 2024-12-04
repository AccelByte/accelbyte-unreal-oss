// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "OnlineSubsystemAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineChatInterfaceAccelByte.h"
#include "Models/AccelByteReportingModels.h"

using namespace AccelByte;

class FOnlineAsyncTaskAccelByteChatReportMessage
	: public FOnlineAsyncTaskAccelByte
	, public TSelfPtr<FOnlineAsyncTaskAccelByteChatReportMessage, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteChatReportMessage(FOnlineSubsystemAccelByte* const InABInterface
		, const FUniqueNetId& InLocalUserId
		, const TSharedRef<FAccelByteChatMessage>& InMessage
		, const FString& InReason
		, const FString& InComment
		, FOnReportChatMessageComplete InCompletionDelegate);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;

protected:
	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteChatReportMessage");
	}

private:
	/**
	 * Shared reference to the chat message to report.
	 */
	TSharedRef<FAccelByteChatMessage> Message;

	/**
	 * String representation of the reason that the chat message is being reported for.
	 */
	FString Reason{};

	/**
	 * String containing further information relating to the chat message report.
	 */
	FString Comment{};

	/**
	 * Delegate run when the report chat message action is complete.
	 */
	FOnReportChatMessageComplete CompletionDelegate{};

	/** Online error information in case any error happen. */
	FOnlineError OnlineError;

	/**
	 * Response of the report action.
	 */
	FAccelByteModelsReportingSubmitResponse ReportResponse{};

	void OnReportMessageSuccess(const FAccelByteModelsReportingSubmitResponse& Result);
	void OnReportMessageError(int32 ErrorCode, const FString& ErrorMessage);

};
