// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "OnlineSubsystemAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByte.h"
#include "AsyncTasks/OnlineAsyncTaskAccelByteUtils.h"

struct FAccelByteModelsChatQueryTopicResponse;

DECLARE_DELEGATE_ThreeParams(FOnChatQueryRoomComplete, bool /*bWasSuccessful*/, TArray<FAccelByteChatRoomInfoRef> /*Result*/, int32 /*LocalUserNum*/);

class FOnlineAsyncTaskAccelByteChatQueryRoom
	: public FOnlineAsyncTaskAccelByte
	, public AccelByte::TSelfPtr<FOnlineAsyncTaskAccelByteChatQueryRoom, ESPMode::ThreadSafe>
{
public:
	FOnlineAsyncTaskAccelByteChatQueryRoom(
		FOnlineSubsystemAccelByte* const InABInterface,
		const FUniqueNetId& InLocalUserId,
		const FAccelByteModelsChatQueryTopicRequest& InQuery,
		const FOnChatQueryRoomComplete& InDelegate);

	virtual void Initialize() override;
	virtual void TriggerDelegates() override;
	virtual void Finalize() override;

protected:
	virtual const FString GetTaskName() const override
	{
		return TEXT("FOnlineAsyncTaskAccelByteChatQueryRoom");
	}

private:
	void OnQueryRoomError(int32 ErrorCode, const FString& ErrorMessage);
	void OnQueryRoomSuccess(const FAccelByteModelsChatQueryTopicResponse& Response);
	/** Delegate handler for when we successfully get all information for each member */
	void OnQueryMemberInformationComplete(bool bIsSuccessful, TArray<FAccelByteUserInfoRef> UsersQueried);
	
	FAccelByteModelsChatQueryTopicRequest Query;
	FOnChatQueryRoomComplete Delegate;
	FString ErrorString{};
	
	TArray<FAccelByteChatRoomInfoRef> ChatRoomInfoList;
	TArray<FAccelByteUserInfoRef> FoundUsers;
};
#pragma once
