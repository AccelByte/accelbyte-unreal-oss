// Copyright (c) 2021 - 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAsyncTaskAccelByteRegisterDedicatedV1Session.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "Core/AccelByteRegistry.h"
#include "GameServerApi/AccelByteServerDSMApi.h"
#include "GameServerApi/AccelByteServerSessionBrowserApi.h"
#include "SocketSubsystem.h"
#include "Core/AccelByteUtilities.h"

FOnlineAsyncTaskAccelByteRegisterDedicatedV1Session::FOnlineAsyncTaskAccelByteRegisterDedicatedV1Session(FOnlineSubsystemAccelByte* const InABInterface, int32 InHostingPlayerNum, FName InSessionName, const FOnlineSessionSettings& InNewSessionSettings, bool InRegisterToSessionBrowser)
	: FOnlineAsyncTaskAccelByte(InABInterface)
	, HostingPlayerNum(InHostingPlayerNum)
	, SessionName(InSessionName)
	, NewSessionSettings(InNewSessionSettings)
	, bRegisterToServerBrowser(InRegisterToSessionBrowser)
{
}

void FOnlineAsyncTaskAccelByteRegisterDedicatedV1Session::Initialize()
{
	Super::Initialize();

	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("SessionName: %s"), *SessionName.ToString());

	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(Subsystem->GetIdentityInterface());
	if (!IdentityInterface.IsValid())
	{
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Failed to register dedicated session with Armada as our identity interface was invalid!"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	// Dedicated server APIs need authentication with client credentials if we are not already authenticated
	//
	// Client authentication is an async process, so we'll have to do that, wait for the delegate result, and
	// from there register either a local server or a server on Armada.
	if (!IdentityInterface->IsServerAuthenticated())
	{		
		const FOnAuthenticateServerComplete OnAuthenticateServerCompleteDelegate = FOnAuthenticateServerComplete::CreateRaw(this, &FOnlineAsyncTaskAccelByteRegisterDedicatedV1Session::OnAuthenticateServerComplete);
		IdentityInterface->AuthenticateAccelByteServer(OnAuthenticateServerCompleteDelegate);

		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Authenticating server with client credentials for session '%s'."), *SessionName.ToString());
		return;
	}

	// Attempt to register either a local dedicated or Armada hosted dedicated server
	//
	// @todo I imagine there are cases that we'd want player-hosted dedicated servers that can be found with a session
	// browser, which is currently unsupported.
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Registering dedicated server with Armada"));
	RegisterAccelByteDedicatedServer();
}

void FOnlineAsyncTaskAccelByteRegisterDedicatedV1Session::Finalize()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bWasSuccessful: %s"), LOG_BOOL_FORMAT(bWasSuccessful));

	if (bWasSuccessful)
	{
		const IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();

		// @sessions Note that by this point, since sessions are managed by the matchmaker, we don't have a session associated with this
		// server. Thus we just want to add the session without an associated ID, and once we get our first RegisterPlayer call,
		// associate the session ID	then.
		FNamedOnlineSession* Session = SessionInterface->GetNamedSession(SessionName);
		if (Session == nullptr)
		{
			AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Could not finish creating a dedicated session as a session with the name '%s' was not found!"), *SessionName.ToString());
			return;
		}

		Session->SessionState = EOnlineSessionState::Pending;
		Session->SessionSettings.Set(SETTING_SESSION_LOCAL, bIsLocal, EOnlineDataAdvertisementType::ViaOnlineService);
		Session->SessionSettings.Set(SETTING_SESSION_SERVER_NAME, ServerName, EOnlineDataAdvertisementType::ViaOnlineService);

		TSharedPtr<FOnlineSessionInfoAccelByteV1> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV1>(Session->SessionInfo);
		if (!SessionInfo.IsValid())
		{
			AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Could not finish creating as session '%s' has invalid session info instance!"), *SessionName.ToString());
			return;
		}

		if (!SessionInfo->GetHostAddr().IsValid())
		{
			AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Could not finish creating as session '%s' has an invalid HostAddr instance in session info!"), *SessionName.ToString());
			return;
		}

		bool bIsIpValid = false;
		SessionInfo->GetHostAddr()->SetIp(*RegisteredIpAddress, bIsIpValid);
		SessionInfo->GetHostAddr()->SetPort(RegisteredPort);
	}

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRegisterDedicatedV1Session::TriggerDelegates()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	const IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
	SessionInterface->TriggerOnCreateSessionCompleteDelegates(SessionName, bWasSuccessful);

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRegisterDedicatedV1Session::OnAuthenticateServerComplete(bool bAuthenticationSuccessful)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("bAuthenticationSuccessful: %s"), LOG_BOOL_FORMAT(bAuthenticationSuccessful))

	if (!bAuthenticationSuccessful)
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to register server as we failed to authenticate with our client credentials!"));
		CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		return;
	}

	RegisterAccelByteDedicatedServer();

	AB_OSS_ASYNC_TASK_TRACE_END(TEXT(""));
}

