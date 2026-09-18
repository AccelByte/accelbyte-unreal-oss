#pragma once

#include "CoreMinimal.h"

#include "Runtime/Launch/Resources/Version.h"
#if ENGINE_MAJOR_VERSION >= 5
#include "Online/CoreOnline.h"
#else
#include "UObject/CoreOnline.h"
#endif

#include "Delegates/IDelegateInstance.h"
#include "OnlineSubsystemAccelByteDefines.h"
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
	 *
	 * The Delegate fires EXACTLY ONCE on every path. On any failure - invalid id, missing
	 * subsystem, refused or failed query, user not found - it fires with DisplayName as the
	 * fallback value.
	 *
	 * MUST be called on the game thread.
	 *
	 * @param SubsystemName Which AccelByte OSS instance to use. Defaults to the process-wide
	 *                      ACCELBYTE subsystem; pass an isolated instance name to target one.
	 */
	static bool GetDisplayName(int32 LocalUserNum, FString UniqueId, FOnGetDisplayNameComplete Delegate, FString DisplayName = TEXT(""), FName SubsystemName = ACCELBYTE_SUBSYSTEM);

	/** @see the FString overload above. Same contract. */
	static bool GetDisplayName(int32 LocalUserNum, FUniqueNetIdPtr UniqueId, FOnGetDisplayNameComplete Delegate, FString DisplayName=TEXT(""), FName SubsystemName = ACCELBYTE_SUBSYSTEM);
	
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
	 * Convert a native platform type enum to a login type enum value. Used to determine which path the token from the native OSS
	 * should take on the backend to properly authenticate.
	 */
	static EAccelByteLoginType GetAccelByteLoginTypeFromPlatformType(EAccelBytePlatformType PlatformType);

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
	 * Per-call state for one in-flight GetDisplayName request. Shared between the registering
	 * frame and the completion handler, so it must outlive the frame and be copyable into the
	 * delegate's payload tuple - hence TSharedRef rather than TUniquePtr.
	 *
	 * Neither field is atomic. All access is confined to the game thread: GetDisplayName must be
	 * called on the game thread, and TriggerDelegates() runs there
	 * (FOnlineAsyncTaskManager::GameTick opens with check(IsInGameThread())).
	 *
	 * NOTE: must be declared before OnQueryUserInfoComplete below - a parameter type in a member
	 * declaration is not in complete-class context, so the type has to be complete at that point.
	 */
	struct FGetDisplayNameRequest
	{
		FDelegateHandle Handle{};
		bool bCompleted{false};
	};

	using FGetDisplayNameRequestRef = TSharedRef<FGetDisplayNameRequest, ESPMode::ThreadSafe>;

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
	 * Handler for when we query user information. Receives the caller's delegate, the id that was
	 * actually requested, the fallback name and the per-call request state as CreateStatic
	 * payloads, so no shared static state is required.
	 */
	static void OnQueryUserInfoComplete(int32 LocalUserNum
		, bool bWasSuccessful
		, const TArray<FUniqueNetIdRef>& FoundIds
		, const FString& ErrorMessage
		, FOnGetDisplayNameComplete Delegate
		, FUniqueNetIdRef RequestedId
		, FString FallbackDisplayName
		, FGetDisplayNameRequestRef Request
		, FName SubsystemName);

};
