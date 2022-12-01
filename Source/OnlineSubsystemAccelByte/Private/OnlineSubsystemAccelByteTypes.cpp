// Copyright (c) 2021 - 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "SocketSubsystem.h"
#include "OnlineSubsystemAccelByteModule.h"
#include "OnlineSubsystemAccelByteDefines.h"
#include "Misc/Base64.h"
#include "JsonObjectConverter.h"

bool IsAccelByteIDValid(const FString& AccelByteId)
{
	// First check if our length is not equal to our typical ID length, if this is the case then we know that this is
	// already an invalid ID
	// Replace the - with empty. The session ID from Session Browser still using vanilla UUID
	const FString FinalAccelByteId = AccelByteId.Replace(TEXT("-"), TEXT(""));
	if (FinalAccelByteId.Len() != ACCELBYTE_ID_LENGTH)
	{
		return false;
	}

	// Iterate through the character array to make sure that the ID is a valid UUID hex string. Subtract one from the
	// length of the array so that we don't include the null terminator that the array has.
	TArray<TCHAR> CharArray = FinalAccelByteId.GetCharArray();
	for (int64 Index = 0; Index < CharArray.Num() - 1; Index++)
	{
		const TCHAR& Character = CharArray[Index];
		if (!CheckTCharIsHex(Character))
		{
			return false;
		}
	}

	return true;
}

#pragma region FAccelByteUniqueIdComposite

FAccelByteUniqueIdComposite::FAccelByteUniqueIdComposite
	( const FString& InId
	, const FString& InPlatformType /*= TEXT("")*/
	, const FString& InPlatformId /*= TEXT("")*/)
	: Id(InId)
	, PlatformType(InPlatformType)
	, PlatformId(InPlatformId)
{
}

bool FAccelByteUniqueIdComposite::operator==(const FAccelByteUniqueIdComposite& OtherComposite) const
{
	return Id == OtherComposite.Id || (PlatformType == OtherComposite.PlatformType && PlatformId == OtherComposite.PlatformId);
}

bool FAccelByteUniqueIdComposite::operator!=(const FAccelByteUniqueIdComposite& OtherComposite) const
{
	return Id != OtherComposite.Id || (PlatformType != OtherComposite.PlatformType && PlatformId != OtherComposite.PlatformId);
}

FString FAccelByteUniqueIdComposite::ToString() const
{
	FString OutString;
	if (!FJsonObjectConverter::UStructToJsonObjectString(*this, OutString))
	{
		return TEXT("FailedToConvert");
	}

	return OutString;
}

#pragma endregion // FAccelByteUniqueIdComposite

#pragma region FUniqueNetIdAccelByteResource

PRAGMA_DISABLE_DEPRECATION_WARNINGS
FUniqueNetIdAccelByteResource::FUniqueNetIdAccelByteResource()
	: FUniqueNetIdString()
{
	Type = ACCELBYTE_SUBSYSTEM;
}

// #NOTE (Maxwell): EOS is also currently just disabling deprecation warnings for FUniqueNetIdString constructors
FUniqueNetIdAccelByteResource::FUniqueNetIdAccelByteResource(const FString& InUniqueNetId)
PRAGMA_DISABLE_DEPRECATION_WARNINGS
	: FUniqueNetIdString(InUniqueNetId, ACCELBYTE_SUBSYSTEM)
PRAGMA_ENABLE_DEPRECATION_WARNINGS
{
}

FUniqueNetIdAccelByteResource::FUniqueNetIdAccelByteResource(FString&& InUniqueNetId)
PRAGMA_DISABLE_DEPRECATION_WARNINGS
	: FUniqueNetIdString(MoveTemp(InUniqueNetId), ACCELBYTE_SUBSYSTEM)
PRAGMA_ENABLE_DEPRECATION_WARNINGS
{
}

