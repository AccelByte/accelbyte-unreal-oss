// Copyright (c) 2021 - 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/AccelByteUtilities.h"
#include "OnlineSubsystemTypes.h"
#include "IPAddress.h"
#include "OnlineSubsystemAccelByteDefines.h"
#include "Runtime/Launch/Resources/Version.h"
#include "OnlineSubsystemAccelBytePackage.h"
#include "Models/AccelByteMatchmakingModels.h"
#include "OnlineAsyncTaskManager.h"
#include "OnlineStats.h"
#include "OnlineSubsystemAccelByteTypes.generated.h"

class FOnlineSubsystemAccelByte;

// Value to represent an invalid NetID, mostly to ease debugging
#define ACCELBYTE_INVALID_ID_VALUE TEXT("INVALID")

/**
 * @brief Does a simple check to see if the actual AccelByte ID for the composite is valid.
 */
bool ONLINESUBSYSTEMACCELBYTE_API IsAccelByteIDValid(FString const& AccelByteId);

/**
 * @brief Enum representing the types of authentication that the Identity interface can use with the AccelByte backend.
 * Will automatically be determined by the OSS, or derived from another OSSs auth type.
 */
UENUM(BlueprintType)
enum class EAccelByteLoginType : uint8
{
	None = 0,
	DeviceId,
	AccelByte,
	Xbox,
	PS4,
	PS5,
	Launcher,
	Steam,
	EOS,
	ExchangeCode, // Epic Launcher
	RefreshToken,
	CachedToken
};

enum class EAccelBytePartyType : uint32
{
	PRIMARY_PARTY = 1
};

/**
 * @brief Enum representing the types of chat room
 */
UENUM(BlueprintType)
enum class EAccelByteChatRoomType : uint8
{
	NORMAL,
	PERSONAL,
	PARTY_V2, // Party from session service
	PARTY_V1, // Party form lobby service
	SESSION_V2,
};

/**
 * @brief Simple structure to represent the JSON encoded data for an FUniqueNetIdAccelByte.
 */
USTRUCT()
struct FAccelByteUniqueIdComposite
{
	GENERATED_BODY()

	/**
	 * @brief AccelByte ID for the composite, should always be present
	 */
	UPROPERTY()
	FString Id {};

	/**
	 * @brief Platform type that corresponds to the platform ID in the composite, can be blank
	 */
	UPROPERTY()
	FString PlatformType {};

	/**
	 * @brief Platform ID that corresponds to the platform type in the composite, can be blank
	 */
	UPROPERTY()
	FString PlatformId {};

	/**
	 * @brief Default constructor.
	 */
	FAccelByteUniqueIdComposite() = default;

	/**
	 * @brief Constructor that accepts AccelByte ID, Platform Type and Platform ID.
	 *
	 * @param InId The AccelByte ID.
	 * @param InPlatformType The Platform Type.
	 * @param InPlatformId The PlatformId.
	 */
	explicit FAccelByteUniqueIdComposite(FString const& InId
		, FString const& InPlatformType = TEXT("")
		, FString const& InPlatformId = TEXT(""));

	bool operator==(FAccelByteUniqueIdComposite const& OtherComposite) const;

	bool operator!=(FAccelByteUniqueIdComposite const& OtherComposite) const;

	FString ToString() const;
};

/**
 * @brief Type name for an AccelByte resource ID. Used to determine if we can cast the ID to a resource type.
 */
#define ACCELBYTE_RESOURCE_ID_TYPE FName(TEXT("AccelByteResource"))

class FUniqueNetIdAccelByteResource;

/**
 * @brief Alias for TSharedPtr<FUniqueNetIdAccelByteResource const>
 */
using FUniqueNetIdAccelByteResourcePtr = TSharedPtr<FUniqueNetIdAccelByteResource const>;

/**
 * @brief Alias for TSharedRef<FUniqueNetIdAccelByteResource const>
 */
using FUniqueNetIdAccelByteResourceRef = TSharedRef<FUniqueNetIdAccelByteResource const>;

