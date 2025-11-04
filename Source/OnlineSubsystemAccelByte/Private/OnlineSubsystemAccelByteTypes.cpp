// Copyright (c) 2021 - 2024 AccelByte Inc. All Rights Reserved.
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
#include "Core/AccelByteTypeConverter.h"
#include "OnlineSubsystemAccelByteLog.h"

void FNotificationMessageManager::PublishToTopic(FString const& InTopic, const FAccelByteModelsNotificationMessage& InMessage, int32 InLocalUserNum)
{
	FOnBroadcastLobbyNotification* LocalUserSubscribers = NotificationMap.Find(InTopic);

	if (LocalUserSubscribers)
	{
		LocalUserSubscribers->Broadcast(InMessage, InLocalUserNum);
	}
}

FDelegateHandle FNotificationMessageManager::SubscribeToTopic(FString const& InTopic, FOnNotificationMessageReceived const& InDelegate)
{
	FOnBroadcastLobbyNotification LocalUserSubscribers = NotificationMap.FindOrAdd(InTopic);

	return LocalUserSubscribers.Add(InDelegate);
}

bool FNotificationMessageManager::UnsubscribeAllDelegatesFromTopic(FString const& InTopic)
{
	bool bResult = false;
	FOnBroadcastLobbyNotification* LocalUserSubscribers = NotificationMap.Find(InTopic);

	if (LocalUserSubscribers)
	{
		LocalUserSubscribers->Clear();
		bResult = true;
	}

	return bResult;
}

bool FNotificationMessageManager::UnsubscribeFromTopic(FString const& InTopic, FDelegateHandle const& InDelegate)
{
	bool bResult = false;
	FOnBroadcastLobbyNotification* LocalUserSubscribers = NotificationMap.Find(InTopic);

	if (LocalUserSubscribers)
	{
		bResult = LocalUserSubscribers->Remove(InDelegate);
	}

	return bResult;
}

bool IsAccelByteIDValid(FString const& AccelByteId)
{
	return FAccelByteUtilities::IsAccelByteIDValid(AccelByteId);
}

#pragma region FAccelByteUniqueIdComposite

FAccelByteUniqueIdComposite::FAccelByteUniqueIdComposite(FString const& InId
	, FString const& InPlatformType /*= TEXT("")*/
	, FString const& InPlatformId /*= TEXT("")*/)
	: Id(InId)
	, PlatformType(InPlatformType)
	, PlatformId(InPlatformId)
{
}

bool FAccelByteUniqueIdComposite::operator==(FAccelByteUniqueIdComposite const& OtherComposite) const
{
	return Id == OtherComposite.Id || (PlatformType == OtherComposite.PlatformType && PlatformId == OtherComposite.PlatformId);
}

bool FAccelByteUniqueIdComposite::operator!=(FAccelByteUniqueIdComposite const& OtherComposite) const
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

FUniqueNetIdAccelByteResource::FUniqueNetIdAccelByteResource()
PRAGMA_DISABLE_DEPRECATION_WARNINGS
	: FUniqueNetIdString()
PRAGMA_ENABLE_DEPRECATION_WARNINGS
{
	Type = ACCELBYTE_RESOURCE_ID_TYPE;
}

FUniqueNetIdAccelByteResource::FUniqueNetIdAccelByteResource(FString const& InNetIdStr)
PRAGMA_DISABLE_DEPRECATION_WARNINGS
	: FUniqueNetIdString(InNetIdStr, ACCELBYTE_RESOURCE_ID_TYPE)
PRAGMA_ENABLE_DEPRECATION_WARNINGS
{
}

FUniqueNetIdAccelByteResource::FUniqueNetIdAccelByteResource(FString && InNetIdStr)
PRAGMA_DISABLE_DEPRECATION_WARNINGS
	: FUniqueNetIdString(MoveTemp(InNetIdStr), ACCELBYTE_RESOURCE_ID_TYPE)
PRAGMA_ENABLE_DEPRECATION_WARNINGS
{
}

FUniqueNetIdAccelByteResource::FUniqueNetIdAccelByteResource(FUniqueNetId const& InNetId)
PRAGMA_DISABLE_DEPRECATION_WARNINGS
	: FUniqueNetIdString(InNetId)