FUniqueNetIdAccelByteResource::FUniqueNetIdAccelByteResource(const FUniqueNetId& Src)
PRAGMA_DISABLE_DEPRECATION_WARNINGS
	: FUniqueNetIdString(Src)
PRAGMA_ENABLE_DEPRECATION_WARNINGS
{
	check(GetType() == ACCELBYTE_SUBSYSTEM);
}

FUniqueNetIdAccelByteResource::FUniqueNetIdAccelByteResource(FString&& InUniqueNetId, const FName InType)
	: FUniqueNetIdString(MoveTemp(InUniqueNetId), InType)
{
	check(InType == ACCELBYTE_SUBSYSTEM);
}
PRAGMA_ENABLE_DEPRECATION_WARNINGS

FUniqueNetIdAccelByteResourceRef FUniqueNetIdAccelByteResource::Cast(const FUniqueNetId& NetId)
{
	if (ensure(NetId.GetType() == ACCELBYTE_SUBSYSTEM))
	{
		return StaticCastSharedRef<const FUniqueNetIdAccelByteResource>(NetId.AsShared());
	}

	return Invalid();
}

const FUniqueNetIdAccelByteResourceRef FUniqueNetIdAccelByteResource::Invalid()
{
	PRAGMA_DISABLE_DEPRECATION_WARNINGS
	return MakeShared<FUniqueNetIdAccelByteResource>(ACCELBYTE_INVALID_ID_VALUE);
	PRAGMA_ENABLE_DEPRECATION_WARNINGS
}

FName FUniqueNetIdAccelByteResource::GetType() const
{
	return ACCELBYTE_SUBSYSTEM;
}

bool FUniqueNetIdAccelByteResource::IsValid() const
{
	return IsAccelByteIDValid(UniqueNetIdStr);
}

#pragma endregion  // FUniqueNetIdAccelByteResource

#pragma region FUniquneNetIdAccelByteUser

PRAGMA_DISABLE_DEPRECATION_WARNINGS
FUniqueNetIdAccelByteUser::FUniqueNetIdAccelByteUser()
	: FUniqueNetIdAccelByteResource()
{
}

FUniqueNetIdAccelByteUser::FUniqueNetIdAccelByteUser(const FString& InUniqueNetId)
	: FUniqueNetIdAccelByteResource(InUniqueNetId)
{
	DecodeIDElements();
}

FUniqueNetIdAccelByteUser::FUniqueNetIdAccelByteUser(FString&& InUniqueNetId)
	: FUniqueNetIdAccelByteResource(MoveTemp(InUniqueNetId))
{
	DecodeIDElements();
}

FUniqueNetIdAccelByteUser::FUniqueNetIdAccelByteUser(const FUniqueNetId& Src)
	: FUniqueNetIdAccelByteResource(Src)
{
	DecodeIDElements();
}

FUniqueNetIdAccelByteUser::FUniqueNetIdAccelByteUser(FString&& InUniqueNetId, const FName InType)
	: FUniqueNetIdAccelByteResource(MoveTemp(InUniqueNetId), InType)
{
	DecodeIDElements();
}

FUniqueNetIdAccelByteUser::FUniqueNetIdAccelByteUser(const FAccelByteUniqueIdComposite& CompositeId, const FString& EncodedComposite)
	: FUniqueNetIdAccelByteResource(EncodedComposite)
	, CompositeStructure(CompositeId)
{
	// Check if this ID is valid and cache it so that we can cut down on processing of the ID later
	if (!bHasCachedValidState)
	{
		bCachedValidState = IsValid();
		bHasCachedValidState = true;
	}
}
PRAGMA_ENABLE_DEPRECATION_WARNINGS

