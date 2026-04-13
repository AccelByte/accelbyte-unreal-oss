// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "Runtime/Launch/Resources/Version.h"
#if ENGINE_MAJOR_VERSION >= 5
#include "Online/CoreOnline.h"
#else
#include "UObject/CoreOnline.h"
#endif

#include "Interfaces/OnlineUserInterface.h"
#include "OnlineSubsystemTypes.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "OnlineError.h"
#include "OnlineErrorAccelByte.h"
#include "OnlineSubsystemAccelBytePackage.h"

class FAccelByteUserPlatformLinkInformation;
typedef TSharedRef<FAccelByteUserPlatformLinkInformation, ESPMode::ThreadSafe> FAccelByteUserPlatformLinkInformationRef;

struct FAccelByteModelsUserProfileUpdateRequest;
struct FAccelByteModelsUserProfileInfo;
struct FAccelByteModelsPublicUserProfileInfo;
struct FAccelByteModelsUserProfileUploadURLResult;
enum class EAccelByteFileType : uint8;
enum class EAccelByteUploadCategory : uint8;

/**
 * Delegate that denotes when a user report has completed.
 *
 * @param bWasSuccessful true if the report was sent successfully, false otherwise.
 */
DECLARE_DELEGATE_OneParam(FOnReportUserComplete, bool /*bWasSuccessful*/);

/**
 * Delegate that denotes when create user profile has completed.
 *
 * @param LocalUserNum the controller number of the associated user that made the request
 * @param bWasSuccessful true if user profile created and/or acquired successfully, false otherwise.
 * @param Error information about the error condition
 */
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnCreateUserProfileComplete, int32 /*LocalUserNum*/, bool /*bWasSuccessful*/, const FOnlineError& /*Error*/);
typedef FOnCreateUserProfileComplete::FDelegate FOnCreateUserProfileCompleteDelegate;

/**
 * Delegate used when the userProfile update request has completed
 *
 * @param LocalUserNum the controller number of the associated user that made the request
 * @param bWasSuccessful true if the async action completed without error, false if there was an error
 * @param Error information about the error condition
 */
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnUpdateUserProfileComplete, int32 /*LocalUserNum*/, bool /*bWasSuccessful*/, const FOnlineError& /*Error*/);
typedef FOnUpdateUserProfileComplete::FDelegate FOnUpdateUserProfileCompleteDelegate;

/**
 * Delegate used when the userProfile query request has completed
 *
 * @param LocalUserNum the controller number of the associated user that made the request
 * @param bWasSuccessful true if the async action completed without error, false if there was an error
 * @param UserIds list of user ids that were queried
 * @param Error information about the error condition
 */
DECLARE_MULTICAST_DELEGATE_FourParams(FOnQueryUserProfileComplete, int32 /*LocalUserNum*/, bool /*bWasSuccessful*/, const TArray<FUniqueNetIdRef>& /*UserIds*/, const FOnlineError& /*Error*/);
typedef FOnQueryUserProfileComplete::FDelegate FOnQueryUserProfileCompleteDelegate;

/**
 * Delegate used when the get own user profile request has completed
 *
 * @param LocalUserNum the controller number of the associated user that made the request
 * @param bWasSuccessful true if the async action completed without error, false if there was an error
 * @param ProfileInfo the complete user profile information including private data
 * @param Error information about the error condition
 */
DECLARE_MULTICAST_DELEGATE_FourParams(FOnGetMyUserProfileComplete, int32 /*LocalUserNum*/, bool /*bWasSuccessful*/, const FAccelByteModelsUserProfileInfo& /*ProfileInfo*/, const FOnlineError& /*Error*/);
typedef FOnGetMyUserProfileComplete::FDelegate FOnGetMyUserProfileCompleteDelegate;

/**
 * Delegate used when the get public user profile by PublicId request has completed
 *
 * @param LocalUserNum the controller number of the associated user that made the request
 * @param bWasSuccessful true if the async action completed without error, false if there was an error
 * @param ProfileInfo the public user profile information
 * @param Error information about the error condition
 */
DECLARE_MULTICAST_DELEGATE_FourParams(FOnGetPublicUserProfileByPublicIdComplete, int32 /*LocalUserNum*/, bool /*bWasSuccessful*/, const FAccelByteModelsPublicUserProfileInfo& /*ProfileInfo*/, const FOnlineError& /*Error*/);
typedef FOnGetPublicUserProfileByPublicIdComplete::FDelegate FOnGetPublicUserProfileByPublicIdCompleteDelegate;

/**
 * Delegate used when the get public user profile info request has completed
 *
 * @param LocalUserNum the controller number of the associated user that made the request
 * @param bWasSuccessful true if the async action completed without error, false if there was an error
 * @param ProfileInfo the public user profile information
 * @param Error information about the error condition
 */
DECLARE_MULTICAST_DELEGATE_FourParams(FOnGetPublicUserProfileInfoComplete, int32 /*LocalUserNum*/, bool /*bWasSuccessful*/, const FAccelByteModelsPublicUserProfileInfo& /*ProfileInfo*/, const FOnlineError& /*Error*/);
typedef FOnGetPublicUserProfileInfoComplete::FDelegate FOnGetPublicUserProfileInfoCompleteDelegate;