void FOnlineAsyncTaskAccelByteRegisterDedicatedV1Session::RegisterAccelByteDedicatedServer()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	// First, set our registered IP and port based on server type
	GetRegisterIpAddress(RegisteredIpAddress, RegisteredPort);
	
	// Armada servers always have a POD_NAME environment variable set. With that in mind, we want to check for the
	// existence of this environment variable. If it exists and is set, then we move on to DSM registration with the
	// Armada pod. If this is not the case, then we want to register a local dedicated server with Armada that we can
	// quickly match with.
	const FString PodName = FPlatformMisc::GetEnvironmentVariable(TEXT("POD_NAME"));
	if (!PodName.IsEmpty())
	{
		ServerName = PodName;

		FVoidHandler OnServerRegisterSuccess = FVoidHandler::CreateLambda([this]() {
			UE_LOG_AB(Log, TEXT("Successfully registered local server to DSM! SessionName: %s, register server session %d"), *SessionName.ToString(), bRegisterToServerBrowser);
			if(!bRegisterToServerBrowser)
			{
				CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
			}
			else
			{
				CreateGameSession();
			}
		});

		FErrorHandler OnServerRegisterError = FErrorHandler::CreateLambda([this](int32 ErrorCode, const FString& ErrorMessage) {
			UE_LOG_AB(Error, TEXT("Failed to register local server to DSM! SessionName: %s; Error code: %d; Error message: %s"), *SessionName.ToString(), ErrorCode, *ErrorMessage);
			CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		});

		FRegistry::ServerDSM.RegisterServerToDSM(RegisteredPort, OnServerRegisterSuccess, OnServerRegisterError);
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Sent off async task to register managed server to dedicated server manager for session '%s'!"), *SessionName.ToString());
	}
	else
	{
		bIsLocal = true;
		
		// Attempt to set the server name to be the server name set as a command line parameter. If this parameter is not
		// set, then we want to fall back to a reasonable default, in this case that will be the PC name for the user
		// running the server.
		if (!FParse::Value(FCommandLine::Get(), TEXT("-ServerName="), ServerName))
		{
			ServerName = FPlatformProcess::ComputerName();
		}

		FVoidHandler OnLocalServerRegisterSuccess = FVoidHandler::CreateLambda([this]() {
			UE_LOG_AB(Log, TEXT("Successfully registered local server to DSM! SessionName: %s, register server session %d"), *SessionName.ToString(), bRegisterToServerBrowser);
			if(!bRegisterToServerBrowser)
			{
				CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
			}
			else
			{
				CreateGameSession();
			}
		});

		FErrorHandler OnLocalServerRegisterError = FErrorHandler::CreateLambda([this](int32 ErrorCode, const FString& ErrorMessage) {
			UE_LOG_AB(Error, TEXT("Failed to register local server to DSM! SessionName: %s; Error code: %d; Error message: %s"), *SessionName.ToString(), ErrorCode, *ErrorMessage);
			CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
		});

		FRegistry::ServerDSM.RegisterLocalServerToDSM(RegisteredIpAddress, RegisteredPort, ServerName, OnLocalServerRegisterSuccess, OnLocalServerRegisterError);
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Sent off async task to register local server to dedicated server manager for session '%s'!"), *SessionName.ToString());
	}
}

void FOnlineAsyncTaskAccelByteRegisterDedicatedV1Session::GetRegisterIpAddress(FString& IpString, int32& Port)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

#if !(ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 26)
	// First grab our current port, as we will always need to grab that from the World URL. Wish there was a better way to
	// grab current port for a server, but this is the best we have...
	UWorld* World = GEngine->GetCurrentPlayWorld();
	if (World == nullptr)
	{
		AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Could not get world instance to get currently bound listen port for local dedicated server! SessionName: %s"), *SessionName.ToString());
		return;
	}

	Port = World->URL.Port;
#else
	Port = 7777; // #TODO(Maxwell): Find a better way to get currently bound port in <4.26...
#endif

	// Check if we are a local server first, if we aren't then it's as simple as using our IP environment variable and the
	// port from the current play world URL
	const bool bIsLocalServer = FPlatformMisc::GetEnvironmentVariable(TEXT("POD_NAME")).IsEmpty();
	if (bIsLocalServer)
	{
		// Grab the socket subsystem to grab our local host address, this way we can register with an IP that can be connected
		// to anywhere on the local network, in case you want to run a server on one machine locally and then play on a different
		// machine on the same network.
		ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
		if (SocketSubsystem == nullptr)
		{
			AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to get socket subsystem to retrieve host address for session '%s'!"), *SessionName.ToString());
			return;
		}

		bool bCanBindAll;
		TSharedPtr<FInternetAddr> LocalIP = SocketSubsystem->GetLocalHostAddr(*GLog, bCanBindAll);
		if (!LocalIP.IsValid())
		{
			AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Warning, TEXT("Failed to retrieve host address for session '%s'!"), *SessionName.ToString());
			return;
		}

		IpString = LocalIP->ToString(false);

		// To handle a specified serverIP provided through localds commandline argument
		// i.e. "-serverip=192.168.x.x" 
		// If args not found, then the value won't be modified
		FAccelByteUtilities::GetValueFromCommandLineSwitch(ACCELBYTE_ARGS_SERVERIP, IpString);
	}
	else
	{
		// Otherwise, just use the IP from the environment variable
		IpString = FPlatformMisc::GetEnvironmentVariable(TEXT("PUBLIC_IP"));
	}
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("IP: %s; Port: %d"), *IpString, Port);
}