FUniqueNetIdAccelByteUserRef FUniqueNetIdAccelByteUser::Create(const FAccelByteUniqueIdComposite& CompositeId)
{
	if (CompositeId.Id.IsEmpty())
	{
		UE_LOG_AB(Warning, TEXT("Failed to create FUniqueNetIdAccelByte as we don't have a value for the AccelByte ID!"));
		return Invalid();
	}

	FString CompositeString;
	if (!FJsonObjectConverter::UStructToJsonObjectString(CompositeId, CompositeString))
	{
		UE_LOG_AB(Warning, TEXT("Failed to convert composite structure for an FUniqueNetIdAccelByte to a JSON string!"));
		return Invalid();
	}

	FString EncodedString = FBase64::Encode(CompositeString);
	if (EncodedString.IsEmpty())
	{
		UE_LOG_AB(Warning, TEXT("Failed to encode composite structure for an FUniqueNetIdAccelByte to a Base64 string!"));
		return Invalid();
	}

	PRAGMA_DISABLE_DEPRECATION_WARNINGS
	return MakeShared<const FUniqueNetIdAccelByteUser>(CompositeId, EncodedString);
	PRAGMA_ENABLE_DEPRECATION_WARNINGS
}

FUniqueNetIdAccelByteUserRef FUniqueNetIdAccelByteUser::Create(const FString& InUniqueNetId)
{
	// Check if this is a Base64 encoded string, meaning that it _most likely_ is a full composite ID. If it is, just pass
	// the string straight to MakeShared, as it will decode the components for you. Otherwise, treat it as just the AccelByte ID.
	FString DecodedString;
	if (!FBase64::Decode(InUniqueNetId, DecodedString))
	{
		return Create(FAccelByteUniqueIdComposite(InUniqueNetId));
	}

	PRAGMA_DISABLE_DEPRECATION_WARNINGS
	return MakeShared<const FUniqueNetIdAccelByteUser>(InUniqueNetId);
	PRAGMA_ENABLE_DEPRECATION_WARNINGS
}

FUniqueNetIdAccelByteUserRef FUniqueNetIdAccelByteUser::Create(const FUniqueNetId& Src)
{
	PRAGMA_DISABLE_DEPRECATION_WARNINGS
	return MakeShared<const FUniqueNetIdAccelByteUser>(FUniqueNetIdAccelByteUser(Src.ToString(), Src.GetType()));
	PRAGMA_ENABLE_DEPRECATION_WARNINGS
}

TSharedRef<const FUniqueNetIdAccelByteUser> FUniqueNetIdAccelByteUser::Cast(const FUniqueNetId& NetId)
{
	if (ensure(NetId.GetType() == ACCELBYTE_SUBSYSTEM))
	{
		return StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(NetId.AsShared());
	}

	return Invalid();
}

TSharedRef<const FUniqueNetIdAccelByteUser> FUniqueNetIdAccelByteUser::Invalid()
{
	PRAGMA_DISABLE_DEPRECATION_WARNINGS
	return MakeShared<FUniqueNetIdAccelByteUser>(ACCELBYTE_INVALID_ID_VALUE);
	PRAGMA_ENABLE_DEPRECATION_WARNINGS
}

FName FUniqueNetIdAccelByteUser::GetType() const
{
	return ACCELBYTE_SUBSYSTEM;
}

bool FUniqueNetIdAccelByteUser::IsValid() const
{
	if (bHasCachedValidState)
	{
		return bCachedValidState;
	}

	// Since our most important piece of the ID is the encoded string, we will crack that open and check validity of that.
	// As well as check if there is a mismatch between our individual elements and the JSON representation itself, which
	// would indicate a major issue.
	FString JSONString;
	if (!FBase64::Decode(UniqueNetIdStr, JSONString))
	{
		UE_LOG_AB(VeryVerbose, TEXT("UniqueID validity test failed: unable to decode Base64 ID with string value of '%s'!"), *UniqueNetIdStr);
		return false;
	}

	FAccelByteUniqueIdComposite DecodedComposite;
	if (!FJsonObjectConverter::JsonObjectStringToUStruct(JSONString, &DecodedComposite, 0, 0))
	{
		UE_LOG_AB(VeryVerbose, TEXT("UniqueID validity test failed: unable to decode JSON composite object with string value of '%s'!"), *JSONString);
		return false;
	}

	if (CompositeStructure != DecodedComposite)
	{
		UE_LOG_AB(VeryVerbose, TEXT("UniqueID validity test failed: underlying components of ID and JSON composite object components do not match! JSON object: %s; Underlying components: %s"), *JSONString, *CompositeStructure.ToString());
		return false;
	}

	if (CompositeStructure.Id.IsEmpty() || !IsAccelByteIDValid(CompositeStructure.Id))
	{
		UE_LOG_AB(VeryVerbose, TEXT("UniqueID validity test failed: ID field for AccelByte ID '%s' is an invalid format!"), *CompositeStructure.Id);
		return false;
	}

	return true;
}

