// Copyright (c) 2021 - 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystemTypes.h"
#include "IPAddress.h"
#include "OnlineSubsystemAccelBytePackage.h"
#include "Models/AccelByteMatchmakingModels.h"
#include "OnlineSubsystemAccelByteTypes.generated.h"

class FOnlineSubsystemAccelByte;

// AccelByte IDs have a max length of 32, as they are UUIDs that are striped of their hyphens
#define ACCELBYTE_ID_LENGTH 32

// Value to represent an invalid NetID, mostly to ease debugging
#define ACCELBYTE_INVALID_ID_VALUE TEXT("INVALID")

/**
 * Does a simple check to see if the actual AccelByte ID for the composite is valid.
 */
bool IsAccelByteIDValid(const FString& AccelByteId);

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
	RefreshToken
};

enum class EAccelBytePartyType : uint32
{
	PRIMARY_PARTY = 1
};

/**
 * Simple structure to represent the JSON encoded data for an FUniqueNetIdAccelByte.
 */
USTRUCT()
struct FAccelByteUniqueIdComposite
{
	GENERATED_BODY()

	/**
	 * AccelByte ID for the composite, should always be present
	 */
	UPROPERTY()
	FString Id;

	/**
	 * Platform type that corresponds to the platform ID in the composite, can be blank
	 */
	UPROPERTY()
	FString PlatformType;

	/**
	 * Platform ID that corresponds to the platform type in the composite, can be blank
	 */
	UPROPERTY()
	FString PlatformId;

	FAccelByteUniqueIdComposite() = default;

	FAccelByteUniqueIdComposite(const FString& InId, const FString& InPlatformType = TEXT(""), const FString& InPlatformId = TEXT(""));

	bool operator==(const FAccelByteUniqueIdComposite& OtherComposite) const;

	bool operator!=(const FAccelByteUniqueIdComposite& OtherComposite) const;

	FString ToString() const;
};

/**
 * @brief Unique ID instance for identifying generic AccelByte resources other than users. User IDs should use
 * FUniqueNetIdAccelByteUser as they have extra composite components that are useful for identifying a
 * user on their specific platform.
 */
using FUniqueNetIdAccelByteResourcePtr = TSharedPtr<const class FUniqueNetIdAccelByteResource>;
using FUniqueNetIdAccelByteResourceRef = TSharedRef<const class FUniqueNetIdAccelByteResource>;
class ONLINESUBSYSTEMACCELBYTE_API FUniqueNetIdAccelByteResource : public FUniqueNetIdString
{
public:
	UE_DEPRECATED(5.0, "Public constructors of FUniqueNetId types are deprecated. Please use the ::Create(Args) method instead to create a FUniqueNetIdRef")
	FUniqueNetIdAccelByteResource();

	UE_DEPRECATED(5.0, "Public constructors of FUniqueNetId types are deprecated. Please use the ::Create(Args) method instead to create a FUniqueNetIdRef")
	explicit FUniqueNetIdAccelByteResource(const FString& InUniqueNetId);

	UE_DEPRECATED(5.0, "Public constructors of FUniqueNetId types are deprecated. Please use the ::Create(Args) method instead to create a FUniqueNetIdRef")
	explicit FUniqueNetIdAccelByteResource(FString&& InUniqueNetId);

	UE_DEPRECATED(5.0, "Public constructors of FUniqueNetId types are deprecated. Please use the ::Create(Args) method instead to create a FUniqueNetIdRef")
	explicit FUniqueNetIdAccelByteResource(const FUniqueNetId& Src);
	
	template<typename... TArgs>
	static FUniqueNetIdAccelByteResourceRef Create(TArgs&&... Args)
	{
		PRAGMA_DISABLE_DEPRECATION_WARNINGS
		return MakeShareable(new FUniqueNetIdAccelByteResource(Forward<TArgs>(Args)...));
		PRAGMA_ENABLE_DEPRECATION_WARNINGS
	}
	
	/**
	 * Takes a const FUniqueNetId reference and converts it to a TSharedRef<FUniqueNetIdAccelByte> if the type matches.
	 *
	 * @param NetId The NetId to attempt to convert to an FUniqueNetIdAccelByte reference.
	 */
	static FUniqueNetIdAccelByteResourceRef Cast(const FUniqueNetId& NetId);

