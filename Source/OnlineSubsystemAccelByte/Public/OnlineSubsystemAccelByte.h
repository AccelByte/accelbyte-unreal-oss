// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystemImpl.h"
#include "OnlineSubsystemAccelByteDefines.h"
#include "OnlineAsyncTaskManagerAccelByte.h"
#include "Models/AccelByteUserModels.h"

/** Log category for any AccelByte OSS logs, including traces */
DECLARE_LOG_CATEGORY_EXTERN(LogAccelByteOSS, Warning, All);

/** Log category for extra logging regarding parties */
DECLARE_LOG_CATEGORY_EXTERN(LogAccelByteOSSParty, Warning, All);

/** Convenience UE_LOG macro that will automatically log to LogAccelByteOSS with the specified Verbosity and Format. See UE_LOG for usage. */
#define UE_LOG_AB(Verbosity, Format, ...) UE_LOG(LogAccelByteOSS, Verbosity, Format, ##__VA_ARGS__)

#define AB_USE_V2_SESSIONS_CONFIG_KEY TEXT("bEnableV2Sessions")

class FOnlineIdentityAccelByte;
class FOnlineSessionV1AccelByte;
class FOnlineSessionV2AccelByte;
class FOnlineIdentityAccelByte;
class FOnlineExternalUIAccelByte;
class FOnlineUserAccelByte;
class FOnlineUserCloudAccelByte;
class FOnlinePresenceAccelByte;
class FOnlineFriendsAccelByte;
class FOnlinePartySystemAccelByte;
class FOnlineUserCacheAccelByte;
class FOnlineAgreementAccelByte;
class FOnlineWalletAccelByte;
class FExecTestBase;

struct FAccelByteModelsNotificationMessage;

/** Shared pointer to the AccelByte implementation of the Session interface */
#if AB_USE_V2_SESSIONS
typedef TSharedPtr<FOnlineSessionV2AccelByte, ESPMode::ThreadSafe> FOnlineSessionAccelBytePtr;
#else
typedef TSharedPtr<FOnlineSessionV1AccelByte, ESPMode::ThreadSafe> FOnlineSessionAccelBytePtr;
#endif

/** Shared pointer to the AccelByte implementation of the Identity interface */
typedef TSharedPtr<FOnlineSessionV2AccelByte, ESPMode::ThreadSafe> FOnlineSessionV2AccelBytePtr;

/** Shared pointer to the AccelByte implementation of the Identity interface */
typedef TSharedPtr<FOnlineIdentityAccelByte, ESPMode::ThreadSafe> FOnlineIdentityAccelBytePtr;

/** Shared pointer to the AccelByte implementation of the External UI interface */
typedef TSharedPtr<FOnlineExternalUIAccelByte, ESPMode::ThreadSafe> FOnlineExternalUIAccelBytePtr;

/** Shared pointer to the AccelByte implementation of the User interface */
typedef TSharedPtr<FOnlineUserAccelByte, ESPMode::ThreadSafe> FOnlineUserAccelBytePtr;

/** Shared pointer to the AccelByte implementation of the User cloud interface */
typedef TSharedPtr<FOnlineUserCloudAccelByte, ESPMode::ThreadSafe> FOnlineUserCloudAccelBytePtr;

/** Shared pointer to the AccelByte implementation of the presence interface */
typedef TSharedPtr<FOnlinePresenceAccelByte, ESPMode::ThreadSafe> FOnlinePresenceAccelBytePtr;

/** Shared pointer to the AccelByte implementation of the friends interface */
typedef TSharedPtr<FOnlineFriendsAccelByte, ESPMode::ThreadSafe> FOnlineFriendsAccelBytePtr;

/** Shared pointer to the AccelByte implementation of the party system interface */
typedef TSharedPtr<FOnlinePartySystemAccelByte, ESPMode::ThreadSafe> FOnlinePartySystemAccelBytePtr;

/** Shared pointer to the AccelByte user store */
typedef TSharedPtr<FOnlineUserCacheAccelByte, ESPMode::ThreadSafe> FOnlineUserCacheAccelBytePtr;

/** Shared pointer to the AccelByte async task manager for this OSS */
typedef TSharedPtr<FOnlineAsyncTaskManagerAccelByte, ESPMode::ThreadSafe> FOnlineAsyncTaskManagerAccelBytePtr;

/** Shared pointer to the AccelByte agreement */
typedef TSharedPtr<FOnlineAgreementAccelByte, ESPMode::ThreadSafe> FOnlineAgreementAccelBytePtr;

typedef TSharedPtr<FOnlineWalletAccelByte, ESPMode::ThreadSafe> FOnlineWalletAccelBytePtr;

class ONLINESUBSYSTEMACCELBYTE_API FOnlineSubsystemAccelByte final : public FOnlineSubsystemImpl
{
public:
	virtual ~FOnlineSubsystemAccelByte() override = default;

