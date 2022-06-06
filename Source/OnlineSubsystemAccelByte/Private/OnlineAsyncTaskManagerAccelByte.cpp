// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskManagerAccelByte.h"
#include "OnlineSubsystemAccelByte.h"

FOnlineAsyncTaskManagerAccelByte::FOnlineAsyncTaskManagerAccelByte(FOnlineSubsystemAccelByte* ParentSubsystem)
	:
	AccelByteSubsystem(ParentSubsystem)
{
}

void FOnlineAsyncTaskManagerAccelByte::OnlineTick()
{
	check(AccelByteSubsystem);
	check(FPlatformTLS::GetCurrentThreadId() == OnlineThreadId);
}