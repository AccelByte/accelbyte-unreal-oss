// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineAuthHandlerComponentAccelByte.h"
#include "OnlineSubsystemAccelByteDefines.h"
#include "Net/Core/Misc/PacketAudit.h"
#include "Misc/AES.h"
#include "OnlineSubsystemUtils.h"

#include "GameFramework/PlayerController.h"
#include "Engine/NetConnection.h"

/** The maximum size (bits) for a packet */
#define MAX_PACKET_BITS ((MAX_PACKET_SIZE) * 8)

/** The maximum size for a data packet */
#define MAX_AES_ENCRYPTION_BITS ((MAX_PACKET_SIZE - FAES::AESBlockSize - 1) * 8)

#define ACCELBYTE_RESEND_REQUEST_INTERVAL 3.0f

enum class EAccelByteAuthMsgType : uint8
{
	RSAKey = 0,
	AESKey,
	Auth,
	Result,
	ResendKey,
	ResendAuth,
	ResendResult,
	Max
};

struct FAccelByteAuthHeader
{
	FAccelByteAuthHeader() : Type(EAccelByteAuthMsgType::Max) {}
	FAccelByteAuthHeader(EAccelByteAuthMsgType InType) : Type(InType) {}
	virtual ~FAccelByteAuthHeader() {}

	EAccelByteAuthMsgType Type;

	virtual void SerializeData(FArchive& Ar)
	{
		Ar << Type;
	}

	friend FArchive& operator<<(FArchive& Ar, FAccelByteAuthHeader& AuthData)
	{
		AuthData.SerializeData(Ar);
		return Ar;
	}
};

struct FAccelByteAuthResult : public FAccelByteAuthHeader
{
	FAccelByteAuthResult() : FAccelByteAuthHeader(EAccelByteAuthMsgType::Result), bWasSuccess(false) {}
	virtual ~FAccelByteAuthResult() {}
	bool bWasSuccess;

	virtual void SerializeData(FArchive& Ar) override
	{
		FAccelByteAuthHeader::SerializeData(Ar);
		Ar << bWasSuccess;
	}

	friend FArchive& operator<<(FArchive& Ar, FAccelByteAuthResult& AuthData)
	{
		AuthData.SerializeData(Ar);
		return Ar;
	}
};

/**
 * OnlineSubsystemAccelByte.AuthHandlerComponentAccelByteFactory
 */
FAuthHandlerComponentAccelByte::FAuthHandlerComponentAccelByte()
	: HandlerComponent(FName(TEXT("AuthHandlerComponentAccelByte")))
	, State(EState::Uninitialized)
	, RSAEncryptor(nullptr)
	, RSADecryptor(nullptr)
	, EncryptorPacket(nullptr)
	, DecryptorPacket(nullptr)
	, OnlineSubsystem(nullptr)
	, bIsEnabled(true)
	, LastTimestamp(0.0f)
	, bEnabledEncryption(false)
	, AuthInterface(nullptr)
{
	OnlineSubsystem = (FOnlineSubsystemAccelByte*)(IOnlineSubsystem::Get(ACCELBYTE_SUBSYSTEM));
	if (nullptr != OnlineSubsystem)
	{
		AuthInterface = OnlineSubsystem->GetAuthInterface();
		if (!AuthInterface.IsValid() || !AuthInterface->IsSessionAuthEnabled())
		{
			bIsEnabled = false;
			SetActive(false);
			SetState(Handler::Component::State::Initialized);
		}
	}
	else
	{
		UE_LOG_AB(Warning, TEXT("AUTH HANDLER: IOnlineSubsystem is null: "));
		bIsEnabled = false;
		SetActive(false);
		SetState(Handler::Component::State::Initialized);
	}
}

FAuthHandlerComponentAccelByte::~FAuthHandlerComponentAccelByte()
{
	Clear();
}

void FAuthHandlerComponentAccelByte::Clear()
{
	RSACrypto.Empty();
	AESCrypto.Empty();
	AuthUserData.Empty();

	State = EState::Uninitialized;
	SetState(Handler::Component::State::UnInitialized);
	LastTimestamp = 0.0f;

	bRequiresReliability = false;
	EncryptorPacket = nullptr;
	DecryptorPacket = nullptr;
}