FString FUniqueNetIdAccelByteUser::ToDebugString() const
{
	// Convert our ID object to a JSON object string without pretty printing (that option is that last 'false' flag on the method call, sigh)
	FString OutString;
	if (!FJsonObjectConverter::UStructToJsonObjectString(CompositeStructure, OutString, 0, 0, 0, nullptr, false))
	{
		return ACCELBYTE_INVALID_ID_VALUE;
	}

	return OutString;
}

FString FUniqueNetIdAccelByteUser::GetAccelByteId() const
{
	return CompositeStructure.Id;
}

FString FUniqueNetIdAccelByteUser::GetPlatformType() const
{
	return CompositeStructure.PlatformType;
}

FString FUniqueNetIdAccelByteUser::GetPlatformId() const
{
	return CompositeStructure.PlatformId;
}

bool FUniqueNetIdAccelByteUser::HasPlatformInformation() const
{
	return !CompositeStructure.PlatformType.IsEmpty() && !CompositeStructure.PlatformId.IsEmpty();
}

TSharedPtr<const FUniqueNetId> FUniqueNetIdAccelByteUser::GetPlatformUniqueId() const
{
	if (!HasPlatformInformation())
	{
		UE_LOG_AB(Warning, TEXT("Cannot convert composite platform information for AccelByte ID as we do not have any platform information set! ID composite object: %s"), *ToDebugString());
		return nullptr;
	}

	const IOnlineSubsystem* NativeSubsystem = IOnlineSubsystem::GetByPlatform();
	if (NativeSubsystem == nullptr)
	{
		UE_LOG_AB(Warning, TEXT("Cannot convert composite platform information for AccelByte ID as we do not have a native platform OSS set!"));
		return nullptr;
	}

	const IOnlineIdentityPtr IdentityInterface = NativeSubsystem->GetIdentityInterface();
	if (!IdentityInterface.IsValid())
	{
		UE_LOG_AB(Warning, TEXT("Cannot convert composite platform information for AccelByte ID as the native platform OSS has an invalid identity interface instance!"));
		return nullptr;
	}

	TSharedPtr<const FUniqueNetId> NativeUniqueId = IdentityInterface->CreateUniquePlayerId(CompositeStructure.PlatformId);
	if (!NativeUniqueId.IsValid() || !NativeUniqueId->IsValid())
	{
		UE_LOG_AB(Warning, TEXT("Cannot convert composite platform information for AccelByte ID as the resulting unique ID from the subsystem was invalid!"));
		return nullptr;
	}

	return NativeUniqueId;
}

FAccelByteUniqueIdComposite FUniqueNetIdAccelByteUser::GetCompositeStructure() const
{
	return CompositeStructure;
}

bool FUniqueNetIdAccelByteUser::Compare(const FUniqueNetId& Other) const
{
	if (Other.GetType() == ACCELBYTE_SUBSYSTEM)
	{
		const TSharedRef<const FUniqueNetIdAccelByteUser> OtherCompositeId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(Other.AsShared());

		// First check whether AccelByte IDs match, if they do then these IDs are definitely equal
		if (GetAccelByteId() == OtherCompositeId->GetAccelByteId())
		{
			return true;
		}
		// Otherwise, check if both IDs have platform information attached...
		else if (HasPlatformInformation() && OtherCompositeId->HasPlatformInformation())
		{
			// If they do, then check whether the platform type and platform ID for both match
			return GetPlatformType() == OtherCompositeId->GetPlatformType() && GetPlatformId() == OtherCompositeId->GetPlatformId();
		}

		return false;
	}

	return FUniqueNetIdString::Compare(Other);
}

