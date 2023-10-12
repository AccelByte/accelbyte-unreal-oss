// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "VoiceChat/AccelByteVoiceChat.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "OnlineSubsystemTypes.h"
#include "VoiceChat.h"
#include "Runtime/Launch/Resources/Version.h"

FAccelByteVoiceChat::FAccelByteVoiceChat(FOnlineSubsystemAccelByte* InSubsystem)
	: AccelByteSubsystem(InSubsystem)
{
}

FAccelByteVoiceChat::~FAccelByteVoiceChat()
{
	for (auto ItRemove = LocalVoiceChatUsers.CreateIterator(); ItRemove; ++ItRemove)
	{
		ReleaseUser(ItRemove->Value.Get());
	}
	LocalVoiceChatUsers.Reset();
	VoiceChatUserPtr.Reset();
	VoiceChatPtr = nullptr;
}

bool FAccelByteVoiceChat::Initialize(IVoiceChatPtr InVoiceChat)
{
	VoiceChatPtr = InVoiceChat;
	if (!VoiceChatPtr.IsValid())
	{
		return false;
	}
	VoiceChatUserPtr = MakeShareable(VoiceChatPtr->CreateUser());
	return Initialize();
}

IVoiceChatUser* FAccelByteVoiceChat::GetVoiceChatUser(const FUniqueNetId& LocalUserId)
{
	IVoiceChatUser* Result = nullptr;

	if (!VoiceChatPtr.IsValid())
	{
		UE_LOG_AB(Warning, TEXT("VoiceChatPtr is not initialized!"));
		return Result;
	}

	if (AccelByteSubsystem->IsLocalPlayer(LocalUserId))
	{
		if (IVoiceChatUserPtr* UserPtr = LocalVoiceChatUsers.Find(LocalUserId.AsShared()))
		{
			Result = (*UserPtr).Get();
		}
		else
		{
			const IVoiceChatUserPtr User = LocalVoiceChatUsers.Emplace(LocalUserId.AsShared(), MakeShareable(VoiceChatPtr->CreateUser()));
			const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(AccelByteSubsystem->GetIdentityInterface());
			if (IdentityInterface.IsValid())
			{
				FPlatformUserId PlatformId = IdentityInterface->GetPlatformUserIdFromUniqueNetId(LocalUserId);
				FString PlayerName = IdentityInterface->GetPlayerNickname(LocalUserId);
				User->Login(PlatformId, PlayerName, FString(), FOnVoiceChatLoginCompleteDelegate());
				Result = User.Get();
			}
		}
	}

	return Result;
}

void FAccelByteVoiceChat::ReleaseUser(const FUniqueNetId& LocalUserId)
{
	if (!VoiceChatPtr.IsValid())
	{
		return;
	}
	if (IVoiceChatUserPtr* UserPtr = LocalVoiceChatUsers.Find(LocalUserId.AsShared()))
	{
		VoiceChatPtr->ReleaseUser((*UserPtr).Get());
		LocalVoiceChatUsers.Remove(LocalUserId.AsShared());
	}
}

IVoiceChatUserPtr FAccelByteVoiceChat::GetVoiceChatUser() const
{
	return VoiceChatUserPtr;
}

bool FAccelByteVoiceChat::Initialize()
{
	if (!VoiceChatPtr.IsValid())
	{
		//Voice chat is not set!!!!
		return false;
	}
	return VoiceChatPtr->Initialize();
}

void FAccelByteVoiceChat::Initialize(const FOnVoiceChatInitializeCompleteDelegate& Delegate)
{
	if (!VoiceChatPtr.IsValid())
	{
		return;
	}
	VoiceChatPtr->Initialize(Delegate);
}

bool FAccelByteVoiceChat::Uninitialize()
{
	if (!VoiceChatPtr.IsValid())
	{
		return false;
	}
	return VoiceChatPtr->Uninitialize();
}

void FAccelByteVoiceChat::Uninitialize(const FOnVoiceChatUninitializeCompleteDelegate& Delegate)
{
	if (!VoiceChatPtr.IsValid())
	{
		return;
	}
	VoiceChatPtr->Uninitialize(Delegate);
}