void FAuthHandlerComponentAccelByte::LoadSettings()
{
	if (GConfig->GetBool(TEXT("OnlineSubsystemAccelByte"), TEXT("EnabledEncryption"), bEnabledEncryption, GEngineIni))
	{
		if (bEnabledEncryption)
		{
			UE_LOG_AB(Warning, TEXT("AUTH HANDLER: (%s) Enabled encryption for all packets.(AES-CBC-256 and HMAC-SHA-256)"), ((Handler->Mode == Handler::Mode::Server) ? TEXT("DS") : TEXT("CL")));
			EncryptorPacket = &FAuthHandlerComponentAccelByte::EncryptAES;
			DecryptorPacket = &FAuthHandlerComponentAccelByte::DecryptAES;
		}
		return;
	}
}

void FAuthHandlerComponentAccelByte::Initialize()
{
	bRequiresReliability = true;
	bRequiresHandshake = true;

	if (!IsValid())
	{
		return;
	}

	SetActive(true);
	SetComponentReady();

	LastTimestamp = FPlatformTime::Seconds();
}

void FAuthHandlerComponentAccelByte::SetComponentReady()
{
	if (Handler == nullptr)
	{
		UE_LOG_AB(Warning, TEXT("AUTH HANDLER: SetCompnentReady: Handler is null."));
		return;
	}

	if (!IsValid())
	{
		return;
	}

	// Message for handshaking.
	LoadSettings();

	if (Handler->Mode == Handler::Mode::Server)
	{
		SetCryptor(true);

		RSACrypto.GenerateNewKey();
		AESCrypto.GenerateKey();
	}
	else
	{
		SetCryptor(false);
	}
}

void FAuthHandlerComponentAccelByte::SetCryptor(bool bIsServer)
{
	if (bIsServer)
	{
		RSAEncryptor = &FRSAEncryptionOpenSSL::EncryptPrivate;
		RSADecryptor = &FRSAEncryptionOpenSSL::DecryptPrivate;
	}
	else
	{
		RSAEncryptor = &FRSAEncryptionOpenSSL::EncryptPublic;
		RSADecryptor = &FRSAEncryptionOpenSSL::DecryptPublic;
	}
}

void FAuthHandlerComponentAccelByte::Tick(float DeltaTime)
{
	if (!IsValid())
	{
		return;
	}

	// Don't do anything if we're not enabled or not ready.
	// Alternatively, if we're already finished then just don't do anything here either
	if (!IsActive())
	{
		if (EState::AuthFail == State)
		{
			NetCleanUp();
		}
		return;
	}
	if (State == EState::Initialized || !Handler)
	{
		return;
	}

	float CurTime = FPlatformTime::Seconds();
	if (LastTimestamp != 0.0)
	{
		if (CurTime - LastTimestamp > ACCELBYTE_RESEND_REQUEST_INTERVAL)
		{
			LastTimestamp = FPlatformTime::Seconds();
			if (EState::RecvedKey == State)
			{
				SendAuthData();
			}
			else if (EState::WaitForAuth == State)
			{
				SendAuthResult();
			}
			else
			{
				RequestResend();
			}
		}
	}
}

void FAuthHandlerComponentAccelByte::SendAuthResult()
{
	if (AuthInterface.IsValid())
	{
		switch (AuthInterface->GetAuthStatus(AuthUserData.UserId))
		{
			case EAccelByteAuthStatus::AuthSuccess:
			{
				AuthenticateUserResult(true);
			}
			break;
			case EAccelByteAuthStatus::AuthFail:
			case EAccelByteAuthStatus::KickUser:
			case EAccelByteAuthStatus::FailKick:
			{
				AuthenticateUserResult(false);
			}
			break;
		}
	}
}

void FAuthHandlerComponentAccelByte::SendKey()
{
	// Exchange the RSA Public key
	if (Handler->Mode == Handler::Mode::Server)
	{
		if (IsActive())
		{
			if (AuthInterface.IsValid() && AuthInterface->IsSessionValid())
			{
				SendPublicKey();
				SendKeyAES();
				SetAuthState(EState::SentKey);
			}
		}
	}
}