/**
 * @brief Unique ID instance for identifying generic AccelByte resources other than users. User IDs should use
 * FUniqueNetIdAccelByteUser as they have extra composite components that are useful for identifying a
 * user on their specific platform.
 */
class ONLINESUBSYSTEMACCELBYTE_API FUniqueNetIdAccelByteResource
	: public FUniqueNetIdString
{
public:
	/**
	 * @brief Create a new AccelByte Resource UniqueNetId instance.
	 *
	 * @param Args The arguments that will be forwarded to the Constructor.
	 *
	 * @return A shared reference of AccelByte Resource UniqueNetId object.
	 */
	template<typename... TArgs>
	static FUniqueNetIdAccelByteResourceRef Create(TArgs&&... Args)
	{
		return MakeShareable(new FUniqueNetIdAccelByteResource(Forward<TArgs>(Args)...));
	}
	
	/**
	 * @brief Takes an UniqueNetId object and converts it to an AccelByte Resource UniqueNetId instance if the type matches.
	 * Will return an Invalid ID reference if the conversion cannot be made. Prefer the other Cast methods if possible.
	 *
	 * @param InNetId The UniqueNetId object to attempt to convert to an AccelByte Resource UniqueNetId instance.
	 *
	 * @return A shared reference of AccelByte Resource UniqueNetId object.
	 */
	static FUniqueNetIdAccelByteResourceRef Cast(FUniqueNetId const& InNetId);

	/**
	 * @brief Attempts to convert an UniqueNetId object to an AccelByte Resource UniqueNetId instance.
	 * Will return nullptr if the cast cannot be made.
	 *
	 * @param InNetId The UniqueNetId object to attempt to convert to an AccelByte Resource UniqueNetId instance.
	 *
	 * @return A shared pointer of AccelByte Resource UniqueNetId object.
	 */
	static FUniqueNetIdAccelByteResourcePtr TryCast(FUniqueNetId const& InNetId);

	/**
	 * @brief Attempts to convert a shared reference of UniqueNetId object to an AccelByte Resource UniqueNetId instance.
	 * Will return nullptr if the cast cannot be made.
	 *
	 * @param InNetIdRef The shared reference of UniqueNetId object to attempt to convert to an AccelByte Resource UniqueNetId instance.
	 *
	 * @return A shared pointer of AccelByte Resource UniqueNetId object.
	 */
	static FUniqueNetIdAccelByteResourcePtr TryCast(FUniqueNetIdRef const& InNetIdRef);

	/**
	 * @brief Convenience method to construct an invalid instance of AccelByte Resource UniqueNetId.
	 *
	 * @return A shared reference of AccelByte Resource UniqueNetId object that has Invalid ID.
	 */
	static FUniqueNetIdAccelByteResourceRef Invalid();

	/**
	 * @brief Return the type of this unique ID. In this case, it should always be ACCELBYTE_RESOURCE_ID_TYPE.
	 *
	 * @return ACCELBYTE_RESOURCE_ID_TYPE.
	 */
	virtual FName GetType() const override;

	/**
	 * @brief Check whether this resource ID is valid or not.
	 */
	virtual bool IsValid() const override;

PACKAGE_SCOPE:
	/**
	 * @brief Attempts to convert an UniqueNetId object to an AccelByte Resource UniqueNetId instance.
	 * Guarded by a check call that will crash if an improper ID is attempted to be casted.
	 *
	 * @param InNetId The UniqueNetId object that will be converted to AccelByte Resource UniqueNetId instance.
	 * 
	 * @return A shared reference of AccelByte Resource UniqueNetId object.
	 */
	static FUniqueNetIdAccelByteResourceRef CastChecked(FUniqueNetId const& InNetId);

	/**
	 * @brief Attempts to convert a shared reference of UniqueNetId object to an AccelByte Resource UniqueNetId instance. 
	 * Guarded by a check call that will crash if an improper ID is attempted to be casted.
	 *
	 * @param InNetIdRef The shared reference of UniqueNetId object that will be converted to AccelByte Resource UniqueNetId instance.
	 * 
	 * @return A shared reference of AccelByte Resource UniqueNetId object.
	 */
	static FUniqueNetIdAccelByteResourceRef CastChecked(FUniqueNetIdRef const& InNetIdRef);
	
protected:
	/**
	 * @brief Default constructor.
	 */
	FUniqueNetIdAccelByteResource();

	/**
	 * @brief Internal constructor that accepts an UniqueNetId string.
	 *
	 * @param InNetIdStr The UniqueNetId string.
	 */
	explicit FUniqueNetIdAccelByteResource(FString const& InNetIdStr);

	/**
	 * @brief Internal constructor that pass the ownership of UniqueNetId string.
	 *
	 * @param InNetIdStr The UniqueNetId string.
	 */
	explicit FUniqueNetIdAccelByteResource(FString && InNetIdStr);

	/**
	 * @brief Internal constructor that accepts an instance of UniqueNetId.
	 *
	 * @param InNetId The UniqueNetId object.
	 */
	explicit FUniqueNetIdAccelByteResource(FUniqueNetId const& InNetId);

	/**
	 * @brief Internal constructor that accepts an UniqueNetId string and its type name.
	 *
	 * @param InNetIdStr The UniqueNetId string.
	 * @param InType The type name of UniqueNetId.
	 */
	explicit FUniqueNetIdAccelByteResource(FString const& InNetIdStr, FName const InType);

	/**
	 * @brief Internal constructor that accepts the ownership of UniqueNetId string and its type name.
	 *
	 * @param InNetIdStr The UniqueNetId string.
	 * @param InType The type name of UniqueNetId.
	 */
	explicit FUniqueNetIdAccelByteResource(FString && InNetIdStr, FName const InType);
};