	//~ Begin IOnlineSubsystem Interface
	virtual bool Init() override;
	virtual bool Shutdown() override;
	virtual FString GetAppId() const override;
	virtual FText GetOnlineServiceName() const override;
	virtual IOnlineSessionPtr GetSessionInterface() const override;
	virtual IOnlineFriendsPtr GetFriendsInterface() const override;
	virtual IOnlineIdentityPtr GetIdentityInterface() const override;
	virtual IOnlineExternalUIPtr GetExternalUIInterface() const override;
	virtual IOnlineUserPtr GetUserInterface() const override;
	virtual IOnlineUserCloudPtr GetUserCloudInterface() const override;
	virtual IOnlinePartyPtr GetPartyInterface() const override;
	virtual IOnlinePresencePtr GetPresenceInterface() const override;
	virtual IOnlineStoreV2Ptr GetStoreV2Interface() const override;
	virtual IOnlinePurchasePtr GetPurchaseInterface() const override;
	virtual IOnlineEntitlementsPtr GetEntitlementsInterface() const override;
	virtual IOnlineAchievementsPtr GetAchievementsInterface() const override;
	virtual FOnlineAgreementAccelBytePtr GetAgreementInterface() const;
	virtual FOnlineWalletAccelBytePtr GetWalletInterface() const;
	virtual bool Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar) override;
	virtual bool IsEnabled() const override;
	//~ End IOnlineSubsystem Interface

	//~ Begin Custom OSS Interface
	//~ End Custom OSS Interface

	/**
	 * Retrieves the user cache instance for this subsystem
	 */
	FOnlineUserCacheAccelBytePtr GetUserCache() const;

	//~ Begin FTickerObjectBase
	virtual bool Tick(float DeltaTime) override;
	//~ End FTickerObjectBase

	/**
	 * Method to check whether we support pass through from a native OSS to our OSS.
	 */
	bool IsNativeSubsystemSupported(const FName& NativeSubsystemName);

	/**
	 * Get the associated native platform subsystem name as a string, used for same platform checks
	 */
	FString GetNativePlatformNameString();

	/**
	 * Get the associated native platform subsystem name as an FName
	 */
	FName GetNativePlatformName();

	/**
	 * Get the app ID associated with this application from the native subsystem, or empty if no native subsystem is found.
	 */
	FString GetNativeAppId();
	
PACKAGE_SCOPE:
	/** Disable the default constructor, instances of the OSS are only to be managed by the factory spawned by the module */
	FOnlineSubsystemAccelByte() = delete;

	/**
	 * Construct an instance of the AccelByte subsystem through the factory.
	 * Interfaces are initialized to nullptr, as they are set up in the FOnlineSubsystemAccelByte::Init method.
	 */
	explicit FOnlineSubsystemAccelByte(FName InInstanceName)
		: FOnlineSubsystemImpl(ACCELBYTE_SUBSYSTEM, InInstanceName)
		, SessionInterface(nullptr)
		, IdentityInterface(nullptr)
		, ExternalUIInterface(nullptr)
		, UserInterface(nullptr)
		, UserCloudInterface(nullptr)
		, FriendsInterface(nullptr)
		, PartyInterface(nullptr)
		, PresenceInterface(nullptr)
		, UserCache(nullptr)
		, AsyncTaskManager(nullptr)
	{
	}

	/** Create and queue an async task to the parallel tasks queue */
	template <typename TOnlineAsyncTask, typename... TArguments>
	FORCEINLINE void CreateAndDispatchAsyncTaskParallel(TArguments&&... Arguments)
	{
		// compile time check to make sure that the template type passed in derives from FOnlineAsyncTask
		static_assert(TIsDerivedFrom<TOnlineAsyncTask, FOnlineAsyncTask>::IsDerived, "Type passed to CreateAndDispatchAsyncTaskParallel must derive from FOnlineAsyncTask");

		check(AsyncTaskManager.IsValid());

		TOnlineAsyncTask* NewTask = new TOnlineAsyncTask(Forward<TArguments>(Arguments)...);
		AsyncTaskManager->AddToParallelTasks(NewTask);
	}

	/** Create and queue an async task to the in queue */
	template <typename TOnlineAsyncTask, typename... TArguments>
	FORCEINLINE void CreateAndDispatchAsyncTaskSerial(TArguments&&... Arguments)
	{
		// compile time check to make sure that the template type passed in derives from FOnlineAsyncTask
		static_assert(TIsDerivedFrom<TOnlineAsyncTask, FOnlineAsyncTask>::IsDerived, "Type passed to CreateAndDispatchAsyncTaskParallel must derive from FOnlineAsyncTask");

		check(AsyncTaskManager.IsValid());

		TOnlineAsyncTask* NewTask = new TOnlineAsyncTask(Forward<TArguments>(Arguments)...);
		AsyncTaskManager->AddToInQueue(NewTask);
	}

	/** Create and queue an async event to be processed in the OutQueue */
	template <typename TOnlineAsyncEvent, typename... TArguments>
	FORCEINLINE void CreateAndDispatchAsyncEvent(TArguments&&... Arguments)
	{
		// compile time check to make sure that the template type passed in derives from TOnlineAsyncEvent
		static_assert(TIsDerivedFrom<TOnlineAsyncEvent, FOnlineAsyncEvent<FOnlineSubsystemAccelByte>>::IsDerived, "Type passed to CreateAndDispatchAsyncEvent must derive from TOnlineAsyncEvent");

		check(AsyncTaskManager.IsValid());

		TOnlineAsyncEvent* NewEvent = new TOnlineAsyncEvent(Forward<TArguments>(Arguments)...);
		AsyncTaskManager->AddToOutQueue(NewEvent);
	}