PRAGMA_ENABLE_DEPRECATION_WARNINGS
{
}

FUniqueNetIdAccelByteResource::FUniqueNetIdAccelByteResource(FString const& InNetIdStr
	, FName const InType)
PRAGMA_DISABLE_DEPRECATION_WARNINGS
	: FUniqueNetIdString(InNetIdStr, InType)
PRAGMA_ENABLE_DEPRECATION_WARNINGS
{
}

FUniqueNetIdAccelByteResource::FUniqueNetIdAccelByteResource(FString && InNetIdStr
	, FName const InType)
PRAGMA_DISABLE_DEPRECATION_WARNINGS
	: FUniqueNetIdString(MoveTemp(InNetIdStr), InType)
PRAGMA_ENABLE_DEPRECATION_WARNINGS
{
}

FUniqueNetIdAccelByteResourceRef FUniqueNetIdAccelByteResource::Invalid()
{
	FString InvalidId = ACCELBYTE_INVALID_ID_VALUE;
	FUniqueNetIdAccelByteResource* Resource = new FUniqueNetIdAccelByteResource(MoveTemp(InvalidId), ACCELBYTE_RESOURCE_ID_TYPE);
	return MakeShareable(Resource);
}

FUniqueNetIdAccelByteResourcePtr FUniqueNetIdAccelByteResource::TryCast(FUniqueNetId const& InNetId)
{
	return TryCast(InNetId.AsShared());
}

FUniqueNetIdAccelByteResourcePtr FUniqueNetIdAccelByteResource::TryCast(FUniqueNetIdRef const& InNetIdRef)
{
	if (ensure(InNetIdRef->GetType() == ACCELBYTE_RESOURCE_ID_TYPE))
	{
		return StaticCastSharedRef<FUniqueNetIdAccelByteResource const>(InNetIdRef);
	}

	return nullptr;
}

FUniqueNetIdAccelByteResourceRef FUniqueNetIdAccelByteResource::Cast(FUniqueNetId const& InNetId)
{
	FUniqueNetIdAccelByteResourcePtr ResourcePtr = TryCast(InNetId);

	if (!ResourcePtr.IsValid())
	{
		return Invalid();
	}

	return ResourcePtr.ToSharedRef();
}

FName FUniqueNetIdAccelByteResource::GetType() const
{
	return ACCELBYTE_RESOURCE_ID_TYPE;
}

bool FUniqueNetIdAccelByteResource::IsValid() const
{
	return IsAccelByteIDValid(UniqueNetIdStr);
}

FUniqueNetIdAccelByteResourceRef FUniqueNetIdAccelByteResource::CastChecked(FUniqueNetId const& InNetId)
{
	return CastChecked(InNetId.AsShared());
}

FUniqueNetIdAccelByteResourceRef FUniqueNetIdAccelByteResource::CastChecked(FUniqueNetIdRef const& InNetIdRef)
{
	if (InNetIdRef->GetType() != ACCELBYTE_RESOURCE_ID_TYPE)
	{
		return Invalid();
	};
	return StaticCastSharedRef<const FUniqueNetIdAccelByteResource>(InNetIdRef);
}

#pragma endregion  // FUniqueNetIdAccelByteResource

#pragma region FUniquneNetIdAccelByteUser

FUniqueNetIdAccelByteUser::FUniqueNetIdAccelByteUser()
	: FUniqueNetIdAccelByteResource(TEXT(""), ACCELBYTE_USER_ID_TYPE)
{
}

FUniqueNetIdAccelByteUser::FUniqueNetIdAccelByteUser(FString const& InNetIdStr)
	: FUniqueNetIdAccelByteResource(InNetIdStr, ACCELBYTE_USER_ID_TYPE)
{
	DecodeIDElements();
}

FUniqueNetIdAccelByteUser::FUniqueNetIdAccelByteUser(FString && InNetIdStr)
	: FUniqueNetIdAccelByteResource(MoveTemp(InNetIdStr), ACCELBYTE_USER_ID_TYPE)
{
	DecodeIDElements();
}

FUniqueNetIdAccelByteUser::FUniqueNetIdAccelByteUser(FUniqueNetId const& InNetId)
	: FUniqueNetIdAccelByteResource(InNetId.ToString(), ACCELBYTE_USER_ID_TYPE)
{
	DecodeIDElements();
}

