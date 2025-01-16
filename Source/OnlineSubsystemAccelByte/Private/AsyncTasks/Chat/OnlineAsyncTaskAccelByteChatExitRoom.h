// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "OnlineSubsystemAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"
#include "Interfaces/OnlineChatInterface.h"

struct FAccelByteChatRoomConfig;
struct FAccelByteModelsChatActionTopicResponse;

class FOnlineAsyncTaskAccelByteChatExitRoom
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteChatExitRoom, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteChatExitRoom(
		FOnlineSubsystemAccelByte* const InABInterface,
		const FUniqueNetId& InLocalUserId,
		const FChatRoomId& InRoomId);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;
	virtual void Finalize() override;

protected:
	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteChatExitRoom");
	}

private:
	void OnExitRoomError(int32 ErrorCode, const FString& ErrorMessage);
	void OnExitRoomSuccess(const FAccelByteModelsChatActionTopicResponse& Response);

	const FString RoomId;

	FString ErrorString{};
};
#pragma once
