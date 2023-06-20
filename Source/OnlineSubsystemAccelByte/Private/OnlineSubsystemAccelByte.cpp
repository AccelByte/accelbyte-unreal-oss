// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineSubsystemAccelByte.h"
#include "OnlineSessionInterfaceV1AccelByte.h"
#include "OnlineSessionInterfaceV2AccelByte.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "OnlineExternalUIInterfaceAccelByte.h"
#include "OnlineUserInterfaceAccelByte.h"
#include "OnlineUserCloudInterfaceAccelByte.h"
#include "OnlineFriendsInterfaceAccelByte.h"
#include "OnlinePartyInterfaceAccelByte.h"
#include "OnlinePresenceInterfaceAccelByte.h"
#include "OnlineUserCacheAccelByte.h"
#include "OnlineAgreementInterfaceAccelByte.h"
#include "OnlineWalletInterfaceAccelByte.h"
#include "OnlineCloudSaveInterfaceAccelByte.h"
#include "OnlineTimeInterfaceAccelByte.h"
#include "OnlineAnalyticsInterfaceAccelByte.h"
#include "OnlineStatisticInterfaceAccelByte.h"
#include "OnlineChatInterfaceAccelByte.h"
#include "OnlineAuthInterfaceAccelByte.h"
#include "OnlineSubsystemAccelByteModule.h"
#include "OnlineVoiceInterfaceAccelByte.h"
#include "Api/AccelByteLobbyApi.h"
#include "Models/AccelByteLobbyModels.h"
#include "Core/AccelByteWebSocketErrorTypes.h"
#include "VoiceChat/AccelByteVoiceChat.h"
#include "VoiceChat.h"
//~ Begin AccelByte Peer to Peer Includes
#include "AccelByteNetworkUtilities.h"
#include "Core/AccelByteRegistry.h"
#include "Models/AccelByteUserModels.h"
//~ End AccelByte Peer to Peer Includes

#if WITH_DEV_AUTOMATION_TESTS
#include "ExecTests/ExecTestBase.h"
#endif

#define LOCTEXT_NAMESPACE "FOnlineSubsystemAccelByte"
#define PARTY_SESSION_TYPE "party"