	/**
	 * @brief Convenience method to construct an invalid instance of a AccelByte net ID
	 */
	static const FUniqueNetIdAccelByteResourceRef Invalid();

	virtual FName GetType() const override;

	virtual bool IsValid() const override;
	
protected:
	UE_DEPRECATED(5.0, "Public constructors of FUniqueNetId types are deprecated. Please use the ::Create(Args) method instead to create a FUniqueNetIdRef")
	FUniqueNetIdAccelByteResource(FString&& InUniqueNetId, const FName InType);
};

/**
 * @brief Unique IDs to be used by the AccelByte online subsystem for users. This implementation follows a composite structure, where we
 * not only store the AccelByte ID in a single structure, but also, optionally, the platform name and ID in the same structure.
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
using FUniqueNetIdAccelByteUserPtr = TSharedPtr<const class FUniqueNetIdAccelByteUser>;
using FUniqueNetIdAccelByteUserRef = TSharedRef<const class FUniqueNetIdAccelByteUser>;
class ONLINESUBSYSTEMACCELBYTE_API FUniqueNetIdAccelByteUser : public FUniqueNetIdAccelByteResource
{
public:
	UE_DEPRECATED(5.0, "Public constructors of FUniqueNetId types are deprecated. Please use the ::Create(Args) method instead to create a FUniqueNetIdRef")
	FUniqueNetIdAccelByteUser();

	UE_DEPRECATED(5.0, "Public constructors of FUniqueNetId types are deprecated. Please use the ::Create(Args) method instead to create a FUniqueNetIdRef")
	explicit FUniqueNetIdAccelByteUser(const FString& InUniqueNetId);

	UE_DEPRECATED(5.0, "Public constructors of FUniqueNetId types are deprecated. Please use the ::Create(Args) method instead to create a FUniqueNetIdRef")
	explicit FUniqueNetIdAccelByteUser(FString&& InUniqueNetId);

	UE_DEPRECATED(5.0, "Public constructors of FUniqueNetId types are deprecated. Please use the ::Create(Args) method instead to create a FUniqueNetIdRef")
	explicit FUniqueNetIdAccelByteUser(const FUniqueNetId& Src);
	
	/**
	 * @brief Tries to create a new FUniqueNetIdAccelByte instance from a composite ID
	 * 
	 * @param CompositeId
	 *
	 * @return Shared ref of UniqueNetId
	 */
	static FUniqueNetIdAccelByteUserRef Create(const FAccelByteUniqueIdComposite& CompositeId);
	
	/**
	 * @brief Tries to create a new FUniqueNetIdAccelByte instance from a generic UniqueNetId
	 * 
	 * @param bBypassValidCheck Bypasses the check to determine if the AccelByte ID passed in is valid. Defaults to false, or to not bypass the check.
	 * @param CompositeId
	 *
	 * @return Shared pointer of UniqueNetId
	 */
	static FUniqueNetIdAccelByteUserRef Create(const FUniqueNetId& Src);

	/**
	 * @brief Takes a const FUniqueNetId reference and converts it to a TSharedRef<FUniqueNetIdAccelByte> if the type matches.
	 *
	 * @param NetId The NetId to attempt to convert to an FUniqueNetIdAccelByte reference.
	 */
	static FUniqueNetIdAccelByteUserRef Cast(const FUniqueNetId& NetId);

	/**
	 * @brief Convenience method to construct an invalid instance of a AccelByte net ID
	 */
	static FUniqueNetIdAccelByteUserRef Invalid();

	virtual FName GetType() const override;

	/**
	 * @brief Whether or not this ID is a valid FUniqueNetIdAccelByte type. Will do a number of checks including:
	 * - Whether the underlying string value is Base64
	 * - Whether upon decoding the string value there is a JSON object with id, platformType, and platformId fields
	 * - Whether the id field in the JSON object is in the correct format for an AccelByte ID
	 */
	virtual bool IsValid() const override;

	/**
	 * @brief Returns the JSON representation of our composite ID, useful for debugging
	 */
	virtual FString ToDebugString() const override;

	/**
	 * @brief Get the string representation of the AccelByte user ID from the composite ID
	 */
	FString GetAccelByteId() const;

	/**
	 * @brief Get the string representation of the type of platform for platform ID from the composite ID
	 */
	FString GetPlatformType() const;

	/**
	 * @brief Get the string representation of the ID of the user for the platform type specified from the composite ID
	 */
	FString GetPlatformId() const;

	/**
	 * @brief Checks whether or not this composite ID has platform information included
	 */
	bool HasPlatformInformation() const;

	/**
	 * @brief Tries to convert the platform ID information into that native platform OSS' unique ID type.
	 */
	TSharedPtr<const FUniqueNetId> GetPlatformUniqueId() const;

	/**
	 * @brief Gets the underlying composite structure for this unique ID
	 */
	FAccelByteUniqueIdComposite GetCompositeStructure() const;
	
	/**
	 * @brief Override equal check operator to check the AccelByte ID first, and then the platform type/ID
	 */
	virtual bool Compare(const FUniqueNetId& Other) const override;