/**
 * Delegate used when the get custom attributes request has completed
 *
 * @param LocalUserNum the controller number of the associated user that made the request
 * @param bWasSuccessful true if the async action completed without error, false if there was an error
 * @param CustomAttributes the public custom attributes
 * @param Error information about the error condition
 */
DECLARE_MULTICAST_DELEGATE_FourParams(FOnGetCustomAttributesComplete, int32 /*LocalUserNum*/, bool /*bWasSuccessful*/, const FJsonObjectWrapper& /*CustomAttributes*/, const FOnlineError& /*Error*/);
typedef FOnGetCustomAttributesComplete::FDelegate FOnGetCustomAttributesCompleteDelegate;

/**
 * [DEPRECATED] GetPublicCustomAttributes has been removed due to security issues.
 * Please use 'GetPublicUserProfileInfo(UserId)' instead, which includes
 * CustomAttributes in the returned FAccelByteModelsPublicUserProfileInfo.
 *
 * @deprecated SDK function deprecated with error code 14901
 */

/**
 * Delegate used when the update custom attributes request has completed
 *
 * @param LocalUserNum the controller number of the associated user that made the request
 * @param bWasSuccessful true if the async action completed without error, false if there was an error
 * @param UpdatedAttributes the updated public custom attributes
 * @param Error information about the error condition
 */
DECLARE_MULTICAST_DELEGATE_FourParams(FOnUpdateCustomAttributesComplete, int32 /*LocalUserNum*/, bool /*bWasSuccessful*/, const FJsonObjectWrapper& /*UpdatedAttributes*/, const FOnlineError& /*Error*/);
typedef FOnUpdateCustomAttributesComplete::FDelegate FOnUpdateCustomAttributesCompleteDelegate;

/**
 * Delegate used when the get private custom attributes request has completed
 *
 * @param LocalUserNum the controller number of the associated user that made the request
 * @param bWasSuccessful true if the async action completed without error, false if there was an error
 * @param PrivateAttributes the private custom attributes
 * @param Error information about the error condition
 */
DECLARE_MULTICAST_DELEGATE_FourParams(FOnGetPrivateCustomAttributesComplete, int32 /*LocalUserNum*/, bool /*bWasSuccessful*/, const FJsonObjectWrapper& /*PrivateAttributes*/, const FOnlineError& /*Error*/);
typedef FOnGetPrivateCustomAttributesComplete::FDelegate FOnGetPrivateCustomAttributesCompleteDelegate;

/**
 * Delegate used when the update private custom attributes request has completed
 *
 * @param LocalUserNum the controller number of the associated user that made the request
 * @param bWasSuccessful true if the async action completed without error, false if there was an error
 * @param UpdatedAttributes the updated private custom attributes
 * @param Error information about the error condition
 */
DECLARE_MULTICAST_DELEGATE_FourParams(FOnUpdatePrivateCustomAttributesComplete, int32 /*LocalUserNum*/, bool /*bWasSuccessful*/, const FJsonObjectWrapper& /*UpdatedAttributes*/, const FOnlineError& /*Error*/);
typedef FOnUpdatePrivateCustomAttributesComplete::FDelegate FOnUpdatePrivateCustomAttributesCompleteDelegate;

/**
 * Delegate used when the generate upload URL request has completed
 *
 * @param LocalUserNum the controller number of the associated user that made the request
 * @param bWasSuccessful true if the async action completed without error, false if there was an error
 * @param UploadURLResult the upload URL information including URL and access URL
 * @param Error information about the error condition
 */
DECLARE_MULTICAST_DELEGATE_FourParams(FOnGenerateUploadURLComplete, int32 /*LocalUserNum*/, bool /*bWasSuccessful*/, const FAccelByteModelsUserProfileUploadURLResult& /*UploadURLResult*/, const FOnlineError& /*Error*/);
typedef FOnGenerateUploadURLComplete::FDelegate FOnGenerateUploadURLCompleteDelegate;

/**
 * Delegate used when the generate upload URL for user content request has completed
 *
 * @param LocalUserNum the controller number of the associated user that made the request
 * @param bWasSuccessful true if the async action completed without error, false if there was an error
 * @param UploadURLResult the upload URL result containing URLs and access information
 * @param Error information about the error condition
 */
DECLARE_MULTICAST_DELEGATE_FourParams(FOnGenerateUploadURLForUserContentComplete, int32 /*LocalUserNum*/, bool /*bWasSuccessful*/, const FAccelByteModelsUserProfileUploadURLResult& /*UploadURLResult*/, const FOnlineError& /*Error*/);
typedef FOnGenerateUploadURLForUserContentComplete::FDelegate FOnGenerateUploadURLForUserContentCompleteDelegate;

/**
 * Delegate used when the bulk get public user profile infos V2 request has completed
 *
 * @param LocalUserNum the controller number of the associated user that made the request
 * @param bWasSuccessful true if the async action completed without error, false if there was an error
 * @param ProfileInfosV2 the bulk public user profile infos with NotProcessed array
 * @param Error information about the error condition
 */
DECLARE_MULTICAST_DELEGATE_FourParams(FOnBulkGetPublicUserProfileInfosV2Complete, int32 /*LocalUserNum*/, bool /*bWasSuccessful*/, const FAccelByteModelsPublicUserProfileInfoV2& /*ProfileInfosV2*/, const FOnlineError& /*Error*/);
typedef FOnBulkGetPublicUserProfileInfosV2Complete::FDelegate FOnBulkGetPublicUserProfileInfosV2CompleteDelegate;

