// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "CoreMinimal.h"
#include "OnlineSubsystemAccelByte.h"
#include "Misc/CoreMisc.h"
#include "VoiceChat.h"

class ONLINESUBSYSTEMACCELBYTE_API FAccelByteVoiceChat
	: public TSharedFromThis<FAccelByteVoiceChat, ESPMode::ThreadSafe>
	, public IVoiceChat
{
PACKAGE_SCOPE:
	FAccelByteVoiceChat(FOnlineSubsystemAccelByte* InSubsystem);
	~FAccelByteVoiceChat() override;
public:
	bool Initialize(IVoiceChatPtr InVoiceChat);
	IVoiceChatUser* GetVoiceChatUser(const FUniqueNetId& LocalUserId);
	void ReleaseUser(const FUniqueNetId& LocalUserId);

	//IVoiceChat Interface
	virtual bool Initialize() override;
	virtual void Initialize(const FOnVoiceChatInitializeCompleteDelegate& Delegate) override;
	virtual bool Uninitialize() override;
	virtual void Uninitialize(const FOnVoiceChatUninitializeCompleteDelegate& Delegate) override;
	virtual bool IsInitialized() const override;
	virtual void Connect(const FOnVoiceChatConnectCompleteDelegate& Delegate) override;
	virtual void Disconnect(const FOnVoiceChatDisconnectCompleteDelegate& Delegate) override;
	virtual bool IsConnecting() const override;
	virtual bool IsConnected() const override;
	virtual FOnVoiceChatConnectedDelegate& OnVoiceChatConnected() override { return OnVoiceChatConnectedDelegate; }
	virtual FOnVoiceChatDisconnectedDelegate& OnVoiceChatDisconnected() override { return OnVoiceChatDisconnectedDelegate; }
	virtual FOnVoiceChatReconnectedDelegate& OnVoiceChatReconnected() override { return OnVoiceChatReconnectedDelegate; }
	virtual IVoiceChatUser* CreateUser() override;
	virtual void ReleaseUser(IVoiceChatUser* VoiceChatUser) override;

	//IVoiceChatUser Interface
	virtual void SetSetting(const FString& Name, const FString& Value) override;
	virtual FString GetSetting(const FString& Name) override;
	virtual void SetAudioInputVolume(float Volume) override;
	virtual void SetAudioOutputVolume(float Volume) override;
	virtual float GetAudioInputVolume() const override;
	virtual float GetAudioOutputVolume() const override;
	virtual void SetAudioInputDeviceMuted(bool bIsMuted) override;
	virtual void SetAudioOutputDeviceMuted(bool bIsMuted) override;
	virtual bool GetAudioInputDeviceMuted() const override;
	virtual bool GetAudioOutputDeviceMuted() const override;
	virtual TArray<FVoiceChatDeviceInfo> GetAvailableInputDeviceInfos() const override;
	virtual TArray<FVoiceChatDeviceInfo> GetAvailableOutputDeviceInfos() const override;
	virtual FOnVoiceChatAvailableAudioDevicesChangedDelegate& OnVoiceChatAvailableAudioDevicesChanged() override;
	virtual void SetInputDeviceId(const FString& InputDeviceId) override;
	virtual void SetOutputDeviceId(const FString& OutputDeviceId) override;
	virtual FVoiceChatDeviceInfo GetInputDeviceInfo() const override;
	virtual FVoiceChatDeviceInfo GetOutputDeviceInfo() const override;
	virtual FVoiceChatDeviceInfo GetDefaultInputDeviceInfo() const override;
	virtual FVoiceChatDeviceInfo GetDefaultOutputDeviceInfo() const override;
	virtual void Login(FPlatformUserId PlatformId, const FString& PlayerName, const FString& Credentials, const FOnVoiceChatLoginCompleteDelegate& Delegate) override;
	virtual void Logout(const FOnVoiceChatLogoutCompleteDelegate& Delegate) override;
	virtual bool IsLoggingIn() const override;
	virtual bool IsLoggedIn() const override;
	virtual FOnVoiceChatLoggedInDelegate& OnVoiceChatLoggedIn() override;
	virtual FOnVoiceChatLoggedOutDelegate& OnVoiceChatLoggedOut() override;
	virtual FString GetLoggedInPlayerName() const override;
	virtual void BlockPlayers(const TArray<FString>& PlayerNames) override;
	virtual void UnblockPlayers(const TArray<FString>& PlayerNames) override;
	virtual void JoinChannel(const FString& ChannelName, const FString& ChannelCredentials, EVoiceChatChannelType ChannelType, const FOnVoiceChatChannelJoinCompleteDelegate& Delegate, TOptional<FVoiceChatChannel3dProperties> Channel3dProperties = TOptional<FVoiceChatChannel3dProperties>()) override;
	virtual void LeaveChannel(const FString& Channel, const FOnVoiceChatChannelLeaveCompleteDelegate& Delegate) override;
	virtual FOnVoiceChatChannelJoinedDelegate& OnVoiceChatChannelJoined() override;
	virtual FOnVoiceChatChannelExitedDelegate& OnVoiceChatChannelExited() override;
	virtual FOnVoiceChatCallStatsUpdatedDelegate& OnVoiceChatCallStatsUpdated() override;
	virtual void Set3DPosition(const FString& ChannelName, const FVector& SpeakerPosition, const FVector& ListenerPosition, const FVector& ListenerForwardDirection, const FVector& ListenerUpDirection) override;
	virtual TArray<FString> GetChannels() const override;
	virtual TArray<FString> GetPlayersInChannel(const FString& ChannelName) const override;
	virtual EVoiceChatChannelType GetChannelType(const FString& ChannelName) const override;
	virtual FOnVoiceChatPlayerAddedDelegate& OnVoiceChatPlayerAdded() override;
	virtual FOnVoiceChatPlayerRemovedDelegate& OnVoiceChatPlayerRemoved() override;
	virtual bool IsPlayerTalking(const FString& PlayerName) const override;
	virtual FOnVoiceChatPlayerTalkingUpdatedDelegate& OnVoiceChatPlayerTalkingUpdated() override;
	virtual void SetPlayerMuted(const FString& PlayerName, bool bMuted) override;
	virtual bool IsPlayerMuted(const FString& PlayerName) const override;
	virtual FOnVoiceChatPlayerMuteUpdatedDelegate& OnVoiceChatPlayerMuteUpdated() override;
	virtual void SetPlayerVolume(const FString& PlayerName, float Volume) override;
	virtual float GetPlayerVolume(const FString& PlayerName) const override;
	virtual FOnVoiceChatPlayerVolumeUpdatedDelegate& OnVoiceChatPlayerVolumeUpdated() override;
	virtual void TransmitToAllChannels() override;
	virtual void TransmitToNoChannels() override;
	virtual void TransmitToSpecificChannel(const FString& ChannelName) override;
	virtual EVoiceChatTransmitMode GetTransmitMode() const override;
	virtual FString GetTransmitChannel() const override;
	virtual FDelegateHandle StartRecording(const FOnVoiceChatRecordSamplesAvailableDelegate::FDelegate& Delegate) override;
	virtual void StopRecording(FDelegateHandle Handle) override;
	virtual FDelegateHandle RegisterOnVoiceChatAfterCaptureAudioReadDelegate(const FOnVoiceChatAfterCaptureAudioReadDelegate::FDelegate& Delegate) override;
	virtual void UnregisterOnVoiceChatAfterCaptureAudioReadDelegate(FDelegateHandle Handle) override;
	virtual FDelegateHandle RegisterOnVoiceChatBeforeCaptureAudioSentDelegate(const FOnVoiceChatBeforeCaptureAudioSentDelegate::FDelegate& Delegate) override;
	virtual void UnregisterOnVoiceChatBeforeCaptureAudioSentDelegate(FDelegateHandle Handle) override;
	virtual FDelegateHandle RegisterOnVoiceChatBeforeRecvAudioRenderedDelegate(const FOnVoiceChatBeforeRecvAudioRenderedDelegate::FDelegate& Delegate) override;
	virtual void UnregisterOnVoiceChatBeforeRecvAudioRenderedDelegate(FDelegateHandle Handle) override;
	virtual FString InsecureGetLoginToken(const FString& PlayerName) override;
	virtual FString InsecureGetJoinToken(const FString& ChannelName, EVoiceChatChannelType ChannelType, TOptional<FVoiceChatChannel3dProperties> Channel3dProperties = TOptional<FVoiceChatChannel3dProperties>()) override;
#if (ENGINE_MAJOR_VERSION == 5) 
	virtual void SetChannelPlayerMuted(const FString& ChannelName, const FString& PlayerName, bool bAudioMuted) override;
	virtual bool IsChannelPlayerMuted(const FString& ChannelName, const FString& PlayerName) const override;
#endif
protected:

	/** Instance of the subsystem that created this interface */
	FOnlineSubsystemAccelByte* AccelByteSubsystem = nullptr;
private:
	FAccelByteVoiceChat() = delete;
	IVoiceChatUserPtr GetVoiceChatUser() const;
	IVoiceChatPtr VoiceChatPtr;
	IVoiceChatUserPtr VoiceChatUserPtr;
	TUniqueNetIdMap<IVoiceChatUserPtr> LocalVoiceChatUsers;

	// IVoiceChat Delegates
	FOnVoiceChatConnectedDelegate OnVoiceChatConnectedDelegate;
	FOnVoiceChatDisconnectedDelegate OnVoiceChatDisconnectedDelegate;
	FOnVoiceChatReconnectedDelegate OnVoiceChatReconnectedDelegate;
	// IVoiceChatUser Delegates
	FOnVoiceChatAvailableAudioDevicesChangedDelegate OnVoiceChatAvailableAudioDevicesChangedDelegate;
};
