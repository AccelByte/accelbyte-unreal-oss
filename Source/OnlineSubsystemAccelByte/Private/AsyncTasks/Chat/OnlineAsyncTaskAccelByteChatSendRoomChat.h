// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "OnlineSubsystemAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"

struct FAccelByteChatRoomConfig;
struct FAccelByteModelsChatActionTopicResponse;

class FOnlineAsyncTaskAccelByteChatSendRoomChat
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteChatSendRoomChat, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteChatSendRoomChat(
		FOnlineSubsystemAccelByte* const InABInterface,
		const FUniqueNetId& InLocalUserId,
		const FChatRoomId& InRoomId,
		const FString& InChatMessage);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;
	virtual void Finalize() override;

protected:
	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteChatSendRoomChat");
	}

private:
	void OnSendRoomChatError(int32 ErrorCode, const FString& ErrorMessage);
	void OnSendRoomChatSuccess(const FAccelByteModelsChatSendChatResponse& Response);

	const FString RoomId;
	const FString ChatMessage;

	FDateTime CreatedAt{};

	FString ErrorString{};
};
#pragma once