/**
 * Delegate used when the get user profile by user ID request has completed
 *
 * @param LocalUserNum the controller number of the associated user that made the request
 * @param bWasSuccessful true if the async action completed without error, false if there was an error
 * @param UserProfileInfo the complete user profile info including private data
 * @param Error information about the error condition
 */
DECLARE_MULTICAST_DELEGATE_FourParams(FOnGetUserProfileComplete, int32 /*LocalUserNum*/, bool /*bWasSuccessful*/, const FAccelByteModelsUserProfileInfo& /*UserProfileInfo*/, const FOnlineError& /*Error*/);
typedef FOnGetUserProfileComplete::FDelegate FOnGetUserProfileCompleteDelegate;

/**
 * Delegate used when the GetUserPlatformLinks has completed
 *
 * @param LocalUserNum the controller number of the associated user that made the request
 * @param bWasSuccessful true if the async action completed without error, false if there was an error
 * @param UserPlatformLinks list of user platform links information that were queried
 * @param Error information about the error condition
 */
DECLARE_MULTICAST_DELEGATE_FourParams(FOnGetUserPlatformLinksComplete, int32 /*LocalUserNum*/, bool /*bWasSuccessful*/, const TArray<FPlatformLink>& /*UserPlatformLinks*/, const FOnlineError& /*Error*/);
typedef FOnGetUserPlatformLinksComplete::FDelegate FOnGetUserPlatformLinksCompleteDelegate;

DECLARE_MULTICAST_DELEGATE_FourParams(FOnListUserByUserIdComplete, int32 /*LocalUserNum*/, bool /*bWasSuccessful*/, const FListUserDataResponse& /*Data*/, const FOnlineError  & /* OnlineError  */);
typedef FOnListUserByUserIdComplete::FDelegate FOnListUserByUserIdCompleteDelegate;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnLinkOtherPlatformComplete, bool /*bWasSuccessful*/, const FOnlineError & /*OnlineError*/);
typedef FOnLinkOtherPlatformComplete::FDelegate FOnLinkOtherPlatformCompleteDelegate;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnUnlinkOtherPlatformComplete, bool /*bWasSuccessful*/,  const FOnlineError & /*OnlineError*/);
typedef FOnUnlinkOtherPlatformComplete::FDelegate FOnUnlinkOtherPlatformCompleteDelegate;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnLinkOtherPlatformIdComplete, bool /*bWasSuccessful*/, const FOnlineError & /*OnlineError*/);
typedef FOnLinkOtherPlatformIdComplete::FDelegate FOnLinkOtherPlatformIdCompleteDelegate;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnUnlinkOtherPlatformIdComplete, bool /*bWasSuccessful*/, const FOnlineError & /*OnlineError*/);
typedef FOnUnlinkOtherPlatformIdComplete::FDelegate FOnUnlinkOtherPlatformIdCompleteDelegate; 

DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnCheckUserAccountAvailabilityComplete, bool /*bWasSuccessful*/, bool /*bUserExisted*/, const FOnlineError & /*OnlineError*/);
typedef FOnCheckUserAccountAvailabilityComplete::FDelegate FOnCheckUserAccountAvailabilityCompleteDelegate;

DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnValidateUserInputComplete, const FUserInputValidationResponse& /*UserInputValidationResponse*/, bool /*bWasSuccessful*/, const FOnlineError & /*OnlineError*/);
typedef FOnValidateUserInputComplete::FDelegate FOnValidateUserInputCompleteDelegate;

DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnGetInputValidationsComplete, const FInputValidation& /*InputValidation*/, bool /*bWasSuccessful*/, const FOnlineError& /*OnlineError*/);
typedef FOnGetInputValidationsComplete::FDelegate FOnGetInputValidationsCompleteDelegate;

DECLARE_DELEGATE_FiveParams(FOnQueryUserIdsMappingComplete, bool /*bWasSuccessful*/, const FUniqueNetId& /*UserId*/, const FString& /*SearchDisplayName*/, const TArray<TSharedRef<const FUniqueNetIdAccelByteUser>>& /*FoundUsers*/, const FString& /*Error*/);

class ONLINESUBSYSTEMACCELBYTE_API FOnlineUserAccelByte : public IOnlineUser, public TSharedFromThis<FOnlineUserAccelByte, ESPMode::ThreadSafe>
{
public:
	FOnlineUserAccelByte(FOnlineSubsystemAccelByte* InSubsystem);
	virtual ~FOnlineUserAccelByte() override = default;

	/**
	 * Convenience method to get an instance of this interface from the subsystem associated with the world passed in.
	 *
	 * @param World World instance that we wish to get the interface from
	 * @param OutInterfaceInstance Instance of the interface that we got from the subsystem, or nullptr if not found
	 * @returns boolean that is true if we could get an instance of the interface, false otherwise
	 */
	static bool GetFromWorld(const UWorld* World, TSharedPtr<FOnlineUserAccelByte, ESPMode::ThreadSafe>& OutInterfaceInstance);