void FAuthHandlerComponentAccelByte::Incoming(FBitReader& Packet)
{
	if (!IsActive())
	{
		UE_LOG_AB(Warning, TEXT("AUTH HANDLER: Incoming: (%s) this handler is not active."), ((Handler->Mode == Handler::Mode::Server) ? TEXT("DS") : TEXT("CL")));
		return;
	}

	if (State != EState::Initialized)
	{
		IncomingHandshake(Packet);
	}
	else
	{
		if (nullptr != DecryptorPacket)
		{
			if (1 == Packet.ReadBit())
			{
				if (!(*this.*DecryptorPacket)(Packet))
				{
					UE_LOG_AB(Warning, TEXT("AUTH HANDLER: Decryption skipped as plain text size is too large. send smaller packets for secure data."));
				}
			}
		}
	}
}

void FAuthHandlerComponentAccelByte::Outgoing(FBitWriter& Packet, FOutPacketTraits& Traits)
{
	if (!IsActive())
	{
		UE_LOG_AB(Warning, TEXT("AUTH HANDLER: Outgoing: (%s) this handler is not active."), ((Handler->Mode == Handler::Mode::Server) ? TEXT("DS") : TEXT("CL")));
		return;
	}

	if (State == EState::Initialized)
	{
		if (nullptr != EncryptorPacket)
		{
			FBitWriter TempPacket(Packet.GetNumBits() + 1, true);
			if ((*this.*EncryptorPacket)(Packet))
			{
				// this is a encryption packet
				TempPacket.WriteBit(1);
			}
			else
			{
				UE_LOG_AB(Warning, TEXT("AUTH HANDLER: Encryption skipped as plain text size is too large. send smaller packets for secure data."));
				// this is a normal packet
				TempPacket.WriteBit(0);
			}
			TempPacket.SerializeBits(Packet.GetData(), Packet.GetNumBits());
			Packet = MoveTemp(TempPacket);
		}
	}
}

void FAuthHandlerComponentAccelByte::NotifyHandshakeBegin()
{
	if (!IsActive())
	{
		UE_LOG_AB(Warning, TEXT("AUTH HANDLER: NotifyHandshakeBegin: (%s) this handler is not active."), ((Handler->Mode == Handler::Mode::Server) ? TEXT("DS") : TEXT("CL")));
		return;
	}

	// Exchange the RSA Public key
	if (Handler->Mode == Handler::Mode::Server)
	{
		if (State == EState::Uninitialized)
		{
			SendKey();
		}
		else
		{
			UE_LOG_AB(Warning, TEXT("AUTH HANDLER: (%s) any wrong state in the handshaking."), ((Handler->Mode == Handler::Mode::Server) ? TEXT("DS") : TEXT("CL")));
		}
	}
	else
	{
		LastTimestamp = FPlatformTime::Seconds();
	}
}

