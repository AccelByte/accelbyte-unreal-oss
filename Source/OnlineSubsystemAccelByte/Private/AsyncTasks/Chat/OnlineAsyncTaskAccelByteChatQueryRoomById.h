// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "OnlineSubsystemAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"

struct FAccelByteModelsChatQueryTopicResponse;

DECLARE_DELEGATE_ThreeParams(FOnChatQueryRoomByIdComplete, bool /*bWasSuccessful*/, FAccelByteChatRoomInfoPtr /*Result*/, int32 /*LocalUserNum*/);

class FOnlineAsyncTaskAccelByteChatQueryRoomById : public FOnlineAsyncTaskAccelByte, public TSelfPtr<FOnlineAsyncTaskAccelByteChatQueryRoomById, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteChatQueryRoomById(
		FOnlineSubsystemAccelByte* const InABInterface,
		const FUniqueNetId& InLocalUserId,
		const FChatRoomId& InRoomId,
		const FOnChatQueryRoomByIdComplete& InDelegate);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;
	virtual void Finalize() override;

protected:
	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteChatQueryRoomById");
	}

private:
	void OnQueryRoomError(int32 ErrorCode, const FString& ErrorMessage);
	void OnQueryRoomSuccess(const FAccelByteModelsChatQueryTopicByIdResponse& Response);
	/** Delegate handler for when we successfully get all information for each member */
	void OnQueryMemberInformationComplete(bool bIsSuccessful, TArray<TSharedRef<FAccelByteUserInfo>> UsersQueried);

	FChatRoomId RoomId;
	FOnChatQueryRoomByIdComplete Delegate;
	FString ErrorString{};
	FAccelByteChatRoomInfoPtr RoomInfo;
	TArray<TSharedRef<FAccelByteUserInfo>> FoundUsers;
};
#pragma once
