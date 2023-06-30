// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#include "OnlineAchievementsInterfaceAccelByte.h"
#include "OnlineSubsystemUtils.h"
#include "AsyncTasks/Achievement/OnlineAsyncTaskAccelByteQueryAchievement.h"
#include "AsyncTasks/Achievement/OnlineAsyncTaskAccelByteQueryUserAchievements.h"


FOnlineAchievementsAccelByte::FOnlineAchievementsAccelByte(FOnlineSubsystemAccelByte* InSubsystem)
	: AccelByteSubsystem(InSubsystem)
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
	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteQueryUserAchievements>(
		AccelByteSubsystem, PlayerId, Delegate);
}

void FOnlineAchievementsAccelByte::QueryAchievementDescriptions(
	const FUniqueNetId& PlayerId,
	const FOnQueryAchievementsCompleteDelegate& Delegate)
{
	AccelByteSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteQueryAchievement>(
		AccelByteSubsystem, PlayerId, Delegate);
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