	/**
	 * Convenience method to get an instance of this interface from the subsystem passed in.
	 *
	 * @param Subsystem Subsystem instance that we wish to get this interface from
	 * @param OutInterfaceInstance Instance of the interface that we got from the subsystem, or nullptr if not found
	 * @returns boolean that is true if we could get an instance of the interface, false otherwise
	 */
	static bool GetFromSubsystem(const IOnlineSubsystem* Subsystem, TSharedPtr<FOnlineUserAccelByte, ESPMode::ThreadSafe>& OutInterfaceInstance);
	static bool GetFromSubsystem(const FOnlineSubsystemAccelByte* SubsystemAB, TSharedPtr<FOnlineUserAccelByte, ESPMode::ThreadSafe>& OutInterfaceInstance);

	/**
	 * Starts an async task that create the profiles for the requesting user. Will trigger OnCreateUserProfileComplete Online Delegate when Complete
	 *
	 * @param UserId the user id requesting the create
	 */
	virtual bool CreateUserProfile(const FUniqueNetId& UserId);

	/**
	 * Delegate used when the userProfile create request has completed
	 *
	 * @param LocalUserNum the controller number of the associated user that made the request
	 * @param bWasSuccessful true if the async action completed without error, false if there was an error
	 * @param Error information about the error condition
	 */
	DEFINE_ONLINE_PLAYER_DELEGATE_TWO_PARAM(MAX_LOCAL_PLAYERS, OnCreateUserProfileComplete, bool /*bWasSuccessful*/, const FOnlineError& /*Error*/);

	/**
	 * Starts an async task that updates the profile for the requesting user. Will trigger OnUpdateUserProfileComplete Online Delegate when Complete
	 *
	 * @param LocalUserNum the local user number requesting the update
	 * @param UserId the user id requesting the update
	 * @param UpdateRequest the profile update request containing the fields to update
	 */
	virtual bool UpdateUserProfile(int32 LocalUserNum, const FUniqueNetId& UserId, const FAccelByteModelsUserProfileUpdateRequest& UpdateRequest);

	/**
	 * Delegate used when the userProfile update request has completed
	 *
	 * @param LocalUserNum the controller number of the associated user that made the request
	 * @param bWasSuccessful true if the async action completed without error, false if there was an error
	 * @param Error information about the error condition
	 */
	DEFINE_ONLINE_PLAYER_DELEGATE_TWO_PARAM(MAX_LOCAL_PLAYERS, OnUpdateUserProfileComplete, bool /*bWasSuccessful*/, const FOnlineError& /*Error*/);

	/**
	 * Delegate used when the userProfile query request has completed
	 *
	 * @param LocalUserNum the controller number of the associated user that made the request
	 * @param bWasSuccessful true if the async action completed without error, false if there was an error
	 * @param UserIds list of user ids that were queried
	 * @param Error information about the error condition
	 */
	DEFINE_ONLINE_PLAYER_DELEGATE_THREE_PARAM(MAX_LOCAL_PLAYERS, OnQueryUserProfileComplete, bool /*bWasSuccessful*/, const TArray< FUniqueNetIdRef >& /*UserIds*/, const FOnlineError& /*Error*/);

	/**
	 * Starts an async task that queries/reads the profiles for a list of users
	 *
	 * @param LocalUserNum the user requesting the query
	 * @param UserIds list of users to read info about
	 */
	virtual bool QueryUserProfile(int32 LocalUserNum, const TArray<TSharedRef<const FUniqueNetId>>& UserIds);

	/**
	 * Starts an async task that gets the authenticated user's own complete profile including private data
	 *
	 * @param LocalUserNum the user requesting their own profile
	 */
	virtual bool GetMyUserProfile(int32 LocalUserNum);

	/**
	 * Starts an async task that gets a user's public profile using their PublicId (friend code)
	 *
	 * @param LocalUserNum the user requesting the query
	 * @param PublicId the human-readable PublicId to look up
	 */
	virtual bool GetPublicUserProfileByPublicId(int32 LocalUserNum, const FString& PublicId);

	/**
	 * Starts an async task that gets a user's public profile using their UserId
	 *
	 * @param LocalUserNum the user requesting the query
	 * @param UserId the UserId to look up
	 */
	virtual bool GetPublicUserProfileInfo(int32 LocalUserNum, const FString& UserId);

	/**
	 * Starts an async task that gets the authenticated user's public custom attributes
	 *
	 * @param LocalUserNum the user requesting their custom attributes
	 */
	virtual bool GetCustomAttributes(int32 LocalUserNum);

	/**
	 * [DEPRECATED] This function has been removed due to security issues.
	 * Please use 'GetPublicUserProfileInfo(UserId)' instead, which includes
	 * CustomAttributes in the returned FAccelByteModelsPublicUserProfileInfo.
	 *
	 * @deprecated SDK function deprecated with error code 14901
	 */
	// virtual bool GetPublicCustomAttributes(int32 LocalUserNum, const FString& UserId); // REMOVED

	/**
	 * Starts an async task that updates the authenticated user's public custom attributes
	 *
	 * @param LocalUserNum the user requesting to update their custom attributes
	 * @param CustomAttributes the new custom attributes data
	 */
	virtual bool UpdateCustomAttributes(int32 LocalUserNum, const FJsonObject& CustomAttributes);

	/**
	 * Starts an async task that gets the authenticated user's private custom attributes
	 *
	 * @param LocalUserNum the user requesting their private custom attributes
	 */
	virtual bool GetPrivateCustomAttributes(int32 LocalUserNum);

