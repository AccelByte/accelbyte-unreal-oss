# AccelByte Cloud Online Subsystem
## Overview
AccelByte Cloud Online Subsystem (**AccelByte Cloud OSS**) is the high-level bridge between Unreal Engine 
and AccelByte services that comprises interfaces that access AccelByte services and its features. 
The AccelByte Cloud OSS is designed to handle higher level logic with asynchronous communication and 
delegates, and is also designed to  be modular by grouping similar service-specific APIs that support features together.
## Supported Unreal Engine

Engine versions supported by each release. Ranges are inclusive; `latest` means the most
recent [release](https://github.com/AccelByte/accelbyte-unreal-oss/releases).

| Unreal Engine | OSS releases                                 |
|---------------|----------------------------------------------|
| 4.27          | `0.13.10` – latest                           |
| 5.0 – 5.4     | `0.13.10` – `0.13.12` (removed in `0.13.13`) |
| 5.5           | `0.13.10` – latest                           |
| 5.6           | `0.13.10` – latest                           |
| 5.7           | `0.13.10` – latest (Beta until `0.13.14`)    |
| 5.8           | `0.13.14` – latest                           |

Releases before `0.13.10` did not declare supported engine versions in this README.

## Dependencies
AccelByte OSS have some dependencies to another Plugins/Modules, such as the following:
1. AccelByte Cloud Unreal Engine SDK ([link](https://github.com/accelbyte/accelbyte-unreal-sdk-plugin)):
   a library that comprises APIs for the game client and game server to send requests to AccelByte services.
2. AccelByte Cloud Network Utilities ([link](https://github.com/AccelByte/accelbyte-unreal-network-utilities)):
   a library that comprises network functionalities to communicate between game clients for P2P networking.
## Documentation
The setup and implementation guideline are available in [our portal](https://docs.accelbyte.io/gaming-services/knowledge-base/sdk-tools/sdk-guides/ags-oss-for-ue/).