PACKAGE_SCOPE:

	/**
	 * @brief Internal constructor to set both composite elements and the encoded string at once without extra processing.
	 */
	UE_DEPRECATED(5.0, "Public constructors of FUniqueNetId types are deprecated. Please use the ::Create(Args) method instead to create a FUniqueNetIdRef")
	explicit FUniqueNetIdAccelByteUser(const FAccelByteUniqueIdComposite& CompositeId, const FString& EncodedComposite);

private:

	/**
	 * @brief Underlying composite IDs that represent a full AccelByte unique ID
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
	UE_DEPRECATED(5.0, "Public constructors of FUniqueNetId types are deprecated. Please use the ::Create(Args) method instead to create a FUniqueNetIdRef")
	FUniqueNetIdAccelByteUser(FString&& InUniqueNetId, const FName InType);
};

/**
 * Key functions for indexing a map with a shared reference to an AccelByte User Unique ID as a key.
 */
template <typename ValueType>
struct ONLINESUBSYSTEMACCELBYTE_API TUserUniqueIdConstSharedRefMapKeyFuncs : public TDefaultMapKeyFuncs<TSharedRef<const FUniqueNetIdAccelByteUser>, ValueType, false>
{
	static TSharedRef<const FUniqueNetIdAccelByteUser> GetSetKey(const TTuple<TSharedRef<const FUniqueNetIdAccelByteUser>, ValueType> Element)
	{
		return Element.Key;
	}

	static uint32 GetKeyHash(const TSharedRef<const FUniqueNetIdAccelByteUser>& Key)
	{
		return GetTypeHash(Key.Get());
	}

	static bool Matches(const TSharedRef<const FUniqueNetIdAccelByteUser>& A, const TSharedRef<const FUniqueNetIdAccelByteUser>& B)
	{
		return (A == B) || (A.Get() == B.Get());
	}
};

/**
 * Array of user IDs corresponding to players in a party in this session
 */
using TPartyMemberArray = TArray<TSharedRef<const FUniqueNetId>>;

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

class ONLINESUBSYSTEMACCELBYTE_API FOnlineSessionInfoAccelByte : public FOnlineSessionInfo
{
public:
	FOnlineSessionInfoAccelByte();

	FOnlineSessionInfoAccelByte(const FOnlineSessionInfoAccelByte& Other);

	virtual ~FOnlineSessionInfoAccelByte() override = default;

	bool operator==(const FOnlineSessionInfoAccelByte& Other) const;

	FOnlineSessionInfoAccelByte& operator=(const FOnlineSessionInfoAccelByte& Src);

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

	TSharedRef<FUniqueNetIdAccelByteResource> GetSessionIdRef() const;

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

	/** @brief Unique Id for this session */
	TSharedRef<FUniqueNetIdAccelByteResource> SessionId;

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
	//~ End AccelByte-specific implementation

private:
	/** User Id represented as a FUniqueNetId */
	FUniqueNetIdAccelByteUserRef UserIdRef;
	
	/** Display name for the AccelByte user associated with this account */
	FString DisplayName;

	/** Access token for the AccelByte user associated with this account */
	FString AccessToken;
	
	/** Additional key/value pair data related to auth */
	TMap<FString, FString> AdditionalAuthData;
	
	/** Additional key/value pair data related to user attribution */
	TMap<FString, FString> UserAttributes;
	
	bool bIsConnectedToLobby{false};
};
