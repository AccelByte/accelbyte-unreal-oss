# AccelByte Online Subsystem
## Overview
AccelByte Cloud Online Subsystem (**OSS**) is the high-level bridge between Unreal Engine and AccelByte
services that comprises interfaces that access AccelByte services and its features. The AccelByte Cloud
OSS is designed to handle higher level logic with asynchronous communication and delegates, and is also
designed to  be modular by grouping similar service-specific APIs that support features together.
## Getting Started

### Dependencies
AccelByte OSS have some dependencies to another Plugins/Modules, such as the following:
1. AccelByte Cloud Unreal Engine SDK ([link](https://github.com/accelbyte/accelbyte-unreal-sdk-plugin)):
   a library that comprises APIs for the game client and game server to send requests to AccelByte services.
2. AccelByte Cloud Network Utilities ([link](https://github.com/AccelByte/accelbyte-unreal-network-utilities)):
   a library that comprises network functionalities to communicate between game clients for P2P networking.

### Configuration
To use AccelByte OSS, some configurations need to be set up first as follows:
1. Add required plugins to .uproject file
```
"Plugins": [
  {
     "Name": "AccelByteUe4Sdk",
     "Enabled": true
  },
  {
     "Name": "OnlineSubsystemAccelByte",
     "Enabled": true
  },
  {
     "Name": "AccelByteNetworkUtilities",
     "Enabled": true
  },
  ...
]
```
2. Add public dependency module to the Build.cs file
```
PublicDependencyModuleNames.AddRange(new string[] { 
  "AccelByteUe4Sdk",
  "AccelByteNetworkUtilities",
  "OnlineSubsystemAccelByte"
});
```
3. Add the following modules to the Target.cs file
```
ExtraModuleNames.AddRange( new string[] { 
  "AccelByteUe4Sdk",
  "OnlineSubsystemAccelByte",
  "AccelByteNetworkUtilities"
});
```
4. Edit the DefaultEngine.ini to include the following settings
```
[OnlineSubsystem]
DefaultPlatformService=AccelByte
; Specifies the name of the platform-specific OSS
NativePlatformService=

[OnlineSubsystemAccelByte]
bEnabled=true
; Enable/Disable multiple local users support
bMultipleLocalUsersEnabled=false
; Specifies to automatically connect to Lobby WebSocket
bAutoLobbyConnectAfterLoginSuccess=true
```
5. Edit the platform specific config ini file located inside the platform's folder (e.g. ```Config/Windows/WindowsEngine.ini```)
```
; Set the OSS configs for Windows to use the AccelByte OSS 
; as the default subsystem, and Steam as the native
[OnlineSubsystem]
NativePlatformService=Steam

[OnlineSubsystemSteam]
bEnabled=true
bUseSteamNetworking=false
SteamDevAppId=<game_app_id>
```
### LocalUserNum
The AccelByte Cloud OSS is able to handle multiple local users playing in the same game instance, by identifying
the controller index of the users. The same controller index is defined as **LocalUserNum** in most of OSS
interfaces. The AccelByte Cloud OSS stores user online information using **LocalUserNum** as the key.
### UniqueNetId
UniqueNetId is the abstraction of the user’s profile in Online services. In AccelByte Cloud OSS, it is derived
as AccelByte Composite UniqueNetId and consists of AccelByte userId, originating platform, and
originating platform userId data fields.
## Interfaces
The OSS has predefined interfaces which can be extended to some degrees, and AccelByte OSS has extended some of
the interfaces which game developers can integrate it to their games. These interfaces will wrap the APIs in
asynchronous calls so the game can still run as usual.
### Online Identity Interface
Online Identity Interface is used for player registration/authentication
#### Login
The AccelByte Cloud OSS offers several different login types based on the native platform or with other platforms
by providing the type and credentials in th **FOnlineAccountCrdentialsAccelByte** class.
##### Login Type
There are several login types available in the AccelByte Cloud OSS. These login types are categorized as
enumerations in **EAccelByteLoginType** and the types include:
- DeviceId
- AccelByte
- Xbox
- PS4
- PS5
- Launcher
- Steam
- RefreshToken
```
const FOnlineIdentityAccelBytePtr IdentityInterface = FOnlineIdentityAccelByte::Get();
 
if (IdentityInterface.IsValid())
{
   FOnlineAccountCredentialsAccelByte Credentials{ EAccelByteLoginType::AccelByte
         , Username, Password };
   // Login
   IdentityInterface->AddOnLoginCompleteDelegate_Handle(LocalUserNum
      , FOnLoginCompleteDelegate::CreateLambda([](int32 LocalUserNum, bool bLoginWasSuccessful, const FUniqueNetId& UserId, const FString& LoginError)
         {
            if (bLoginWasSuccessful)
            {
               // Do something when player successfully logged in
            } 
            else
            {
               // Do something when player failed to log in
            }
         })
      );
   IdentityInterface->Login(LocalUserNum, Credentials);
}
```
After successfully loggin in, the user will need to connect to Lobby to be able to use social-related
interfaces and multiplayer features such as Friends, Party, Presence, Matchmaking. The user can connect
to Lobby manually by calling **ConnectAccelByteLobby** or by setting **bAutoLobbyConnectAfterLoginSucces**
to true in DefaultEngine.ini
```
const FOnlineIdentityAccelBytePtr IdentityInterface = FOnlineIdentityAccelByte::Get();
 
if (IdentityInterface.IsValid())
{
   // Connect to AccelByte Lobby Websocket, can be automatically called after
   // successful login by configuring bAutoLobbyConnectAfterLoginSuccess to true
   // in DefaultEngine.ini
   IdentityInterface->ConnectAccelByteLobby(LocalUserNum);
}
```
##### Logout
When the user is finished playing, log out using the following code
```
const FOnlineIdentityAccelBytePtr IdentityInterface = FOnlineIdentityAccelByte::Get();
 
if (IdentityInterface.IsValid())
{
   // Logout
   IdentityInterface->AddOnLogoutCompleteDelegate_Handle(LocalUserNum
      , FOnLogoutCompleteDelegate::CreateLambda([](int32 LocalUserNum, bool bLogoutWasSuccessful)
         {
            if (bLogoutWasSuccessful)
            {
               // Do something when player successfully logged out
            } 
            else
            {
               // Do something when player failed to log out
            }
         })
      );
   IdentityInterface->Logout(LocalUserNum);
}
```