void FAuthHandlerComponentAccelByte::IncomingHandshake(FBitReader& Packet)
{
	if (!AuthInterface.IsValid())
	{
		UE_LOG_AB(Warning, TEXT("AUTH HANDLER: (%s) AuthInterface is not valid."), ((Handler->Mode == Handler::Mode::Server) ? TEXT("DS") : TEXT("CL")));
		return;
	}

	bool bHandledPacket = !!Packet.ReadBit() && !Packet.IsError();
	if (bHandledPacket)
	{
		FAccelByteAuthHeader Header;
		Packet << Header;

		if (Packet.IsError())
		{
			UE_LOG_AB(Warning, TEXT("AUTH HANDLER: IncomingHandshake: accelbyte auth packet could not be properly serialized."));
			return;
		}

		switch (Header.Type)
		{
			case EAccelByteAuthMsgType::RSAKey:
			{
				RecvPublicKey(Packet);
				return;
			}
			case EAccelByteAuthMsgType::AESKey:
			{
				if (!RecvKeyAES(Packet))
				{
					// request resending key to DS.
					RequestResend();
				}
				return;
			}
			case EAccelByteAuthMsgType::Auth:
			{
				RecvAuthData(Packet);
				return;
			}
			case EAccelByteAuthMsgType::Result:
			{
				if (State == EState::SentAuth)
				{
					bool AuthStatusResult = false;
					Packet << AuthStatusResult;

					if (Packet.IsError())
					{
						// Really this is if we somehow overflow and cannot fit the packet.
						UE_LOG_AB(Warning, TEXT("AUTH HANDLER: IncomingHandshake: accelbyte auth packet could not be properly serialized."));
						return;
					}

					CompletedHandshaking(AuthStatusResult);
					return;
				}
				else
				{
					bHandledPacket = false;
				}
				break;
			}
			case EAccelByteAuthMsgType::ResendKey:
			{
				SendKey();
				break;
			}
			case EAccelByteAuthMsgType::ResendAuth:
			{
				SendAuthData();
				break;
			}
			case EAccelByteAuthMsgType::ResendResult:
			{
				SendAuthResult();
				break;
			}
			default:
			{
				bHandledPacket = false;
				break;
			}
		}
	}
	else
	{
		UE_LOG_AB(Warning, TEXT("AUTH HANDLER: (%s) packet is wrong for the handshaking."), ((Handler->Mode == Handler::Mode::Server) ? TEXT("DS") : TEXT("CL")));
		Packet.SetError();
		return;
	}

	if (!bHandledPacket)
	{
		UE_LOG_AB(Warning, TEXT("AUTH HANDLER: (%s) got IncomingHandshake() the other packet when not yet initialized."), ((Handler->Mode == Handler::Mode::Server) ? TEXT("DS") : TEXT("CL")));
		Packet.SetError();
	}
}

void FAuthHandlerComponentAccelByte::CompletedHandshaking(bool bResult)
{
	if (bResult)
	{
		SetAuthState(EState::Initialized);
		HandlerComponent::Initialized();

		if (bEnabledEncryption)
		{
			return;
		}
		UE_LOG_AB(Warning, TEXT("AUTH HANDLER: (%s) complete handshake.(T)"), ((Handler->Mode == Handler::Mode::Server) ? TEXT("DS") : TEXT("CL")));
	}
	else
	{
		SetAuthState(EState::AuthFail);
		UE_LOG_AB(Warning, TEXT("AUTH HANDLER: (%s) complete handshake.(F)"), ((Handler->Mode == Handler::Mode::Server) ? TEXT("DS") : TEXT("CL")));
	}

	bRequiresReliability = false;
	SetActive(false);
	EncryptorPacket = nullptr;
	DecryptorPacket = nullptr;

	Clear();
}

void FAuthHandlerComponentAccelByte::NetCleanUp()
{
	if (GWorld)
	{
		if (APlayerController* PC = GWorld->GetFirstPlayerController())
		{
			if (UNetConnection* NetConnection = PC->GetNetConnection())
			{
				NetConnection->CleanUp();
			}
		}
	}
	Clear();
}

void FAuthHandlerComponentAccelByte::SendPublicKey()
{
	// The Server sends the RSA publick Key.
	FBitWriter OutPacket(0, true);

	PackPublicKey(OutPacket);
	SendPacket(OutPacket);
}

void FAuthHandlerComponentAccelByte::RecvPublicKey(FBitReader& Packet)
{
#if !PLATFORM_SWITCH
	UnpackPublicKey(Packet);

	// Generate/send the session key and initialization vector, and setup the symmetric encryption locally
	if (Packet.IsError())
	{
		UE_LOG_AB(Warning, TEXT("AUTH HANDLER: RSA: Error unpacking the RSA key, can't complete handshake."));
	}
#endif
}

