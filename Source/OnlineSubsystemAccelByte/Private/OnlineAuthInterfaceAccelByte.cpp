// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAuthInterfaceAccelByte.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "OnlineSessionInterfaceV1AccelByte.h"
#include "OnlineSessionInterfaceV2AccelByte.h"
#include "OnlinePartyInterfaceAccelByte.h"
#include "Core/AccelByteUtilities.h"

#include "AsyncTasks/Auth/OnlineAsyncTaskAccelByteJwks.h"
#include "AsyncTasks/Auth/OnlineAsyncTaskAccelByteAuthUser.h"
#include "Engine/NetConnection.h"
#include "JsonUtilities.h"
#include "Misc/Base64.h"

// Headers needed to kick users.
#include "GameFramework/GameModeBase.h"
#include "GameFramework/GameSession.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/PlayerController.h"

// Determining if we need to be disabled or not
#include "Misc/ConfigCacheIni.h"
#include "Misc/AES.h"
#include "PacketHandler.h"

using namespace UE;
using AccelByte::FErrorHandler;

#define ACCELBYTE_KICK_INTERVAL 1.0
#define ACCELBYTE_PENDING_KICK_TIMEOUT 10.0

FOnlineAuthAccelByte::FOnlineAuthAccelByte(FOnlineSubsystemAccelByte* InSubsystem) :
	OnlineSubsystem(InSubsystem),
	bEnabled(false),
	LastTimestamp(0.0),
	bRequestJwks(false),
	SessionInterface(nullptr)
{
	const FString AccelByteModuleName(TEXT("AuthHandlerComponentAccelByte"));
	if (!PacketHandler::DoesAnyProfileHaveComponent(AccelByteModuleName))
	{
		// Pull the components to see if there's anything we can use.
		TArray<FString> ComponentList;
		GConfig->GetArray(TEXT("PacketHandlerComponents"), TEXT("Components"), ComponentList, GEngineIni);

		// Check if AccelByte Authentication is enabled anywhere.
		for (FString CompStr : ComponentList)
		{
			if (CompStr.Contains(AccelByteModuleName))
			{
				bEnabled = true;
				break;
			}
		}
	}
	else
	{
		bEnabled = true;
	}

	if (IsSessionAuthEnabled())
	{
		LastTimestamp = FPlatformTime::Seconds();

		UE_LOG_AB(Log, TEXT("AUTH: AccelByte Authentication Enabled."));

#if AB_USE_V2_SESSIONS
		SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(OnlineSubsystem->GetSessionInterface());
#else
		SessionInterface = StaticCastSharedPtr<FOnlineSessionV1AccelByte>(OnlineSubsystem->GetSessionInterface());
#endif //#if AB_USE_V2_SESSIONS
		if (!SessionInterface.IsValid())
		{
			UE_LOG_AB(Warning, TEXT("AUTH: AccelByte Authentication Disabled. (Session interface is null.)"));
			bEnabled = false;
		}
	}
}

FOnlineAuthAccelByte::FOnlineAuthAccelByte() :
	OnlineSubsystem(nullptr),
	bEnabled(false),
	LastTimestamp(0.0),
	bRequestJwks(false),
	SessionInterface(nullptr)
{
}

FOnlineAuthAccelByte::~FOnlineAuthAccelByte()
{
	AuthUsers.Empty();
}

bool FOnlineAuthAccelByte::IsSessionValid() const
{
	if (IsSessionAuthEnabled() && SessionInterface.IsValid())
	{
		const FNamedOnlineSession* NamedSessionPtr = SessionInterface->GetNamedSession(NAME_GameSession);
		if (NamedSessionPtr != nullptr)
		{
			TSharedPtr<class FOnlineSessionInfo> SessionInfo = NamedSessionPtr->SessionInfo;
			if (SessionInfo != nullptr)
			{
				return SessionInfo->IsValid();
			}
		}
	}

	return false;
}

