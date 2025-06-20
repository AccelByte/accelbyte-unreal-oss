// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineTimeInterfaceAccelByte.h"
#include "OnlineSubsystemUtils.h"
#include "AsyncTasks/Time/OnlineAsyncTaskAccelByteGetServerTime.h"
#include "Core/ServerTime/AccelByteTimeManager.h"

FOnlineTimeAccelByte::FOnlineTimeAccelByte(FOnlineSubsystemAccelByte* InSubsystem) 
#if ENGINE_MAJOR_VERSION >= 5
		: AccelByteSubsystem(InSubsystem->AsWeak())
#else
		: AccelByteSubsystem(InSubsystem->AsShared())
#endif
{
}

bool FOnlineTimeAccelByte::GetFromSubsystem(const IOnlineSubsystem* Subsystem, FOnlineTimeAccelBytePtr& OutInterfaceInstance)
{
	OutInterfaceInstance = StaticCastSharedPtr<FOnlineTimeAccelByte>(Subsystem->GetTimeInterface());
	return OutInterfaceInstance.IsValid();
}

bool FOnlineTimeAccelByte::GetFromWorld(const UWorld* World, FOnlineTimeAccelBytePtr& OutInterfaceInstance)
{
	const IOnlineSubsystem* Subsystem = ::Online::GetSubsystem(World);
	if (Subsystem == nullptr)
	{
		OutInterfaceInstance = nullptr;
		return false;
	}

	return GetFromSubsystem(Subsystem, OutInterfaceInstance);
}

bool FOnlineTimeAccelByte::QueryServerUtcTime()
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return false;
	}
	
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteGetServerTime>(AccelByteSubsystemPtr.Get());
	return true;
}

FString FOnlineTimeAccelByte::GetLastServerUtcTime()
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return TEXT("");
	}
	
	FAccelByteInstancePtr AccelByteInstance = AccelByteSubsystemPtr->GetAccelByteInstance().Pin();
	if(!AccelByteInstance.IsValid())
	{
		return TEXT("");
	}

	AccelByte::FAccelByteTimeManagerPtr TimeManager = AccelByteInstance->GetTimeManager().Pin();
	if(!TimeManager.IsValid())
	{
		return TEXT("");
	}
	
	return TimeManager->GetCachedServerTime().ToString();
}

FString FOnlineTimeAccelByte::GetCurrentServerUtcTime()
{
	FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if (!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Warning, TEXT("Failed, AccelbyteSubsystem is invalid"));
		return TEXT("");
	}
	
	const FAccelByteInstancePtr AccelByteInstance = AccelByteSubsystemPtr->GetAccelByteInstance().Pin();
	if(!AccelByteInstance.IsValid())
	{
		return TEXT("");
	}

	const AccelByte::FAccelByteTimeManagerPtr TimeManager = AccelByteInstance->GetTimeManager().Pin();
	if(!TimeManager.IsValid())
	{
		return TEXT("");
	}
	
	return TimeManager->GetCurrentServerTime().ToString();
}