/**
 * Type name for an AccelByte user ID. Used to determine if we can cast the ID to a user type.
 */
#define ACCELBYTE_USER_ID_TYPE ACCELBYTE_SUBSYSTEM

class FUniqueNetIdAccelByteUser;

/**
 * @brief Alias for TSharedPtr<FUniqueNetIdAccelByteUser const>
 */
using FUniqueNetIdAccelByteUserPtr = TSharedPtr<FUniqueNetIdAccelByteUser const>;

/**
 * @brief Alias for TSharedRef<FUniqueNetIdAccelByteUser const>
 */
using FUniqueNetIdAccelByteUserRef = TSharedRef<FUniqueNetIdAccelByteUser const>;

/**
 * @brief Unique IDs to be used by the AccelByte online subsystem for users. This implementation follows a composite
 * structure, where we not only store the AccelByte ID in a single structure, but also, optionally,
 * the platform name and ID in the same structure.
 *
 * These IDs can be casted to and broken apart to get those IDs as needed, such as for interop with native platform SDKs.
 *
 * For this, there will be three extra fields in an FUniqueNetIdAccelByte, which are as follows:
 * - AccelByteIDString
 * - PlatformTypeString
 * - PlatformIDString
 *
 * When any of these are set, the underlying string for FUniqueNetIdString is set to a Base64 encoded JSON object that contains
 * all of these fields individually. An example of this JSON object is as follows:
 * {
 *     "id": "<AccelByte ID>",
 *     "platformType": "<type of platform the platformId field corresponds to, can be blank>",
 *     "platformId": "<ID of the platform that platformType corresponds to, can be blank>"
 * }
 *
 * ToString will return the encoded version of this string, while ToDebugString will return the decoded version.
 *
 * To get any of these fields from the ID, you will want to cast to an FUniqueNetIdAccelByte and use the getters there.
 * However, most of the ID manipulation is handled for you by the OSS, so there shouldn't be a need to crack open these
 * IDs unless you need to make SDK calls yourself.
 */