FOnlineAuthAccelByte::SharedAuthUserPtr FOnlineAuthAccelByte::GetUser(const FString& InUserId)
{
	if (SharedAuthUserPtr* AuthUserPtr = AuthUsers.Find(*InUserId))
	{
		return *AuthUserPtr;
	}

	UE_LOG_AB(Warning, TEXT("AUTH: (%s) Trying to fetch user '%s' (%d) entry but the user does not exist."), (IsServer() ? TEXT("DS") : TEXT("CL")), *InUserId, AuthUsers.Num());
	return nullptr;
}

FOnlineAuthAccelByte::SharedAuthUserPtr FOnlineAuthAccelByte::GetOrCreateUser(const FString& InUserId)
{
	if (SharedAuthUserPtr* AuthUserPtr = AuthUsers.Find(*InUserId))
	{
		return *AuthUserPtr;
	}

	SharedAuthUserPtr AuthUserPtr = MakeShareable(new FAuthUser);
	AuthUsers.Add(InUserId, AuthUserPtr);

	return AuthUserPtr;
}

bool FOnlineAuthAccelByte::UpdateJwks()
{
	if (JwkSet.keys.Num() > 0)
	{
		return true;
	}

	if (!bRequestJwks)
	{
		bRequestJwks = true;
		// Create the JWKS in the list if we don't already have them.
		OnlineSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteJwks>(OnlineSubsystem,
			FOnJwksCompleted::CreateLambda([this](const FJwkSet& InJwkSet) {
				OnJwksCompleted(InJwkSet);
				}));
	}

	return false;
}

bool FOnlineAuthAccelByte::VerifyAuthToken(const FString& AuthToken, FString& UserId)
{
	FJwt Jwt(AuthToken);

	// Verify the JWT with the RSA public key
	for (const FJsonObjectWrapper& Key : JwkSet.keys)
	{
		const FRsaPublicKey PublicKey(Key.JsonObject->GetStringField(TEXT("n")), Key.JsonObject->GetStringField(TEXT("e")));
		const EJwtResult VerifyResult = Jwt.VerifyWith(PublicKey);
		if (VerifyResult == EJwtResult::Ok)
		{
			// Extract the userId from the payload
			TSharedPtr<FJsonObject> Payload = Jwt.Payload();
			if (Payload.IsValid())
			{
				if (Payload->TryGetStringField(TEXT("sub"), UserId))
				{
					// userId successfully extracted from the payload

					SharedAuthUserPtr TargetUser = GetUser(UserId);
					if (!TargetUser.IsValid())
					{
						// If we are missing an user here, this means that they were recently deleted or we never knew about them.
						UE_LOG_AB(Warning, TEXT("AUTH: (%s) Could not find user data on result callback for %s, were they were recently deleted?"), (IsServer() ? TEXT("DS") : TEXT("CL")),
							*UserId);
						return false;
					}

					TArray<FString> Bans;
					if (Payload->TryGetStringArrayField(TEXT("bans"), Bans))
					{
						TargetUser->SetBans(Bans);
					}
					else
					{
						// "bans" field not found in payload
						UE_LOG_AB(Warning, TEXT("AUTH: (%s) The field name 'bans' was not found."), (IsServer() ? TEXT("DS") : TEXT("CL")));
					}
					return true;
				}
				else
				{
					// Handle invalid userId
					UE_LOG_AB(Warning, TEXT("AUTH: (%s) The field name is not 'sub' for userId."), (IsServer() ? TEXT("DS") : TEXT("CL")));
				}
			}
			else
			{
				// Handle invalid Payload
				UE_LOG_AB(Warning, TEXT("AUTH: (%s) Payload is invalid."), (IsServer() ? TEXT("DS") : TEXT("CL")));
			}
		}
		else
		{
			// Handle invalid VerifyResult
			UE_LOG_AB(Warning, TEXT("AUTH: (%s) Fail to verify result(%d)."), (IsServer() ? TEXT("DS") : TEXT("CL")), VerifyResult);
		}
	}

	return false;
}

