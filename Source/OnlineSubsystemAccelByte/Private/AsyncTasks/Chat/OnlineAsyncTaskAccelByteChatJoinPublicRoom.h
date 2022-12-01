// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "OnlineSubsystemAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"

struct FAccelByteChatRoomConfig;
struct FAccelByteModelsChatActionTopicResponse;

class FOnlineAsyncTaskAccelByteChatJoinPublicRoom : public FOnlineAsyncTaskAccelByte, public TSelfPtr<FOnlineAsyncTaskAccelByteChatJoinPublicRoom, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteChatJoinPublicRoom(
		FOnlineSubsystemAccelByte* const InABInterface,
		const FUniqueNetId& InLocalUserId,
		const FChatRoomId& InRoomId);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;
	virtual void Finalize() override;

protected:
	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteChatJoinPublicRoom");
	}

private:
	void OnJoinPublicRoomError(int32 ErrorCode, const FString& ErrorMessage);
	void OnJoinPublicRoomSuccess(const FAccelByteModelsChatActionTopicResponse& Response);
	void OnQueryTopicByIdAfterJoinRoomSuccess(bool bQueryRoomSuccess, FAccelByteChatRoomInfoPtr Result, int32 UserNum);

	const FString RoomId;

	FAccelByteChatRoomInfoPtr JoinedRoomInfoPtr;
	
	FString ErrorString{};
};
#pragma once