void FOnlineAsyncTaskAccelByteRegisterDedicatedV1Session::CreateGameSession()
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	const IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
			
	FNamedOnlineSession* Session = SessionInterface->GetNamedSession(SessionName);
	if (Session == nullptr)
	{
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Could not finish creating a dedicated session as a session with the name '%s' was not found!"), *SessionName.ToString());
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}
	
	const auto Setting = Session->SessionSettings;
	FString GameMapName;
	const FString GameVersion = FString::Printf(TEXT("%d"), GetBuildUniqueId());
	int GameNumBot = 0;
	int MaxSpectator = 0;
	Setting.Get(SETTING_GAMEMODE, GameMode);
	Setting.Get(SETTING_MAPNAME, GameMapName);
	Setting.Get(SETTING_NUMBOTS, GameNumBot);

#if !(ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 26)
	Setting.Get(SETTING_MAXSPECTATORS, MaxSpectator);
#endif
	
	if(GameMode.IsEmpty())
	{
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Could not finish creating a dedicated session as a session with the name '%s' game mode is empty!"), *SessionName.ToString());
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	if(Setting.NumPublicConnections <= 0)
	{
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Could not finish creating a dedicated session as a session with the name '%s' public connection must be greater than 0!"), *SessionName.ToString());
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	if(GameMapName.IsEmpty())
	{
		AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Could not finish creating a dedicated session as a session with the name '%s' map name is empty!"), *SessionName.ToString());
		CompleteTask(EAccelByteAsyncTaskCompleteState::InvalidState);
		return;
	}

	auto SettingJson = MakeShared<FJsonObject>();    	
	for(const auto &Set : Setting.Settings)
	{
		auto const &Data = Set.Value.Data;
		auto JsonShared = Set.Value.Data.ToJson();
		SettingJson->SetField(Set.Key.ToString(), JsonShared->TryGetField(TEXT("value")));
	}

	AccelByte::THandler<FAccelByteModelsSessionBrowserData> OnSessionCreateSuccessDelegate =
		AccelByte::THandler<FAccelByteModelsSessionBrowserData>::CreateRaw(
			this,
			&FOnlineAsyncTaskAccelByteRegisterDedicatedV1Session::OnSessionCreateSuccess
		);

	AccelByte::FErrorHandler OnSessionCreateFailedDelegate = AccelByte::FErrorHandler::CreateLambda(
		[this, Session](int32 ErrorCode, const FString& ErrorMessage)
			{
				AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Could not start create server session, creating server session failed with code %d message %s!"), ErrorCode, *ErrorMessage);
				CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
				return;
			});

	AccelByte::FRegistry::ServerSessionBrowser.CreateGameSession(
		EAccelByteSessionType::dedicated,
		GameMode,
		GameMapName,
		GameVersion,
		GameNumBot,
		Setting.NumPublicConnections,
		MaxSpectator,
		"",
		SettingJson,
		OnSessionCreateSuccessDelegate,
		OnSessionCreateFailedDelegate);
	
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Sent off async task to create dedicated game session"));
}

void FOnlineAsyncTaskAccelByteRegisterDedicatedV1Session::OnSessionCreateSuccess(const FAccelByteModelsSessionBrowserData& Data)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT("SessionName: %s"), *SessionName.ToString());
	RegisterCreatedGameSession(Data.Session_id);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Sent off async task to register created dedicated game session"));
}

void FOnlineAsyncTaskAccelByteRegisterDedicatedV1Session::RegisterCreatedGameSession(FString SessionId)
{
	AB_OSS_ASYNC_TASK_TRACE_BEGIN(TEXT(""));

	const auto OnSessionRegisterSuccess = THandler<FAccelByteModelsServerCreateSessionResponse>::CreateLambda(
		[this](const FAccelByteModelsServerCreateSessionResponse& Result)
		{
			UE_LOG_AB(Log, TEXT("Successfully registered dedicated server session with name: %s"), *SessionName.ToString());
			CompleteTask(EAccelByteAsyncTaskCompleteState::Success);
			return;
		});

	const auto OnSessionRegisterFailed = AccelByte::FErrorHandler::CreateLambda(
		[this](int32 ErrorCode, const FString& ErrorMessage)
			{
				AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Could not start create server session, server session registration failed with code %d message %s!"), ErrorCode, *ErrorMessage);
				CompleteTask(EAccelByteAsyncTaskCompleteState::RequestFailed);
				return;
			});
	
	AccelByte::FRegistry::ServerDSM.RegisterServerGameSession(SessionId, GameMode, OnSessionRegisterSuccess, OnSessionRegisterFailed);
	AB_OSS_ASYNC_TASK_TRACE_END(TEXT("Sent off async task to register created dedicated game session"));
}
