// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#include "OnlineAchievementsInterfaceAccelByte.h"
#include "OnlineSubsystemUtils.h"
#include "AsyncTasks/Achievement/OnlineAsyncTaskAccelByteQueryAchievement.h"
#include "AsyncTasks/Achievement/OnlineAsyncTaskAccelByteQueryUserAchievements.h"
#include "AsyncTasks/Achievement/OnlineAsyncTaskAccelByteSendPSNEvents.h"

using namespace AccelByte;

FOnlineAchievementsAccelByte::FOnlineAchievementsAccelByte(FOnlineSubsystemAccelByte* InSubsystem)
#if ENGINE_MAJOR_VERSION >= 5
	: AccelByteSubsystem(InSubsystem->AsWeak())
#else
	: AccelByteSubsystem(InSubsystem->AsShared())
#endif
{
}

void FOnlineAchievementsAccelByte::AddPublicAchievementToMap(
	const FString& AchievementCode,
	TSharedRef<FOnlineAchievementDesc> Achievement)
{
	FScopeLock ScopeLock(&AchievementMapLock);

	PublicAchievementMap.Emplace(AchievementCode, Achievement);
}

void FOnlineAchievementsAccelByte::AddUserAchievementToMap(const TSharedRef<const FUniqueNetIdAccelByteUser>& UserId, TSharedRef<FOnlineAchievement> Achievement)
{
	FScopeLock ScopeLock(&AchievementMapLock);
	FUserAchievementMap& UserAchievement = UserAchievementMap.FindOrAdd(UserId);

	UserAchievement.Emplace(Achievement->Id, Achievement);
}


bool FOnlineAchievementsAccelByte::GetFromSubsystem(
	const IOnlineSubsystem* Subsystem,
	TSharedPtr<FOnlineAchievementsAccelByte, ESPMode::ThreadSafe>& OutInterfaceInstance)
{
	OutInterfaceInstance = StaticCastSharedPtr<FOnlineAchievementsAccelByte>(Subsystem->GetAchievementsInterface());
	return OutInterfaceInstance.IsValid();
}

bool FOnlineAchievementsAccelByte::GetFromWorld(
	const UWorld* World,
	TSharedPtr<FOnlineAchievementsAccelByte, ESPMode::ThreadSafe>& OutInterfaceInstance)
{
	const IOnlineSubsystem* Subsystem = ::Online::GetSubsystem(World);
	if (Subsystem == nullptr)
	{
		OutInterfaceInstance = nullptr;
		return false;
	}

	return GetFromSubsystem(Subsystem, OutInterfaceInstance);
}

void FOnlineAchievementsAccelByte::QueryAchievements(
	const FUniqueNetId& PlayerId,
	const FOnQueryAchievementsCompleteDelegate& Delegate)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("UserId: %s"), *PlayerId.ToDebugString());

	FReport::LogDeprecated(FString(__FUNCTION__), TEXT("old 'QueryAchievements' function is deprecated - please use the new 'QueryAchievements' for the replacement"));

	const FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Failed to query achievement, AccelByteSubsystem ptr is invalid"));
		return;
	}
	
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteQueryUserAchievements>(
		AccelByteSubsystemPtr.Get(), PlayerId, Delegate);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineAchievementsAccelByte::QueryAchievements(
	const FUniqueNetId& PlayerId,
	const FAccelByteQueryAchievementsParameters& RequestParameters,
	const FOnQueryAchievementsCompleteDelegate& Delegate)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("UserId: %s"), *PlayerId.ToDebugString());

	const FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Failed to query achievement, AccelByteSubsystem ptr is invalid"));
		return;
	}

	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteQueryUserAchievements>(
		AccelByteSubsystemPtr.Get(),
		PlayerId,
		RequestParameters,
		Delegate);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineAchievementsAccelByte::QueryAchievementDescriptions(
	const FUniqueNetId& PlayerId,
	const FOnQueryAchievementsCompleteDelegate& Delegate)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("UserId: %s"), *PlayerId.ToDebugString());

	FReport::LogDeprecated(FString(__FUNCTION__), TEXT("old 'QueryAchievementDescriptions' function is deprecated - please use the new 'QueryAchievementDescriptions' for the replacement"));

	const FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Failed to query achievement description, AccelByteSubsystem ptr is invalid"));
		return;
	}
	
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteQueryAchievement>(
		AccelByteSubsystemPtr.Get(), PlayerId, Delegate);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