void FAuthHandlerComponentAccelByte::PackPublicKey(FBitWriter& Packet)
{
	FBitWriter Local;
	Local.AllowAppend(true);
	Local.SetAllowResize(true);
	FAccelByteAuthHeader Header;
	Header.Type = EAccelByteAuthMsgType::RSAKey;

	Local.WriteBit(1);
	Local << Header;

#if !PLATFORM_SWITCH
	const uint32 MaxModulusNum = (RSACrypto.GetKeySizeInBits() + 7) >> 3;
	const uint32 MaxExponentNum = (RSACrypto.GetKeySizeInBits() + 7) >> 3;

	// Decrement by one, to allow serialization of #Num == Max#Num
	uint32 ModulusSerializeNum = RSACrypto.GetModulus().Num() - 1;
	uint32 ExponentSerializeNum = RSACrypto.GetPublicExponent().Num() - 1;

	Local.SerializeInt(ModulusSerializeNum, MaxModulusNum);
	Local.Serialize(RSACrypto.GetModulus().GetData(), RSACrypto.GetModulus().Num());

	Local.SerializeInt(ExponentSerializeNum, MaxExponentNum);
	Local.Serialize(RSACrypto.GetPublicExponent().GetData(), RSACrypto.GetPublicExponent().Num());

	Local.Serialize(Packet.GetData(), Packet.GetNumBytes());
#endif
	Packet = Local;
}

void FAuthHandlerComponentAccelByte::UnpackPublicKey(FBitReader& Packet)
{
#if !PLATFORM_SWITCH
	/* Remote's Public Key */
	TArray<uint8> RSARemotePublicExponent;
	/* Remote's Modulus for RSA Key */
	TArray<uint8> RSARemoteModulus;

	const uint32 MaxModulusNum = (RSACrypto.GetKeySizeInBits() + 7) >> 3;
	const uint32 MaxExponentNum = (RSACrypto.GetKeySizeInBits() + 7) >> 3;

	uint32 ModulusNum = 0;

	Packet.SerializeInt(ModulusNum, MaxModulusNum);

	ModulusNum++;

	if (int32(ModulusNum * 8) > RSACrypto.GetKeySizeInBits())
	{
		UE_LOG_AB(Warning, TEXT("AUTH HANDLER: RSA: Modulus size '%i bits' should not exceed key size '%i'"),
			(ModulusNum * 8), RSACrypto.GetKeySizeInBits())

		Packet.SetError();
	}

	if (!Packet.IsError())
	{
		RSARemoteModulus.SetNumUninitialized(ModulusNum);
		Packet.Serialize(RSARemoteModulus.GetData(), ModulusNum);

		uint32 ExponentNum = 0;

		Packet.SerializeInt(ExponentNum, MaxExponentNum);

		ExponentNum++;

		if ((ExponentNum * 8) <= MAX_EXPONENT_BITS)
		{
			RSARemotePublicExponent.SetNumUninitialized(ExponentNum);
			Packet.Serialize(RSARemotePublicExponent.GetData(), ExponentNum);
		}
		else
		{
			UE_LOG_AB(Warning, TEXT("AUTH HANDLER: RSA: Exponent size '%i bits' should not exceed MAX_EXPONENT_BITS"),
				(ExponentNum * 8));

			Packet.SetError();
		}
	}

	if (!Packet.IsError())
	{
		// Make sure the packet has no remaining data now
		RSACrypto.CreateKey(RSARemotePublicExponent, RSARemotePublicExponent, RSARemoteModulus);
	}
#endif
}

bool FAuthHandlerComponentAccelByte::EncryptRSA(FBitWriter& Packet)
{
#if !PLATFORM_SWITCH
	// Serialize size of plain text data
	uint32 NumberOfBytesInPlaintext = static_cast<uint32>(Packet.GetNumBytes());

	TArray<uint8> PlainText;

	// Copy byte stream to PlainText from Packet
	for (int32 i = 0; i < Packet.GetNumBytes(); ++i)
	{
		PlainText.Add(Packet.GetData()[i]);
	}

	Packet.Reset();

	if (NumberOfBytesInPlaintext == 0 || NumberOfBytesInPlaintext > static_cast<uint32>(RSACrypto.GetMaxDataSize()))
	{
		if (NumberOfBytesInPlaintext > static_cast<uint32>(RSACrypto.GetMaxDataSize()))
		{
			UE_LOG_AB(Warning, TEXT("AUTH HANDLER: RSA Encryption skipped as plain text size is too large for this key size. Increase key size or send smaller packets. '%i/%i'."),
				NumberOfBytesInPlaintext, RSACrypto.GetMaxDataSize());
		}

		uint32 NoBytesEncrypted = 0;
		Packet.SerializeIntPacked(NoBytesEncrypted);
		Packet.Serialize(PlainText.GetData(), PlainText.Num());
		return false;
	}

	TArray<uint8> CipherText;

	(RSACrypto.*RSAEncryptor)(PlainText, CipherText);

	// Serialize invalid amount of bytes
	Packet.SerializeIntPacked(NumberOfBytesInPlaintext);

	// Serialize Packet from CipherText
	Packet.Serialize(CipherText.GetData(), CipherText.Num());
#endif
	return true;
}

