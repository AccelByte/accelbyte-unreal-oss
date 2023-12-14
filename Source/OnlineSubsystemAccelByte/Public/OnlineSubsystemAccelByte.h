// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "OnlineAchievementsInterfaceAccelByte.h"
#include "OnlineSubsystemImpl.h"
#include "OnlineSubsystemAccelByteDefines.h"
#include "OnlineAsyncTaskManagerAccelByte.h"
#include "OnlineEntitlementsInterfaceAccelByte.h"
#include "OnlinePurchaseInterfaceAccelByte.h"
#include "OnlineStoreInterfaceV2AccelByte.h"
#include "OnlineChatInterfaceAccelByte.h"
#include "OnlineGroupsInterfaceAccelByte.h"
#include "Core/AccelByteApiClient.h"
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
class FOnlineEntitlementsAccelByte;
class FOnlineStoreV2AccelByte;
class FOnlinePurchaseAccelByte;
class FOnlineAgreementAccelByte;
class FOnlineWalletAccelByte;
class FOnlineCloudSaveAccelByte;
class FOnlineTimeAccelByte;
class FOnlineAnalyticsAccelByte;
class FOnlineStatisticAccelByte;
class FOnlineLeaderboardAccelByte;
class FOnlineChatAccelByte;
class FOnlineGroupsAccelByte;
class FOnlineAuthAccelByte;
class FOnlineAchievementsAccelByte; 
class FOnlinePredefinedEventAccelByte;
class FOnlineGameStandardEventAccelByte;
class FExecTestBase;
class FOnlineVoiceAccelByte;

class IVoiceChat;
class IVoiceChatUser;
class FAccelByteVoiceChat;

class FOnlineAsyncEpicTaskAccelByte;
class FOnlineAsyncTaskAccelByte;

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

/** Shared pointer to the AccelByte entitlements */
typedef TSharedPtr<FOnlineEntitlementsAccelByte, ESPMode::ThreadSafe> FOnlineEntitlementsAccelBytePtr;

/** Shared pointer to the AccelByte store */
typedef TSharedPtr<FOnlineStoreV2AccelByte, ESPMode::ThreadSafe> FOnlineStoreV2AccelBytePtr;

/** Shared pointer to the AccelByte Purchasing */
typedef TSharedPtr<FOnlinePurchaseAccelByte, ESPMode::ThreadSafe> FOnlinePurchaseAccelBytePtr;

/** Shared pointer to the AccelByte Agreement */
typedef TSharedPtr<FOnlineAgreementAccelByte, ESPMode::ThreadSafe> FOnlineAgreementAccelBytePtr;

/** Shared pointer to the AccelByte Wallet */
typedef TSharedPtr<FOnlineWalletAccelByte, ESPMode::ThreadSafe> FOnlineWalletAccelBytePtr;

/** Shared pointer to the AccelByte Cloud Save */
typedef TSharedPtr<FOnlineCloudSaveAccelByte, ESPMode::ThreadSafe> FOnlineCloudSaveAccelBytePtr;

/** Shared pointer to the AccelByte Time */
typedef TSharedPtr<FOnlineTimeAccelByte, ESPMode::ThreadSafe> FOnlineTimeAccelBytePtr;

/** Shared pointer to the AccelByte Analytics */
typedef TSharedPtr<FOnlineAnalyticsAccelByte, ESPMode::ThreadSafe> FOnlineAnalyticsAccelBytePtr;

/** Shared pointer to the AccelByte Statistic */
typedef TSharedPtr<FOnlineStatisticAccelByte, ESPMode::ThreadSafe> FOnlineStatisticAccelBytePtr;

/** Shared pointer to the AccelByte Leaderboard */
typedef TSharedPtr<FOnlineLeaderboardAccelByte, ESPMode::ThreadSafe> FOnlineLeaderboardAccelBytePtr;

/** Shared pointer to the AccelByte Chat  */
typedef TSharedPtr<FOnlineChatAccelByte, ESPMode::ThreadSafe> FOnlineChatAccelBytePtr;

/** Shared pointer to the AccelByte Groups  */
typedef TSharedPtr<FOnlineGroupsAccelByte, ESPMode::ThreadSafe> FOnlineGroupsAccelBytePtr;

/** Shared pointer to the AccelByte implementation of the Auth interface */
typedef TSharedPtr<FOnlineAuthAccelByte, ESPMode::ThreadSafe> FOnlineAuthAccelBytePtr;

typedef TSharedPtr<FOnlineAchievementsAccelByte, ESPMode::ThreadSafe> FOnlineAchievementsAccelBytePtr;

/** Shared ponter to the AccelByte implementation of the Voice Chat interface */
typedef TSharedPtr<FOnlineVoiceAccelByte, ESPMode::ThreadSafe> FOnlineVoiceAccelBytePtr;