bool FAccelByteVoiceChat::IsInitialized() const
{
	if (!VoiceChatPtr.IsValid())
	{
		return false;
	}
	return VoiceChatPtr->Uninitialize();
}

void FAccelByteVoiceChat::Connect(const FOnVoiceChatConnectCompleteDelegate& Delegate)
{
	if (!VoiceChatPtr.IsValid())
	{
		return;
	}
	VoiceChatPtr->Connect(Delegate);
}

void FAccelByteVoiceChat::Disconnect(const FOnVoiceChatDisconnectCompleteDelegate& Delegate)
{
	if (!VoiceChatPtr.IsValid())
	{
		return;
	}
	VoiceChatPtr->Disconnect(Delegate);
}

bool FAccelByteVoiceChat::IsConnecting() const
{
	if (!VoiceChatPtr.IsValid())
	{
		return false;
	}
	return VoiceChatPtr->IsConnecting();
}

bool FAccelByteVoiceChat::IsConnected() const
{
	if (!VoiceChatPtr.IsValid())
	{
		return false;
	}
	return VoiceChatPtr->IsConnected();
}

IVoiceChatUser* FAccelByteVoiceChat::CreateUser()
{
	if (!VoiceChatPtr.IsValid())
	{
		return nullptr;
	}
	return VoiceChatPtr->CreateUser();
}

void FAccelByteVoiceChat::ReleaseUser(IVoiceChatUser* VoiceChatUser)
{
	if (!VoiceChatPtr.IsValid())
	{
		return;
	}
	VoiceChatPtr->ReleaseUser(VoiceChatUser);
}

#pragma region IVoiceChatUser
void FAccelByteVoiceChat::SetSetting(const FString& Name, const FString& Value)
{
	GetVoiceChatUser()->SetSetting(Name, Value);
}

FString FAccelByteVoiceChat::GetSetting(const FString& Name)
{
	return GetVoiceChatUser()->GetSetting(Name);
}

void FAccelByteVoiceChat::SetAudioInputVolume(float Volume)
{
	GetVoiceChatUser()->SetAudioInputVolume(Volume);
}

void FAccelByteVoiceChat::SetAudioOutputVolume(float Volume)
{
	GetVoiceChatUser()->SetAudioOutputVolume(Volume);
}

float FAccelByteVoiceChat::GetAudioInputVolume() const
{
	return GetVoiceChatUser()->GetAudioInputVolume();
}

float FAccelByteVoiceChat::GetAudioOutputVolume() const
{
	return GetVoiceChatUser()->GetAudioOutputVolume();
}

void FAccelByteVoiceChat::SetAudioInputDeviceMuted(bool bIsMuted)
{
	GetVoiceChatUser()->SetAudioInputDeviceMuted(bIsMuted);
}

void FAccelByteVoiceChat::SetAudioOutputDeviceMuted(bool bIsMuted)
{
	GetVoiceChatUser()->SetAudioOutputDeviceMuted(bIsMuted);
}

bool FAccelByteVoiceChat::GetAudioInputDeviceMuted() const
{
	return GetVoiceChatUser()->GetAudioInputDeviceMuted();
}

bool FAccelByteVoiceChat::GetAudioOutputDeviceMuted() const
{
	return GetVoiceChatUser()->GetAudioOutputDeviceMuted();
}

TArray<FVoiceChatDeviceInfo> FAccelByteVoiceChat::GetAvailableInputDeviceInfos() const
{
	return GetVoiceChatUser()->GetAvailableOutputDeviceInfos();
}

TArray<FVoiceChatDeviceInfo> FAccelByteVoiceChat::GetAvailableOutputDeviceInfos() const
{
	return GetVoiceChatUser()->GetAvailableOutputDeviceInfos();
}

FOnVoiceChatAvailableAudioDevicesChangedDelegate& FAccelByteVoiceChat::OnVoiceChatAvailableAudioDevicesChanged()
{
	return GetVoiceChatUser()->OnVoiceChatAvailableAudioDevicesChanged();
}