class ONLINESUBSYSTEMACCELBYTE_API FUniqueNetIdAccelByteUser
	: public FUniqueNetIdAccelByteResource
{
public:
	/**
	 * @brief Create a new AccelByte User UniqueNetId instance from a composite ID.
	 * 
	 * @param InCompositeId The AccelByte User UniqueNetId in composite structure.
	 *
	 * @returns A shared reference of AccelByte User UniqueNetId object.
	 */
	static FUniqueNetIdAccelByteUserRef Create(FAccelByteUniqueIdComposite const& InCompositeId);

	/**
	 * @brief Create a new AccelByte User UniqueNetId instance from a string of UniqueNetId.
	 * 
	 * @param InNetIdStr The UniqueNetId string.
	 *
	 * @returns A shared reference of AccelByte User UniqueNetId object.
	 */
	static FUniqueNetIdAccelByteUserRef Create(FString const& InNetIdStr);
	
	/**
	 * @brief Create a new AccelByte User UniqueNetId instance from a generic UniqueNetId.
	 *
	 * @param InNetId The UniqueNetId object.
	 *
	 * @return A shared reference of AccelByte User UniqueNetId object.
	 */
	static FUniqueNetIdAccelByteUserRef Create(FUniqueNetId const& InNetId);

	/**
	 * @brief Takes an UniqueNetId object and converts it to an AccelByte User UniqueNetId instance if the type matches.
	 * Will return an Invalid ID reference if the conversion cannot be made. Prefer the other Cast methods if possible.
	 *
	 * @param InNetId The UniqueNetId object to attempt to convert to an AccelByte User UniqueNetId instance.
	 *
	 * @return A shared reference of AccelByte User UniqueNetId object.
	 */
	static FUniqueNetIdAccelByteUserRef Cast(FUniqueNetId const& InNetId);

	/**
	 * @brief Attempts to convert an UniqueNetId object to an AccelByte User UniqueNetId instance. 
	 * Will return nullptr if the cast cannot be made.
	 *
	 * @param InNetId The UniqueNetId object to attempt to convert to an AccelByte User UniqueNetId instance.
	 *
	 * @return A shared pointer of AccelByte User UniqueNetId object.
	 */
	static FUniqueNetIdAccelByteUserPtr TryCast(FUniqueNetId const& InNetId);

	/**
	 * @brief Attempts to convert a shared reference of UniqueNetId object to an AccelByte User UniqueNetId instance. 
	 * Will return nullptr if the cast cannot be made.
	 *
	 * @param InNetId The shared reference of UniqueNetId object to attempt to convert to an AccelByte User UniqueNetId instance.
	 *
	 * @return A shared pointer of AccelByte User UniqueNetId object.
	 */
	static FUniqueNetIdAccelByteUserPtr TryCast(FUniqueNetIdRef const& InNetId);

	/**
	 * @brief Convenience method to construct an invalid instance of an AccelByte UniqueNetId.
	 *
	 * @return A shared reference of AccelByte User UniqueNetId object that has Invalid ID.
	 */
	static FUniqueNetIdAccelByteUserRef Invalid();

	/**
	 * @brief Return the type of this unique ID. In this case, it should always be ACCELBYTE_USER_ID_TYPE.
	 *
	 * @returns ACCELBYTE_USER_ID_TYPE.
	 */
	virtual FName GetType() const override;

	/**
	 * @brief Whether or not this ID is a valid AccelByte UniqueNetId type. Will do a number of checks including:
	 * - Whether the underlying string value is Base64
	 * - Whether upon decoding the string value there is a JSON object with id, platformType, and platformId fields
	 * - Whether the id field in the JSON object is in the correct format for an AccelByte ID
	 */
	virtual bool IsValid() const override;

	/**
	 * @brief Returns the JSON representation of AccelByte composite ID, useful for debugging.
	 *
	 * @return a string of JSON representation of the composite ID.
	 */
	virtual FString ToDebugString() const override;

	/**
	 * @brief Get the string representation of the AccelByte user ID from the composite ID
	 *
	 * @return AccelByte ID in UUID format.
	 */
	FString GetAccelByteId() const;

	/**
	 * @brief Get the string representation of the type of platform for platform ID from the composite ID.
	 *
	 * @return a string representation of the type of platform.
	 */
	FString GetPlatformType() const;

	/**
	 * @brief Get the string representation of the ID of the user for the platform type specified from the composite ID.
	 *
	 * @return Platform user ID. 
	 */
	FString GetPlatformId() const;

	/**
	 * @brief Checks whether or not this composite ID has platform information included.
	 *
	 * @return true if composite ID has platform information and false if no platform information presents.
	 */
	bool HasPlatformInformation() const;

	/**
	 * @brief Tries to convert the platform ID information into that native platform OSS' unique ID type.
	 *
	 * @return A pointer to Native Platform UniqueNetId.
	 */
	FUniqueNetIdPtr GetPlatformUniqueId() const;

	/**
	 * @brief Gets the underlying composite structure for this unique ID.
	 *
	 * @return AccelByte Unique ID in composite structure.
	 */
	FAccelByteUniqueIdComposite GetCompositeStructure() const;
	
	/**
	 * @brief Override equal check operator to check the AccelByte ID first, and then the platform type/ID.
	 *
	 * @param Other Another UniqueNetId object.
	 * 
	 * @return true if both object has the same ID and false if not the same
	 */
	virtual bool Compare(FUniqueNetId const& Other) const override;

PACKAGE_SCOPE:
	/**
	 * @brief Internal constructor to set both composite elements and the encoded string at once without extra processing.
	 *
	 * @param CompositeId UserId in CompositeId object
	 * @param EncodedComposite UserId in a Base64 string format
	 */
	explicit FUniqueNetIdAccelByteUser(FAccelByteUniqueIdComposite const& CompositeId, FString const& EncodedComposite);
	
	/**
	 * @brief Attempts to convert an UniqueNetId object to an AccelByte User UniqueNetId instance. 
	 * Guarded by a check call that will crash if an improper ID is attempted to be casted.
	 *
	 * @param InNetId The UniqueNetId object that will be converted to AccelByte User UniqueNetId instance.
	 *
	 * @return A shared reference of AccelByte User UniqueNetId object.
	 */
	static FUniqueNetIdAccelByteUserRef CastChecked(FUniqueNetId const& InNetId);

	/**
	 * @brief Attempts to convert a shared reference of UniqueNetId object to an AccelByte User UniqueNetId instance.
	 * Guarded by a check call that will will crash if an improper ID is attempted to be casted.
	 * 
	 * @param InNetIdRef The shared reference of UniqueNetId object that will be converted to AccelByte User UniqueNetId instance.
	 *
	 * @return A shared reference of AccelByte User UniqueNetId object.
	 */
	static FUniqueNetIdAccelByteUserRef CastChecked(FUniqueNetIdRef const& InNetIdRef);

private:

	/**
	 * @brief Underlying composite IDs that represent a full AccelByte UniqueNetId
	 */
	FAccelByteUniqueIdComposite CompositeStructure;

	/**
	 * @brief Flag denoting whether the validity of this ID has already been checked
	 */
	bool bHasCachedValidState = false;

	/**
	 * @brief Cached state of this ID's validity
	 */
	bool bCachedValidState = false;

	/**
	 * @brief Method that will decode a given string from Base64 into the correct ID format, as well as fill out necessary fields.
	 */
	void DecodeIDElements();

protected:
	/**
	 * @brief Default constructor.
	 */
	FUniqueNetIdAccelByteUser();

	/**
	 * @brief Internal constructor that accepts an UniqueNetId string.
	 *
	 * @param InNetIdStr The UniqueNetId string.
	 */
	explicit FUniqueNetIdAccelByteUser(FString const& InNetIdStr);

	/**
	 * @brief Internal constructor that pass the ownership of UniqueNetId string.
	 *
	 * @param InNetIdStr The UniqueNetId string.
	 */
	explicit FUniqueNetIdAccelByteUser(FString && InNetIdStr);

	/**
	 * @brief Internal constructor that accepts an instance of UniqueNetId.
	 *
	 * @param InNetId The UniqueNetId object.
	 */
	explicit FUniqueNetIdAccelByteUser(FUniqueNetId const& InNetId);

	/**
	 * @brief Internal constructor that accepts an UniqueNetId string and its type name.
	 *
	 * @param InNetIdStr The UniqueNetId string.
	 * @param InType The type name of UniqueNetId.
	 */
	explicit FUniqueNetIdAccelByteUser(FString const& InNetIdStr, FName const InType);

	/**
	 * @brief Internal constructor that accepts the ownership of UniqueNetId string and its type name.
	 *
	 * @param InNetIdStr The UniqueNetId string.
	 * @param InType The type name of UniqueNetId.
	 */
	explicit FUniqueNetIdAccelByteUser(FString && InNetIdStr, FName const InType);
};