bool FOnlineSubsystemAccelByte::Init()
{
	// Create each shared instance of our interface implementations, passing in ourselves as the parent
#if AB_USE_V2_SESSIONS
	SessionInterface = MakeShared<FOnlineSessionV2AccelByte, ESPMode::ThreadSafe>(this);
	StaticCastSharedPtr<FOnlineSessionV2AccelByte>(SessionInterface)->Init();
#else
	SessionInterface = MakeShared<FOnlineSessionV1AccelByte, ESPMode::ThreadSafe>(this);
#endif

	IdentityInterface = MakeShared<FOnlineIdentityAccelByte, ESPMode::ThreadSafe>(this);
	ExternalUIInterface = MakeShared<FOnlineExternalUIAccelByte, ESPMode::ThreadSafe>(this);
	UserInterface = MakeShared<FOnlineUserAccelByte, ESPMode::ThreadSafe>(this);
	UserCloudInterface = MakeShared<FOnlineUserCloudAccelByte, ESPMode::ThreadSafe>(this);
	FriendsInterface = MakeShared<FOnlineFriendsAccelByte, ESPMode::ThreadSafe>(this);
	PartyInterface = MakeShared<FOnlinePartySystemAccelByte, ESPMode::ThreadSafe>(this);
	PresenceInterface = MakeShared<FOnlinePresenceAccelByte, ESPMode::ThreadSafe>(this);
	UserCache = MakeShared<FOnlineUserCacheAccelByte, ESPMode::ThreadSafe>(this);
	AgreementInterface = MakeShared<FOnlineAgreementAccelByte, ESPMode::ThreadSafe>(this);
	WalletInterface = MakeShared<FOnlineWalletAccelByte, ESPMode::ThreadSafe>(this);
	CloudSaveInterface = MakeShared<FOnlineCloudSaveAccelByte, ESPMode::ThreadSafe>(this);
	EntitlementsInterface = MakeShared<FOnlineEntitlementsAccelByte, ESPMode::ThreadSafe>(this);
	StoreV2Interface = MakeShared<FOnlineStoreV2AccelByte, ESPMode::ThreadSafe>(this);
	PurchaseInterface = MakeShared<FOnlinePurchaseAccelByte, ESPMode::ThreadSafe>(this);
	TimeInterface = MakeShared<FOnlineTimeAccelByte, ESPMode::ThreadSafe>(this);
	AnalyticsInterface = MakeShared<FOnlineAnalyticsAccelByte, ESPMode::ThreadSafe>(this);
	StatisticInterface = MakeShared<FOnlineStatisticAccelByte, ESPMode::ThreadSafe>(this);
	ChatInterface = MakeShared<FOnlineChatAccelByte, ESPMode::ThreadSafe>(this);
	AuthInterface = MakeShared<FOnlineAuthAccelByte, ESPMode::ThreadSafe>(this);
	AchievementInterface = MakeShared<FOnlineAchievementsAccelByte, ESPMode::ThreadSafe>(this);
	VoiceInterface = MakeShared<FOnlineVoiceAccelByte, ESPMode::ThreadSafe>(this);
	VoiceChatInterface = MakeShared<FAccelByteVoiceChat, ESPMode::ThreadSafe>(this);
	
	// Create an async task manager and a thread for the manager to process tasks on
	AsyncTaskManager = MakeShared<FOnlineAsyncTaskManagerAccelByte, ESPMode::ThreadSafe>(this);
	AsyncTaskManagerThread.Reset(FRunnableThread::Create(AsyncTaskManager.Get(), *FString::Printf(TEXT("OnlineAsyncTaskThread %s"), *InstanceName.ToString())));
	check(AsyncTaskManagerThread.IsValid());

	for(int UserNum = 0; UserNum < MAX_LOCAL_PLAYERS; UserNum++)
	{
		// Note @damar disabling this, this should be handled for each user.
		IdentityInterface->AddOnLoginCompleteDelegate_Handle(UserNum, FOnLoginCompleteDelegate::CreateRaw(this, &FOnlineSubsystemAccelByte::OnLoginCallback));
		IdentityInterface->AddOnConnectLobbyCompleteDelegate_Handle(UserNum, FOnConnectLobbyCompleteDelegate::CreateRaw(this, &FOnlineSubsystemAccelByte::OnLobbyConnectedCallback));
	}

	GConfig->GetBool(TEXT("OnlineSubsystemAccelByte"), TEXT("bAutoLobbyConnectAfterLoginSuccess"), bIsAutoLobbyConnectAfterLoginSuccess, GEngineIni);
	GConfig->GetBool(TEXT("OnlineSubsystemAccelByte"), TEXT("bAutoChatConnectAfterLoginSuccess"), bIsAutoChatConnectAfterLoginSuccess, GEngineIni);

	GConfig->GetBool(TEXT("OnlineSubsystemAccelByte"), TEXT("bMultipleLocalUsersEnabled"), bIsMultipleLocalUsersEnabled, GEngineIni);
	
	return true;
}

bool FOnlineSubsystemAccelByte::Shutdown()
{
	// Shut down our async task thread if it is a valid handle
	if (AsyncTaskManagerThread.IsValid())
	{
		AsyncTaskManagerThread->Kill(true);
		AsyncTaskManagerThread.Reset();
	}

	// Clear our async task manager if it is a valid handle once we've killed its thread
	if (AsyncTaskManager.IsValid())
	{
		AsyncTaskManager.Reset();
	}

#if WITH_DEV_AUTOMATION_TESTS
	// Clear out any exec tests that we have added
	ActiveExecTests.Empty();
#endif

	for (int32 UserNum = 0; UserNum < MAX_LOCAL_PLAYERS; UserNum++)
	{
		// #NOTE (Maxwell): Seems that in PIE shutdown will be called twice for the OSS, rendering the IdentityInterface
		// invalid on the second pass and causing a crash here. Mitigate this by skipping this loop if interface is invalid.
		if (!IdentityInterface.IsValid())
		{
			break;
		}

		IdentityInterface->ClearOnLoginCompleteDelegates(UserNum, this);
		IdentityInterface->ClearOnConnectLobbyCompleteDelegates(UserNum, this);

		TSharedPtr<const FUniqueNetId> PlayerId = IdentityInterface->GetUniquePlayerId(UserNum);
		if (!PlayerId.IsValid())
		{
			continue;
		}

		TSharedRef<const FUniqueNetIdAccelByteUser> AccelByteCompositeId = FUniqueNetIdAccelByteUser::CastChecked(PlayerId.ToSharedRef());
		AccelByte::FMultiRegistry::RemoveApiClient(AccelByteCompositeId->GetAccelByteId());
	}

	// Reset all of our references to our shared interfaces to effectively destroy them if nothing else is using the memory
	PartyInterface.Reset();
	PresenceInterface.Reset();
	FriendsInterface.Reset();
	UserCloudInterface.Reset();
	UserInterface.Reset();
	ExternalUIInterface.Reset();
	IdentityInterface.Reset();
	SessionInterface.Reset();
	UserCache.Reset();
	AgreementInterface.Reset();
	WalletInterface.Reset();
	EntitlementsInterface.Reset();
	StoreV2Interface.Reset();
	PurchaseInterface.Reset();
	TimeInterface.Reset();
	AnalyticsInterface.Reset();
	StatisticInterface.Reset();
	ChatInterface.Reset();
	AuthInterface.Reset();
	AchievementInterface.Reset();
	VoiceInterface.Reset();
	VoiceChatInterface.Reset();
	
	return true;
}