void FAccelByteVoiceChat::SetInputDeviceId(const FString& InputDeviceId)
{
	GetVoiceChatUser()->SetInputDeviceId(InputDeviceId);
}

void FAccelByteVoiceChat::SetOutputDeviceId(const FString& OutputDeviceId)
{
	GetVoiceChatUser()->SetOutputDeviceId(OutputDeviceId);
}

FVoiceChatDeviceInfo FAccelByteVoiceChat::GetInputDeviceInfo() const
{
	return GetVoiceChatUser()->GetInputDeviceInfo();
}

FVoiceChatDeviceInfo FAccelByteVoiceChat::GetOutputDeviceInfo() const
{
	return GetVoiceChatUser()->GetOutputDeviceInfo();
}

FVoiceChatDeviceInfo FAccelByteVoiceChat::GetDefaultInputDeviceInfo() const
{
	return GetVoiceChatUser()->GetDefaultInputDeviceInfo();
}

FVoiceChatDeviceInfo FAccelByteVoiceChat::GetDefaultOutputDeviceInfo() const
{
	return GetVoiceChatUser()->GetDefaultOutputDeviceInfo();
}

void FAccelByteVoiceChat::Login(FPlatformUserId PlatformId, const FString& PlayerName, const FString& Credentials, const FOnVoiceChatLoginCompleteDelegate& Delegate)
{
	GetVoiceChatUser()->Login(PlatformId, PlayerName, Credentials, Delegate);
}

void FAccelByteVoiceChat::Logout(const FOnVoiceChatLogoutCompleteDelegate& Delegate)
{
	GetVoiceChatUser()->Logout(Delegate);
}

bool FAccelByteVoiceChat::IsLoggingIn() const
{
	return GetVoiceChatUser()->IsLoggingIn();
}

bool FAccelByteVoiceChat::IsLoggedIn() const
{
	return GetVoiceChatUser()->IsLoggedIn();
}

FOnVoiceChatLoggedInDelegate& FAccelByteVoiceChat::OnVoiceChatLoggedIn()
{
	return GetVoiceChatUser()->OnVoiceChatLoggedIn();
}

FOnVoiceChatLoggedOutDelegate& FAccelByteVoiceChat::OnVoiceChatLoggedOut()
{
	return GetVoiceChatUser()->OnVoiceChatLoggedOut();
}

FString FAccelByteVoiceChat::GetLoggedInPlayerName() const
{
	return GetVoiceChatUser()->GetLoggedInPlayerName();
}

void FAccelByteVoiceChat::BlockPlayers(const TArray<FString>& PlayerNames)
{
	GetVoiceChatUser()->BlockPlayers(PlayerNames);
}

void FAccelByteVoiceChat::UnblockPlayers(const TArray<FString>& PlayerNames)
{
	GetVoiceChatUser()->UnblockPlayers(PlayerNames);
}

void FAccelByteVoiceChat::JoinChannel(const FString& ChannelName, const FString& ChannelCredentials, EVoiceChatChannelType ChannelType, const FOnVoiceChatChannelJoinCompleteDelegate& Delegate, TOptional<FVoiceChatChannel3dProperties> Channel3dProperties)
{
	GetVoiceChatUser()->JoinChannel(ChannelName, ChannelCredentials, ChannelType, Delegate, Channel3dProperties);
}

void FAccelByteVoiceChat::LeaveChannel(const FString& Channel, const FOnVoiceChatChannelLeaveCompleteDelegate& Delegate)
{
	GetVoiceChatUser()->LeaveChannel(Channel, Delegate);
}

FOnVoiceChatChannelJoinedDelegate& FAccelByteVoiceChat::OnVoiceChatChannelJoined()
{
	return GetVoiceChatUser()->OnVoiceChatChannelJoined();
}

FOnVoiceChatChannelExitedDelegate& FAccelByteVoiceChat::OnVoiceChatChannelExited()
{
	return GetVoiceChatUser()->OnVoiceChatChannelExited();
}

FOnVoiceChatCallStatsUpdatedDelegate& FAccelByteVoiceChat::OnVoiceChatCallStatsUpdated()
{
	return GetVoiceChatUser()->OnVoiceChatCallStatsUpdated();
}