void FOnlineAuthAccelByte::OnJwksCompleted(const FJwkSet& InJwkSet)
{
	JwkSet = InJwkSet;
	bRequestJwks = false;
}

bool FOnlineAuthAccelByte::AuthenticateUser(const FString& InUserId)
{
	// this is working on a dedicated server.
	if (IsServer() && IsSessionAuthEnabled())
	{
		SharedAuthUserPtr TargetUser = GetUser(InUserId);
		if (TargetUser.IsValid())
		{
			if (IsInSessionUser(InUserId))
			{
				return CheckUserState(InUserId);
			}
			else
			{
				UE_LOG_AB(Warning, TEXT("AUTH: This user (%s) didn't join a session."), *InUserId);
				TargetUser->Status = EAccelByteAuthStatus::KickUser;
				return false;
			}
		}
	}
	else
	{
		UE_LOG_AB(Warning, TEXT("AUTH: (%s) The user (%s) is not in session"), (IsServer() ? TEXT("DS") : TEXT("CL")), *InUserId);
	}

	return false;
}

bool FOnlineAuthAccelByte::CheckUserState(const FString& InUserId)
{
	SharedAuthUserPtr TargetUser = GetUser(InUserId);

	if (!TargetUser.IsValid())
	{
		// If we are missing an user here, this means that they were recently deleted or we never knew about them.
		UE_LOG_AB(Warning, TEXT("AUTH: (%s) Could not find user data on result callback for %s, were they were recently deleted?"), (IsServer() ? TEXT("DS") : TEXT("CL")),
			*InUserId);
		return false;
	}

	if (EnumHasAnyFlags(TargetUser->Status, EAccelByteAuthStatus::HasOrIsPendingAuth))
	{
		UE_LOG_AB(Log, TEXT("AUTH: The user (%s) is currently authenticating.(%f)"), *InUserId, (FPlatformTime::Seconds() - TargetUser->PendingTimestamp));
		return true;
	}

	if (EnumHasAnyFlags(TargetUser->Status, EAccelByteAuthStatus::FailKick))
	{
		UE_LOG_AB(Log, TEXT("AUTH: If the user (%s) has already failed auth, do not attempt to re-auth them."), *InUserId);
		return false;
	}

	GetBanUser(InUserId);

	return true;
}

void FOnlineAuthAccelByte::GetBanUser(const FString& InUserId)
{
	SharedAuthUserPtr TargetUser = GetUser(InUserId);

	if (!TargetUser.IsValid())
	{
		// If we are missing an user here, this means that they were recently deleted or we never knew about them.
		UE_LOG_AB(Warning, TEXT("AUTH: (%s) Could not find user data on result callback for %s, were they were recently deleted?"), (IsServer() ? TEXT("DS") : TEXT("CL")),
			*InUserId);
		return;
	}

	bool bDoubleCheckBanned = true;
	if (SessionInterface.IsValid())
	{
#if AB_USE_V2_SESSIONS
		const FNamedOnlineSession* NamedSession = SessionInterface->GetNamedSession(NAME_GameSession);
		if (NamedSession)
		{
			TSharedPtr<FOnlineSessionInfoAccelByteV2> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(NamedSession->SessionInfo);
			if (ensure(SessionInfo.IsValid()))
			{
				bDoubleCheckBanned = (SessionInfo->GetServerType() == EAccelByteV2SessionConfigurationServerType::DS);
			}
		}
#else
		bDoubleCheckBanned = IsRunningDedicatedServer();
#endif
	}

	if (TargetUser->IsBanned(InUserId))
	{
		UE_LOG_AB(Warning, TEXT("AUTH: (%s) GetBanUser: user(%s) is banned."), (IsServer() ? TEXT("DS") : TEXT("CL")), *InUserId);
		OnAuthUserCompleted(false, InUserId);
		return;
	}

	if (bDoubleCheckBanned)
	{
		GetBanUserInfo(InUserId);
		TargetUser->Status |= EAccelByteAuthStatus::ValidationStarted;
	}
	else
	{
		OnAuthUserCompleted(true, InUserId);
	}
}

