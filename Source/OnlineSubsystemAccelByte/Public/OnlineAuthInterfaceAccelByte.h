// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineSubsystemAccelBytePackage.h"
#include "OnlinePartyInterfaceAccelByte.h"
#include "OnlineSessionInterfaceV2AccelByte.h"

#include "Core/AccelByteApiClient.h"
#include "GameServerApi/AccelByteServerUserApi.h"

struct FAccelByteAuthUserData
{
	FAccelByteAuthUserData() {}
	~FAccelByteAuthUserData() {}

	FString UserId;

	void SerializeData(FArchive& Ar)
	{
		Ar << UserId;
	}

	friend FArchive& operator<<(FArchive& Ar, FAccelByteAuthUserData& AuthData)
	{
		AuthData.SerializeData(Ar);
		return Ar;
	}

	void Empty()
	{
		UserId.Empty();
	}
};

/** AccelByte Authentication Interface.
 *
 *  For the most part, this is fully automated. You simply just need to add the packet handler and your server will now
 *  require AccelByte Authentication for any incoming users. If a player fails to respond correctly, they will be kicked.
 */
enum class EAccelByteAuthStatus : uint8
{
	None = 0,
	AuthSuccess = 1 << 0,
	AuthFail = 1 << 1,
	ValidationStarted = 1 << 2,
	KickUser = 1 << 3,
	FailKick = AuthFail | KickUser,
	HasOrIsPendingAuth = AuthSuccess | ValidationStarted
};

ENUM_CLASS_FLAGS(EAccelByteAuthStatus);

class ONLINESUBSYSTEMACCELBYTE_API FOnlineAuthAccelByte : public TSharedFromThis<FOnlineAuthAccelByte, ESPMode::ThreadSafe>
{
PACKAGE_SCOPE:
	FOnlineAuthAccelByte(FOnlineSubsystemAccelByte* InSubsystem);

	/** Data pertaining the current authentication state of the users in the game */
	struct FAuthUser
	{
		FAuthUser() : Status(EAccelByteAuthStatus::None), ErrorCode(0) { }
		void SetFail(const int32 InErrorCode, const FString& InErrorMessage) { ErrorCode = InErrorCode; ErrorMessage = InErrorMessage; }

		EAccelByteAuthStatus Status;
		int32 ErrorCode;
		FString ErrorMessage;
	};

	typedef TSharedPtr<FAuthUser, ESPMode::NotThreadSafe> SharedAuthUserPtr;

	/** Get authentication information */
	SharedAuthUserPtr GetUser(const FString& InUserId);
	SharedAuthUserPtr GetOrCreateUser(const FString& InUserId);

	bool GetAuthData(FString& UserId);

	/** Authenticate User */
	bool AuthenticateUser(const FAccelByteAuthUserData& InUserData);
	void OnAuthSuccess(const FString& InUserId);
	void OnAuthFail(const FString& InUserId, const int32 InErrorCode, const FString& InErrorMessage);
	EAccelByteAuthStatus GetAuthStatus(const FString& InUserId);
	void MarkUserForKick(const FString& InUserId);
	bool KickUser(const FString& InUserId, bool bSuppressFailure);
	bool Tick(float DeltaTime);

protected:
	FOnlineAuthAccelByte();

private:
	typedef TMap<FString, SharedAuthUserPtr> AccelByteAuthentications;
	AccelByteAuthentications AuthUsers;

	/** Utility functions */
	FORCEINLINE bool IsServer() const
	{
		return OnlineSubsystem != nullptr && OnlineSubsystem->IsServer();
	}

	bool IsInSessionUser(const FString& InUserId) const;
	void RemoveUser(const FString& InTargetUser);

	/** Pointer to the AccelByte OSS instance that instantiated this online user interface. */
	FOnlineSubsystemAccelByte* OnlineSubsystem;

	/** API client that should be used for this task, use GetApiClient to get a valid instance */
	FApiClientPtr ApiClientPtr;

	/** Settings */
	bool bEnabled;
	float LastTimestamp;

	FOnlineSessionAccelBytePtr SessionInterface;

	/** Testing flags */
PACKAGE_SCOPE:

	/** Setting Getters */
	bool IsSessionAuthEnabled() const { return bEnabled; }
	bool IsSessionValid() const;

public:
	virtual ~FOnlineAuthAccelByte();

	static int32 GetMaxTokenSizeInBytes();

	/**
	 * Handler for when we finish querying for all pending authonication
	 */
	void OnAuthUserCompleted(bool bWasSuccessful, const FString& UserId);

	
};