	/**
	 * Starts an async task that updates the authenticated user's private custom attributes
	 *
	 * @param LocalUserNum the user requesting to update their private custom attributes
	 * @param PrivateAttributes the new private custom attributes data
	 */
	virtual bool UpdatePrivateCustomAttributes(int32 LocalUserNum, const FJsonObject& PrivateAttributes);

	/**
	 * Starts an async task that generates a presigned upload URL for profile file uploads
	 *
	 * @param LocalUserNum the user requesting the upload URL
	 * @param Folder the folder name for the upload (1-256 chars, no whitespace)
	 * @param FileType the type of file to upload (jpeg, jpg, png, bmp, gif, mp3, bin, webp)
	 */
	virtual bool GenerateUploadURL(int32 LocalUserNum, const FString& Folder, EAccelByteFileType FileType);

	/**
	 * Starts an async task that generates an upload URL for user content with category support
	 *
	 * @param LocalUserNum the user requesting the upload URL
	 * @param UserId the user ID for the content upload
	 * @param FileType the type of file to upload (jpeg, jpg, png, bmp, gif, mp3, bin, webp)
	 * @param Category the upload category (DEFAULT, REPORTING)
	 */
	virtual bool GenerateUploadURLForUserContent(int32 LocalUserNum, const FString& UserId, EAccelByteFileType FileType, EAccelByteUploadCategory Category = EAccelByteUploadCategory::DEFAULT);

	/**
	 * Starts an async task that gets bulk public user profile infos with V2 endpoint (includes NotProcessed array)
	 *
	 * @param LocalUserNum the user requesting the bulk profile infos
	 * @param UserIds array of user IDs to query
	 */
	virtual bool BulkGetPublicUserProfileInfosV2(int32 LocalUserNum, const TArray<FString>& UserIds);

	/**
	 * Creates a user profile for the authenticated user (parameterless overload)
	 *
	 * @param LocalUserNum the user requesting to create their profile
	 */
	virtual bool CreateUserProfile(int32 LocalUserNum);

	/**
	 * Updates the authenticated user's profile (parameterless overload)
	 *
	 * @param LocalUserNum the user requesting to update their profile
	 * @param UpdateRequest the profile update data
	 */
	virtual bool UpdateUserProfile(int32 LocalUserNum, const FAccelByteModelsUserProfileUpdateRequest& UpdateRequest);

	/**
	 * Gets a specific user's complete profile by UserId (may require admin privileges)
	 *
	 * @param LocalUserNum the user requesting the profile
	 * @param UserId the user ID whose complete profile to retrieve
	 */
	virtual bool GetUserProfile(int32 LocalUserNum, const FString& UserId);

	/**
	 * Delegate used when the get own user profile request has completed
	 *
	 * @param LocalUserNum the controller number of the associated user that made the request
	 * @param bWasSuccessful true if the async action completed without error, false if there was an error
	 * @param ProfileInfo the complete user profile information including private data
	 * @param Error information about the error condition
	 */
	DEFINE_ONLINE_PLAYER_DELEGATE_THREE_PARAM(MAX_LOCAL_PLAYERS, OnGetMyUserProfileComplete, bool /*bWasSuccessful*/, const FAccelByteModelsUserProfileInfo& /*ProfileInfo*/, const FOnlineError& /*Error*/);

	/**
	 * Delegate used when the get public user profile by PublicId request has completed
	 *
	 * @param LocalUserNum the controller number of the associated user that made the request
	 * @param bWasSuccessful true if the async action completed without error, false if there was an error
	 * @param ProfileInfo the public user profile information
	 * @param Error information about the error condition
	 */
	DEFINE_ONLINE_PLAYER_DELEGATE_THREE_PARAM(MAX_LOCAL_PLAYERS, OnGetPublicUserProfileByPublicIdComplete, bool /*bWasSuccessful*/, const FAccelByteModelsPublicUserProfileInfo& /*ProfileInfo*/, const FOnlineError& /*Error*/);

	/**
	 * Delegate used when the get public user profile info request has completed
	 *
	 * @param LocalUserNum the controller number of the associated user that made the request
	 * @param bWasSuccessful true if the async action completed without error, false if there was an error
	 * @param ProfileInfo the public user profile information
	 * @param Error information about the error condition
	 */
	DEFINE_ONLINE_PLAYER_DELEGATE_THREE_PARAM(MAX_LOCAL_PLAYERS, OnGetPublicUserProfileInfoComplete, bool /*bWasSuccessful*/, const FAccelByteModelsPublicUserProfileInfo& /*ProfileInfo*/, const FOnlineError& /*Error*/);

	/**
	 * Delegate used when the get custom attributes request has completed
	 *
	 * @param LocalUserNum the controller number of the associated user that made the request
	 * @param bWasSuccessful true if the async action completed without error, false if there was an error
	 * @param CustomAttributes the public custom attributes
	 * @param Error information about the error condition
	 */
	DEFINE_ONLINE_PLAYER_DELEGATE_THREE_PARAM(MAX_LOCAL_PLAYERS, OnGetCustomAttributesComplete, bool /*bWasSuccessful*/, const FJsonObjectWrapper& /*CustomAttributes*/, const FOnlineError& /*Error*/);