void FOnlineAuthAccelByte::GetBanUserInfo(const FString& InUserId)
{
	// Create the user in the list if we don't already have them.
	OnlineSubsystem->CreateAndDispatchAsyncTaskParallel<FOnlineAsyncTaskAccelByteAuthUser>(OnlineSubsystem, InUserId,
		FOnAuthUserCompleted::CreateLambda([this](bool bWasSuccessful, const FString& UserId) {
			OnAuthUserCompleted(bWasSuccessful, UserId);
			}));
}

void FOnlineAuthAccelByte::OnAuthSuccess(const FString& InUserId)
{
	SharedAuthUserPtr TargetUser = GetUser(InUserId);

	if (!TargetUser.IsValid())
	{
		// If we are missing an user here, this means that they were recently deleted or we never knew about them.
		UE_LOG_AB(Warning, TEXT("AUTH: (%s) Could not find user data on result callback for %s, were they were recently deleted?"), (IsServer() ? TEXT("DS") : TEXT("CL")),
			*InUserId);
		return;
	}
	TargetUser->Status &= ~EAccelByteAuthStatus::ValidationStarted;
	TargetUser->Status |= EAccelByteAuthStatus::AuthSuccess;
}

void FOnlineAuthAccelByte::OnAuthUserCompleted(bool bWasSuccessful, const FString& InUserId)
{
	if (bWasSuccessful)
	{
		OnAuthSuccess(InUserId);
	}
	else
	{
		OnAuthFail(InUserId, 0, "Baned User.");
	}
}

void FOnlineAuthAccelByte::OnAuthFail(const FString& InUserId, const int32 InErrorCode, const FString& InErrorMessage)
{
	SharedAuthUserPtr TargetUser = GetUser(InUserId);
	if (!TargetUser.IsValid())
	{
		// If we are missing an user here, this means that they were recently deleted or we never knew about them.
		UE_LOG_AB(Warning, TEXT("AUTH: (%s) Could not find user data on result callback for %s, were they were recently deleted?"), (IsServer() ? TEXT("DS") : TEXT("CL")),
			*InUserId);
		return;
	}

	// Remove the validation start flag
	TargetUser->Status &= ~EAccelByteAuthStatus::ValidationStarted;
	TargetUser->Status |= EAccelByteAuthStatus::AuthFail;

	TargetUser->SetFail(InErrorCode, InErrorMessage);
	UE_LOG_AB(Warning, TEXT("AUTH: (%s) OnAuthFail: (ErrCode:%d) %s"), (IsServer() ? TEXT("DS") : TEXT("CL")), InErrorCode, *InErrorMessage);
}

EAccelByteAuthStatus FOnlineAuthAccelByte::GetAuthStatus(const FString& InUserId)
{
	SharedAuthUserPtr TargetUser = GetUser(InUserId);
	if (!TargetUser.IsValid())
	{
		// If we are missing an user here, this means that they were recently deleted or we never knew about them.
		UE_LOG_AB(Warning, TEXT("AUTH: (%s) Could not find user data on result callback for %s, were they were recently deleted?"), (IsServer() ? TEXT("DS") : TEXT("CL")),
			*InUserId);
		return EAccelByteAuthStatus::AuthFail;
	}
	return TargetUser->Status;
}

void FOnlineAuthAccelByte::MarkUserForKick(const FString& InUserId)
{
	SharedAuthUserPtr TargetUser = GetUser(InUserId);
	if (TargetUser.IsValid())
	{
		TargetUser->Status |= EAccelByteAuthStatus::AuthFail;
		UE_LOG_AB(Warning, TEXT("AUTH: (%s) Marking (%s) user for kick"), (IsServer() ? TEXT("DS") : TEXT("CL")), *InUserId);
		LastTimestamp = FPlatformTime::Seconds();
	}
}