FString FOnlineSubsystemAccelByte::GetAppId() const
{
	// AccelByte uses a namespace to identify an application on the platform, thus we can return the game namespace as an "app ID"
	return FRegistry::Settings.Namespace;
}

FText FOnlineSubsystemAccelByte::GetOnlineServiceName() const
{
	return NSLOCTEXT("OnlineSubsystemAccelByte", "OnlineServiceName", "AccelByte");
}

IOnlineSessionPtr FOnlineSubsystemAccelByte::GetSessionInterface() const
{
	return SessionInterface;
}

IOnlineFriendsPtr FOnlineSubsystemAccelByte::GetFriendsInterface() const
{
	return FriendsInterface;
}

IOnlineIdentityPtr FOnlineSubsystemAccelByte::GetIdentityInterface() const
{
	return IdentityInterface;
}

IOnlineExternalUIPtr FOnlineSubsystemAccelByte::GetExternalUIInterface() const
{
	return ExternalUIInterface;
}

IOnlineUserPtr FOnlineSubsystemAccelByte::GetUserInterface() const
{
	return UserInterface;
}

IOnlineUserCloudPtr FOnlineSubsystemAccelByte::GetUserCloudInterface() const
{
	return UserCloudInterface;
}

IOnlinePartyPtr FOnlineSubsystemAccelByte::GetPartyInterface() const
{
	return PartyInterface;
}

IOnlinePresencePtr FOnlineSubsystemAccelByte::GetPresenceInterface() const 
{
	return PresenceInterface;
}

IOnlineStoreV2Ptr FOnlineSubsystemAccelByte::GetStoreV2Interface() const 
{
	return StoreV2Interface;
}

IOnlinePurchasePtr FOnlineSubsystemAccelByte::GetPurchaseInterface() const 
{
	return PurchaseInterface;
}

FOnlineUserCacheAccelBytePtr FOnlineSubsystemAccelByte::GetUserCache() const
{
	return UserCache;
}

IOnlineEntitlementsPtr FOnlineSubsystemAccelByte::GetEntitlementsInterface() const
{
	return EntitlementsInterface;
}

IOnlineAchievementsPtr FOnlineSubsystemAccelByte::GetAchievementsInterface() const
{
	return AchievementInterface;
}

FOnlineAgreementAccelBytePtr FOnlineSubsystemAccelByte::GetAgreementInterface() const
{
	return AgreementInterface;
}

FOnlineWalletAccelBytePtr FOnlineSubsystemAccelByte::GetWalletInterface() const
{
	return WalletInterface;
}

FOnlineCloudSaveAccelBytePtr FOnlineSubsystemAccelByte::GetCloudSaveInterface() const
{
	return CloudSaveInterface;
}

IOnlineTimePtr FOnlineSubsystemAccelByte::GetTimeInterface() const
{
	return TimeInterface;
}

FOnlineAnalyticsAccelBytePtr FOnlineSubsystemAccelByte::GetAnalyticsInterface() const
{
	return AnalyticsInterface;
}

IOnlineStatsPtr FOnlineSubsystemAccelByte::GetStatsInterface() const
{
	return StatisticInterface;
}