/**
 * Key functions for indexing a map with a shared reference to an AccelByte User Unique ID as a key.
 */
template <typename ValueType>
struct ONLINESUBSYSTEMACCELBYTE_API TUserUniqueIdConstSharedRefMapKeyFuncs
	: public TDefaultMapKeyFuncs<FUniqueNetIdAccelByteUserRef, ValueType, false>
{
	static FUniqueNetIdAccelByteUserRef GetSetKey(TTuple<FUniqueNetIdAccelByteUserRef, ValueType> const Element)
	{
		return Element.Key;
	}

	static uint32 GetKeyHash(FUniqueNetIdAccelByteUserRef const& Key)
	{
		return GetTypeHash(Key.Get());
	}

	static bool Matches(FUniqueNetIdAccelByteUserRef const& A, FUniqueNetIdAccelByteUserRef const& B)
	{
		return (A == B) || (A.Get() == B.Get());
	}
};

/**
 * Array of user IDs corresponding to players in a party in this session
 */
using TPartyMemberArray = TArray<FUniqueNetIdRef>;

/**
 * Array of parties for the session, contains a nested array of user IDs for members
 */
using TSessionPartyArray = TArray<TPartyMemberArray>;

/**
 * Delegate fired when we get information about matchmaking teams
 */
DECLARE_DELEGATE_OneParam(FOnTeamInformationReceived, const TUniqueNetIdMap<int32>&);