bool FAuthHandlerComponentAccelByte::DecryptRSA(FBitReader& Packet)
{
#if !PLATFORM_SWITCH
	// Serialize size of plain text data
	uint32 NumberOfBytesInPlaintext;
	Packet.SerializeIntPacked(NumberOfBytesInPlaintext);

	if (NumberOfBytesInPlaintext == 0 || NumberOfBytesInPlaintext > static_cast<uint32>(RSACrypto.GetKeySizeInBytes()))
	{
		// Remove header
		FBitReader Copy(Packet.GetData() + (Packet.GetNumBytes() - Packet.GetBytesLeft()), Packet.GetBitsLeft());
		Packet = Copy;

		if (NumberOfBytesInPlaintext > static_cast<uint32>(RSACrypto.GetKeySizeInBytes()))
		{
			UE_LOG_AB(Warning, TEXT("AUTH HANDLER: RSA Decryption skipped as cipher text size is too large for this key size. Increase key size or send smaller packets. '%i/%i'."),
				NumberOfBytesInPlaintext, static_cast<uint32>(RSACrypto.GetKeySizeInBytes()));
		}
		return false;
	}

	TArray<uint8> PlainText;
	TArray<uint8> CipherText;

	PlainText.AddUninitialized(RSACrypto.GetKeySizeInBytes());
	CipherText.AddUninitialized(RSACrypto.GetKeySizeInBytes());
	Packet.Serialize(CipherText.GetData(), RSACrypto.GetKeySizeInBytes());

	(RSACrypto.*RSADecryptor)(CipherText, PlainText);

	FBitReader Copy(PlainText.GetData(), NumberOfBytesInPlaintext * 8);
	Packet = Copy;
#endif
	return true;
}

void FAuthHandlerComponentAccelByte::SetAuthState(EState InState)
{
	State = InState;
}

int32 FAuthHandlerComponentAccelByte::GetReservedPacketBits() const
{
	if (IsValid())
	{
		if ((State != EState::Initialized) || bEnabledEncryption)
		{
			return 1;
		}
	}

	return 0;
}

void FAuthHandlerComponentAccelByte::SendKeyAES()
{
	FAccelByteAuthHeader Header;
	Header.Type = EAccelByteAuthMsgType::AESKey;

	FBitWriter OutPacket(0, true);
	OutPacket.WriteBit(1);
	OutPacket << Header;
#if !PLATFORM_SWITCH
	// Now send the initialization vector and session key
	FBitWriter TempPacket(0, true);

	TempPacket.Serialize(AESCrypto.GetIV().GetData(), AESCrypto.GetBlockSize());
	TempPacket.Serialize(AESCrypto.GetKey().GetData(), AESCrypto.GetKeySizeInBytes());

	EncryptRSA(TempPacket);

	OutPacket.SerializeBits(TempPacket.GetData(), TempPacket.GetNumBits());
#endif
	SendPacket(OutPacket);
}

bool FAuthHandlerComponentAccelByte::RecvKeyAES(FBitReader& Packet)
{
#if PLATFORM_SWITCH
	SetAuthState(EState::RecvedKey);
	return true;
#else
	// for Client
	DecryptRSA(Packet);

	AESCrypto.GetIV().SetNumUninitialized(AESCrypto.GetBlockSize());
	AESCrypto.GetKey().SetNumUninitialized(AESCrypto.GetKeySizeInBytes());

	Packet.Serialize(AESCrypto.GetIV().GetData(), AESCrypto.GetBlockSize());
	Packet.Serialize(AESCrypto.GetKey().GetData(), AESCrypto.GetKeySizeInBytes());

	if (!Packet.IsError())
	{
		AESCrypto.Initialize();
		SetAuthState(EState::RecvedKey);

		return true;
	}
	else
	{
		UE_LOG_AB(Warning, TEXT("AUTH HANDLER: Failed to initialize AES encryption."));
	}
	return false;
#endif
}