/** Shared ponter to the AccelByte implementation of the Predefined Event interface */
typedef TSharedPtr<FOnlinePredefinedEventAccelByte, ESPMode::ThreadSafe> FOnlinePredefinedEventAccelBytePtr;

/** Shared ponter to the AccelByte implementation of the Game Standard Event interface */
typedef TSharedPtr<FOnlineGameStandardEventAccelByte, ESPMode::ThreadSafe> FOnlineGameStandardEventAccelBytePtr;

typedef TSharedPtr<IVoiceChat, ESPMode::ThreadSafe> IVoiceChatPtr;
typedef TSharedPtr<IVoiceChatUser, ESPMode::ThreadSafe> IVoiceChatUserPtr;
typedef TSharedPtr<FAccelByteVoiceChat, ESPMode::ThreadSafe> FAccelByteVoiceChatPtr;

class ONLINESUBSYSTEMACCELBYTE_API FOnlineSubsystemAccelByte final
	: public FOnlineSubsystemImpl
	, public TSharedFromThis<FOnlineSubsystemAccelByte, ESPMode::ThreadSafe>
{
	DECLARE_MULTICAST_DELEGATE(FOnLocalUserNumCachedDelegate);

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
	virtual FOnlineCloudSaveAccelBytePtr GetCloudSaveInterface() const; 
	virtual IOnlineTimePtr GetTimeInterface() const override;
	virtual FOnlineAnalyticsAccelBytePtr GetAnalyticsInterface() const;
	virtual IOnlineStatsPtr GetStatsInterface() const override;
	virtual IOnlineLeaderboardsPtr GetLeaderboardsInterface() const override;
	virtual IOnlineChatPtr GetChatInterface() const override;
	virtual IOnlineGroupsPtr GetGroupsInterface() const override;
	virtual FOnlineAuthAccelBytePtr GetAuthInterface() const;
	virtual IOnlineVoicePtr GetVoiceInterface() const override;
	virtual FOnlinePredefinedEventAccelBytePtr GetPredefinedEventInterface() const;
	virtual FOnlineGameStandardEventAccelBytePtr GetGameStandardEventInterface() const;
	IVoiceChatPtr GetVoiceChatInterface();

#if (ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION <= 25)
	IOnlineTurnBasedPtr GetTurnBasedInterface() const override;
	IOnlineTournamentPtr GetTournamentInterface() const override;
#endif

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
	
	void ResetLocalUserNumCached();
	void SetLocalUserNumCached(int32 InLocalUserNum);
	int32 GetLocalUserNumCached();

	/**
	 * Get the FApiClient that is used for a particular user by their net ID.
	 *
	 * Used to make raw SDK calls for user if needed.
	 */
	AccelByte::FApiClientPtr GetApiClient(const FUniqueNetId& UserId);

	/**
	 * Get the FApiClient that is used for a particular user by their net ID.
	 *
	 * Used to make raw SDK calls for user if needed.
	 */
	AccelByte::FApiClientPtr GetApiClient(int32 LocalUserNum);

	FString GetLanguage();

	void SetLanguage(const FString & InLanguage);


	FOnLocalUserNumCachedDelegate& OnLocalUserNumCached();
	
PACKAGE_SCOPE:
	/** Disable the default constructor, instances of the OSS are only to be managed by the factory spawned by the module */
	FOnlineSubsystemAccelByte() = delete;

	/**
	 * Construct an instance of the AccelByte subsystem through the factory.
	 * Interfaces are initialized to nullptr, as they are set up in the FOnlineSubsystemAccelByte::Init method.
	 */
	explicit FOnlineSubsystemAccelByte(FName InInstanceName)
		: FOnlineSubsystemImpl(ACCELBYTE_SUBSYSTEM, InInstanceName)
		, LocalUserNumCached(-1)
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
		, TimeInterface(nullptr)
		, AnalyticsInterface(nullptr)
		, StatisticInterface(nullptr)
		, LeaderboardInterface(nullptr)
		, ChatInterface(nullptr)
		, AuthInterface(nullptr)
		, AchievementInterface(nullptr)
		, VoiceInterface(nullptr)
		, PredefinedEventInterface(nullptr)
		, GameStandardEventInterface(nullptr)
		, Language(FGenericPlatformMisc::GetDefaultLanguage())
		, AccelBytePluginList(TArray<FString>
			{
				"AccelByteUe4Sdk",
				"OnlineSubsystemAccelByte",
				"AccelByteNetworkUtilities"
			})
	{
	}

	/** Create and enqueue an Epic to the task manager's ParallelTasks queue */
	FOnlineAsyncEpicTaskAccelByte* CreateAndDispatchEpic(int32 LocalUserNum, const AccelByte::FVoidHandler& InDelegate);

// To allow the automation test
OVERRIDE_PACKAGE_SCOPE:
	/** Wrap the actual implementation to the source file to prevent function call to a forward-declared class (FOnlineAsyncEpicTaskAccelByte) */
	void CreateAndDispatchAsyncTaskImplementation(FOnlineAsyncTaskInfo TaskInfo, FOnlineAsyncTask* CreatedTask);

	/** Create and queue an async task to the tasks queue */
	template <typename TOnlineAsyncTask, typename... TArguments>
	FORCEINLINE void CreateAndDispatchAsyncTask(FOnlineAsyncTaskInfo TaskInfo, TArguments&&... Arguments)
	{
		// compile time check to make sure that the template type passed in derives from FOnlineAsyncTask
		static_assert(TIsDerivedFrom<TOnlineAsyncTask, FOnlineAsyncTaskAccelByte>::IsDerived, "Type passed to CreateAndDispatchAsyncTask must derive from FOnlineAsyncTask");

		check(AsyncTaskManager.IsValid());

		TOnlineAsyncTask* NewTask = new TOnlineAsyncTask(Forward<TArguments>(Arguments)...);
		
		CreateAndDispatchAsyncTaskImplementation(TaskInfo, NewTask);
	}

PACKAGE_SCOPE:
	/** Create and queue an async task to the parallel tasks queue */
	template <typename TOnlineAsyncTask, typename... TArguments>
	FORCEINLINE void CreateAndDispatchAsyncTaskParallel(TArguments&&... Arguments)
	{
		FOnlineAsyncTaskInfo TaskInfo;
		TaskInfo.Type = ETypeOfOnlineAsyncTask::Parallel;
		CreateAndDispatchAsyncTask<TOnlineAsyncTask>(TaskInfo, Forward<TArguments>(Arguments)...);
	}

	/** Create and queue an async task to the in queue */
	template <typename TOnlineAsyncTask, typename... TArguments>
	FORCEINLINE void CreateAndDispatchAsyncTaskSerial(TArguments&&... Arguments)
	{
		FOnlineAsyncTaskInfo TaskInfo;
		TaskInfo.Type = ETypeOfOnlineAsyncTask::Serial;
		CreateAndDispatchAsyncTask<TOnlineAsyncTask>(TaskInfo, Forward<TArguments>(Arguments)...);
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

	/** Obtain the reference to Lock and set the upcoming Epic. To prevent racing condition.*/
	FCriticalSection* GetEpicTaskLock() { return &EpicTaskLock; } 
	
	/** Obtain the reference to Lock and set the upcoming Parent Task. To prevent racing condition.*/
	FCriticalSection* GetUpcomingParentTaskLock() { return &UpcomingTaskParentLock; }
	
	/** To check the whether an Epic already set or not */
	bool IsUpcomingEpicAlreadySet();

	/** Register an Epic to the subsystem.
	* Therefore, every upcoming task created will be considered as sub-task of this Epic.
	* It can be set by another async task using an existing Epic, or create a new Epic.
	*/
	void SetUpcomingEpic(FOnlineAsyncEpicTaskAccelByte* Epic);

	/** Register an AsyncTask to the subsystem as a parent task.
	* Therefore, every upcoming task created will be considered as child of this parent task.
	* It willl set by another async task when it creates a child task through execute critical section.
	*/
	void SetUpcomingParentTask(FOnlineAsyncTaskAccelByte* Parent);

	/** Unregister Epic that has been registered to the subsystem. */
	void ResetEpicHasBeenSet();
	/** Unregister Async Task/parent task that has been registered to the subsystem. */
	void ResetParentTaskHasBeenSet();

	/** Used by Epic to remove the task from Epic’s container for completion (OutQueue) by the subsystem’s AsyncTaskManager. */
	void AddTaskToOutQueue(FOnlineAsyncTaskAccelByte* Task);

	/** Used by subsystem to Enqueue a sub-task/child task into the currently assigned Epic */
	void EnqueueTaskToEpic(FOnlineAsyncEpicTaskAccelByte* EpicPtr, FOnlineAsyncTaskAccelByte* TaskPtr, ETypeOfOnlineAsyncTask TaskType);
	void EnqueueTaskToEpic(FOnlineAsyncTaskAccelByte* TaskPtr, ETypeOfOnlineAsyncTask TaskType);

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

	bool IsAutoConnectLobby() const;

	bool IsAutoConnectChat() const;
	
	bool IsMultipleLocalUsersEnabled() const;

	bool IsLocalUserNumCached() const;

private:
	bool bIsAutoLobbyConnectAfterLoginSuccess = false;
	bool bIsAutoChatConnectAfterLoginSuccess = false;
	bool bIsMultipleLocalUsersEnabled = false;

	bool bIsInitializedEventSent = false;
	FDateTime PluginInitializedTime;

	/** Used to track whether there is already user cached */
	bool bIsLocalUserNumCached = false;

	/** Used to store the currently logged in account's LocalUserNum value */
	int32 LocalUserNumCached;
	mutable FCriticalSection LocalUserNumCachedLock;

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
	
	/** Shared instance of our entitlement interface implementation */
	FOnlineEntitlementsAccelBytePtr EntitlementsInterface;

	/** Shared instance of our storev2 interface implementation */
	FOnlineStoreV2AccelBytePtr StoreV2Interface;

	/** Shared instance of our purchase interface implementation */
	FOnlinePurchaseAccelBytePtr PurchaseInterface;

	/** Shared instance of our wallet interface implementation */
	FOnlineWalletAccelBytePtr WalletInterface;

	/** Shared instance of our cloud save interface implementation */
	FOnlineCloudSaveAccelBytePtr CloudSaveInterface;

	/** Shared instance of our time interface implementation */
	FOnlineTimeAccelBytePtr TimeInterface;
	
	/** Shared instance of our analytics interface implementation */
	FOnlineAnalyticsAccelBytePtr AnalyticsInterface;

	/** Shared instance of our statistic interface implementation */
	FOnlineStatisticAccelBytePtr StatisticInterface;

	/** Shared instance of our leaderboard interface implementation */
	FOnlineLeaderboardAccelBytePtr LeaderboardInterface;

	/** Shared instance of our chat interface implementation */
	FOnlineChatAccelBytePtr ChatInterface;

	/** Shared instance of our groups interface implementation */
	FOnlineGroupsAccelBytePtr GroupsInterface;

	/** Shared instance of our auth implementation */
	FOnlineAuthAccelBytePtr AuthInterface;

	/** Shared instance of our achievement implementation */
	FOnlineAchievementsAccelBytePtr AchievementInterface;

	/** Shared instance of our voice chat implementation */
	mutable FOnlineVoiceAccelBytePtr VoiceInterface;

	/** Shared instance of our predefined event implementation */
	FOnlinePredefinedEventAccelBytePtr PredefinedEventInterface;

	/** Shared instance of our game standard event implementation */
	FOnlineGameStandardEventAccelBytePtr GameStandardEventInterface;

	/** Thread spawned to run the FOnlineAsyncTaskManagerAccelBytePtr instance */
	TUniquePtr<FRunnableThread> AsyncTaskManagerThread;
	IVoiceChatPtr VoiceChatInterface;

	/** Language to be used on AccelByte Service Requests*/
	FString Language;

	mutable bool bVoiceInterfaceInitialized = false;

#if WITH_DEV_AUTOMATION_TESTS
	/** An array of console command exec tests that are marked as incomplete. Completed tests will be removed on each tick. */
	TArray<TSharedPtr<FExecTestBase>> ActiveExecTests;
#endif

	/** This is lock to prevent a racing condition when adding and reading Epic */
	FCriticalSection EpicTaskLock;
	
	/** This is lock to prevent a racing condition when adding the UpcomingTaskParent */
	FCriticalSection UpcomingTaskParentLock;

	/** The address of Epic that will be set to the upcoming async task creation */
	FOnlineAsyncEpicTaskAccelByte* EpicForUpcomingTask = nullptr;

	/** The address of Parent Task that will be set to the upcoming async task creation */
	FOnlineAsyncTaskAccelByte* ParentTaskForUpcomingTask = nullptr;

	/** The number will be incremented safely for each created Epic number */
	FThreadSafeCounter EpicCounter;

	const TArray<FString> AccelBytePluginList;

	/**
	 * @p2p Delegate handler fired when we get a successful login from the OSS on the first user. Used to initialize our network manager.
	 */
	void OnLoginCallback(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error);

	void OnMessageNotif(const FAccelByteModelsNotificationMessage &InMessage, int32 LocalUserNum);

	void OnLobbyConnectedCallback(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UniqueNetId, const FString& ErrorMessage);

	void OnLobbyConnectionClosed(int32 StatusCode, const FString& Reason, bool WasClean, int32 InLocalUserNum);

	void OnLobbyReconnected(int32 InLocalUserNum);

	void SendInitializedEvent();

	DECLARE_DELEGATE(FLogOutFromInterfaceDelegate)
	FLogOutFromInterfaceDelegate LogoutDelegate {};

	FOnLocalUserNumCachedDelegate OnLocalUserNumCachedDelegate;
};

/** Shared pointer to the AccelByte implementation of the OnlineSubsystem */
typedef TSharedPtr<FOnlineSubsystemAccelByte, ESPMode::ThreadSafe> FOnlineSubsystemAccelBytePtr;