IOnlineChatPtr FOnlineSubsystemAccelByte::GetChatInterface() const
{
	return ChatInterface;
}

FOnlineAuthAccelBytePtr FOnlineSubsystemAccelByte::GetAuthInterface() const
{
	return AuthInterface;
}

IOnlineVoicePtr FOnlineSubsystemAccelByte::GetVoiceInterface() const
{
	if (!bVoiceInterfaceInitialized)
	{
		if (!StaticCastSharedPtr<FOnlineVoiceAccelByte>(VoiceInterface)->Init())
		{
			VoiceInterface.Reset();
		}
		bVoiceInterfaceInitialized = true;
	}
	return VoiceInterface;
}

IVoiceChatPtr FOnlineSubsystemAccelByte::GetVoiceChatInterface()
{
	return VoiceChatInterface;
}

#if (ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION <= 25)
IOnlineTurnBasedPtr FOnlineSubsystemAccelByte::GetTurnBasedInterface() const
{
	return nullptr;
}

IOnlineTournamentPtr FOnlineSubsystemAccelByte::GetTournamentInterface() const
{
	return nullptr;
}
#endif

bool FOnlineSubsystemAccelByte::IsAutoConnectLobby() const
{
	return bIsAutoLobbyConnectAfterLoginSuccess;
}

bool FOnlineSubsystemAccelByte::IsAutoConnectChat() const
{
	return bIsAutoChatConnectAfterLoginSuccess;
}

bool FOnlineSubsystemAccelByte::IsMultipleLocalUsersEnabled() const
{
	return bIsMultipleLocalUsersEnabled;
}

bool FOnlineSubsystemAccelByte::Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar)
{
	bool bWasHandled = false;

	// Keeping in line with the Util exec tests, we want to check if we have the keyword test, and if so try and spawn a
	// test for specific interface methods based on this
	if (FParse::Command(&Cmd, TEXT("TEST")))
	{
	// OnlineSubsystemUtils uses this macro to check whether we can run test commands, keep with that pattern
#if WITH_DEV_AUTOMATION_TESTS
		if (FParse::Command(&Cmd, TEXT("USER")) && UserInterface.IsValid())
		{
			bWasHandled = UserInterface->TestExec(InWorld, Cmd, Ar);
		}
#endif
	}
	
	// If we didn't handle any exec tests, then just pass handling to the super method
	if (!bWasHandled)
	{
		bWasHandled = FOnlineSubsystemImpl::Exec(InWorld, Cmd, Ar);
	}

	return bWasHandled;
}

bool FOnlineSubsystemAccelByte::IsEnabled() const
{
	return FOnlineSubsystemImpl::IsEnabled();
}

bool FOnlineSubsystemAccelByte::Tick(float DeltaTime) 
{
	if (!FOnlineSubsystemImpl::Tick(DeltaTime))
	{
		return false;
	}
	
	if (AsyncTaskManager)
	{
		AsyncTaskManager->GameTick();
	}

	if (SessionInterface.IsValid())
	{
		SessionInterface->Tick(DeltaTime);
	}

	if (AuthInterface.IsValid())
	{
		AuthInterface->Tick(DeltaTime);
	}

	if (bVoiceInterfaceInitialized && VoiceInterface.IsValid())
	{
		VoiceInterface->Tick(DeltaTime);
	}

	// If we have automation testing enabled, check if we have any exec tests that are complete and if so, remove them
#if WITH_DEV_AUTOMATION_TESTS
	ActiveExecTests.RemoveAll([](const TSharedPtr<FExecTestBase>& ExecTest) { return ExecTest->bIsComplete; });
#endif

	if (LogoutDelegate.IsBound())
	{
		LogoutDelegate.ExecuteIfBound();
	}
	return true;
}

bool FOnlineSubsystemAccelByte::IsNativeSubsystemSupported(const FName& NativeSubsystemName)
{
	// Convert the subsystem FName to a string and compare to OSSes we know to support
	const FString SubsystemStr = NativeSubsystemName.ToString();
	return SubsystemStr.Equals(TEXT("GDK"), ESearchCase::IgnoreCase) ||
		SubsystemStr.Equals(TEXT("Live"), ESearchCase::IgnoreCase) ||
		SubsystemStr.Equals(TEXT("PS4"), ESearchCase::IgnoreCase) ||
		SubsystemStr.Equals(TEXT("PS5"), ESearchCase::IgnoreCase) ||
		SubsystemStr.Equals(TEXT("STEAM"), ESearchCase::IgnoreCase);
}