void FOnlineAchievementsAccelByte::QueryAchievementDescriptions(
	const FUniqueNetId& PlayerId,
	const FAccelByteQueryAchievementDescriptionParameters& RequestParameters,
	const FOnQueryAchievementsCompleteDelegate& Delegate)
{
	AB_OSS_PTR_INTERFACE_TRACE_BEGIN(TEXT("UserId: %s"), *PlayerId.ToDebugString());

	const FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Failed to query achievement description, AccelByteSubsystem ptr is invalid"));
		return;
	}
	
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteQueryAchievement>(
		AccelByteSubsystemPtr.Get(),
		PlayerId,
		RequestParameters,
		Delegate);

	AB_OSS_PTR_INTERFACE_TRACE_END(TEXT(""));
}

EOnlineCachedResult::Type FOnlineAchievementsAccelByte::GetCachedAchievementDescription(
	const FString& AchievementId, FOnlineAchievementDesc& OutAchievementDesc)
{
	const TSharedRef<FOnlineAchievementDesc>* AchievementPtr = PublicAchievementMap.Find(AchievementId);
	if (!AchievementPtr)
	{
		return EOnlineCachedResult::NotFound;
	}

	OutAchievementDesc = AchievementPtr->Get();

	return EOnlineCachedResult::Success;
}

EOnlineCachedResult::Type FOnlineAchievementsAccelByte::GetCachedAchievements(const FUniqueNetId& PlayerId, TArray<FOnlineAchievement>& OutAchievements)
{
	const TSharedRef<const FUniqueNetIdAccelByteUser> SharedUserId = FUniqueNetIdAccelByteUser::CastChecked(PlayerId);
	FScopeLock ScopeLock(&AchievementMapLock);

	FUserAchievementMap* UserAchievementPtr = UserAchievementMap.Find(SharedUserId);
	if(UserAchievementPtr)
	{
		for(const auto& Pair : *UserAchievementPtr)
		{
			OutAchievements.Add(Pair.Value.Get());
		}
		return EOnlineCachedResult::Success;
	}
	return EOnlineCachedResult::NotFound;
}

EOnlineCachedResult::Type FOnlineAchievementsAccelByte::GetCachedAchievement(const FUniqueNetId& PlayerId, const FString& AchievementId, FOnlineAchievement& OutAchievement)
{
	const TSharedRef<const FUniqueNetIdAccelByteUser> SharedUserId = FUniqueNetIdAccelByteUser::CastChecked(PlayerId);
	FScopeLock ScopeLock(&AchievementMapLock);

	FUserAchievementMap* UserAchievementPtr = UserAchievementMap.Find(SharedUserId);
	if(UserAchievementPtr)
	{
		TSharedRef<FOnlineAchievement>* Result = UserAchievementPtr->Find(AchievementId);
		if(Result)
		{
			OutAchievement = Result->Get();
			return EOnlineCachedResult::Success;
		}
	}
	
	return EOnlineCachedResult::NotFound;
}

void FOnlineAchievementsAccelByte::WriteAchievements(const FUniqueNetId& PlayerId, FOnlineAchievementsWriteRef& WriteObject, const FOnAchievementsWrittenDelegate& Delegate)
{
	UE_LOG_AB(Error, TEXT("error. not implemented function. FOnlineAchievementsAccelByte::WriteAchievements"))
}

#if !UE_BUILD_SHIPPING
bool FOnlineAchievementsAccelByte::ResetAchievements(const FUniqueNetId& PlayerId)
{
	UE_LOG_AB(Error, TEXT("error. not implemented function. FOnlineAchievementsAccelByte::ResetAchievements"))
	return false;
}
#endif //!UE_BUILD_SHIPPING

void FOnlineAchievementsAccelByte::SendPSNEvents(const FAccelByteModelsAchievementBulkCreatePSNEventRequest& Request
	, const FOnSendPSNEventsCompleteDelegate& CompletionDelegate)
{
	const FOnlineSubsystemAccelBytePtr AccelByteSubsystemPtr = AccelByteSubsystem.Pin();
	if(!AccelByteSubsystemPtr.IsValid())
	{
		AB_OSS_PTR_INTERFACE_TRACE_END(TEXT("Failed to send psn events, AccelByteSubsystem ptr is invalid"));
		return;
	}
	
	AccelByteSubsystemPtr->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteSendPSNEvents>(AccelByteSubsystemPtr.Get()
		, Request
		, CompletionDelegate);
}