void FAccelByteVoiceChat::Set3DPosition(const FString& ChannelName, const FVector& SpeakerPosition, const FVector& ListenerPosition, const FVector& ListenerForwardDirection, const FVector& ListenerUpDirection)
{
	GetVoiceChatUser()->Set3DPosition(ChannelName, SpeakerPosition, ListenerPosition, ListenerForwardDirection, ListenerUpDirection);
}

TArray<FString> FAccelByteVoiceChat::GetChannels() const
{
	return GetVoiceChatUser()->GetChannels();
}

TArray<FString> FAccelByteVoiceChat::GetPlayersInChannel(const FString& ChannelName) const
{
	return GetVoiceChatUser()->GetPlayersInChannel(ChannelName);
}

EVoiceChatChannelType FAccelByteVoiceChat::GetChannelType(const FString& ChannelName) const
{
	return GetVoiceChatUser()->GetChannelType(ChannelName);
}

FOnVoiceChatPlayerAddedDelegate& FAccelByteVoiceChat::OnVoiceChatPlayerAdded()
{
	return GetVoiceChatUser()->OnVoiceChatPlayerAdded();
}

FOnVoiceChatPlayerRemovedDelegate& FAccelByteVoiceChat::OnVoiceChatPlayerRemoved()
{
	return GetVoiceChatUser()->OnVoiceChatPlayerRemoved();
}

bool FAccelByteVoiceChat::IsPlayerTalking(const FString& PlayerName) const
{
	return GetVoiceChatUser()->IsPlayerTalking(PlayerName);
}

FOnVoiceChatPlayerTalkingUpdatedDelegate& FAccelByteVoiceChat::OnVoiceChatPlayerTalkingUpdated()
{
	return GetVoiceChatUser()->OnVoiceChatPlayerTalkingUpdated();
}

void FAccelByteVoiceChat::SetPlayerMuted(const FString& PlayerName, bool bMuted)
{
	GetVoiceChatUser()->SetPlayerMuted(PlayerName, bMuted);
}

bool FAccelByteVoiceChat::IsPlayerMuted(const FString& PlayerName) const
{
	return GetVoiceChatUser()->IsPlayerMuted(PlayerName);
}

FOnVoiceChatPlayerMuteUpdatedDelegate& FAccelByteVoiceChat::OnVoiceChatPlayerMuteUpdated()
{
	return GetVoiceChatUser()->OnVoiceChatPlayerMuteUpdated();
}

void FAccelByteVoiceChat::SetPlayerVolume(const FString& PlayerName, float Volume)
{
	GetVoiceChatUser()->SetPlayerVolume(PlayerName, Volume);
}

float FAccelByteVoiceChat::GetPlayerVolume(const FString& PlayerName) const
{
	return GetVoiceChatUser()->GetPlayerVolume(PlayerName);
}

FOnVoiceChatPlayerVolumeUpdatedDelegate& FAccelByteVoiceChat::OnVoiceChatPlayerVolumeUpdated()
{
	return GetVoiceChatUser()->OnVoiceChatPlayerVolumeUpdated();
}

void FAccelByteVoiceChat::TransmitToAllChannels()
{
	GetVoiceChatUser()->TransmitToAllChannels();
}

void FAccelByteVoiceChat::TransmitToNoChannels()
{
	GetVoiceChatUser()->TransmitToNoChannels();
}

void FAccelByteVoiceChat::TransmitToSpecificChannel(const FString& ChannelName)
{
#if (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 3)
	TSet<FString> ChannelNames({ ChannelName });
	TransmitToSpecificChannels(ChannelNames);
#else
	GetVoiceChatUser()->TransmitToSpecificChannel(ChannelName);
#endif // (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 3)
}

#if (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 3)
void FAccelByteVoiceChat::TransmitToSpecificChannels(const TSet<FString>& ChannelNames)
{
	GetVoiceChatUser()->TransmitToSpecificChannels(ChannelNames);
}
#endif // (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 3)

EVoiceChatTransmitMode FAccelByteVoiceChat::GetTransmitMode() const
{
	return GetVoiceChatUser()->GetTransmitMode();
}

