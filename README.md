# AccelByte Cloud Online Subsystem
## Overview
AccelByte Cloud Online Subsystem (**AccelByte Cloud OSS**) is the high-level bridge between Unreal Engine 
and AccelByte services that comprises interfaces that access AccelByte services and its features. 
The AccelByte Cloud OSS is designed to handle higher level logic with asynchronous communication and 
delegates, and is also designed to  be modular by grouping similar service-specific APIs that support features together.
## Supported Unreal Engine
- [ ] Unreal Engine 4.25
- [x] Unreal Engine 4.26
- [x] Unreal Engine 4.27
- [x] Unreal Engine 5.0
- [x] Unreal Engine 5.1
- [x] Unreal Engine 5.2
## Dependencies
AccelByte OSS have some dependencies to another Plugins/Modules, such as the following:
1. AccelByte Cloud Unreal Engine SDK ([link](https://github.com/accelbyte/accelbyte-unreal-sdk-plugin)):
   a library that comprises APIs for the game client and game server to send requests to AccelByte services.
2. AccelByte Cloud Network Utilities ([link](https://github.com/AccelByte/accelbyte-unreal-network-utilities)):
   a library that comprises network functionalities to communicate between game clients for P2P networking.
## Documentation
The setup and implementation guideline are available in [our portal](https://docs.accelbyte.io/gaming-services/sdk-tools/sdk-guides/ags-oss-for-ue/).