	/**
	 * Delegate used when the get public custom attributes request has completed
	 *
	 * @param LocalUserNum the controller number of the associated user that made the request
	 * @param bWasSuccessful true if the async action completed without error, false if there was an error
	 * @param CustomAttributes the public custom attributes of the requested user
	 * @param Error information about the error condition
	 */
	// DEFINE_ONLINE_PLAYER_DELEGATE_THREE_PARAM(MAX_LOCAL_PLAYERS, OnGetPublicCustomAttributesComplete, bool /*bWasSuccessful*/, const FJsonObjectWrapper& /*CustomAttributes*/, const FOnlineError& /*Error*/); // DEPRECATED - REMOVED

	/**
	 * Delegate used when the update custom attributes request has completed
	 *
	 * @param LocalUserNum the controller number of the associated user that made the request
	 * @param bWasSuccessful true if the async action completed without error, false if there was an error
	 * @param UpdatedAttributes the updated public custom attributes
	 * @param Error information about the error condition
	 */
	DEFINE_ONLINE_PLAYER_DELEGATE_THREE_PARAM(MAX_LOCAL_PLAYERS, OnUpdateCustomAttributesComplete, bool /*bWasSuccessful*/, const FJsonObjectWrapper& /*UpdatedAttributes*/, const FOnlineError& /*Error*/);

	/**
	 * Delegate used when the get private custom attributes request has completed
	 *
	 * @param LocalUserNum the controller number of the associated user that made the request
	 * @param bWasSuccessful true if the async action completed without error, false if there was an error
	 * @param PrivateAttributes the private custom attributes
	 * @param Error information about the error condition
	 */
	DEFINE_ONLINE_PLAYER_DELEGATE_THREE_PARAM(MAX_LOCAL_PLAYERS, OnGetPrivateCustomAttributesComplete, bool /*bWasSuccessful*/, const FJsonObjectWrapper& /*PrivateAttributes*/, const FOnlineError& /*Error*/);

	/**
	 * Delegate used when the update private custom attributes request has completed
	 *
	 * @param LocalUserNum the controller number of the associated user that made the request
	 * @param bWasSuccessful true if the async action completed without error, false if there was an error
	 * @param UpdatedAttributes the updated private custom attributes
	 * @param Error information about the error condition
	 */
	DEFINE_ONLINE_PLAYER_DELEGATE_THREE_PARAM(MAX_LOCAL_PLAYERS, OnUpdatePrivateCustomAttributesComplete, bool /*bWasSuccessful*/, const FJsonObjectWrapper& /*UpdatedAttributes*/, const FOnlineError& /*Error*/);

	/**
	 * Delegate used when the generate upload URL request has completed
	 *
	 * @param LocalUserNum the controller number of the associated user that made the request
	 * @param bWasSuccessful true if the async action completed without error, false if there was an error
	 * @param UploadURLResult the upload URL information including URL and access URL
	 * @param Error information about the error condition
	 */
	DEFINE_ONLINE_PLAYER_DELEGATE_THREE_PARAM(MAX_LOCAL_PLAYERS, OnGenerateUploadURLComplete, bool /*bWasSuccessful*/, const FAccelByteModelsUserProfileUploadURLResult& /*UploadURLResult*/, const FOnlineError& /*Error*/);

	/**
	 * Delegate used when the generate upload URL for user content request has completed
	 *
	 * @param LocalUserNum the controller number of the associated user that made the request
	 * @param bWasSuccessful true if the async action completed without error, false if there was an error
	 * @param UploadURLResult the upload URL result containing URLs and access information
	 * @param Error information about the error condition
	 */
	DEFINE_ONLINE_PLAYER_DELEGATE_THREE_PARAM(MAX_LOCAL_PLAYERS, OnGenerateUploadURLForUserContentComplete, bool /*bWasSuccessful*/, const FAccelByteModelsUserProfileUploadURLResult& /*UploadURLResult*/, const FOnlineError& /*Error*/);

	/**
	 * Delegate used when the bulk get public user profile infos V2 request has completed
	 *
	 * @param LocalUserNum the controller number of the associated user that made the request
	 * @param bWasSuccessful true if the async action completed without error, false if there was an error
	 * @param ProfileInfosV2 the bulk public user profile infos with NotProcessed array
	 * @param Error information about the error condition
	 */
	DEFINE_ONLINE_PLAYER_DELEGATE_THREE_PARAM(MAX_LOCAL_PLAYERS, OnBulkGetPublicUserProfileInfosV2Complete, bool /*bWasSuccessful*/, const FAccelByteModelsPublicUserProfileInfoV2& /*ProfileInfosV2*/, const FOnlineError& /*Error*/);

	/**
	 * Delegate used when the get user profile by user ID request has completed
	 *
	 * @param LocalUserNum the controller number of the associated user that made the request
	 * @param bWasSuccessful true if the async action completed without error, false if there was an error
	 * @param UserProfileInfo the complete user profile info including private data
	 * @param Error information about the error condition
	 */
	DEFINE_ONLINE_PLAYER_DELEGATE_THREE_PARAM(MAX_LOCAL_PLAYERS, OnGetUserProfileComplete, bool /*bWasSuccessful*/, const FAccelByteModelsUserProfileInfo& /*UserProfileInfo*/, const FOnlineError& /*Error*/);

