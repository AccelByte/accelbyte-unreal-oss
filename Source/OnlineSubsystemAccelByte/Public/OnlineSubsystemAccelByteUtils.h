#pragma once

#include "CoreMinimal.h"

#if ENGINE_MAJOR_VERSION >= 5
#include "Online/CoreOnline.h"
#else
#include "UObject/CoreOnline.h"
#endif

#include "Delegates/IDelegateInstance.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "Models/AccelByteUserModels.h"

DECLARE_DELEGATE_OneParam(FOnGetDisplayNameComplete, FString /*DisplayName*/);

DECLARE_DELEGATE_TwoParams(FOnRequestCompleted, bool /*bWasSuccessful*/, const FString& /*Error*/);

class ONLINESUBSYSTEMACCELBYTE_API FOnlineSubsystemAccelByteUtils
{
public:
	static FUniqueNetIdRef GetUniqueIdFromString(FString UniqueIdString, bool bIsEncoded = true);

	/**
	* Gets UniqueId for the specific platform (used to properly call platform functions)
	*/
	static FUniqueNetIdPtr GetPlatformUniqueIdFromUniqueId(const FUniqueNetId& UniqueId);

	// ~Begin AccelByte Util 
	static bool IsPlayerOnSamePlatform(const FUniqueNetId& UniqueId);
	static bool IsPlayerOnSamePlatform(FString UniqueIdString);
	
	static FString GetAccelByteIdFromUniqueId(const FUniqueNetId& UniqueId);
	static FString GetPlatformNameFromUniqueId(const FUniqueNetId& UniqueId);
	static FString GetPlatformIdStringFromUniqueId(const FUniqueNetId& UniqueId);
	static FString GetPlatformName();
	// ~End AccelByte Util 

	/**
	 * Convenience method to get display name for either a remote or local user.
	 */
	static bool GetDisplayName(int32 LocalUserNum, FString UniqueId, FOnGetDisplayNameComplete Delegate, FString DisplayName = TEXT(""));

	/**
	 * Convenience method to get display name for either a remote or local user.
	 */
	static bool GetDisplayName(int32 LocalUserNum, TSharedPtr<const FUniqueNetId> UniqueId, FOnGetDisplayNameComplete Delegate, FString DisplayName=TEXT(""));
	
	/**
	 * Convert Platform User Id string into UniqueNetId
	 */
	static FUniqueNetIdPtr GetPlatformUniqueIdFromPlatformUserId(const FString& PlatformUserId);
	
	/**
	 * Return AccelByte userId, decode if it's composite userId
	 */
	static FUniqueNetIdRef GetAccelByteUserIdFromUniqueId(const FUniqueNetId& UniqueId);

	/**
	 * Convert a native subsystem name to a login type enum value. Used to determine which path the token from the native OSS
	 * should take on the backend to properly authenticate.
	 */
	static EAccelByteLoginType GetAccelByteLoginTypeFromNativeSubsystem(const FName& SubsystemName);

	static void AddUserPlatform(const FString &UserId, const FString PlatformName);
	static FString GetUserPlatform(const FString &UserId);
	static void AddUserJoinTime(const FString &UserId, const FString Value);
	static FString GetUserJoinTime(const FString &UserId);
	static void AddUserDisconnectedTime(const FString &UserId, const FString Value);
	static FString GetUserDisconnectedTime(const FString &UserId);
	
	/**
	 * Method to calculate a local offset timestamp from UTC
	 */
	static FString GetLocalTimeOffsetFromUTC();

	static EAccelBytePlatformType GetCurrentAccelBytePlatformType(const FName& NativeSubsystemName);
	
private:

	/**
	 * Delegate handle for querying user info
	 */
	static FDelegateHandle QueryUserHandle;

	/**
	 * Cache the user platform
	 */
	static TMap<FString, FString> UserPlatformMaps;

	/**
	* Cache the user join
	*/
	static TMap<FString, FString> UserJoinCached;

	/**
	* Cache the user progression
	*/
	static TMap<FString, FString> UserDisconnectedCached;

	/**
	 * Handler for when we successfully query user information
	 */
	static void OnQueryUserInfoComplete(int32 LocalUserNum, bool bWasSuccessful, const TArray<TSharedRef<const FUniqueNetId>>& FoundIds, const FString& ErrorMessage, FOnGetDisplayNameComplete Delegate);

};