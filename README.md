# AccelByte Online Subsystem
## Overview
AccelByte Online Subsystem (**OSS**) will provide a high-level bridge between the Unreal Engine game and AccelByte services that comprises interfaces to access the functionality of the services and its features. OSS is designed to handle higher level logic with asynchronous communication and delegates, It is also designed to be modular by grouping similar service-specific APIs that support features together.

## Getting Started

### Dependencies
AccelByte OSS have some dependencies to another Plugins/Modules, such as the following:
1. AccelByte Unreal Engine SDK ([link](https://github.com/accelbyte/accelbyte-unreal-sdk-plugin)):
   a library that comprises APIs for the game client and game server to send requests to AccelByte services.
2. AccelByte Network Utilities ([link](https://github.com/AccelByte/accelbyte-unreal-network-utilities)):
   a library that comprises network functionalities to communicate between game clients for P2P networking.

### Configurations
To use AccelByte OSS, some configurations need to be set up first as follows:
1. Edit DefaultEngine.ini
```
[OnlineSubsystem]
DefaultPlatformService=AccelByte
; Specifies the name of the platform-specific OSS
NativePlatformService=

[OnlineSubsystemAccelByte]
bEnabled=true
```
2. Edit platform specific config ini file which is located inside the platform folder name (e.g. ```Config/Windows/WindowsEngine.ini```)
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
