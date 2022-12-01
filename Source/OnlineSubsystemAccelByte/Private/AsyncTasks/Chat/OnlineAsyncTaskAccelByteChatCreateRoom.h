// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "OnlineSubsystemAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"

struct FAccelByteChatRoomConfig;
struct FAccelByteModelsChatActionTopicResponse;

class FOnlineAsyncTaskAccelByteChatCreateRoom : public FOnlineAsyncTaskAccelByte, public TSelfPtr<FOnlineAsyncTaskAccelByteChatCreateRoom, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteChatCreateRoom(
		FOnlineSubsystemAccelByte* const InABInterface,
		const FUniqueNetId& InLocalUserId,
		const FAccelByteChatRoomConfig& InChatRoomConfig);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;
	virtual void Finalize() override;

protected:
	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteChatCreateRoom");
	}

private:
	void OnCreateGroupTopicError(int32 ErrorCode, const FString& ErrorMessage);
	void OnCreateGroupTopicSuccess(const FAccelByteModelsChatActionTopicResponse& Response);

	const FAccelByteChatRoomConfig ChatRoomConfig;

	FString RoomId;
	FString ErrorString{};
};