bool FOnlineAuthAccelByte::KickUser(const FString& InUserId, bool bSuppressFailure)
{
	bool bKickSuccess = false;
	SharedAuthUserPtr TargetUser = GetUser(InUserId);
	if (!TargetUser.IsValid())
	{
		// If we are missing an user here, this means that they were recently deleted or we never knew about them.
		UE_LOG_AB(Warning, TEXT("AUTH: (%s) Could not find user data on result callback for %s, were they were recently deleted?"), (IsServer() ? TEXT("DS") : TEXT("CL")),
			*InUserId);
		return false;
	}

	// Create a new user ID instance for the user that we just logged in as
	FAccelByteUniqueIdComposite CompositeId;
	CompositeId.Id = InUserId;

	// Add platform information to composite ID for login
	CompositeId.PlatformType = "";
	CompositeId.PlatformId = "";

	TSharedPtr<const FUniqueNetIdAccelByteUser> KickUserId = FUniqueNetIdAccelByteUser::Create(CompositeId);
	if (!KickUserId.IsValid())
	{
		bKickSuccess = true;
	}
	else
	{
		if (GWorld)
		{
			for (FConstPlayerControllerIterator Itr = GWorld->GetPlayerControllerIterator(); Itr; ++Itr)
			{
				APlayerController* PC = Itr->Get();
				if (PC && PC->PlayerState != nullptr && PC->PlayerState->GetUniqueId().IsValid() &&
					*(PC->PlayerState->GetUniqueId().GetUniqueNetId()) == KickUserId->AsShared().Get())
				{
					if (UNetConnection* NetConnection = PC->GetNetConnection())
					{
						NetConnection->CleanUp();
						bKickSuccess = true;
						break;
					}
				}
			}
		}

		if (SessionInterface.IsValid())
		{
			const FNamedOnlineSession* NamedSession = SessionInterface->GetNamedSession(NAME_GameSession);
			if (NamedSession)
			{
				SessionInterface->UnregisterPlayer(NamedSession->SessionName, *KickUserId);
			}
		}
	}

	// If we were able to kick them properly, call to remove their data.
	// Otherwise, they'll be attempted to be kicked again later.
	if (bKickSuccess)
	{
		UE_LOG_AB(Log, TEXT("AUTH: (%s) Successfully kicked the user %s"), (IsServer() ? TEXT("DS") : TEXT("CL")), *InUserId);
		RemoveUser(InUserId);
	}

	return bKickSuccess;
}

void FOnlineAuthAccelByte::RemoveUser(const FString& InTargetUser)
{
	if (!IsServer() || !IsSessionAuthEnabled())
	{
		UE_LOG_AB(Warning, TEXT("AUTH: (%s) this is not the server or disabled"), (IsServer() ? TEXT("DS") : TEXT("CL")));
		return;
	}

	AuthUsers.Remove(InTargetUser);
}

bool FOnlineAuthAccelByte::IsInSessionUser(const FString& InUserId)