	/**
	 * Delegate used when the userProfile query request has completed
	 *
	 * @param LocalUserNum the controller number of the associated user that made the request
	 * @param bWasSuccessful true if the async action completed without error, false if there was an error
	 * @param UserIds list of user ids that were queried
	 * @param Error information about the error condition
	 */
	DEFINE_ONLINE_PLAYER_DELEGATE_THREE_PARAM(MAX_LOCAL_PLAYERS, OnGetUserPlatformLinksComplete, bool /*bWasSuccessful*/, const TArray<FPlatformLink>& /*UserPlatformLinks*/, const FOnlineError& /*Error*/);

	/**
	 * Starts an async task that queries/reads user's platform accounts linked to user's account.
	 *
	 * @param LocalUserNum the user requesting the query
	 */
	virtual bool GetUserPlatformLinks(int32 LocalUserNum);

	//~ Begin IOnlineUser overrides
	virtual bool QueryUserInfo(int32 LocalUserNum, const TArray<TSharedRef<const FUniqueNetId>>& UserIds) override;
	virtual bool GetAllUserInfo(int32 LocalUserNum, TArray<TSharedRef<class FOnlineUser>>& OutUsers) override;
	virtual TSharedPtr<FOnlineUser> GetUserInfo(int32 LocalUserNum, const class FUniqueNetId& UserId) override;
	virtual bool QueryUserIdMapping(const FUniqueNetId& UserId, const FString& DisplayNameOrEmail, const FOnQueryUserMappingComplete& Delegate = FOnQueryUserMappingComplete()) override;
	virtual bool QueryUserIdMappingWithPlatform(const FUniqueNetId& UserId, const FString& DisplayNameOrEmail, EAccelBytePlatformType PlatformType, const FOnQueryUserMappingComplete& Delegate);
	virtual bool QueryUserIdMappingWithPlatformId(const FUniqueNetId& UserId, const FString& DisplayNameOrEmail, const FString& PlatformId, const FOnQueryUserMappingComplete& Delegate);
	virtual bool QueryExternalIdMappings(const FUniqueNetId& UserId, const FExternalIdQueryOptions& QueryOptions, const TArray<FString>& ExternalIds, const FOnQueryExternalIdMappingsComplete& Delegate = FOnQueryExternalIdMappingsComplete()) override;
	virtual void GetExternalIdMappings(const FExternalIdQueryOptions& QueryOptions, const TArray<FString>& ExternalIds, TArray<FUniqueNetIdPtr>& OutIds) override;
	virtual FUniqueNetIdPtr GetExternalIdMapping(const FExternalIdQueryOptions& QueryOptions, const FString& ExternalId) override;
	//~ End IOnlineUser overrides

	/**
	 * Get bulk list user by UserId. Only for request by Game Server
	 * 
	 * @param LocalUserNum Index of user(server) that is attempting to create the stats
	 * @param UserIds User to create stats for 
	 */
	virtual void ListUserByUserId(const int32 LocalUserNum, const TArray<FString>& UserIds);
	
	DEFINE_ONLINE_PLAYER_DELEGATE_THREE_PARAM(MAX_LOCAL_PLAYERS, OnListUserByUserIdComplete, bool /*bWasSuccessful*/, const FListUserDataResponse& /* ListUser*/, const FOnlineError & /* OnlineError */);

	DEFINE_ONLINE_DELEGATE_TWO_PARAM(OnLinkOtherPlatformComplete, bool /*bWasSuccessful*/, const FOnlineError & /*OnlineError*/);
	/*
	 * Links user's current account to their other account in other platform.
	 * Ticket from platform (Platform Token/Authorization Code), can be obtained from Platform OSS or Plugin. 
	 * The browser will redirect the URL to a site with a code in form of parameter URL.
	 */
	void LinkOtherPlatform(const FUniqueNetId& UserId, EAccelBytePlatformType PlatformType, const FString& Ticket);
		
	DEFINE_ONLINE_DELEGATE_TWO_PARAM(OnUnlinkOtherPlatformComplete, bool /*bWasSuccessful*/, const FOnlineError & /*OnlineError*/);
	/*
	 * Unlinks user's current account from their other account in other platform
	 */
	void UnlinkOtherPlatform(const FUniqueNetId& UserId, EAccelBytePlatformType PlatformType);

	DEFINE_ONLINE_DELEGATE_TWO_PARAM(OnLinkOtherPlatformIdComplete, bool /*bWasSuccessful*/, const FOnlineError & /*OnlineError*/);
	/*
	 * Links user's current account to their other account in other platform, especially to support OIDC.
	 * Ticket from platform (Platform Token/Authorization Code), can be obtained from Platform OSS or Plugin.
	 * The browser will redirect the URL to a site with a code in form of parameter URL.
	 */
	void LinkOtherPlatformId(const FUniqueNetId& UserId, const FString& PlatformId, const FString& Ticket);

	/*
	 * Unlinks user's current account from their other account in other platform, especially to support OIDC
	 */
	DEFINE_ONLINE_DELEGATE_TWO_PARAM(OnUnlinkOtherPlatformIdComplete, bool /*bWasSuccessful*/, const FOnlineError & /*OnlineError*/);
	void UnlinkOtherPlatformId(const FUniqueNetId& UserId, const FString& PlatformId);

