// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineTimeInterfaceAccelByte.h"
#include "OnlineSubsystemUtils.h"
#include "AsyncTasks/Time/OnlineAsyncTaskAccelByteGetServerTime.h"

FOnlineTimeAccelByte::FOnlineTimeAccelByte(FOnlineSubsystemAccelByte* InSubsystem) 
	: AccelByteSubsystem(InSubsystem)
	, ServerTimestamp(-1)
{
}

void FOnlineTimeAccelByte::UpdateServerTime(const FDateTime& Time)
{
	FScopeLock ScopeLock(&ServerTimeLock);
	ServerTimestamp = FTimespan::FromSeconds(FPlatformTime::Seconds());
	LastServerTime = Time;
}

bool FOnlineTimeAccelByte::GetFromSubsystem(const IOnlineSubsystem* Subsystem, FOnlineTimeAccelBytePtr& OutInterfaceInstance)
{
	OutInterfaceInstance = StaticCastSharedPtr<FOnlineTimeAccelByte>(Subsystem->GetTimeInterface());
	return OutInterfaceInstance.IsValid();
}

bool FOnlineTimeAccelByte::GetFromWorld(const UWorld* World, FOnlineTimeAccelBytePtr& OutInterfaceInstance)
{
	const IOnlineSubsystem* Subsystem = Online::GetSubsystem(World);
	if (Subsystem == nullptr)
	{
		OutInterfaceInstance = nullptr;
		return false;
	}

	return GetFromSubsystem(Subsystem, OutInterfaceInstance);
}

bool FOnlineTimeAccelByte::QueryServerUtcTime()
{
	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGetServerTime>(AccelByteSubsystem);
	return true;
}

FString FOnlineTimeAccelByte::GetLastServerUtcTime()
{
	if (ServerTimestamp < 0)
	{
		return FDateTime::MinValue().ToString();
	}

	FScopeLock ScopeLock(&ServerTimeLock);
	return LastServerTime.ToString();
}

FString FOnlineTimeAccelByte::GetBackCalculatedServerTime()
{
	if (ServerTimestamp < 0)
	{
		return FDateTime::MinValue().ToString();
	}

	FScopeLock ScopeLock(&ServerTimeLock);
	FTimespan Timespan = FTimespan::FromSeconds(FPlatformTime::Seconds());
	Timespan -= ServerTimestamp;
	FDateTime BackCalculated = LastServerTime + Timespan;
	return BackCalculated.ToString();
}