FString FOnlineSubsystemAccelByte::GetNativePlatformNameString()
{
	return GetNativePlatformName().ToString();
}

FName FOnlineSubsystemAccelByte::GetNativePlatformName()
{
	IOnlineSubsystem* NativeSubsystem = IOnlineSubsystem::GetByPlatform();
	if (NativeSubsystem == nullptr)
	{
		return FName(TEXT(""));
	}

	return NativeSubsystem->GetSubsystemName();
}

FString FOnlineSubsystemAccelByte::GetNativeAppId()
{
	IOnlineSubsystem* NativeSubsystem = IOnlineSubsystem::GetByPlatform();
	if (NativeSubsystem == nullptr)
	{
		return TEXT("");
	}

	return NativeSubsystem->GetAppId();
}

bool FOnlineSubsystemAccelByte::GetAccelBytePlatformTypeFromAuthType(const FString& InAuthType, EAccelBytePlatformType& Result)
{
	if (InAuthType.Equals(TEXT("STEAM"), ESearchCase::IgnoreCase))
	{
		Result = EAccelBytePlatformType::Steam;
		return true;
	}
	else if (InAuthType.Equals(TEXT("PS4"), ESearchCase::IgnoreCase))
	{
		Result = EAccelBytePlatformType::PS4CrossGen;
		return true;
	}
	else if (InAuthType.Equals(TEXT("PS5"), ESearchCase::IgnoreCase))
	{
		Result = EAccelBytePlatformType::PS5;
		return true;
	}
	else if (InAuthType.Equals(TEXT("LIVE"), ESearchCase::IgnoreCase) || InAuthType.Equals(TEXT("GDK"), ESearchCase::IgnoreCase))
	{
		Result = EAccelBytePlatformType::Live;
		return true;
	}
	return false;
}

FString FOnlineSubsystemAccelByte::GetAccelBytePlatformStringFromAuthType(const FString& InAuthType)
{
	if (InAuthType.Equals(TEXT("steam"), ESearchCase::IgnoreCase))
	{
		return TEXT("steam");
	}
	else if (InAuthType.Equals(TEXT("ps4"), ESearchCase::IgnoreCase))
	{
		return TEXT("ps4");
	}
	else if (InAuthType.Equals(TEXT("ps5"), ESearchCase::IgnoreCase))
	{
		return TEXT("ps5");
	}
	else if (InAuthType.Equals(TEXT("live"), ESearchCase::IgnoreCase) || InAuthType.Equals(TEXT("gdk"), ESearchCase::IgnoreCase))
	{
		return TEXT("live");
	}
	return TEXT("");
}

FString FOnlineSubsystemAccelByte::GetNativeSubsystemNameFromAccelBytePlatformString(const FString& InAccelBytePlatform)
{
	if (InAccelBytePlatform.Equals(TEXT("steam"), ESearchCase::IgnoreCase))
	{
		return TEXT("STEAM");
	}
	else if (InAccelBytePlatform.Equals(TEXT("ps4"), ESearchCase::IgnoreCase))
	{
		return TEXT("PS4");
	}
	else if (InAccelBytePlatform.Equals(TEXT("ps5"), ESearchCase::IgnoreCase))
	{
		return TEXT("PS5");
	}
	else if (InAccelBytePlatform.Equals(TEXT("live"), ESearchCase::IgnoreCase))
	{
		return TEXT("GDK");
	}
	return TEXT("");
}

FString FOnlineSubsystemAccelByte::GetNativePlatformTypeAsString()
{
	IOnlineSubsystem* NativeSubsystem = IOnlineSubsystem::GetByPlatform();
	if (NativeSubsystem == nullptr)
	{
		return TEXT("");
	}

	return GetAccelBytePlatformStringFromAuthType(GetNativePlatformNameString());
}

FString FOnlineSubsystemAccelByte::GetSimplifiedNativePlatformName()
{
	return GetSimplifiedNativePlatformName(GetNativePlatformNameString());
}

