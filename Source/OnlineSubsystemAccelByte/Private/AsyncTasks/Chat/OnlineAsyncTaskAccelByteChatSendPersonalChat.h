// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "OnlineSubsystemAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"

struct FAccelByteChatRoomConfig;
struct FAccelByteModelsChatActionTopicResponse;

class FOnlineAsyncTaskAccelByteChatSendPersonalChat
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteChatSendPersonalChat, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteChatSendPersonalChat(
		FOnlineSubsystemAccelByte* const InABInterface,
		const FUniqueNetId& InLocalUserId,
		const FUniqueNetId& InRecipientId,
		const FString& InChatMessage);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;
	virtual void Finalize() override;

protected:
	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteChatSendPersonalChat");
	}

private:
	void CreatePersonalTopic();
	void SendPersonalChat();
	void OnSendRoomChatError(int32 InErrorCode, const FString& ErrorMessage);
	void OnSendRoomChatSuccess(const FAccelByteModelsChatSendChatResponse& Response);
	void OnCreatePersonalTopicError(int32 InErrorCode, const FString& ErrorMessage);
	void OnCreatePersonalTopicSuccess(const FAccelByteModelsChatActionTopicResponse& Response);

	TSharedPtr<const FUniqueNetIdAccelByteUser> RecipientId;
	const FString ChatMessage;
	FString RoomId;

	int32 ErrorCode{ 0 };
	FString ErrorString{};

	FAccelByteModelsChatSendChatResponse SendChatResponse{};
};
#pragma once