/**
 * Delegate fired when we get information about matchmaking parties
 */
DECLARE_DELEGATE_OneParam(FOnPartyInformationReceived, const TSessionPartyArray&);

class ONLINESUBSYSTEMACCELBYTE_API FOnlineSessionInfoAccelByteV1
	: public FOnlineSessionInfo
{
public:
	FOnlineSessionInfoAccelByteV1();

	FOnlineSessionInfoAccelByteV1(const FOnlineSessionInfoAccelByteV1& Other);

	virtual ~FOnlineSessionInfoAccelByteV1() override = default;

	bool operator==(const FOnlineSessionInfoAccelByteV1& Other) const;

	FOnlineSessionInfoAccelByteV1& operator=(const FOnlineSessionInfoAccelByteV1& Src);

	virtual const uint8* GetBytes() const override;

	virtual int32 GetSize() const override;

	virtual bool IsValid() const override;

	virtual FString ToString() const override;

	virtual FString ToDebugString() const override;

	/**
	 * @brief Get the Remote ID of the P2P connection peer
	 */
	virtual const FString& GetRemoteId() const;

	/**
	 * @brief Set the Remote ID of the P2P connection peer
	 */
	virtual void SetRemoteId(const FString& InRemoteId);

	FUniqueNetIdAccelByteResourceRef GetSessionIdRef() const;

	/**
	 * @brief Get the Session ID
	 */
	virtual const FUniqueNetId& GetSessionId() const override;

	/**
	 * @brief Set the Session ID
	 */
	virtual void SetSessionId(const FString& InSessionId);

	/**
	 * @brief Get the Host IP Address
	 */
	virtual TSharedPtr<FInternetAddr> GetHostAddr() const;

	/**
	 * @brief Set the Host IP Address
	 */
	virtual void SetHostAddr(const TSharedRef<FInternetAddr>& InHostAddr);

	/**
	 * @brief Whether or not we have information regarding teams for this session's information, will only be true if this is a matchmaking session
	 */
	bool HasTeamInfo() const;

	/**
	 * @brief Attempts to get a team index for a user, returns INDEX_NONE if not found
	 * 
	 * Note that team indices are just mapped from 0 to how ever many teams are matched. If you have special team numbers
	 * you'll want to map these values on the client side.
	 */
	int32 GetTeamIndex(const FUniqueNetId& UserId) const;

	const TUniqueNetIdMap<int32>& GetTeams() const;
	
	void SetTeams(const TUniqueNetIdMap<int32>& InTeams);

	/**
	 * @brief Whether or not we have information regarding parties for this session, will only be true if this is a matchmaking session
	 */
	bool HasPartyInfo() const;

	const TSessionPartyArray& GetParties() const;

	void SetParties(const TSessionPartyArray& InParties);

	const FAccelByteModelsMatchmakingResult& GetSessionResult() const;

	void SetSessionResult(const FAccelByteModelsMatchmakingResult& InSessionResult);

	void SetP2PChannel(int32 InChannel);

	int32 GetP2PChannel();
	
PACKAGE_SCOPE:

	// #AB #TODO (Afif) : make it accessible from game
	//FOnlineSessionInfoAccelByte();

	/**
	 * Set up the session info to match a new session that is using the AccelByte P2P relay.
	 */
	void SetupP2PRelaySessionInfo(const FOnlineSubsystemAccelByte& Subsystem);
	
private:
	/**
	 * @brief The IP and port of the host address of the session. This will be a loopback on P2P relay sessions, or will be an
	 * actual ip:port for dedicated/LAN sessions.
	 */
	TSharedPtr<FInternetAddr> HostAddr;

	/** @brief Remote ID of the P2P connection peer */
	FString RemoteId;

	/** @brief Channel of the P2P connection */
	int32 P2PChannel;

	/** @brief Unique Id for this session */
	FUniqueNetIdAccelByteResourceRef SessionId = FUniqueNetIdAccelByteResource::Invalid();

	/** @brief Mapping of user IDs to the index representing the team they are on, usually will be zero or one */
	TUniqueNetIdMap<int32> Teams;

	/** 
	 * @brief Array of multiple players to represent a party
	 */
	TSessionPartyArray Parties;

	/** Delegate for when we get team information for a session, should be subscribed to on the game server side */
	FOnTeamInformationReceived OnTeamInformationReceivedDelegate;

	/** Delegate for when we get party information for a session, should be subscribed to on the game server side */
	FOnPartyInformationReceived OnPartyInformationReceivedDelegate;

	FAccelByteModelsMatchmakingResult SessionResult;
};