#if WITH_DEV_AUTOMATION_TESTS
	/**
	 * Add a single exec test to the list of active exec tests that this subsystem instance is managing.
	 */
	void AddExecTest(const TSharedPtr<FExecTestBase>& ExecTest)
	{
		ActiveExecTests.Add(ExecTest);
	}
#endif

	/**
	 * Attempt to get the corresponding EAccelBytePlatformType enum value from an OSS auth type string
	 * 
	 * @param InAuthType FName corresponding to the type of the UniqueId that you want to get a platform type for
	 * @param Result Enum value that corresponds to the string passed in, if you want to check validity of this enum, check
	 * the return boolean value
	 * @return a boolean that is true if a match is found, and false otherwise
	 */
	bool GetAccelBytePlatformTypeFromAuthType(const FString& InAuthType, EAccelBytePlatformType& Result);

	/**
	 * Attempt to get the corresponding AccelByte backend platform name from an OSS auth type string
	 *
	 * @param InAuthType FName corresponding to the type of the UniqueId that you want to get a platform type for
	 * @return String matching auth type, or blank if none corresponds
	 */
	FString GetAccelBytePlatformStringFromAuthType(const FString& InAuthType);

	/**
	 * Convert an AccelByte platform type string to a string that represents the native subsystem name that it is associated with.
	 */
	FString GetNativeSubsystemNameFromAccelBytePlatformString(const FString& InAccelBytePlatform);

	/**
	 * Gets the current native platform type as a string
	 */
	FString GetNativePlatformTypeAsString();

	/**
	 * Gets a simplified string for the native platform subsystem that is active.
	 * Ex. if we are on GDK, then "xbox" will be returned.
	 */
	FString GetSimplifiedNativePlatformName();

	/**
	 * Gets a simplified string from the platform name passed in.
	 * Ex. if we are on GDK, then "xbox" will be returned.
	 */
	FString GetSimplifiedNativePlatformName(const FString& PlatformName);

	bool bAutoLobbyConnectAfterLoginSuccess;

private:

	/** Shared instance of our session implementation */
	FOnlineSessionAccelBytePtr SessionInterface;

	/** Shared instance of our identity implementation */
	FOnlineIdentityAccelBytePtr IdentityInterface;

	/** Shared instance of our external UI implementation */
	FOnlineExternalUIAccelBytePtr ExternalUIInterface;

	/** Shared instance of our user interface implementation */
	FOnlineUserAccelBytePtr UserInterface;

	/** Shared instance of our user cloud interface implementation */
	FOnlineUserCloudAccelBytePtr UserCloudInterface;

	/** Shared instance of our friends interface implementation */
	FOnlineFriendsAccelBytePtr FriendsInterface;

	/** Shared instance of our party system interface implementation */
	FOnlinePartySystemAccelBytePtr PartyInterface;

	/** Shared instance of our presence interface implementation */
	FOnlinePresenceAccelBytePtr PresenceInterface;

	/** Shared instance of our user cache */
	FOnlineUserCacheAccelBytePtr UserCache;

	/** Async task manager used by interfaces in our OSS to handle async */
	FOnlineAsyncTaskManagerAccelBytePtr AsyncTaskManager;

	/** Shared instance of our agreement interface implementation */
	FOnlineAgreementAccelBytePtr AgreementInterface;

	/** Shared instance of our wallet interface implementation */
	FOnlineWalletAccelBytePtr WalletInterface;

	/** Thread spawned to run the FOnlineAsyncTaskManagerAccelBytePtr instance */
	TUniquePtr<FRunnableThread> AsyncTaskManagerThread;

#if WITH_DEV_AUTOMATION_TESTS
	/** An array of console command exec tests that are marked as incomplete. Completed tests will be removed on each tick. */
	TArray<TSharedPtr<FExecTestBase>> ActiveExecTests;
#endif

	/**
	 * @p2p Delegate handler fired when we get a successful login from the OSS on the first user. Used to initialize our network manager.
	 */
	void OnLoginCallback(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error);

	void OnMessageNotif(const FAccelByteModelsNotificationMessage &InMessage, int32 LocalUserNum);
};

/** Shared pointer to the AccelByte implementation of the OnlineSubsystem */
typedef TSharedPtr<FOnlineSubsystemAccelByte, ESPMode::ThreadSafe> FOnlineSubsystemAccelBytePtr;
