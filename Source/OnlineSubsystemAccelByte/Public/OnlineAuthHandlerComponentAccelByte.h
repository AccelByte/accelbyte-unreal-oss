// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "PacketHandler.h"
#include "Core/AccelByteServerCredentials.h"
#include "Core/AccelByteCredentials.h"
#include "Core/AccelByteOpenSSL.h"
#include "Core/AccelByteApiClient.h"
#include "Core/AccelByteMultiRegistry.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineAuthInterfaceAccelByte.h"

#include "HandlerComponentFactory.h"
#include "OnlineAuthHandlerComponentAccelByte.generated.h"

class FAuthHandlerComponentAccelByte : public HandlerComponent {
public:
	FAuthHandlerComponentAccelByte();
	virtual ~FAuthHandlerComponentAccelByte() override;

	//~ Begin HandlerComponent interface
	virtual bool IsValid() const { return bIsEnabled; }
	virtual void Initialize() override;
	virtual int32 GetReservedPacketBits() const override;
	virtual void NotifyHandshakeBegin() override;
	virtual void Incoming(FBitReader& Packet) override;
	virtual void Outgoing(FBitWriter& Packet, FOutPacketTraits& Traits) override;
	virtual void Tick(float DeltaTime) override;
	//~ Begin HandlerComponent interface

private:
	enum class EState : uint8
	{
		Uninitialized = 0,
		SentKey,
		RecvedKey,
		WaitForJwks,
		ReadyJwks,
		WaitForAuth,
		SentAuth,
		AuthFail,
		Initialized
	};

	/**
	 * Processes an incoming packet during the handshake stage.
	 */
	void IncomingHandshake(FBitReader& Packet);

	/* Pack the local key into a packet */
	void PackPublicKey(FBitWriter& Packet);

	/* Unpack the remote key from a packet */
	void UnpackPublicKey(FBitReader& Packet);

	/* RSA encrypt outgoing packets */
	bool EncryptRSA(FBitWriter& Packet);

	/* RSA decrypt incoming packets */
	bool DecryptRSA(FBitReader& Packet);

	/* Set the state of the handler */
	void SetAuthState(EState State);

	/* send all key to remote connection. */
	void SendKey();

	/* send public key to remote connection. */
	void SendPublicKey();

	/* receive public key from remote connection. */
	void RecvPublicKey(FBitReader& Packet);

	/* send AES key to remote connection. */
	void SendKeyAES();

	/* receive AES key from remote connection. */
	bool RecvKeyAES(FBitReader& Packet);

	/* completed handshaking with authenication */
	void CompletedHandshaking(bool bResult);

	/* AES encrypt outgoing packets */
	bool EncryptAES(FBitWriter& Packet);

	/* AES decrypt incoming packets */
	bool DecryptAES(FBitReader& Packet);

	/** Authenticate User */
	bool SetAuthData(FString& AuthToken);
	void SendAuthData();
	bool SendAuthData(TArray<uint8>& Packet, uint32& SegmentsNum);
	void RecvAuthData(FBitReader& Packet);

	void VerifyAuthToken();
	void OnVerifyAuthToken();
	void OnAuthenticateUser();

	void SendAuthResult();
	bool OnSendAuthResult(bool InAuthResult);

	void AuthenticateUserResult(bool Result);
	void RequestResend();

	void LoadSettings();
	void SetComponentReady();

	void SendPacket(FBitWriter& Packet);

	bool IsServer() const;

	void SetCryptor();

	void Clear();
	void NetCleanUp();

	static int32 GetMaxTokenSizeInBytes();

private:
	/* State of the handler */
	EState State;

	/** handler for encrypting with the RSA key */
	AccelByte::FRSAEncryptionOpenSSL RSACrypto;

	/** handler for encrypting with the AES key */
	AccelByte::FAESEncryptionOpenSSL AESCrypto;

	typedef int32(AccelByte::FRSAEncryptionOpenSSL::* PF_CRYPTO)(const TArrayView<const uint8>, TArray<uint8>&);

	PF_CRYPTO RSAEncryptor;
	PF_CRYPTO RSADecryptor;

	typedef bool(FAuthHandlerComponentAccelByte::* PF_ENCRYPTO)(FBitWriter& Packet);
	typedef bool(FAuthHandlerComponentAccelByte::* PF_DECRYPTO)(FBitReader& Packet);

	PF_ENCRYPTO EncryptorPacket;
	PF_DECRYPTO DecryptorPacket;

	FOnlineSubsystemAccelByte* OnlineSubsystem;

private:
	uint32 RecvSegCount;
	bool bIsEnabled;
	double LastTimestamp;
	bool bEnabledEncryption;
	bool bOriginRequiresReliability;

	FString UserId;
	FString AuthData;

	FOnlineAuthAccelBytePtr AuthInterface;
};

/**
 * Factory class for loading HandlerComponent's contained within Engine
 */
UCLASS()
class UAuthHandlerComponentAccelByteFactory : public UHandlerComponentFactory
{
	GENERATED_UCLASS_BODY()

public:
	virtual TSharedPtr<HandlerComponent> CreateComponentInstance(FString& Options) override;
};