/**
 * Attribute key for a stored user account's publisher level avatar
 */
#define ACCELBYTE_ACCOUNT_PUBLISHER_AVATAR_URL FString(TEXT("PublisherAvatarUrl"))

/**
 * Attribute key for a stored user account's game level avatar
 */
#define ACCELBYTE_ACCOUNT_GAME_AVATAR_URL FString(TEXT("GameAvatarUrl"))

/**
 * @brief Info associated with an user account generated by AccelByte online service
 */
class ONLINESUBSYSTEMACCELBYTE_API FUserOnlineAccountAccelByte : public FUserOnlineAccount
{
public:
	explicit FUserOnlineAccountAccelByte(const FString& InUserId = TEXT(""));

	explicit FUserOnlineAccountAccelByte(const TSharedRef<const FUniqueNetId>& InUserId);

	explicit FUserOnlineAccountAccelByte(const TSharedRef<const FUniqueNetId>& InUserId, const FString& InDisplayName);

	explicit FUserOnlineAccountAccelByte(const FAccelByteUniqueIdComposite& InCompositeId);

	virtual ~FUserOnlineAccountAccelByte() override = default;
	
	//~ Begin FOnlineUser Interface
	virtual TSharedRef<const FUniqueNetId> GetUserId() const override { return UserIdRef; }
	virtual FString GetRealName() const override;
	virtual FString GetDisplayName(const FString& Platform = FString()) const override;
	virtual bool GetUserAttribute(const FString& AttrName, FString& OutAttrValue) const override;
	virtual bool SetUserLocalAttribute(const FString& AttrName, const FString& InAttrValue) override;
	//~ End FOnlineUser Interface
	
	//~ Begin FUserOnlineAccount Interface
	virtual FString GetAccessToken() const override;
	virtual bool SetUserAttribute(const FString& AttrName, const FString& AttrValue) override;
	virtual bool GetAuthAttribute(const FString& AttrName, FString& OutAttrValue) const override;
	//~ End FUserOnlineAccount Interface