bool FAuthHandlerComponentAccelByte::EncryptAES(FBitWriter& Packet)
{
#if PLATFORM_SWITCH
	return true;
#else
	int32 PacketNumBytes = Packet.GetNumBytes();

	if (PacketNumBytes > 0)
	{
		uint32 NumberOfBitsInPlaintext = Packet.GetNumBits();
		// Pad along 16 byte boundary, in order to encrypt properly
		int32 PaddedSize = (PacketNumBytes + AESCrypto.GetBlockSize() - 1) / AESCrypto.GetBlockSize() * AESCrypto.GetBlockSize();
		if (NumberOfBitsInPlaintext <= MAX_AES_ENCRYPTION_BITS)
		{
			TArray<uint8> PlainText;
			TArray<uint8> CipherText;

			PlainText.AddUninitialized(PaddedSize);
			PlainText[PlainText.Num() - 1] = 0;

			CipherText.AddUninitialized(PaddedSize);
			CipherText[CipherText.Num() - 1] = 0;

			FMemory::Memcpy(PlainText.GetData(), Packet.GetData(), PacketNumBytes);

			if (PaddedSize > PacketNumBytes)
			{
				FMemory::Memzero(PlainText.GetData() + PacketNumBytes, (PaddedSize - PacketNumBytes));
			}

			AESCrypto.Encrypt(PlainText, CipherText, PaddedSize);

			Packet.Reset();

			NumberOfBitsInPlaintext--;
			Packet.SerializeInt(NumberOfBitsInPlaintext, MAX_AES_ENCRYPTION_BITS);
			Packet.Serialize(CipherText.GetData(), PaddedSize);
			return true;
		}
		else
		{
			UE_LOG_AB(Warning, TEXT("AUTH HANDLER: AES: None Encryption: Specified PlainText size exceeds (over: %i/%i/%i Bytes)."), PaddedSize, NumberOfBitsInPlaintext, MAX_AES_ENCRYPTION_BITS);
		}
	}

	return false;
#endif
}

bool FAuthHandlerComponentAccelByte::DecryptAES(FBitReader& Packet)
{
#if PLATFORM_SWITCH
	return true;
#else
	if (!Packet.IsError())
	{
		uint32 NumberOfBitsInPlaintext = 0;
		Packet.SerializeInt(NumberOfBitsInPlaintext, MAX_AES_ENCRYPTION_BITS);
		NumberOfBitsInPlaintext++;

		int32 NumberOfBytesInPlaintext = (NumberOfBitsInPlaintext + 7) >> 3;
		int32 PaddedSize = (NumberOfBytesInPlaintext + AESCrypto.GetBlockSize() - 1) / AESCrypto.GetBlockSize() * AESCrypto.GetBlockSize();

		TArray<uint8> CipherText;
		TArray<uint8> PlainText;

		CipherText.AddUninitialized(PaddedSize);
		PlainText.AddUninitialized(PaddedSize);

		Packet.Serialize(CipherText.GetData(), PaddedSize);

		if (!Packet.IsError())
		{
			AESCrypto.Decrypt(CipherText, PlainText, PaddedSize);
			FBitReader Copy(PlainText.GetData(), NumberOfBitsInPlaintext);
			Packet = Copy;
			return true;
		}
		else
		{
			UE_LOG_AB(Warning, TEXT("AUTH HANDLER: AES: serializing PlainText is error."));
		}
	}
	else
	{
		UE_LOG_AB(Warning, TEXT("AUTH HANDLER: AES: serializing incoming packet is error."));
	}
	return false;
#endif
}