void FUniqueNetIdAccelByteUser::DecodeIDElements()
{
	// If this is supposed to be an invalid ID, then just return that accordingly
	if (UniqueNetIdStr == ACCELBYTE_INVALID_ID_VALUE)
	{
		CompositeStructure.Id = ACCELBYTE_INVALID_ID_VALUE;
		return;
	}

	FString JSONString;
	if (!FBase64::Decode(UniqueNetIdStr, JSONString))
	{
		UE_LOG_AB(Warning, TEXT("Failed to decode ID with string value of '%s' to Base64 AccelByte composite ID format!"), *UniqueNetIdStr);
		return;
	}

	if (!FJsonObjectConverter::JsonObjectStringToUStruct(JSONString, &CompositeStructure, 0, 0))
	{
		UE_LOG_AB(Warning, TEXT("Failed to get JSON object for AccelByte composite ID from decoded string value of '%s'!"), *JSONString);
		return;
	}

	// Finally, cache a valid state from this ID if we haven't yet
	if (!bHasCachedValidState)
	{
		bCachedValidState = IsValid();
		bHasCachedValidState = true;
	}
}

#pragma endregion // FUniquneNetIdAccelByteUser

#pragma region FOnlineSessionInfoAccelByte

FOnlineSessionInfoAccelByteV1::FOnlineSessionInfoAccelByteV1()
	: FOnlineSessionInfo()
	, HostAddr(ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr())
	, RemoteId(TEXT(""))
	, SessionId(MakeShared<FUniqueNetIdAccelByteResource>(ACCELBYTE_INVALID_ID_VALUE))
{
}

FOnlineSessionInfoAccelByteV1::FOnlineSessionInfoAccelByteV1(const FOnlineSessionInfoAccelByteV1& Other)
	: FOnlineSessionInfo(Other)
	, HostAddr(Other.HostAddr->Clone())
	, RemoteId(Other.RemoteId)
	, SessionId(Other.SessionId)
	, Teams(Other.Teams)
	, Parties(Other.Parties)
{
}

bool FOnlineSessionInfoAccelByteV1::operator==(const FOnlineSessionInfoAccelByteV1& Other) const
{
	return false;
}

FOnlineSessionInfoAccelByteV1& FOnlineSessionInfoAccelByteV1::operator=(const FOnlineSessionInfoAccelByteV1& Src)
{
	return *this;
}

const uint8* FOnlineSessionInfoAccelByteV1::GetBytes() const
{
	return nullptr;
}

int32 FOnlineSessionInfoAccelByteV1::GetSize() const
{
	return sizeof(uint64) + sizeof(TSharedPtr<class FInternetAddr>);
}

bool FOnlineSessionInfoAccelByteV1::IsValid() const
{
	const bool bIsValidDedicatedSession = (SessionId->IsValid() && (HostAddr.IsValid() && HostAddr->IsValid()));
	const bool bIsValidP2PSession = !RemoteId.IsEmpty();
	return bIsValidDedicatedSession || bIsValidP2PSession;
}

FString FOnlineSessionInfoAccelByteV1::ToString() const
{
	return SessionId->ToString();
}

