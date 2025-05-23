#pragma once

#include "CoreMinimal.h"

#include "Runtime/Launch/Resources/Version.h"
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
	static bool GetDisplayName(int32 LocalUserNum, FUniqueNetIdPtr UniqueId, FOnGetDisplayNameComplete Delegate, FString DisplayName=TEXT(""));
	
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

	/**
	 * Convert a native subsystem name to a login type enum value. Used to determine which path the token from the native OSS
	 * should take on the backend to properly authenticate.
	 */
	static EAccelBytePlatformType GetAccelBytePlatformTypeFromAuthType(const FString& InAuthType);

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

	/**
	 * Query string table using int32 as the Key and FString as the result value
	 * 
	 * @param Key the Key to search on the String Table
	 */
	static FString GetStringFromStringTable(const FString& StringTable, const int32 Key);

	static EAccelBytePlatformType GetCurrentAccelBytePlatformType(const FName& NativeSubsystemName);

	/**
	 * Check if the local user num is in valid range of maximum local user.
	 *
	 * @return True if the local user num is in the range of maximum local user for current platform.
	 */
	static bool IsValidLocalUserNum(const int32& InLocalUserNum);

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