	/**
	 * Delegate called when a controller-user check user account availability.
	 */
	DEFINE_ONLINE_DELEGATE_THREE_PARAM(OnCheckUserAccountAvailabilityComplete, bool /*bWasSuccessful*/, bool /*bUserExisted*/, const FOnlineErrorAccelByte & /*OnlineError*/);
	/**
	 *  Check users's account availability, available only using displayName field.   
	 *
	 * @param UserId The user's user ID
	 * @param DisplayName User's display name value to be checked.
	 * @param bIsSearchUniqueDisplayName Whether search account availability by unique display name.
	 */
	void CheckUserAccountAvailability(const FUniqueNetId& UserId, const FString& DisplayName, bool bIsSearchUniqueDisplayName = false);

	/**
	 * Add a third-party account linked with the AccelByte account to cache.
	 *
	 * @param UserId Unique net ID of the user for whom we want to add the linked account
	 * @param LinkedAccount Third-party account information
	 */
	void AddNewLinkedUserAccountToCache(const TSharedRef<const FUniqueNetId>& UserId, const TArray<FPlatformLink>& LinkedAccounts);

	/**
	 * Remove all third-party account linked mappings for a user by their unique net ID.
	 *
	 * @param UserId Unique net ID of the user for whom we wish to remove mappings.
	 *
	 */
	bool RemoveLinkedUserAccountFromCache(const TSharedRef<const FUniqueNetId>& UserId);

	/**
	 * Get all third-party account linked mappings for a user by their unique net ID.
	 *
	 * @param UserId Unique net ID of the user for whom we wish to get.
	 * @param OutLinkedAccounts A list of linked account from the selected user.
	 *
	 */
	void GetLinkedUserAccountFromCache(const TSharedRef<const FUniqueNetId>& UserId, TArray<FAccelByteUserPlatformLinkInformationRef>& OutLinkedAccounts);

	/*
	 * Unlinks user's current account from their other account in other platform, especially to support OIDC
	 */
	DEFINE_ONLINE_DELEGATE_THREE_PARAM(OnValidateUserInputComplete, const FUserInputValidationResponse& /*UserInputValidationResponse*/, bool /*bWasSuccessful*/, const FOnlineError & /*OnlineError*/);
	void ValidateUserInput(int32 LocalUserNum, const FUserInputValidationRequest& UserInputValidationRequest);

	/**
	 * Get input validation rules for the configured namespace.
	 * The namespace is automatically retrieved from SDK settings.
	 *
	 * @param LocalUserNum Index of the user making the request.
	 * @param LanguageCode Targeted language code using ISO-639.
	 * @param bDefaultOnEmpty If true, returns default input validation when no custom rules are found.
	 */
	DEFINE_ONLINE_DELEGATE_THREE_PARAM(OnGetInputValidationsComplete, const FInputValidation& /*InputValidation*/, bool /*bWasSuccessful*/, const FOnlineError& /*OnlineError*/);
	void GetInputValidations(int32 LocalUserNum, const FString& LanguageCode, bool bDefaultOnEmpty = true);

	/*
	 * Query users based on DisplayNameOrEmail, support multiple results.
	 */
	virtual bool QueryUserIdsMapping(const FUniqueNetId& UserId, const FString& DisplayNameOrEmail, const FOnQueryUserIdsMappingComplete& Delegate = FOnQueryUserIdsMappingComplete(), int32 Offset = 0, int32 Limit = 100);

PACKAGE_SCOPE:

#if WITH_DEV_AUTOMATION_TESTS
	/**
	 * Internal method for handling extra exec tests for this interface.
	 */
	bool TestExec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar);
#endif

	/**
	 * Internal method used by FOnlineAsyncTaskAccelByteQueryUserInfo to move user info instances back to this interface.
	 */
	void AddUserInfo(const TSharedRef<const FUniqueNetId>& UserId, const TSharedRef<FUserOnlineAccountAccelByte>& UserInfo)
	{
		IDToUserInfoMap.Add(UserId, UserInfo);
	}

	/**
	 * Internal method used by FOnlineAsyncTaskAccelByteQueryExternalIdMappings to move external ID to AccelByte ID references back to this interface.
	 */
	void AddExternalIdMappings(const TMap<FString, TSharedRef<const FUniqueNetId>>& InMap)
	{
		ExternalIDToAccelByteIDMap.Append(InMap);
	}

	/*
	 * Delegate for after completed get user profile on login
	 */
	void PostLoginBulkGetUserProfileCompleted(int32 LocalUserNum, bool bWasSuccessful, const TArray<FUniqueNetIdRef>& UserIds, const FOnlineError& ErrorStr);
	/** Thread-safe helper to get delegate reference for async task creation */

private:

	/** Pointer to the AccelByte OSS instance that instantiated this online user interface. */
	FOnlineSubsystemAccelByteWPtr AccelByteSubsystem = nullptr;

	/** Map of AccelByte IDs to AccelByte accounts */
	TUniqueNetIdMap<TSharedRef<FUserOnlineAccountAccelByte>> IDToUserInfoMap;

	/** Map of external IDs (external platform user IDs or display names) to AccelByte IDs */
	TMap<FString, TSharedRef<const FUniqueNetId>> ExternalIDToAccelByteIDMap;

	/** Mapping of AccelByte net IDs to AccelByte linked user accounts */
	TUniqueNetIdMap<TArray<FAccelByteUserPlatformLinkInformationRef>> NetIdToLinkedOnlineAccountMap;
};