FString FOnlineSubsystemAccelByte::GetSimplifiedNativePlatformName(const FString& PlatformName)
{
	if (PlatformName.Equals(TEXT("gdk"), ESearchCase::IgnoreCase) || PlatformName.Equals(TEXT("live"), ESearchCase::IgnoreCase))
	{
		return TEXT("XBOX");
	}
	else if (PlatformName.Equals(TEXT("ps4"), ESearchCase::IgnoreCase) || PlatformName.Equals(TEXT("ps5"), ESearchCase::IgnoreCase))
	{
		return TEXT("PSN");
	}
	else if (PlatformName.Equals(TEXT("steam"), ESearchCase::IgnoreCase))
	{
		return TEXT("STEAM");
	}

	return PlatformName;
}

void FOnlineSubsystemAccelByte::OnLoginCallback(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error)
{
	if (!bWasSuccessful)
	{
		return;
	}

	// listen to Message Notif Lobby
	const AccelByte::FApiClientPtr ApiClient = IdentityInterface->GetApiClient(LocalUserNum); 
	const AccelByte::Api::Lobby::FMessageNotif Delegate = AccelByte::Api::Lobby::FMessageNotif::CreateRaw(this, &FOnlineSubsystemAccelByte::OnMessageNotif, LocalUserNum);		
	if (!ApiClient.IsValid())
	{
		return;
	}
	
	ApiClient->Lobby.SetMessageNotifDelegate(Delegate);
	if (bIsAutoLobbyConnectAfterLoginSuccess && IdentityInterface.IsValid())
	{
		IdentityInterface->ConnectAccelByteLobby(LocalUserNum);
	}

	if(bIsAutoChatConnectAfterLoginSuccess && ChatInterface.IsValid())
	{
		ChatInterface->Connect(LocalUserNum);
	}
}

void FOnlineSubsystemAccelByte::OnMessageNotif(const FAccelByteModelsNotificationMessage& InMessage, int32 LocalUserNum)
{
	UE_LOG_AB(Verbose, TEXT("Got freeform notification from backend at %s!\nTopic: %s\nPayload: %s"), *InMessage.SentAt.ToString(), *InMessage.Topic, *InMessage.Payload);
}

void FOnlineSubsystemAccelByte::OnLobbyConnectedCallback(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UniqueNetId, const FString& ErrorMessage)
{
	AccelByte::FApiClientPtr ApiClient = GetApiClient(LocalUserNum);
	
	auto OnLobbyConnectionClosedDelegate = AccelByte::Api::Lobby::FConnectionClosed::CreateThreadSafeSP(AsShared(), &FOnlineSubsystemAccelByte::OnLobbyConnectionClosed, LocalUserNum);
	ApiClient->Lobby.SetConnectionClosedDelegate(OnLobbyConnectionClosedDelegate);
	
	// #NOTE (Wiwing): Overwrite connect Lobby success delegate for reconnection
	auto OnLobbyReconnectionDelegate = Api::Lobby::FConnectSuccess::CreateThreadSafeSP(AsShared(), &FOnlineSubsystemAccelByte::OnLobbyReconnected, LocalUserNum);
	ApiClient->Lobby.SetConnectSuccessDelegate(OnLobbyReconnectionDelegate);
}


