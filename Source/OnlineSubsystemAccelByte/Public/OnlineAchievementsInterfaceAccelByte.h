// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "OnlineSubsystemAccelByte.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "Core/AccelByteApiClient.h"
#include "Interfaces/OnlineAchievementsInterface.h"
#include "Models/AccelByteAchievementModels.h"
#include "InterfaceModels/OnlineAchievementInterfaceAccelByteModels.h"
#include "OnlineSubsystemAccelBytePackage.h"

class IOnlineSubsystem;
class FOnlineSubsystemAccelByte;

using FUserAchievementMap = TMap<FString, TSharedRef<FOnlineAchievement>>;
using FUserIdToUserAchievementMap = TMap<TSharedRef<const FUniqueNetIdAccelByteUser>, FUserAchievementMap, FDefaultSetAllocator, TUserUniqueIdConstSharedRefMapKeyFuncs<FUserAchievementMap>>;

DECLARE_DELEGATE_TwoParams(FOnSendPSNEventsCompleteDelegate, bool /*bWasSuccessful*/, FAccelByteModelsAchievementBulkCreatePSNEventResponse /*Response*/);

/**
 * Delegate fired when bulk achievement have been unlocked
 */
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnBulkAchievementUnlocked, const FUniqueNetId&, const TArray<FAccelByteModelsAchievementBulkUnlockRespone>&);
typedef FOnBulkAchievementUnlocked::FDelegate FOnBulkAchievementUnlockDelegate;

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

	/**
	 * [DEPRECATED] Please use the new 'QueryUserAchievements' for the replacement
	 * Used by a user to query a list of their achievements.
	 * Include achieved and in-progress.
	 *
	 * @param PlayerId - User ID requesting to query their achievements
	 * @param Delegate - Return delegate indicating the success of this method
	 */
	virtual void QueryAchievements(
		const FUniqueNetId& PlayerId,
		const FOnQueryAchievementsCompleteDelegate& Delegate) override;

	/**
	 * Used by a user to query a list of their achievements.
	 * Include achieved and in-progress.
	 *
	 * @param PlayerId - User ID requesting to query their achievements
	 * @param RequestParameters - Optional parameters that contains of Sort By, Page, Tag and Unlocked Flag.
	 * Each of the optional parameters have their own default value.
	 * @param Delegate - Return delegate indicating the success of this method
	 */
	virtual void QueryAchievements(
		const FUniqueNetId& PlayerId,
		const FAccelByteQueryAchievementsParameters& RequestParameters,
		const FOnQueryAchievementsCompleteDelegate& Delegate);

	/**
	 * [DEPRECATED] Please use the new 'QueryAllAchievements' for the replacement
	 * Used by a user to query a list of all available achievements.
	 *
	 * @param PlayerId - User ID requesting to query their achievements
	 * @param Delegate - Return delegate indicating the success of this method
	 */
	virtual void QueryAchievementDescriptions(
		const FUniqueNetId& PlayerId,
		const FOnQueryAchievementsCompleteDelegate& Delegate) override;

	/**
	 * Used by a user to query a list of all available achievements.
	 *
	 * @param PlayerId - User ID requesting to query their achievements
	 * @param RequestParameters - Request parameters that contains of Sort By, Page, Tag and Global Achievement Flag. 
	 * Each of the optional parameters have their own default value.
	 * @param Delegate - Return delegate indicating the success of this method
	 */
	virtual void QueryAchievementDescriptions(
		const FUniqueNetId& PlayerId,
		const FAccelByteQueryAchievementDescriptionParameters& RequestParameters,
		const FOnQueryAchievementsCompleteDelegate& Delegate);

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
#if !UE_BUILD_SHIPPING
	virtual bool ResetAchievements(const FUniqueNetId& PlayerId) override;
#endif //!UE_BUILD_SHIPPING
	//~ End IOnlineAchievement Interface

	/**
	 * @brief Server call to send events pertaining to achievements to PSN
	 * 
	 * @param Request Request model describing events to send to PSN
	 * @param CompletionDelegate Delegate fired when request to send PSN events has completed
	 */
	void SendPSNEvents(const FAccelByteModelsAchievementBulkCreatePSNEventRequest& Request
		, const FOnSendPSNEventsCompleteDelegate& CompletionDelegate);

	/**
	 * @brief Unlock multiple achievement for current user
	 * 
	 * @param UserID Current local user that will unlock its achievement.
	 * @param AchievementCodes Collection of achievement code that will be unlocked for the targete user.
	 * @param OperationResponseDelegate Delegate that inform the result of the operation, consists an array of result.
	 */

	void UnlockAchievementBulkOperation(const FUniqueNetId& UserID, TArray<FString> AchievementCodes, const FOnBulkAchievementUnlockDelegate& OperationResponseDelegate);

	/**
	 * @brief Game server perform unlock multiple achievement for the specified user.
	 * 
	 * @param UserID Targeted user operation.
	 * @param AchievementCodes Collection of achievement code that will be unlocked for the targete user.
	 * @param OperationResponseDelegate Delegate that inform the result of the operation, consists an array of result.
	 */
	void UnlockAchievementBulkOperationByGameServer(const FUniqueNetId& UserID, TArray<FString> AchievementCodes, const FOnBulkAchievementUnlockDelegate& OperationResponseDelegate);
	void UnlockAchievementBulkOperationByGameServer(const FString& AccelByteUserID, TArray<FString> AchievementCodes, const FOnBulkAchievementUnlockDelegate& OperationResponseDelegate);

protected:
	FOnlineSubsystemAccelByteWPtr AccelByteSubsystem;

private:
	FOnlineAchievementsAccelByte() = delete;

	TMap<FString, TSharedRef<FOnlineAchievementDesc>> PublicAchievementMap;
	FUserIdToUserAchievementMap UserAchievementMap;

	mutable FCriticalSection AchievementMapLock;
};