FString FAccelByteVoiceChat::GetTransmitChannel() const
{
	FString Result;
#if (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 3)
	auto TransmitChannels = GetTransmitChannels();
	if (TransmitChannels.Num() > 0)
	{
		Result = TransmitChannels.Get(FSetElementId::FromInteger(0));
	}
#else
	Result = GetVoiceChatUser()->GetTransmitChannel();
#endif // (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 3)
	return Result;
}

#if (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 3)
TSet<FString> FAccelByteVoiceChat::GetTransmitChannels() const
{
	return GetVoiceChatUser()->GetTransmitChannels();
}
#endif // (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 3)

FDelegateHandle FAccelByteVoiceChat::StartRecording(const FOnVoiceChatRecordSamplesAvailableDelegate::FDelegate& Delegate)
{
	return GetVoiceChatUser()->StartRecording(Delegate);
}

void FAccelByteVoiceChat::StopRecording(FDelegateHandle Handle)
{
	GetVoiceChatUser()->StopRecording(Handle);
}

FDelegateHandle FAccelByteVoiceChat::RegisterOnVoiceChatAfterCaptureAudioReadDelegate(const FOnVoiceChatAfterCaptureAudioReadDelegate::FDelegate& Delegate)
{
	return GetVoiceChatUser()->RegisterOnVoiceChatAfterCaptureAudioReadDelegate(Delegate);
}

void FAccelByteVoiceChat::UnregisterOnVoiceChatAfterCaptureAudioReadDelegate(FDelegateHandle Handle)
{
	GetVoiceChatUser()->UnregisterOnVoiceChatAfterCaptureAudioReadDelegate(Handle);
}

FDelegateHandle FAccelByteVoiceChat::RegisterOnVoiceChatBeforeCaptureAudioSentDelegate(const FOnVoiceChatBeforeCaptureAudioSentDelegate::FDelegate& Delegate)
{
	return GetVoiceChatUser()->RegisterOnVoiceChatBeforeCaptureAudioSentDelegate(Delegate);
}

void FAccelByteVoiceChat::UnregisterOnVoiceChatBeforeCaptureAudioSentDelegate(FDelegateHandle Handle)
{
	GetVoiceChatUser()->UnregisterOnVoiceChatBeforeCaptureAudioSentDelegate(Handle);
}

FDelegateHandle FAccelByteVoiceChat::RegisterOnVoiceChatBeforeRecvAudioRenderedDelegate(const FOnVoiceChatBeforeRecvAudioRenderedDelegate::FDelegate& Delegate)
{
	return GetVoiceChatUser()->RegisterOnVoiceChatBeforeRecvAudioRenderedDelegate(Delegate);
}

void FAccelByteVoiceChat::UnregisterOnVoiceChatBeforeRecvAudioRenderedDelegate(FDelegateHandle Handle)
{
	GetVoiceChatUser()->UnregisterOnVoiceChatBeforeRecvAudioRenderedDelegate(Handle);
}

FString FAccelByteVoiceChat::InsecureGetLoginToken(const FString& PlayerName)
{
	return GetVoiceChatUser()->InsecureGetLoginToken(PlayerName);
}

FString FAccelByteVoiceChat::InsecureGetJoinToken(const FString& ChannelName, EVoiceChatChannelType ChannelType, TOptional<FVoiceChatChannel3dProperties> Channel3dProperties)
{
	return GetVoiceChatUser()->InsecureGetJoinToken(ChannelName, ChannelType, Channel3dProperties);
}

#if (ENGINE_MAJOR_VERSION == 5)
void FAccelByteVoiceChat::SetChannelPlayerMuted(const FString& ChannelName, const FString& PlayerName, bool bAudioMuted)
{
	return GetVoiceChatUser()->SetChannelPlayerMuted(ChannelName, PlayerName, bAudioMuted);
}

bool FAccelByteVoiceChat::IsChannelPlayerMuted(const FString & ChannelName, const FString & PlayerName) const
{
	return GetVoiceChatUser()->IsChannelPlayerMuted(ChannelName, PlayerName);
}
#endif

#pragma endregion IVoiceChatUser