void FOnlineSessionInfoAccelByteV1::SetupP2PRelaySessionInfo(const FOnlineSubsystemAccelByte& Subsystem)
{
	// Read the IP from the system
	bool bCanBindAll;
	HostAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetLocalHostAddr(*GLog, bCanBindAll);

	// The below is a workaround for systems that set hostname to a distinct address from 127.0.0.1 on a loopback interface.
	// See e.g. https://www.debian.org/doc/manuals/debian-reference/ch05.en.html#_the_hostname_resolution
	// and http://serverfault.com/questions/363095/why-does-my-hostname-appear-with-the-address-127-0-1-1-rather-than-127-0-0-1-in
	// Since we bind to 0.0.0.0, we won't answer on 127.0.1.1, so we need to advertise ourselves as 127.0.0.1 for any other loopback address we may have.
	uint32 HostIp = 0; // will return in host order
	// if this address is on loopback interface, advertise it as 127.0.0.1
	HostAddr->GetIp(HostIp);
	if ((HostIp & 0xff000000) == 0x7f000000)
	{
		HostAddr->SetIp(0x7f000001);	// 127.0.0.1
	}

	// Now set the port that was configured
	HostAddr->SetPort(GetPortFromNetDriver(Subsystem.GetInstanceName()));

	FGuid OwnerGuid;
	FPlatformMisc::CreateGuid(OwnerGuid);
	SessionId = MakeShared<FUniqueNetIdAccelByteResource>(OwnerGuid.ToString());
}

FString FOnlineSessionInfoAccelByteV1::ToDebugString() const
{
	if (!RemoteId.IsEmpty())
	{
		return FString::Printf(TEXT("ID: %s SessionId: %s"),
			*RemoteId,
			*SessionId->ToDebugString());
	}
	return FString::Printf(TEXT("HOST: %s SessionId: %s"),
		*RemoteId,
		*SessionId->ToDebugString());
}

const FString& FOnlineSessionInfoAccelByteV1::GetRemoteId() const
{
	return RemoteId;
}

void FOnlineSessionInfoAccelByteV1::SetRemoteId(const FString& InRemoteId)
{
	RemoteId = InRemoteId;
}

TSharedRef<FUniqueNetIdAccelByteResource> FOnlineSessionInfoAccelByteV1::GetSessionIdRef() const
{
	return SessionId;
}

const FUniqueNetId& FOnlineSessionInfoAccelByteV1::GetSessionId() const
{
	return SessionId.Get();
}

void FOnlineSessionInfoAccelByteV1::SetSessionId(const FString& InSessionId)
{
	SessionId = MakeShared<FUniqueNetIdAccelByteResource>(InSessionId);
}

TSharedPtr<FInternetAddr> FOnlineSessionInfoAccelByteV1::GetHostAddr() const
{
	return HostAddr;
}

void FOnlineSessionInfoAccelByteV1::SetHostAddr(const TSharedRef<FInternetAddr>& InHostAddr)
{
	HostAddr = InHostAddr;
}

bool FOnlineSessionInfoAccelByteV1::HasTeamInfo() const
{
	return Teams.Num() > 0;
}

int32 FOnlineSessionInfoAccelByteV1::GetTeamIndex(const FUniqueNetId& UserId) const
{
	const int32* FoundTeamIndex = Teams.Find(UserId.AsShared());
	if (FoundTeamIndex != nullptr)
	{
		return *FoundTeamIndex;
	}

	return INDEX_NONE;
}

const TUniqueNetIdMap<int32>& FOnlineSessionInfoAccelByteV1::GetTeams() const
{
	return Teams;
}

void FOnlineSessionInfoAccelByteV1::SetTeams(const TUniqueNetIdMap<int32>& InTeams)
{
	Teams = InTeams;
	OnTeamInformationReceivedDelegate.ExecuteIfBound(Teams);
}

bool FOnlineSessionInfoAccelByteV1::HasPartyInfo() const
{
	return Parties.Num() > 0;
}

const TSessionPartyArray& FOnlineSessionInfoAccelByteV1::GetParties() const
{
	return Parties;
}


void FOnlineSessionInfoAccelByteV1::SetParties(const TSessionPartyArray& InParties)
{
	Parties = InParties;
	OnPartyInformationReceivedDelegate.ExecuteIfBound(Parties);
}

