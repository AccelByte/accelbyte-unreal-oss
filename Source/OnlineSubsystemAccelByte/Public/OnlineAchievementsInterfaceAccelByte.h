// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "OnlineSubsystemAccelByteTypes.h"
#include "Core/AccelByteApiClient.h"
#include "Interfaces/OnlineAchievementsInterface.h"

class IOnlineSubsystem;
class FOnlineSubsystemAccelByte;

using FUserAchievementMap = TMap<FString, TSharedRef<FOnlineAchievement>>;
using FUserIdToUserAchievementMap = TMap<TSharedRef<const FUniqueNetIdAccelByteUser>, FUserAchievementMap, FDefaultSetAllocator, TUserUniqueIdConstSharedRefMapKeyFuncs<FUserAchievementMap>>;

class ONLINESUBSYSTEMACCELBYTE_API FOnlineAchievementsAccelByte
	: public IOnlineAchievements
	, public TSharedFromThis<FOnlineAchievementsAccelByte, ESPMode::ThreadSafe>
{
PACKAGE_SCOPE:
	FOnlineAchievementsAccelByte(FOnlineSubsystemAccelByte* InSubsystem);

	virtual void AddPublicAchievementToMap(
		const FString& AchievementCode,
		TSharedRef<FOnlineAchievementDesc> Achievement);
	virtual void AddUserAchievementToMap(
		const TSharedRef<const FUniqueNetIdAccelByteUser>& UserId,
		TSharedRef<FOnlineAchievement> Achievement);

public:
	static bool GetFromSubsystem(
		const IOnlineSubsystem* Subsystem,
		TSharedPtr<FOnlineAchievementsAccelByte, ESPMode::ThreadSafe>& OutInterfaceInstance);
	static bool GetFromWorld(
		const UWorld* World,
		TSharedPtr<FOnlineAchievementsAccelByte, ESPMode::ThreadSafe>& OutInterfaceInstance);

	//~ Begin IOnlineAchievement Interface
	virtual void QueryAchievements(
		const FUniqueNetId& PlayerId,
		const FOnQueryAchievementsCompleteDelegate& Delegate) override;
	virtual void QueryAchievementDescriptions(
		const FUniqueNetId& PlayerId,
		const FOnQueryAchievementsCompleteDelegate& Delegate) override;
	virtual void WriteAchievements(
		const FUniqueNetId& PlayerId,
		FOnlineAchievementsWriteRef& WriteObject,
		const FOnAchievementsWrittenDelegate& Delegate) override;
	virtual EOnlineCachedResult::Type GetCachedAchievementDescription(
		const FString& AchievementId,
	    FOnlineAchievementDesc& OutAchievementDesc) override;
	virtual EOnlineCachedResult::Type GetCachedAchievements(
		const FUniqueNetId& PlayerId,
	    TArray<FOnlineAchievement>& OutAchievements) override;
	virtual EOnlineCachedResult::Type GetCachedAchievement(
		const FUniqueNetId& PlayerId,
		const FString& AchievementId,
	    FOnlineAchievement& OutAchievement) override;
	virtual bool ResetAchievements(const FUniqueNetId& PlayerId) override;
	//~ End IOnlineAchievement Interface
	
protected:
	FOnlineSubsystemAccelByte* AccelByteSubsystem = nullptr;

private:
	FOnlineAchievementsAccelByte() = delete;

	TMap<FString, TSharedRef<FOnlineAchievementDesc>> PublicAchievementMap;
	FUserIdToUserAchievementMap UserAchievementMap;

	mutable FCriticalSection AchievementMapLock;
};