{
	if (!SessionInterface.IsValid())
	{
		UE_LOG_AB(Warning, TEXT("AUTH: (%s) Session interface is invalid."), (IsServer() ? TEXT("DS") : TEXT("CL")));
		return false;
	}

	SharedAuthUserPtr AuthUser = GetUser(InUserId);
	if (!AuthUser.IsValid())
	{
		// If we are missing an user here, this means that they were recently deleted or we never knew about them.
		UE_LOG_AB(Warning, TEXT("AUTH: (%s) The user(%s) has something wrong."), (IsServer() ? TEXT("DS") : TEXT("CL")), *InUserId);
		return false;
	}

	const FNamedOnlineSession* NamedSession = SessionInterface->GetNamedSession(NAME_GameSession);
	if (NamedSession == nullptr)
	{
		// If session is not supported, 'true' is returned as only AccessToken is verified.
		return true;
	}

#if AB_USE_V2_SESSIONS
	TSharedPtr<FOnlineSessionInfoAccelByteV2> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(NamedSession->SessionInfo);
	if (!ensure(SessionInfo.IsValid()))
	{
		if (!EnumHasAnyFlags(AuthUser->Status, EAccelByteAuthStatus::PendingSession))
		{
			AuthUser->Status |= EAccelByteAuthStatus::PendingSession;
			AuthUser->PendingTimestamp = FPlatformTime::Seconds();
			UE_LOG_AB(Warning, TEXT("AUTH: (%s) The user(%s) is pending for session information."), (IsServer() ? TEXT("DS") : TEXT("CL")), *InUserId);
		}
		return true;
	}

	if (SessionInfo->GetServerType() != EAccelByteV2SessionConfigurationServerType::DS)
	{
		AuthUser->Status &= ~EAccelByteAuthStatus::PendingSession;
		return true;
	}
#else
	if (!IsRunningDedicatedServer()) {
		return true;
	}
#endif

	TArray< FUniqueNetIdRef > PartyMembers = NamedSession->RegisteredPlayers;
	for (auto Member : PartyMembers)
	{
		if (Member.Get().IsValid())
		{
			TSharedRef<const FUniqueNetIdAccelByteUser> Player = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(Member);
			if (Player->GetAccelByteId() == InUserId)
			{
				// Remove the pending session flag
				AuthUser->Status &= ~EAccelByteAuthStatus::PendingSession;
				return true;
			}
		}
	}

	if (!EnumHasAnyFlags(AuthUser->Status, EAccelByteAuthStatus::PendingSession))
	{
		AuthUser->Status |= EAccelByteAuthStatus::PendingSession;
		AuthUser->PendingTimestamp = FPlatformTime::Seconds();
		UE_LOG_AB(Warning, TEXT("AUTH: (%s) The user(%s) is pending for session information."), (IsServer() ? TEXT("DS") : TEXT("CL")), *InUserId);
	}

	return true;
}

bool FOnlineAuthAccelByte::Tick(float DeltaTime)
{
	if (!bEnabled || !IsServer())
	{
		return true;
	}

	float CurTime = FPlatformTime::Seconds();
	if (LastTimestamp != 0.0)
	{
		if ((CurTime - LastTimestamp) < ACCELBYTE_KICK_INTERVAL)
		{
			return true;
		}
	}

	LastTimestamp = FPlatformTime::Seconds();
	// Loop through all users to detect if we need to do anything regarding resends
	for (AccelByteAuthentications::TIterator It(AuthUsers); It; ++It)
	{
		if (It->Value.IsValid())
		{
			SharedAuthUserPtr CurUser = It->Value;
			const FString& CurUserId = *It->Key;

			// Kick any players that have failed authentication.
			if (EnumHasAnyFlags(CurUser->Status, EAccelByteAuthStatus::FailKick))
			{
				if (KickUser(CurUserId, EnumHasAnyFlags(CurUser->Status, EAccelByteAuthStatus::KickUser)))
				{
					// If we've modified the list, we can just end this frame.
					return true;
				}
				CurUser->Status |= EAccelByteAuthStatus::KickUser;
			}
			else if (EnumHasAnyFlags(CurUser->Status, EAccelByteAuthStatus::PendingSession))
			{
				if (IsInSessionUser(CurUserId))
				{
					if ((LastTimestamp - CurUser->PendingTimestamp) > ACCELBYTE_PENDING_KICK_TIMEOUT)
					{
						CurUser->Status &= ~EAccelByteAuthStatus::PendingSession;
						CurUser->Status |= EAccelByteAuthStatus::KickUser;
					}
					else if (!CheckUserState(CurUserId))
					{
						CurUser->Status &= ~EAccelByteAuthStatus::PendingSession;
						CurUser->Status |= EAccelByteAuthStatus::KickUser;
					}
				}
				else
				{
					CurUser->Status &= ~EAccelByteAuthStatus::PendingSession;
					CurUser->Status |= EAccelByteAuthStatus::KickUser;
				}
			}
		}
	}

	UpdateJwks();

	return true;
}
