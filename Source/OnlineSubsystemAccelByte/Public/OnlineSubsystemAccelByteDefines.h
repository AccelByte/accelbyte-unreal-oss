/**
 * A collection of defines relating to the OnlineSubsystemAccelByte. Should be defines that are also relevant to developers
 * using our OnlineSubsystem.
 */
#pragma once

/**
 * Session setting key for whether we want to use our ICE layer (NAT relay) for sessions. Can have a value of true or false.
 */
#define SETTING_ACCELBYTE_ICE_ENABLED TEXT("accelbyte_ice_enabled")

/**
* Session setting key for whether we want the Dedicated server to register the session,
* instead of waiting to be created from matchmaker.
* Can have a value of true or false.
*/
#define SETTING_SERVER_DEDICATED_REGISTER_SESSION TEXT("accelbyte_ds_register_session")

/**
 * Session setting key for the search type that we want to use for FindSessions in the SessionInterface.
 */
#define SETTING_SEARCH_TYPE FName(TEXT("ABSESSIONSEARCHTYPE"))

/**
 * Value for our FindSessions search type setting that looks specifically for dedicated sessions.
 */
#define SETTING_SEARCH_TYPE_DEDICATED TEXT("dedicated")

/**
 * Value for our FindSessions search type setting that specifically looks for peer to peer sessions through our relay.
 */
#define SETTING_SEARCH_TYPE_PEER_TO_PEER_RELAY TEXT("p2p")

/**
 * Session setting for matchmaking sessions that determines whether the matchmaking flow will automatically send a
 * ready consent for the player.
 *
 * Defaults to false, meaning that we don't want an explicit ready consent, and that we want to automatically ready
 * the player. If set to true, you will manually have to call SessionInt->SendReady.
 */
#define SETTING_REQUEST_EXPLICIT_READY FName(TEXT("ABREQUESTEXPLICITREADY"))

/**
 * If we haven't already defined the name of our subsystem, then we want to define it as an FName of "ACCELBYTE"
 */
#ifndef ACCELBYTE_SUBSYSTEM
#define ACCELBYTE_SUBSYSTEM FName(TEXT("ACCELBYTE"))
#endif

/**
 * Travel URL prefix used for peer to peer sessions sent through our relay for connection.
 */
#define ACCELBYTE_URL_PREFIX TEXT("accelbyte.")

/**
 * Session setting key for whether this server is local or not.
 */
#define SETTING_SESSION_LOCAL FName(TEXT("ABSESSIONISLOCAL"))

/**
 * Session setting key for the name of the server that owns the session
 */
#define SETTING_SESSION_SERVER_NAME FName(TEXT("ABSESSIONSERVER"))

#define SETTING_SESSION_START_TIME FName(TEXT("SESSIONSTARTTIME"))
#define SETTING_SESSION_END_TIME FName(TEXT("SESSIONENDTIME"))

#define ACCELBYTE_ARGS_SERVERIP TEXT("serverip")

const FString ClientIdPrefix = FString(TEXT("client-"));