FUniqueNetIdAccelByteUser::FUniqueNetIdAccelByteUser(FString const& InNetIdStr
	, FName const InType)
	: FUniqueNetIdAccelByteResource(InNetIdStr, InType)
{
	DecodeIDElements();
}

FUniqueNetIdAccelByteUser::FUniqueNetIdAccelByteUser(FString && InNetIdStr
	, FName const InType)
	: FUniqueNetIdAccelByteResource(MoveTemp(InNetIdStr), InType)
{
	DecodeIDElements();
}

FUniqueNetIdAccelByteUser::FUniqueNetIdAccelByteUser(FAccelByteUniqueIdComposite const& CompositeId
	, FString const& EncodedComposite)
	: FUniqueNetIdAccelByteResource(EncodedComposite, ACCELBYTE_USER_ID_TYPE)
	, CompositeStructure(CompositeId)
{
	// Check if this ID is valid and cache it so that we can cut down on processing of the ID later
	if (!bHasCachedValidState)
	{
		bCachedValidState = IsValid();
		bHasCachedValidState = true;
	}
}

FUniqueNetIdAccelByteUserRef FUniqueNetIdAccelByteUser::Create(FAccelByteUniqueIdComposite const& CompositeId)
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

	FUniqueNetIdAccelByteUser* User = new FUniqueNetIdAccelByteUser(MoveTemp(EncodedString), ACCELBYTE_USER_ID_TYPE);
	User->CompositeStructure = CompositeId;
	
	return MakeShareable(User);
}

FUniqueNetIdAccelByteUserRef FUniqueNetIdAccelByteUser::Create(FString const& InNetIdStr)
{
	// Prioritize check 32-character alphanumeric rather than proceed to decode & deserialize
	bool bIsAccelByteUUIDFormat = FAccelByteUtilities::IsAccelByteIDValid(InNetIdStr);
	if (bIsAccelByteUUIDFormat)
	{
		return Create(FAccelByteUniqueIdComposite(InNetIdStr));
	}

	// Check if this is a Base64 encoded string first before anything. If it is, then we want to check if we can parse
	// JSON from it. If so, then we just want to pass it directly into a new instance of a FUniqueNetIdAccelByteUser.
	// Otherwise, we want to pass the string directly as the AccelByte ID component of a new FUniqueNetIdAccelByteUser's
	// encoded data.
	FString DecodedString{};
	FAccelByteUniqueIdComposite CompositeId{};

	const bool bIsEncodedCompositeString = FBase64::Decode(InNetIdStr, DecodedString) &&
		FAccelByteJsonConverter::JsonObjectStringToUStruct(DecodedString, &CompositeId);
	if (!bIsEncodedCompositeString)
	{
		return Create(FAccelByteUniqueIdComposite(InNetIdStr));
	}

	return MakeShareable<FUniqueNetIdAccelByteUser const>(new FUniqueNetIdAccelByteUser(CompositeId, InNetIdStr));
}

FUniqueNetIdAccelByteUserRef FUniqueNetIdAccelByteUser::Create(FUniqueNetId const& InNetId)
{
	return MakeShareable<FUniqueNetIdAccelByteUser const>(new FUniqueNetIdAccelByteUser(InNetId));
}

FUniqueNetIdAccelByteUserRef FUniqueNetIdAccelByteUser::Invalid()
{
	return MakeShareable<FUniqueNetIdAccelByteUser const>(new FUniqueNetIdAccelByteUser(ACCELBYTE_INVALID_ID_VALUE));
}

FUniqueNetIdAccelByteUserPtr FUniqueNetIdAccelByteUser::TryCast(FUniqueNetId const& InNetId)
{
	return TryCast(InNetId.AsShared());
}

FUniqueNetIdAccelByteUserPtr FUniqueNetIdAccelByteUser::TryCast(FUniqueNetIdRef const& InNetIdRef)
{
	if (ensure(InNetIdRef->GetType() == ACCELBYTE_USER_ID_TYPE))
	{
		return StaticCastSharedRef<FUniqueNetIdAccelByteUser const>(InNetIdRef);
	}

	return nullptr;
}