	//~ Begin AccelByte-specific implementation
	/**
	 * @brief Get user's PublicCode on AccelByte services
	 *
	 * @return The user's PublicCode
	 */
	FString GetPublicCode();
	
	/**
	 * @brief Set user's display name on AccelByte services
	 *
	 * @param InDisplayName The user's display name
	 */
	void SetDisplayName(const FString& InDisplayName);

	/**
	 * @brief Set user's token to access AccelByte services
	 *
	 * @param InAccessToken The user's access token
	 */
	void SetAccessToken(const FString& InAccessToken);

	/**
	 * @brief Set user's PublicCode on AccelByte services
	 *
	 * @param InPublicCode The user's PublicCode
	 */
	void SetPublicCode(const FString& InPublicCode);

	/**
	 * @brief A flag that indicates whether the user is connected to AccelByte Lobby or not
	 *
	 * @return bool Return connected status, true for connected and false for not connected
	 */
	bool IsConnectedToLobby() const;

	/**
	 * @brief Set user's connected status to AccelByte Lobby
	 *
	 * @param bIsConnected connected status, true for connected and false for not connected
	 */
	void SetConnectedToLobby(bool bIsConnected);

	/**
	 * @brief A flag that indicates whether the user is connected to AccelByte Chat or not
	 *
	 * @return bool Return connected status, true for connected and false for not connected
	 */
	bool IsConnectedToChat() const;

	/**
	 * @brief Set user's connected status to AccelByte Chat
	 *
	 * @param bIsConnected connected status, true for connected and false for not connected
	 */
	void SetConnectedToChat(bool bIsConnected);
	//~ End AccelByte-specific implementation

private:
	/** User Id represented as a FUniqueNetId */
	FUniqueNetIdAccelByteUserRef UserIdRef = FUniqueNetIdAccelByteUser::Invalid();
	
	/** Display name for the AccelByte user associated with this account */
	FString DisplayName;

	/** Access token for the AccelByte user associated with this account */
	FString AccessToken;

	/** Generated public user identifier code, usually used as a friend code **/
	FString PublicCode;
	
	/** Additional key/value pair data related to auth */
	TMap<FString, FString> AdditionalAuthData;
	
	/** Additional key/value pair data related to user attribution */
	TMap<FString, FString> UserAttributes;
	
	bool bIsConnectedToLobby{false};

	bool bIsConnectedToChat{false};
};

UENUM(BlueprintType)
enum class ETypeOfOnlineAsyncTask :uint8
{
	Serial = 0,
	Parallel = 1
};

class FOnlineAsyncTaskAccelByte;

/**
 * Information that will be passed when CreateAndDispatch Online Task
 */
struct ONLINESUBSYSTEMACCELBYTE_API FOnlineAsyncTaskInfo
{
	FOnlineAsyncTaskInfo() {}

	FOnlineAsyncTaskInfo(ETypeOfOnlineAsyncTask InType, bool inBool):
		Type(InType),
		bCreateEpicForThis(inBool)
	{}

	ETypeOfOnlineAsyncTask Type = ETypeOfOnlineAsyncTask::Serial;
	bool bCreateEpicForThis = false;
};

// 4.27 feature (https://docs.unrealengine.com/4.27/en-US/WhatsNew/Builds/ReleaseNotes/4_27/)
#if (ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 27)

// FUniqueNetId Thread Safety: It is now possible to use ESPMode's class method ThreadSafe with FUniqueNetId shared pointers and references 
// Do this by changing the UNIQUENETID_ESPMODE definition from ESPMode::Fast to ESPMode::ThreadSafe
#define UNIQUENETID_ESPMODE ESPMode::Fast
typedef TSharedRef<const FUniqueNetId, UNIQUENETID_ESPMODE> FUniqueNetIdRef;
typedef TSharedPtr<const FUniqueNetId, UNIQUENETID_ESPMODE> FUniqueNetIdPtr;
typedef TWeakPtr<const FUniqueNetId, UNIQUENETID_ESPMODE> FUniqueNetIdWeakPtr;
#endif 