void FOnlineSubsystemAccelByte::OnLobbyConnectionClosed(int32 StatusCode, const FString& Reason, bool WasClean, int32 InLocalUserNum)
{
	UE_LOG_AB(Warning, TEXT("Lobby connection closed. Reason '%s' Code : '%d'"), *Reason, StatusCode);

	if (!IdentityInterface.IsValid() || !PartyInterface.IsValid())
	{
		UE_LOG_AB(Warning, TEXT("Error due to either IdentityInterface or PartyInterface is invalid"));
		return;
	}

	const TSharedPtr<const FUniqueNetId> UserIdPtr = IdentityInterface->GetUniquePlayerId(InLocalUserNum);
	TSharedPtr<FUserOnlineAccount> UserAccount;

	if (UserIdPtr.IsValid())
	{
		const FUniqueNetId& UserId = UserIdPtr.ToSharedRef().Get();
		UserAccount = IdentityInterface->GetUserAccount(UserId);
	}

	if (UserAccount.IsValid())
	{
		const TSharedPtr<FUserOnlineAccountAccelByte> UserAccountAccelByte = StaticCastSharedPtr<FUserOnlineAccountAccelByte>(UserAccount);
		UserAccountAccelByte->SetConnectedToLobby(false);
	}

	if (FOnlineIdentityAccelByte::IsLogoutRequired(StatusCode) == false)
	{
		return;
	}

	int32 ClosedAbnormally = static_cast<int32>(AccelByte::EWebsocketErrorTypes::LocalClosedAbnormally);
	FString LogoutReason = (StatusCode != ClosedAbnormally) ? Reason : TEXT("network-disconnection");

	AccelByte::FApiClientPtr ApiClient = GetApiClient(InLocalUserNum);
	ApiClient->CredentialsRef->ForgetAll();

#if !AB_USE_V2_SESSIONS
		TSharedPtr<FUniqueNetIdAccelByteUser const> LocalUserId = StaticCastSharedPtr<FUniqueNetIdAccelByteUser const>(IdentityInterface->GetUniquePlayerId(InLocalUserNum));
		PartyInterface->RemovePartyFromInterface(LocalUserId.ToSharedRef());
#endif
	LogoutDelegate.Unbind();
	LogoutDelegate = FLogOutFromInterfaceDelegate::CreateLambda([&, InLocalUserNum, LogoutReason]()
	{
		IdentityInterface->Logout(InLocalUserNum, LogoutReason);
		LogoutDelegate.Unbind();
	});
}

void FOnlineSubsystemAccelByte::OnLobbyReconnected(int32 InLocalUserNum)
{
	UE_LOG_AB(Log, TEXT("Lobby successfully reconnected."));

#if !AB_USE_V2_SESSIONS
	if (IdentityInterface.IsValid() && PartyInterface.IsValid())
	{
		TSharedPtr<FUniqueNetIdAccelByteUser const> LocalUserId = StaticCastSharedPtr<FUniqueNetIdAccelByteUser const>(IdentityInterface->GetUniquePlayerId(InLocalUserNum));

		PartyInterface->RemovePartyFromInterface(LocalUserId.ToSharedRef());
		PartyInterface->RestoreParties(LocalUserId.ToSharedRef().Get(), FOnRestorePartiesComplete());
	}
#endif
}

void FOnlineSubsystemAccelByte::SetLocalUserNumCached(int32 InLocalUserNum)
{
	LocalUserNumCached = InLocalUserNum;
}

int32 FOnlineSubsystemAccelByte::GetLocalUserNumCached()
{
	return LocalUserNumCached;
}

AccelByte::FApiClientPtr FOnlineSubsystemAccelByte::GetApiClient(const FUniqueNetId& NetId)
{
	if (NetId.GetType() != ACCELBYTE_USER_ID_TYPE)
	{
		UE_LOG_AB(Warning, TEXT("Failed to retrieve an API client for user '%s'!"), *NetId.ToDebugString());
		return nullptr;
	}

	// Grab the AccelByte composite user ID passed in to make sure that we're getting the right client
	TSharedRef<const FUniqueNetIdAccelByteUser> AccelByteCompositeId = FUniqueNetIdAccelByteUser::CastChecked(NetId);
	AccelByte::FApiClientPtr ApiClient = AccelByte::FMultiRegistry::GetApiClient(AccelByteCompositeId->GetAccelByteId());
	if (!ApiClient.IsValid())
	{
		UE_LOG_AB(Warning, TEXT("Failed to retrieve an API client for user '%s'!"), *AccelByteCompositeId->ToDebugString());
		return nullptr;
	}

	return ApiClient;
}

AccelByte::FApiClientPtr FOnlineSubsystemAccelByte::GetApiClient(int32 LocalUserNum)
{
	TSharedPtr<const FUniqueNetId> PlayerId = IdentityInterface->GetUniquePlayerId(LocalUserNum);
	AccelByte::FApiClientPtr ApiClient;

	if (PlayerId.IsValid())
	{
		ApiClient = GetApiClient(PlayerId.ToSharedRef().Get());
	}
	else
	{
		UE_LOG_AB(Warning, TEXT("Failed to retrieve an API client because local user num %d is not found!"), LocalUserNum);
	}

	return ApiClient;
}

FString FOnlineSubsystemAccelByte::GetLanguage()
{
	return Language;
}

void FOnlineSubsystemAccelByte::SetLanguage(const FString& InLanguage)
{
	Language = InLanguage;
}

#undef LOCTEXT_NAMESPACE