FUniqueNetIdAccelByteUserRef FUniqueNetIdAccelByteUser::Cast(FUniqueNetId const& InNetId)
{
	FUniqueNetIdAccelByteUserPtr UserPtr = TryCast(InNetId);

	if (!UserPtr.IsValid())
	{
		return Invalid();
	}

	return UserPtr.ToSharedRef();
}

FUniqueNetIdAccelByteUserRef FUniqueNetIdAccelByteUser::CastChecked(FUniqueNetId const& InNetId)
{
	return CastChecked(InNetId.AsShared());
}

FUniqueNetIdAccelByteUserRef FUniqueNetIdAccelByteUser::CastChecked(FUniqueNetIdRef const& InNetIdRef)
{
	if (InNetIdRef->GetType() != ACCELBYTE_USER_ID_TYPE) 
	{ 
		return Invalid();
	}
	return StaticCastSharedRef<FUniqueNetIdAccelByteUser const>(InNetIdRef);
}

FName FUniqueNetIdAccelByteUser::GetType() const
{
	return ACCELBYTE_USER_ID_TYPE;
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
	if (!FAccelByteJsonConverter::JsonObjectStringToUStruct(JSONString, &DecodedComposite))
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
	// Apin: using FJsonObjectConverter::UStructToJsonObjectString make crashes when OSS shutdown
	return FString::Printf(TEXT("{\"id\": \"%s\", \"platformType\": \"%s\", \"platformId\": \"%s\"}"),
		*CompositeStructure.Id, *CompositeStructure.PlatformType, *CompositeStructure.PlatformId);
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

FUniqueNetIdPtr FUniqueNetIdAccelByteUser::GetPlatformUniqueId() const
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

	FUniqueNetIdPtr NativeUniqueId = IdentityInterface->CreateUniquePlayerId(CompositeStructure.PlatformId);
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

bool FUniqueNetIdAccelByteUser::Compare(FUniqueNetId const& Other) const
{
	if (Other.GetType() == ACCELBYTE_USER_ID_TYPE)
	{
		const FUniqueNetIdAccelByteUserRef OtherCompositeId = FUniqueNetIdAccelByteUser::CastChecked(Other);

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

	if (!FAccelByteJsonConverter::JsonObjectStringToUStruct(JSONString, &CompositeStructure))
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
#if 1 // MMv1 Deprecation
FOnlineSessionInfoAccelByteV1::FOnlineSessionInfoAccelByteV1()
	: FOnlineSessionInfo()
	, HostAddr(ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr())
	, RemoteId(TEXT(""))
	, SessionId(FUniqueNetIdAccelByteResource::Invalid())
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
	FString Guid = OwnerGuid.ToString();
	SessionId =  FUniqueNetIdAccelByteResource::Create(MoveTemp(Guid), ACCELBYTE_RESOURCE_ID_TYPE);
}

void FOnlineSessionInfoAccelByteV1::SetP2PChannel(int32 InChannel)
{
	P2PChannel = InChannel;
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

FUniqueNetIdAccelByteResourceRef FOnlineSessionInfoAccelByteV1::GetSessionIdRef() const
{
	return SessionId;
}

const FUniqueNetId& FOnlineSessionInfoAccelByteV1::GetSessionId() const
{
	return SessionId.Get();
}

void FOnlineSessionInfoAccelByteV1::SetSessionId(const FString& InSessionId)
{
	FString TempString = InSessionId;
	SessionId = FUniqueNetIdAccelByteResource::Create(MoveTemp(TempString), ACCELBYTE_RESOURCE_ID_TYPE);
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

int32 FOnlineSessionInfoAccelByteV1::GetP2PChannel()
{
	return P2PChannel;
}
#endif // MMv1 Deprecation
#pragma endregion // FOnlineSessionInfoAccelByte

#pragma region FUserOnlineAccountAccelByte

FUserOnlineAccountAccelByte::FUserOnlineAccountAccelByte
	( const FString& InUserId /*= TEXT("")*/ )
{
	UserIdRef = FUniqueNetIdAccelByteUser::Create(InUserId);
}

FUserOnlineAccountAccelByte::FUserOnlineAccountAccelByte
	( const TSharedRef<const FUniqueNetId>& InUserId )
	: UserIdRef(FUniqueNetIdAccelByteUser::CastChecked(InUserId))
{
}

FUserOnlineAccountAccelByte::FUserOnlineAccountAccelByte
	( const TSharedRef<const FUniqueNetId>& InUserId
	, const FString& InDisplayName )
	: UserIdRef(FUniqueNetIdAccelByteUser::CastChecked(InUserId))
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
	if (!UniqueDisplayName.IsEmpty())
	{
		return UniqueDisplayName;
	}
	return DisplayName;
}

FString FUserOnlineAccountAccelByte::GetDisplayName(const FString& Platform) const
{
	if (!UniqueDisplayName.IsEmpty())
	{
		return UniqueDisplayName;
	}

	if (Platform.IsEmpty())
	{
		return DisplayName;
	}

	// Caller has provided an explicit platform source, try and convert to platform type enum, and attempt to find linked account
	const EAccelBytePlatformType PlatformType = FAccelByteUtilities::GetUEnumValueFromString<EAccelBytePlatformType>(Platform);
	if (PlatformType == EAccelBytePlatformType::None)
	{
		// Platform type did not evaluate to a proper type, warn and return default display name
		UE_LOG_AB(Warning, TEXT("Invalid platform type passed to GetDisplayName, returning default: %s"), *Platform);
		return DisplayName;
	}

	FScopeLock Lock(&PlatformUsersLock);

	// Attempt to find matching platform type in linked accounts
	const FOnlinePlatformUserAccelByte* FoundPlatformUser = PlatformUsers.Find(Platform);
	if (FoundPlatformUser == nullptr)
	{
		// Platform is not linked to user account, return default and warn
		UE_LOG_AB(Warning, TEXT("Platform passed to GetDisplayName is not linked to user account, returning default: %s"), *Platform);
		return DisplayName;
	}

	return FoundPlatformUser->GetDisplayName();
}

FString FUserOnlineAccountAccelByte::GetPublicCode()
{
	return PublicCode;
}

void FUserOnlineAccountAccelByte::SetDisplayName(const FString& InDisplayName)
{
	DisplayName = InDisplayName;
}

void FUserOnlineAccountAccelByte::SetUniqueDisplayName(const FString& InUniqueDisplayName)
{
	UniqueDisplayName = InUniqueDisplayName;
}

FString FUserOnlineAccountAccelByte::GetUserCountry() const
{
	return UserCountry;
}

void FUserOnlineAccountAccelByte::SetUserCountry(const FString& InUserCountry)
{
	UserCountry = InUserCountry;
}

FString FUserOnlineAccountAccelByte::GetAccessToken() const
{
	auto PinnedCredentialRef = CredentialsRef.Pin();
	if (PinnedCredentialRef.IsValid())
	{
		return PinnedCredentialRef->GetAccessToken();
	}

	return FString();
}

void FUserOnlineAccountAccelByte::SetCredentialsRef(AccelByte::FBaseCredentialsRef InCredentialsRef)
{
	CredentialsRef = InCredentialsRef;
}

void FUserOnlineAccountAccelByte::SetPublicCode(const FString& InPublicCode)
{
	PublicCode = InPublicCode;
}

FString FUserOnlineAccountAccelByte::GetPlatformUserId() const
{
	return PlatformUserId;
}

void FUserOnlineAccountAccelByte::SetPlatformUserId(const FString& InPlatformUserId)
{
	PlatformUserId = InPlatformUserId;
}

FString FUserOnlineAccountAccelByte::GetSimultaneousPlatformID() const
{
	return SimultaneousPlatformId;
}

void FUserOnlineAccountAccelByte::SetSimultaneousPlatformID(const FString& InSimultaneousPlatformID)
{
	SimultaneousPlatformId = InSimultaneousPlatformID;
}

FString FUserOnlineAccountAccelByte::GetSimultaneousPlatformUserID() const
{
	return SimultaneousPlatformUserId;
}

void FUserOnlineAccountAccelByte::SetSimultaneousPlatformUserID(const FString& InSimultaneousPlatformUserID)
{
	SimultaneousPlatformUserId = InSimultaneousPlatformUserID;
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

void FUserOnlineAccountAccelByte::AddPlatformUser(const FOnlinePlatformUserAccelByte& PlatformUser)
{
	// Assuming PlatformUser has a GetPlatformId method that returns a unique ID for each platform user
	FString Key = PlatformUser.GetPlatformId(); 

	FScopeLock Lock(&PlatformUsersLock);

	// Add the PlatformUser to the map with Key as its identifier
	PlatformUsers.Add(Key, PlatformUser);
}

const FOnlinePlatformUserAccelByte* FUserOnlineAccountAccelByte::GetPlatformUser(const FString& PlatformId) const
{
	FScopeLock Lock(&PlatformUsersLock);

	// Find returns a pointer to the value type (FOnlinePlatformUserAccelByte) in the map
	const FOnlinePlatformUserAccelByte* FoundUser = PlatformUsers.Find(PlatformId);
	if (FoundUser != nullptr)
	{
		// FoundUser is already a pointer to the FOnlinePlatformUserAccelByte
		return FoundUser;
	}

	return nullptr; // Return null if the user is not found
}

#pragma endregion // FUserOnlineAccountAccelByte

#pragma region FOnlinePlatformUserAccelByte

FOnlinePlatformUserAccelByte::FOnlinePlatformUserAccelByte
	( const FString& InUserId /*= TEXT("")*/ )
{
	UserIdRef = FUniqueNetIdAccelByteUser::Create(InUserId);
}

FOnlinePlatformUserAccelByte::FOnlinePlatformUserAccelByte
	( const TSharedRef<const FUniqueNetId>& InUserId )
	: UserIdRef(FUniqueNetIdAccelByteUser::CastChecked(InUserId))
{
}

FOnlinePlatformUserAccelByte::FOnlinePlatformUserAccelByte
	( const TSharedRef<const FUniqueNetId>& InUserId
	, const FString& InDisplayName )
	: UserIdRef(FUniqueNetIdAccelByteUser::CastChecked(InUserId))
	, DisplayName(InDisplayName)
{
}

FOnlinePlatformUserAccelByte::FOnlinePlatformUserAccelByte(const FAccelByteUniqueIdComposite& InCompositeId)
	: UserIdRef(FUniqueNetIdAccelByteUser::Create(InCompositeId))
{
}

// Add implementation for the functions
void FOnlinePlatformUserAccelByte::SetAvatarUrl(const FString& InAvatarUrl)
{
	AvatarUrl = InAvatarUrl;
}

void FOnlinePlatformUserAccelByte::SetDisplayName(const FString& InDisplayName)
{
	DisplayName = InDisplayName;
}

void FOnlinePlatformUserAccelByte::SetPlatformId(const FString& InPlatformId)
{
	PlatformId = InPlatformId;
}

void FOnlinePlatformUserAccelByte::SetPlatformGroup(const FString& InPlatformGroup)
{
	PlatformGroup = InPlatformGroup;
}

void FOnlinePlatformUserAccelByte::SetPlatformUserId(const FString& InPlatformUserId)
{
	PlatformUserId = InPlatformUserId;
}

FString FOnlinePlatformUserAccelByte::GetAvatarUrl() const
{
	return AvatarUrl;
}

FString FOnlinePlatformUserAccelByte::GetDisplayName(const FString& /*Platform*/) const
{
	return DisplayName;
}

FString FOnlinePlatformUserAccelByte::GetPlatformId() const
{
	return PlatformId;
}

FString FOnlinePlatformUserAccelByte::GetPlatformGroup() const
{
	return PlatformGroup;
}

FString FOnlinePlatformUserAccelByte::GetPlatformUserId() const
{
	return PlatformUserId;
}

FString FOnlinePlatformUserAccelByte::GetRealName() const
{
	return DisplayName;
}

bool FOnlinePlatformUserAccelByte::GetUserAttribute(const FString& AttrName, FString& OutAttrValue) const
{
	const FString* FoundAttr = UserAttributes.Find(AttrName);
	if (FoundAttr != nullptr)
	{
		OutAttrValue = *FoundAttr;
		return true;
	}

	return false;
}

bool FOnlinePlatformUserAccelByte::SetUserLocalAttribute(const FString& AttrName, const FString& AttrValue)
{
	const FString* FoundAttr = UserAttributes.Find(AttrName);
    	if (FoundAttr == nullptr || *FoundAttr != AttrValue)
    	{
    		UserAttributes.Add(AttrName, AttrValue);
    		return true;
    	}
    
    	return false;
}

#pragma endregion // FOnlinePlatformUserAccelByte 
