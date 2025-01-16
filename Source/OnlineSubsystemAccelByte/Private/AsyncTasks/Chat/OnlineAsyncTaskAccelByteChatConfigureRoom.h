// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "OnlineSubsystemAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "OnlineChatInterfaceAccelByte.h"

struct FAccelByteChatRoomConfig;
struct FAccelByteModelsChatActionTopicResponse;

class FOnlineAsyncTaskAccelByteChatConfigureRoom
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteChatConfigureRoom, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteChatConfigureRoom(
		FOnlineSubsystemAccelByte* const InABInterface,
		const FUniqueNetId& InLocalUserId,
		const FChatRoomId& InRoomId,
		const FAccelByteChatRoomConfig& InChatRoomConfig);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;
	virtual void Finalize() override;

protected:
	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteChatConfigureRoom");
	}

private:
	void OnUpdateTopicError(int32 ErrorCode, const FString& ErrorMessage);
	void OnUpdateTopicSuccess(const FAccelByteModelsChatActionTopicResponse& Response);

	const FString RoomId;
	const FAccelByteChatRoomConfig ChatRoomConfig;

	FString ErrorString{};
};
#pragma once