void FAuthHandlerComponentAccelByte::RequestResend()
{
	FAccelByteAuthHeader Header;
	if (Handler::Mode::Server == Handler->Mode)
	{
		if (EState::WaitForAuth != State)
		{
			return;
		}
		Header.Type = EAccelByteAuthMsgType::ResendAuth;
	}
	else
	{
		if (EState::Uninitialized == State)
		{
			Header.Type = EAccelByteAuthMsgType::ResendKey;
		}
		else
		{
			return;
		}
	}

	FBitWriter OutPacket(sizeof(FAccelByteAuthHeader) * 8 + 1, true);
	OutPacket.WriteBit(1);
	OutPacket << Header;

	SendPacket(OutPacket);
}

void FAuthHandlerComponentAccelByte::SendPacket(FBitWriter& OutPacket)
{
	// SendPacket is a low level send
	FOutPacketTraits Traits;
	Handler->SendHandlerPacket(this, OutPacket, Traits);
	LastTimestamp = FPlatformTime::Seconds();
}

bool FAuthHandlerComponentAccelByte::OnSendAuthResult(bool InAuthResult)
{
	FBitWriter OutPacket(sizeof(FAccelByteAuthResult) * 8 + 1, true);
	OutPacket.WriteBit(1);

	FAccelByteAuthResult AllowedPacket;

	AllowedPacket.bWasSuccess = InAuthResult;
	OutPacket << AllowedPacket;

	SendPacket(OutPacket);
	return true;
}

bool FAuthHandlerComponentAccelByte::SendAuthData()
{
	AuthUserData.Empty();
	if (!SetAuthData(AuthUserData.UserId))
	{
		return false;
	}

	FBitWriter TempPacket(0, true);
	TempPacket << AuthUserData;

	if (!EncryptAES(TempPacket))
	{
		UE_LOG_AB(Warning, TEXT("AUTH HANDLER: AES Encryption skipped as plain text size is too large. send smaller packets for secure data. over '%i' bytes."),
			(TempPacket.GetNumBits() - MAX_AES_ENCRYPTION_BITS) / 8);

		return false;
	}

	FBitWriter OutPacket((sizeof(FAccelByteAuthHeader) + FOnlineAuthAccelByte::GetMaxTokenSizeInBytes()) * 8 + 1, true);
	OutPacket.WriteBit(1);
	FAccelByteAuthHeader Header;
	Header.Type = EAccelByteAuthMsgType::Auth;
	OutPacket << Header;
	OutPacket.SerializeBits(TempPacket.GetData(), TempPacket.GetNumBits());

	SendPacket(OutPacket);
	SetAuthState(EState::SentAuth);

	return true;
}

void FAuthHandlerComponentAccelByte::RecvAuthData(FBitReader& Packet)
{
	DecryptAES(Packet);

	Packet << AuthUserData;

	OnAuthenticateUser();
}

void FAuthHandlerComponentAccelByte::OnAuthenticateUser()
{
	/** the DS handler */
	if (AuthInterface->AuthenticateUser(AuthUserData))
	{
		SetAuthState(EState::WaitForAuth);
	}
}

void FAuthHandlerComponentAccelByte::AuthenticateUserResult(bool Result)
{
	if (OnSendAuthResult(Result))
	{
		CompletedHandshaking(Result);
	}

	if (!Result)
	{
		AuthInterface->MarkUserForKick(AuthUserData.UserId);
	}
}

/**
 * set up the client's secure data from Credentials information.
 */
bool FAuthHandlerComponentAccelByte::SetAuthData(FString& UserId)
{
	if (IsServer())
	{
		UE_LOG_AB(Warning, TEXT("AUTH HANDLER: (DS) SetAuthData. the Dedicated Server doesn't need this."));
		return false;
	}

	/** the client handler */
	if(IsValid() && AuthInterface.IsValid())
	{
		if (AuthInterface->GetAuthData(UserId))
		{
			return true;
		}
	}

	return false;
}

bool FAuthHandlerComponentAccelByte::IsServer() const
{
	return OnlineSubsystem != nullptr && OnlineSubsystem->IsServer();
}

/**
 * MODULE INTERFACE
 */
UAuthHandlerComponentAccelByteFactory::UAuthHandlerComponentAccelByteFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

TSharedPtr<HandlerComponent> UAuthHandlerComponentAccelByteFactory::CreateComponentInstance(FString& Options)
{
	return MakeShareable(new FAuthHandlerComponentAccelByte);
}