const FAccelByteModelsMatchmakingResult& FOnlineSessionInfoAccelByteV1::GetSessionResult() const
{
	return SessionResult;
}

void FOnlineSessionInfoAccelByteV1::SetSessionResult(const FAccelByteModelsMatchmakingResult& InSessionResult)
{
	SessionResult = InSessionResult;
}

#pragma endregion // FOnlineSessionInfoAccelByte

#pragma region FUserOnlineAccountAccelByte

FUserOnlineAccountAccelByte::FUserOnlineAccountAccelByte
	( const FString& InUserId /*= TEXT("")*/ )
	: UserIdRef(MakeShared<const FUniqueNetIdAccelByteUser>(InUserId))
{
}

FUserOnlineAccountAccelByte::FUserOnlineAccountAccelByte
	( const TSharedRef<const FUniqueNetId>& InUserId )
	: UserIdRef(StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(InUserId))
{
}

FUserOnlineAccountAccelByte::FUserOnlineAccountAccelByte
	( const TSharedRef<const FUniqueNetId>& InUserId
	, const FString& InDisplayName )
	: UserIdRef(StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(InUserId))
	, DisplayName(InDisplayName)
{
}

FUserOnlineAccountAccelByte::FUserOnlineAccountAccelByte(const FAccelByteUniqueIdComposite& InCompositeId)
	: UserIdRef(FUniqueNetIdAccelByteUser::Create(InCompositeId))
{
}

bool FUserOnlineAccountAccelByte::GetAuthAttribute(const FString& AttrName, FString& OutAttrValue) const
{
	const FString* FoundAttr = AdditionalAuthData.Find(AttrName);
	if (FoundAttr)
	{
		OutAttrValue = *FoundAttr;
		return true;
	}

	return false;
}

FString FUserOnlineAccountAccelByte::GetRealName() const
{
	return DisplayName;
}

FString FUserOnlineAccountAccelByte::GetDisplayName(const FString& /*Platform*/) const
{
	return DisplayName;
}

void FUserOnlineAccountAccelByte::SetDisplayName(const FString& InDisplayName)
{
	DisplayName = InDisplayName;
}

FString FUserOnlineAccountAccelByte::GetAccessToken() const
{
	return AccessToken;
}

void FUserOnlineAccountAccelByte::SetAccessToken(const FString& InAccessToken)
{
	AccessToken = InAccessToken;
}

bool FUserOnlineAccountAccelByte::GetUserAttribute(const FString& AttrName, FString& OutAttrValue) const
{
	const FString* FoundAttr = UserAttributes.Find(AttrName);
	if (FoundAttr != nullptr)
	{
		OutAttrValue = *FoundAttr;
		return true;
	}

	return false;
}

bool FUserOnlineAccountAccelByte::SetUserLocalAttribute(const FString& AttrName, const FString& AttrValue)
{
	return SetUserAttribute(AttrName, AttrValue);
}

bool FUserOnlineAccountAccelByte::SetUserAttribute(const FString& AttrName, const FString& AttrValue)
{
	const FString* FoundAttr = UserAttributes.Find(AttrName);
	if (FoundAttr == nullptr || *FoundAttr != AttrValue)
	{
		UserAttributes.Add(AttrName, AttrValue);
		return true;
	}

	return false;
}

bool FUserOnlineAccountAccelByte::IsConnectedToLobby() const
{
	return bIsConnectedToLobby;
}

void FUserOnlineAccountAccelByte::SetConnectedToLobby(bool bIsConnected)
{
	bIsConnectedToLobby = bIsConnected;
}

bool FUserOnlineAccountAccelByte::IsConnectedToChat() const
{
	return bIsConnectedToChat;
}

void FUserOnlineAccountAccelByte::SetConnectedToChat(bool bIsConnected)
{
	bIsConnectedToChat = bIsConnected;
}

#pragma endregion // FUserOnlineAccountAccelByte
