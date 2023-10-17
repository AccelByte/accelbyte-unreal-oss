# Changelog

All notable changes to this project will be documented in this file. See [standard-version](https://github.com/conventional-changelog/standard-version) for commit guidelines.

### [0.11.25](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.11.25%0D0.11.24) (2023-10-17)


### Bug Fixes

* crash when logout after subsystem is destroyed ([4582f16](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/4582f1634edde337a613998e3c387535a58c5778))

### [0.11.23](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.11.23%0D0.11.22) (2023-10-05)

### [0.11.22](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.11.22%0D0.11.21) (2023-09-25)


### Features

* add SyncThirdPartyPlatformFriendV2 ([5fd0e85](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/5fd0e854899ffa32d44dcd0cd48848e89b20d586))
* GroupInterface [See Description] ([3241f61](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/3241f61cfb114ef6aa5fdb539788e9cceb992ac7))
* GroupInterface adding comments ([3708d2b](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/3708d2b8a30bece26eecd09f1785639b8f467066))
* GroupInterface adding FScopeLock for arrays ([6bf5b9a](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/6bf5b9ab36de348947bbfab2c615c1e956bbf3e2))
* GroupInterface adding more critical sections and fixing spelling errors ([9b0509b](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/9b0509b5a156cc090c4efb9d3fe8adebe899f255))
* GroupInterface Adding TRACE ([af20a78](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/af20a78d284130768d2f5a8629cf71a5728c8ea6))
* GroupInterface Cancel Join Request and Query Group Join Requests ([35afdbf](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/35afdbfd0c441a0501130a769a6092b9db027118))
* GroupInterface FindGroups ([a6bfdd5](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/a6bfdd5ca71a766d9fd9f2386da413232d8473b0))
* GroupInterface GetGroupMembersByGroupIds ([90a8002](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/90a80027325496e36beb0f2d77cdb0c39444abce))
* GroupInterface getting rid of hardcoded success status ([67a80e7](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/67a80e702d9d9affac3ec1dd263a41ddf455a57a))
* GroupInterface more cleanup ([d1dc436](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/d1dc4364acaf73e9cd962335bc293058cfb3c52d))
* GroupInterface Query Group Info ([5769bf1](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/5769bf1c66d39692bc03df4ea070ee0c050cb54d))
* GroupInterface rebase with master ([b7c4090](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/b7c4090d005f44b92fe5e0d2f86d552b32e30065))
* GroupInterface Some cleanup and standardization ([4cce847](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/4cce8475aac1ccbfa50b3ae55ee21c909f9ad911))
* GroupInterface some logging and reorganizing ([03c1b83](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/03c1b83877b58563ec0a9c56c13fd31e4e81cdc4))
* GroupInterface Update Group Info ([3c224b9](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/3c224b92c0678ff2a091cc84fc7b48bfe565e56e))
* **MPv2:** implement session storage for leader and member ([44faf9c](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/44faf9c0f0b841aaeeac326af572c2d884a36cf4))
* **predefinedEvent:** send predefined event payload on storage scope ([e22327f](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/e22327f144c95bfb259ec00c11a5efff3be508e5))


### Bug Fixes

* execute leave party delegate have wrong success state ([6e3f657](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/6e3f6576879af008a168d41b0322113e8ad4a051))
* fix error code reporting when join session responded with error ([a44fbe2](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/a44fbe2fbedd37f1f332d6915cc0709ccbb2869f))
* prevent destroying session when a session is still in creating state ([426c51f](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/426c51f8def17505021ef8de748ef18eb82d73df))
* set async task success to false when session instance is invalid ([297b6d0](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/297b6d0fe2837544a763f5b87a6ba6287242a2ed))
* trigger session error delegates in failed scenario ([b188f32](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/b188f3279a440645bca36d81a1c8fce0eb16fa6f))

### [0.11.21](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.11.21%0D0.11.20) (2023-09-11)


### Features

* **accelByteSubsystem:** init localUserNumCached to -1 before no one login, set localUserNumCached after successfully logged in as server, reset it to -1 when user logged out ([91053ce](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/91053cea18f83122df556d48173489a4c039cecd))
* add FOnlineSessionV2AccelByte::ContainedMember to check if a player is a session member in DS ([6f7eacd](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/6f7eacdc5c49054b6a89c89991dab0a77ceedef4))
* add predefined event interface ([5f40f14](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/5f40f1432ad51db65ee370da7a792da2de046f6c))
* **MPv2:** implement session storage for leader and member ([267a6d6](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/267a6d667f3c80c78f30ddf39c010169d1707b50))
* **predefinedEvent:** send event payload for sdkInitialized, LoginSucceded, LoginFailed, AgreementNotAccepted, AgreementAccepted ([f7a4e35](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/f7a4e35a785ba4e118d48232320465144c430117))


### Bug Fixes

* move commit hash of sdk ([bc8198c](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/bc8198c9a14b9f62a54ef36af9168fbfb128ca87))
* **Statistics:** Change the flow after updating multiple stat item ([4322d73](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/4322d73a83c27a50e37bf7c93775ac14dc73a4c4))


### Refactors

* renamed FOnlineSessionV2AccelByte::ContainedMember to SessionContainsMember ([0ab7f4c](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/0ab7f4cc724a938e92babee3ec24414568cb1be2))

### [0.11.20](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.11.20%0D0.11.19) (2023-09-08)


### Bug Fixes

* deprecation warning for IWYU support in Unreal Engine 5.2 ([03800e7](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/03800e74dfb99c8c3e68c5d5b631f0d814ec2357))

### [0.11.19](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.11.19%0D0.11.18) (2023-08-28)


### Features

* add FOnlineSessionV2AccelByte::ContainedMember to check if a player is a session member in DS ([25bfbca](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/25bfbca0b9443358cd1847e443246310fc75f89d))
* **AsyncTask:** refactor achievement, agreement,auth, chat along with the interface ([1edf3b0](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/1edf3b0b724c0a545f3e7a150d90e0d7036728fb))
* GroupInterface Invite ([487356a](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/487356a6eb2537d698f503062e71c586ec8d576b))
* GroupInterface Invite ([62f711e](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/62f711eb8dd9e50582b6c2065dbe96bc0ef2ffb2))
* GroupInterface Invite ([e49cf88](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/e49cf8817c6e3ce00c5d4464a259ab6d238f522b))
* GroupInterface Promote Demote Kick Member ([c5789bc](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/c5789bcf6bd9ae7fbf385aaf1bdcb792d6e32369))
* GroupInterface Promote, Demote, Kick, Invite ([783e065](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/783e0650b2abb2de2a02abccef2b9a388c43585b))
* GroupInterface Promote, Demote, Kick, Invite ([744252c](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/744252cf16986f755aec88c1740ed08c01f7c6ef))
* GroupInterface Promote, Demote, Kick, Invite, Delete ([b82dae4](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/b82dae45688f5a62ba8c17ac057d347e7b7c8a68))
* GroupInterface Promote, Demote, Kick, Invite, Delete ([0b4a63b](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/0b4a63be3014e458052021eaf21ceb49b832dc8c))
* handle error when friend request limit reached ([6193431](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/6193431975fa4408ba8c1675c60cfa3d7761e9ed))
* **UserAsyncTask:** refactor and use Epic Task ([768797b](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/768797b7eb85a5485dee78aa2747cf47c4abf097))


### Bug Fixes

* **AsyncTaskLogin:** wrong passed variable ([ef50b1e](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/ef50b1ea55a9f320bb5a9852b17e2b98a3631f9f))
* change I64d to lli ([1a3f315](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/1a3f315c1344af291fd10650c1e2da6a56351497))
* if ams flag enabled, the server cannot register as local server. ([0099b9f](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/0099b9f46df6b6f70389b352437a9e6778edbca7))
* incorrect display name on user cache ([4c4726e](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/4c4726e27ac273277eec57727ece9c1437254c95))
* **partyInterface:** add scopelock to prevent incoming PartyLeaveNotif causes racing condition ([0930946](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/0930946f928f4e344ce6a744130a95048e8b55fc))
* **QueryUserInfo:** public code is overridden an become empty due to missing field ([2f2ae77](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/2f2ae7798d9bfdcafee9113f782eb4e6a4395bd4))
* remove 'using namespace' in public header ([c219edc](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/c219edcb936ed3e16662af66a51a7ec98edba7e0))
* **Statistics:** Change the flow after updating multiple stat item ([3387918](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/3387918aa5adca6c2379ac239b2e73bfb757b430))


### Refactors

* renamed FOnlineSessionV2AccelByte::ContainedMember to SessionContainsMember ([b1d8b23](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/b1d8b23ec256aaf4800bafb0561488fe506191ca))

### [0.11.18](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.11.18%0D0.11.17) (2023-08-25)


### Bug Fixes

* missing headers on several files using Unreal Engine 5.2 ([2f05c1d](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/2f05c1d693d7fe1f1e044eb97538d6604966bf2e))

### [0.11.17](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.11.17%0D0.11.16) (2023-08-23)


### Bug Fixes

* missing functions in Group Interface for Unreal Engine 5 ([7ad889d](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/7ad889d0b84b1a35bc95d7f3250b2a43e3a9347e))

### [0.11.16](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.11.16%0D0.11.15) (2023-08-14)


### Features

* **AuthHandler:** AccelByte AuthHandlerComponent to verify players using JWT (Access token) ([1d348a0](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/1d348a0826f632f3b9535157f83700cf1eed4d2c))
* GroupInterface LeaveGroup ([79d5766](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/79d5766e9266c13bd9658d77708c67421af09428))
* Support EOS integration as native platform in AB OSS ([afc62ba](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/afc62ba70c2f7c76a1076263e0bed182a8b643cf))


### Bug Fixes

* fix missing session attribute with key length equal to an AccelByte ID ([97937e3](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/97937e30eff307979c592c22148f9ccfb63b4033))
* fix typo include file ([fa7effb](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/fa7effbc774e13e2a3f7df090ec1f5fcb716e9f3))
* incorrect display name on user cache ([c0cc1e0](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/c0cc1e09b4b566223c9dd4cfd4acfbc432b5793a))
* **MPv2:** set LocalOwnerId for auto joined game session ([bf6d440](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/bf6d440da88d9b11e9f8991cbc89289333dad1cc))

### [0.11.15](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.11.15%0D0.11.14) (2023-08-03)


### Features

* **MPv2:** handle auto join game session ([a8bd08b](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/a8bd08b624fb77ba6564f62fe37c5452ab7ed6df))

### [0.11.14](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.11.14%0D0.11.13) (2023-07-31)


### Features

* implement online error accelbyte to identity and agreement interface ([b3286c6](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/b3286c647e6cc3841681e43d5cbfdd5051c5924e))
* introduce CreateAndDispatch generic and Epic Task manager ([43b553f](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/43b553f973d32806c35b214cbd56afb57aa0bb49))
* **MPv2:** call new accept backfill proposal from SDK and update internal game session after accepting backfill ([b0469fd](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/b0469fd97b7534f7e52d63ecd1c70fb9275b61be))
* redirect readme to docs portal ([e9bb731](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/e9bb731ecf23739c9a6b81a127d98bc78787a4d7))
* **SessionV2:** add GetMyActiveMatchTicket ([ee736d7](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/ee736d7286bc39b02200d72bbcfa7fe83e90e39a))


### Bug Fixes

* **MMv2:** fix potential hang in matchmaking process when notifications arrive out of order ([71a3023](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/71a30236461f525eee986ad1ef1c4a6f092c625f))
* **MMv2:** potential case where matchmaking start notification would come after cancel, leading to stuck state ([046a22e](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/046a22ef2454f934e85e40b82cc037b5dfc94f51))
* **MPv2:** potential crash when attempting to fill member settings ([437e698](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/437e6986504ffc4fb5fe673280208b54da1e9456))
* skip adding teams in session update request when joinability is closed ([b453291](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/b45329181940bf5922b209236545388cbef0df14))


### Refactors

* Remove invite_only joinable validation check when creating v2 party session. ([6eee4c5](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/6eee4c5eb185f8da7dc1f1d5130f2edc2ae668bf))

### [0.11.13](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.11.13%0D0.11.12) (2023-07-26)


### Bug Fixes

* fix typo include file ([7dc16b7](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/7dc16b7dd8f1f21accfba42715d31f87e49c1f76))

### [0.11.12](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.11.12%0D0.11.11) (2023-07-25)


### Features

* **AMS:** add config to enable/disable AMS ([090cc7b](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/090cc7b185b34d389c061f3533741f1a6ec9c1f6))
* **AMS:** send ready message async on registering server ([7202fb9](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/7202fb9eeca8a31cae666563b04208d94377afa4))
* **launcher:** get auth code either from command line or environment variable ([05f2e16](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/05f2e16620216a1b347ea8007c0cfe8d37d89dcb))

### [0.11.11](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.11.11%0D0.11.10) (2023-07-17)


### Features

* implement game session code management in OSS ([a925d0e](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/a925d0eaa411a25be37368a7a3c0ec7d91ea6bad))
* **MPv2:** implement per-member settings for sessions ([153d6b6](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/153d6b68accc5f3cc3603d9c0179d8441d6ebfad))
* **MPv2:** SessionV2 join error handling to EOnJoinSessionCompleteResult ([6ef8f84](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/6ef8f84865f1b287fb55d9ad83043b892924b3df))


### Bug Fixes

* make sure localUserId still valid when remove party on lobby disconnect ([fb1ce8d](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/fb1ce8d7d8dbb3f4ec9df166847b0cc6235cc22f))
* move TriggerOnLogoutCompleteDelegates after SetLoginStatus, add guard before use ApiClient on Lobby ConnectionClosed ([0f2ae8d](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/0f2ae8dfb450a5bb1d7e01de8e978eeff487bab2))
* **MPv2:** avoid unnacesarry ensure triggered when JoinSessionResult not success ([f86e5f7](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/f86e5f7a509929d5bc1e9cb841faa29397f7d692))
* static cast ErrorCode on JoinGameSessionError ([2a63265](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/2a6326539a742d54162273741142be8e0d502853))

### [0.11.10](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.11.10%0D0.11.9) (2023-07-03)


### Features

* Add GetLinkedAccountAuthToken in the Identity Interface ([e301422](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/e3014226739e05ae8eb133645220aba796d35fa1))
* Add some functionalities to set OnlineSessionSettings with array of strings and doubles ([7813570](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/781357020d61418b212b2d573a37383a0a60e5f2))
* Adding initial WIP Groups Interface that assigns basic leader on create ([95b1df7](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/95b1df71ca0a1affd3a23c6804ef1d3c9bff4dee))
* exposed linking/unlinking other platform ([1ff659a](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/1ff659a7bbeae834b0ad2976ac411a1dc86f8632))
* Group Interface Create Group Test ([f1166a1](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/f1166a1575d4b1aaa682f78d7f74b2ef13a1f1cc))
* Group Interface: Setting return status and cleaning up testing ([dfecc89](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/dfecc899f78b245bbbf53def028fba1b3107f513))
* GroupInteface error codes ([042d79a](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/042d79ac2f59570704c8f4911e116bda7796b0de))
* GroupInterface - Removing hardcoded group role id ([813b2d2](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/813b2d2c06b93eac47f05ea9715ae89b23abf105))
* GroupInterface adding change to update branch names ([c2498c1](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/c2498c13553d486beb74395950e0e846cb8c8e49))
* GroupInterface adding generic unreal CreateGroup option ([61deade](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/61deade08bfc6ad9ecabfc8dfa9337ee21a9f6da))
* GroupInterface Create and some cleanup ([055253d](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/055253d58d299f66f0624bab1928bcb354e0d2c7))
* GroupInterface Create Group ([d6a9a1b](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/d6a9a1bacbceea27e117d9675dfd4a60945d857a))
* GroupInterface Create Group ([32f1a1e](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/32f1a1efa5d4949a4eea40ff558c231980187eae))
* GroupInterface CreateGroup ([d9315ce](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/d9315ce326ac2fce9f3e639025b961c21a32a520))
* GroupInterface CreateGroup ([7b1b0a9](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/7b1b0a9551013f5da1bb967447fbaa9ef3187eb2))
* GroupInterface creating generic CreateGroup method ([840a6a2](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/840a6a2b3b662c5dd282e0d284f92ac074659c1d))
* GroupInterface fixing null check ([544b70e](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/544b70ef650437ec8f14816a1b435dfa819dcde6))
* GroupInterface removing unused variable for jenkins ([df02bba](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/df02bbabeed2a70fc0b0f62ebfdcb767d5042b8b))
* **Login:** implement steam callback on GetAuthSessionTicket ([32ab627](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/32ab6279a4406cc4f93a8c085b7bd0bfa999d05c))
* **MMv2:** add matchpool info in current matchmaking search handle ([032bc03](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/032bc03f73314ff76d3d81f41e3683328eb0d877))
* OSS listen to DSHub SESSION_MEMBER_CHANGED ([a065beb](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/a065beb8e0959b29dbb932e1b0ff95e10adcee4f))


### Bug Fixes

* Check Identity Interface validity in the GetApiClient ([e179fde](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/e179fdec034076d8d3384e95365f967b51bed559))
* move TriggerOnLogoutCompleteDelegates after SetLoginStatus, add guard before use ApiClient on Lobby ConnectionClosed ([a6db8d6](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/a6db8d6182c627ad87a549db22410011cd2ea068))
* renaming Blacklist to Denylist in Groups Interface due to changes in Unreal Engine 5 ([9e0fe96](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/9e0fe9650e30909cd99e4b008cac6269e85d6849))

### [0.11.9](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.11.9%0D0.11.8) (2023-06-30)


### Bug Fixes

* fix the ambiguity error on 5.2 ([8ebcf25](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/8ebcf25e167022f16b1bac6e8eb7489b1864399e))

### [0.11.8](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.11.8%0D0.11.7) (2023-06-23)


### Bug Fixes

* incorrect Type in FUniqueNetIdAccelByteUser ([b63a451](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/b63a451fe55004f10f25fe160789a2dc7deafd8a))

### [0.11.7](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.11.7%0D0.11.6) (2023-06-21)


### Bug Fixes

* fix compile error on unreal ver 5.2 ([79e6a2e](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/79e6a2e5b1cbb1c2c2282deec0e7de285e4d1d99))
* missing IVoiceChat method implementation in UE5 ([cf79ade](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/cf79ade6ffc5df697101fb39ebfd4636db06ae45))

### [0.11.6](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.11.6%0D0.11.5) (2023-06-20)


### Features

* **VoiceChat:** Implement IVoiceChat wrapper ([ac17c5f](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/ac17c5f4149b6cd03b0c61f7dde11fbc35a150cb))

### [0.11.5](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.11.5%0D0.11.4) (2023-06-19)


### Features

* **MPv2:** add delegate to handle stale session on refresh ([6fefe26](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/6fefe26467bd36d5805d34ef978d740424207d8e))
* **MPv2:** add promote game session leader API to session interface ([425c4b0](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/425c4b085ac96bffc48688d9a1439337059e7920))
* update OSS implementation to support Unreal Engine 5.2 ([ba9b3c0](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/ba9b3c0a94acc86d7564195f8be4881c6441117d))
* **voicechat:** implement voice chat interface ([4fd2654](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/4fd265485885c9c5f11ab5a4cb9e3237970d69df))


### Bug Fixes

* **log:** fix string format ([83e7b96](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/83e7b968f0df57de5c99276d3afeaa9951207c92))
* **Login:** apiClient became null and causing crash when the loading takes too long ([d44c604](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/d44c6044fe901c8806c55db8f8db6796f84c2555))
* **MPv2:** ignore sessions still in creating or destroying state when ticking ([41dc042](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/41dc0425e44086e688c58571f8359e9c103cee74))


### Refactors

* **AMS:** change ServerWatchdog to ServerAMS ([48b2775](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/48b277519d5271befb0a0ea455ed2bcdc9540ce5))

### [0.11.4](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.11.4%0D0.11.3) (2023-06-14)


### Bug Fixes

* compile errors on shipping build with Linux platform due to missing override in Achievements Interface ([56e3d9c](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/56e3d9c3c92adce4a5047299a797d71509f51cab))
* compile warnings due to deprecation warnings on Linux dedicated server ([40f967c](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/40f967c5c7807eba56aaeaf8b92ff23286343b2e))
* incorrect base constructor params in the AsyncTasks ([b899984](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/b899984d51e8a26939ea87f7f819f4760223b407))
* **UserCache:** get null on uniqueNetId after logging in using deviceId ([f374a88](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/f374a88a95524dabf22c2f46a6f600774b18e6ca))

### [0.11.3](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.11.3%0D0.11.2) (2023-06-05)


### Features

* **session:** add server query game and party sessions in session interface v2 ([98bb66e](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/98bb66eaaec45836b77cfd7201e7a408379a015c))


### Bug Fixes

* **MMv1:** Remove trigger OnCancelMatchmakingComplete after receive CancelMatchmakingResponse ([4292b0a](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/4292b0a07e4ce42e76b9a4a3015fa26a30536f72))

### [0.11.2](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.11.2%0D0.11.1) (2023-05-25)


### Bug Fixes

* **User:** CachedUserInfo might return nullptr ([9115f05](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/9115f055a0de714cfe1b5d9c2bceffe57a50c538))

### [0.11.1](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.11.1%0D0.11.0) (2023-05-23)


### Bug Fixes

* **MMv2:** TriggerOnMatchmakingStartedDelegates in StartV2Matchmaking async task when have no party sessionId ([2410bbb](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/2410bbbe14e3f0554ef1f4871942f4956b16487c))

## [0.11.0](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.11.0%0D0.10.6) (2023-05-22)


### ⚠ BREAKING CHANGES

* **MMv2:** start matchmaking no longer fails when already in a game session

### Bug Fixes

* **MMv2:** Remove failing start matchmaking request when already in a game session ([6484f0d](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/6484f0d34964e883a0c445940ca25f9e5c6be02f))
* **MPv2:** only trigger OnMatchmakingStarted when lobby send corresponding notification ([b717a57](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/b717a57be595f0557f09443bc622f5f5bb0edc55))
* **statistic:** fix some statistic asynctask behavior that make task can't be finished ([6ac3a39](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/6ac3a39dea5f3270637d02c3ff0d7934dcc89197))

### [0.10.6](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.10.6%0D0.10.5) (2023-05-18)


### Bug Fixes

* compile warnings on AsyncTask Statistic Interface ([d7f4b4d](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/d7f4b4d8049c0d9a16515743fb85f849e2a271f8))

### [0.10.5](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.10.5%0D0.10.4) (2023-05-11)


### Bug Fixes

* AsyncTask flags mistakenly assigned as LocalUserNum ([2856180](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/28561800c54802e95a1a7ef84535f8d79ec9c8ca))

### [0.10.4](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.10.4%0D0.10.3) (2023-05-09)


### Bug Fixes

* missing include headers when the project doesn't use shared precompiled headers ([adf825e](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/adf825e413e93494af3913fb537db84e64b1f6ec))

### [0.10.3](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.10.3%0D0.10.2) (2023-05-08)


### Bug Fixes

* **chat:** prevent crash when OnQueryChatRoomById_TriggerChatRoomMemberJoin returns failed ([7db2fb0](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/7db2fb09cd7e986855babab7fd975507fcec6cc9))
* FOnlineAsyncTaskAccelByteConnectLobby call lobby connect success handler when lobby is already connected ([9df72bf](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/9df72bf0a4c1d3dfbc9065f8a570d86360bbca93))
* **presence:** bulk presence will need to requested multiple times if userIds over the limit ([335aed1](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/335aed112849ffdf80b2aaf9d89d6ca18870a62c))

### [0.10.2](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.10.2%0D0.10.1) (2023-04-28)

### [0.10.1](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.10.1%0D0.10.0) (2023-04-26)


### Features

* **DS:** expose metric exporter to session interface ([8830406](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/88304060af00212094d7b5ed9e2aa02259f91d0b))
* **OnlineError:** create OnlineErrorAccelByte as standart error model on oss ([531f1b6](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/531f1b609a3d39a1edc577a7dfe37e616f03b1e5))
* support build without PCHs ([a9f109c](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/a9f109cdce6757400b6da16b490be235ef77794d))


### Bug Fixes

* bump sdk to fix missing header in chat models ([a1242ac](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/a1242ac2ddba41d38778c7dcec5ab2e38082a0ac))

## [0.10.0](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.10.0%0D0.9.3) (2023-04-10)


### ⚠ BREAKING CHANGES

* **User:** change UserInfo's PublicId to PublicCode.

### Features

* achievement oss ([1c9fd59](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/1c9fd59b62351f5db49d8fd62dca4d047e517d31))
* **friend:** add sync third party platform friend ([82ae7af](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/82ae7af0fbc01427e3cd9b439d741cb9b1cbc5a5))
* **friend:** invite friend using friendId ([5436843](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/5436843674b91daa12f04de3d71d4f7057da036a))
* **User:** add get and create user profile ([d0a4188](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/d0a41887288c7c70726a47ddc71a90354619b88a))


### Bug Fixes

* fix SDK cannot login from Odin launcher from UE5 project, replace the old function for getting environment variable with the new API. ([91797fb](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/91797fb708f2e2ef45e3b25ae88314be971d3d5b))
* **userProfile:** only add new auth user when it is their own account info ([9ebae54](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/9ebae54054e652b5a93951e27555f1e3aaea579d))

### [0.9.3](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.9.3%0D0.9.2) (2023-03-27)


### Bug Fixes

* **analytics:** remove login status check for SetTelemetrySendInterval and SetTelemetryImmediateEventList ([0ca75ed](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/0ca75ed23644d19b2938365f6a1931ec7028147f))
* fix SDK cannot login from Odin launcher from UE5 project, replace the old function for getting environment variable with the new API. ([1c5a2ce](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/1c5a2ce73b60e8a4f3aa0d35780b95b3de6d36b4))
* **Friends:** skip bulk presence and return true if friendlist is empty ([5f27e90](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/5f27e9043a6c219b4dddbbd0599eb60301be2da0))
* skip mpv2 server authentication when already authenticated on AutoLogin ([12c2a47](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/12c2a4756b8b71269e73b8eb9aa1911ff2ce067f))
* wrong UserId->ToString() usage to UserId->GetAccelByteId() ([c7e92ef](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/c7e92ef35716c64c1936bc73092c2eacec641d14))

### [0.9.2](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.9.2%0D0.9.1) (2023-03-20)


### ⚠ BREAKING CHANGES

* **logout:** on websocket/lobby disconnection, IdentityInterface Logout won't always be triggered. Only triggered on specific closure code

### Bug Fixes

* **logout:** prevent logout for specifc connection close status code ([b3372ee](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/b3372ee02f68c1c8bb286d16c30f8f4b10056289))

### [0.9.1](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.9.1%0D0.9.0) (2023-03-17)


### Features

* **cloudsave:** add undef ONLINE_ERROR_NAMESPACE ([4e63ebd](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/4e63ebdcf2cf0e56e7e065b1597122da3c48b75d))

## [0.9.0](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.9.0%0D0.8.16) (2023-03-13)


### ⚠ BREAKING CHANGES

* changing delegate signature will breaks client's project who is already using these feature


Approved-by: Jonathan Eka Sulistya Putra
Approved-by: Wijanarko Sukma Pamungkas
Approved-by: Teoderikus Ferdian Yanottama Yatna Putra

### Bug Fixes

* **asyncTasks:** add asyncTask to inherit TselfPtr, change CreateRaw to CreateThreadSafeSelfPtr ([8658341](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/865834163ee3cec8b86af11a162b9feb4560e3ce))


* Merged in feature/JES-479-support-add-key-in-cloud-save-interface (pull request #230) ([9946c5e](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/9946c5e90db2740ef6aa15af3bbd95585c4401cc)), closes [#230](https://accelbyte.atlassian.net/browse/230)

### [0.8.16](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.8.16%0D0.8.15) (2023-02-28)


### Features

* **DS:** expose watchdog to session interface ([d209400](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/d209400abe95c0f5285db8ae2b7123ca765200eb))
* **MMv2:** add calls to manually start and stop backfilling ([3f030c6](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/3f030c6714f7731d9be040d978157e4897fa3f8b))
* **MPv2:** add party code generation and revoking, as well as join by party code ([28188d2](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/28188d2460193ad60b6d645d62e912f23defd1ae))
* **presence:** add Away status presence ([afb9737](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/afb97370784c2277d1be28283c76ff903c946ced))
* **statistic:** add query specific stats for ds ([15c7e67](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/15c7e67efbd9390cfc6175563d1a24bd7318ae11))
* **statistic:** fix querystats function, add querystats for ds ([ac1ef9a](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/ac1ef9aa5b961bdca27489beee296a5964b5dee3))
* **statistic:** query multi users multi stats for ds ([a427753](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/a427753d4496025292c95ef2ba3b88895393f331))
* **statistic:** update emplace stats ([6e992dc](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/6e992dc70def9d04457ff71aacf6c78ae3aa0cfc))


### Bug Fixes

* **asyncTasks:** add asyncTask to inherit TselfPtr, change CreateRaw to CreateThreadSafeSelfPtr ([1a91132](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/1a91132c717fc047c77ab4c80293cd3fbe165315))
* **p2p:** update networking status usage ([64e71cf](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/64e71cfbe6d3bfc521ed35c524cffc88f0da00f5))
* set LastTaskUpdateInSeconds to current time and fix float log ([bc09b0d](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/bc09b0dc44bd7200df2ceaaf6235997686d8f299))
* **statistic:** change statuser to NetIdAccelByte, change emplace loop to append ([35754c8](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/35754c81764390ef407260ba1d063d9c3065ae2a))
* **UniqueId:** fix a bug where passing an AccelByte ID to CreateUniquePlayerId would return an invalid ID ([08e3fdb](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/08e3fdb0726b7690047470c7ec37fa041b16a727))

### [0.8.15](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.8.15%0D0.8.14) (2023-02-20)


### Bug Fixes

* **asyncTask:** change default shouldTimeOut to true ([b96aa4c](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/b96aa4c23b606f92b2413b41f47dfc27f72839ab))

### [0.8.14](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.8.14%0D0.8.13) (2023-02-16)


### Bug Fixes

* **asyncTasks:** add asyncTask to inherit TselfPtr, change CreateRaw to CreateThreadSafeSelfPtr ([2e5a6cc](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/2e5a6cc49c51315ce5f15fbfeedbc46b1ee43e31))

### [0.8.13](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.8.13%0D0.8.12) (2023-02-13)


### Features

* **MPv2:** add delegate to notify when a kick player call has completed ([165044c](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/165044c7739c7aa3b05c7326ffe0e94aa76d2de3))
* **p2p:** update hosting state in network manager when hosting p2p session ([ca0eed5](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/ca0eed58abe61b94cd0616c3fb6afecf58fa40a0))
* **statistic:** add create statitem for ds ([a7d3cf6](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/a7d3cf6623a7d0ca4f1ef8bf7ed631ca51a6a5b2))


### Bug Fixes

* **editor:** crash on exit PIE instance due to invalid reference access ([f7854f8](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/f7854f8dfff8918b0756d6dbedcb344a4a73f1b8))
* invalid subsystem instance in the static delegate ([a510b25](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/a510b250fecf25a8d2708e1b3d1fe14571434790))
* **MPv2:** invalid usages of LocalOwnerId that could have side effects on servers ([d735c89](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/d735c89390a452b3deb9f0754d8a9dc633816922))
* **MPv2:** joining a new session would not register players that were already joined to the session ([67e2c5e](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/67e2c5ec28c7b9f5ad43ae0805ac4e09505eb1a4))

### [0.8.12](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.8.12%0D0.8.11) (2023-02-10)


### Bug Fixes

* **cloudsave:** replace replace user record callback with replace game record callback ([8673968](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/8673968655df64bf113e99e76a89d8623820c301))

### [0.8.11](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.8.11%0D0.8.10) (2023-01-31)


### Bug Fixes

* missing AES_BLOCK_SIZE when compiled in UE 5.1 ([879e6e8](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/879e6e85815efe40fa2beb40720bb4527ccda862))

### [0.8.10](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.8.10%0D0.8.9) (2023-01-30)


### Features

* bump sdk and network utilities to enable turn manager send metric ([542feab](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/542feabab63e2e538ad3a0591ee750563b8cc473))
* **cloudsave:** add bulk get user record with FUniqueNetIds as param ([a5ce304](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/a5ce304924987f6cda9357e7c350bdcb79c791ae))
* **cloudsave:** add GetPublicUserRecord for other userid ([2bee89b](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/2bee89ba6126d692be475551c00c299e97323426))
* **cloudsave:** GetUserRecord for server ([c616006](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/c61600684bbddeeafa9cb90d22acbdb346e72383))
* **cloudsave:** modify to separate request between client and server ([f0d8e52](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/f0d8e524829d8f7c285e45225bca793008b55eae))
* **Login:** login using cached token (windows only) ([797325b](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/797325bb296742f55179b9aebc615c4e870d4f71))
* **sessionV1:** remove user from session in the channel ([70127d3](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/70127d350d58234b950b713d25612c75bfaedbf1))
* **sessionV2:** add get member's partyID in session info ([7ad7ea5](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/7ad7ea595d1510f312b5340930868b1c240bb4a6))


### Bug Fixes

* **MPv2:** fix issue where P2P connection was not setup on initial session join ([684f409](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/684f409ac28a247659f08af938c1b3ba18a2c3b3))
* remove "client-" prefix when validating user id ([63c5c90](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/63c5c90bc77fd90d5843c9549be0eb233ac5c310))
* **SessionV2:** remove GetSimplifiedNativePlatformName when setting party creator platform type ([3a43080](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/3a430809bfcae2636e079928f80c483cc6054fbe))
* targetted SDK submodule commit ([b2cd876](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/b2cd87653b41d13d390d7b731c2aec5c15681919))

### [0.8.9](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.8.9%0D0.8.8) (2023-01-30)


### Features

* **taskManager:** print warning when MaxParallelTasks reached ([b399135](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/b399135b051033aa388f1afcd49d020cf5e22e88))


### Reverts

* Revert "fix(user): add timeout on query user info, change to direct api request insteall of call another async task" ([f21c9c2](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/f21c9c2ab95e9e25a5b7245e860e964760560638))

### [0.8.8](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.8.8%0D0.8.7) (2023-01-26)


### Bug Fixes

* **user:** add timeout on query user info, change to direct api request insteall of call another async task ([cc40837](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/cc408376488a0cab6126ca940f453e883f3e094e))

### [0.8.7](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.8.7%0D0.8.6) (2023-01-16)


### Bug Fixes

* **sessionV1:** timeout occurred for an MM session ([c4f5900](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/c4f590053cebbd3dae826b8c05e4761571cc98e8))

### [0.8.6](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.8.6%0D0.8.5) (2023-01-13)


### Bug Fixes

* **users:** only one user returned for multiple user query ([bf2f7a2](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/bf2f7a287966b15aa71b038e273364c1f3abc0b1))

### [0.8.5](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.8.5%0D0.8.4) (2023-01-06)


### Bug Fixes

* **statistic:** unable to find AsyncTask in non shipping build ([01c9f8f](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/01c9f8f418e9b5956a97204f3e0163186454410d))

### [0.8.4](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.8.4%0D0.8.3) (2023-01-06)


### Bug Fixes

* **statistic:** compile error in shipping build because ResetStats async task is included ([2737569](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/2737569c484e0f784f4c77e5f061d061697cb728))

### [0.8.3](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.8.3%0D0.8.2) (2023-01-03)


### Features

* **MPv2:** implement separate delegate for when an update to a session is received ([4e2cf80](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/4e2cf8063951bfacefed8a3a79790d43415f9fda))


### Bug Fixes

* **MPv2:** update create session and send game session invite calls to allow servers to make them ([3a807f2](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/3a807f2b1a96ae3d6a92b6955b7a679d382cdc9a))
* **MPv2:** update create session and send game session invite calls to allow servers to make them ([2898913](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/2898913084e1cbca5bbb159e5d7c8240db28a890))
* **party:** change how initialize JoinInfo ([fe4f61f](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/fe4f61f43bc82018ecd425496f1a541931340709))
* revert some conflicted changes ([7388a7c](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/7388a7ce375b0d0ba7350d4463a344c93b425e82))
* **uniqueid:** update types for FUniqueIdAccelByteResource and FUniqueIdAccelByteUser ([dfe8bd6](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/dfe8bd6fbb8a2a95d5e176ed34509eeda32e2b38))

### [0.8.2](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.8.2%0D0.8.1) (2022-12-19)


### Features

* add chat interface and connect chat on login ([7300b5a](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/7300b5a0b8670ee4cd39ae79a793a2398ad8de05))
* add GetFromSubsystem and GetFromWorld in FOnlineChatAccelByte ([aae33b3](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/aae33b34a8fdc8479fcb10d060944ead96082a59))
* add TextChat option on session ([81a990f](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/81a990f1972a02ea5271c17537d851871a93ad69))
* Bulk stats update on some users ([031beae](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/031beae38a1fef8cdfea743ba3d89a6cd5e8e15d))
* chat interface functions and events ([0fe55de](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/0fe55dee8172efb9a060a280aba907d1c0b5b9b2))
* **EOS:** Support EOS login (launcher and accountPortal) ([a5c6583](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/a5c6583a46fe41726f8e91cd67f418749e783631))
* **friendPresence:** Auto update friend presence. ([d736994](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/d7369948e1ca787a4e926c338d993eb74f25ca8f))
* Get cached stats from a specified stats name and a specified user ([3c9f6c4](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/3c9f6c4f11f598ca96f08f424676d1f36b448bf3))
* load last message for private message ([9ae37a1](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/9ae37a1ac3cb40a8cb11421010f717cb93899844))
* **matchmaking:** set SETTING_GAMEMODE after getting match notif (this only works after AR-3776 done) ([60997b7](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/60997b7f9d64726077c0bb3d4f41b6309d873218))
* **MMv2:** implement setting to override session ID passed to created matchmaking ticket ([c861fca](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/c861fca1b7d223b13666ff1cb8b1e6c06b89a5bb))
* Reset stats from a specified user ([fa2d726](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/fa2d72618c85d077f5e6a9e1907f4496fe2cadf2))
* **sdk:** move to latest sdk ([31ec23d](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/31ec23d2f0d1e51cc8239c41fe28264dcc1065a5))
* **user:** implement avatar URL to user accounts ([155e0d2](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/155e0d21793db50dc0b975574dd845b9e0a20708))
* **V1SessionInfo:** add delegate call when retrieve session info ([9eec43f](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/9eec43f786efc08291b5f20c31777b0a4f9bbaf0))


### Bug Fixes

* add another case for Busy and Invisible. ([44f6f7b](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/44f6f7bf340716631436c478dcc07be57408a424))
* add missing code ([d756257](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/d7562576f3932a2e4b284edf34402f3b7725ddf4))
* add query room data after user successfully join a chat room ([48ff9ba](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/48ff9bae8e37bca3e9e81c2a4f716afa4e024914))
* chat interface include ([15bfea1](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/15bfea18cc52f9c4b361a8681db12342f2047a37))
* **chat:** fix fstring initialization compile failure with ue5 ([95fadcc](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/95fadcc14362eaf5fca326e58932e043b1188793))
* deprecation warnings in FUniqueNetId during compilation ([ed7203d](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/ed7203d78af926ede59492a20a69f6c7a663741a))
* implement GetChatInterface override method in OnlineSubsystemAccelByte ([6d84315](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/6d84315fab55f6ce80d7d244b6a424624cae912c))
* **MMv2:** fix refresh session on game server failing to update ([aabf5bb](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/aabf5bbbdcae9d639c86c8811dc90bb623d90838))
* **MPv2:** fix join session using potentially stale data for created session ([f6d0d5b](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/f6d0d5bdf9199e8cfca8dc013be824ce3cf4bf65))
* **party:** Presence party member ([d124928](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/d1249289cee1980bf5a4ee8d4701f854f53750aa))
* register player V1 doesn't use any checking for dedicated server ([6ac5c76](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/6ac5c76eac6de8a69621c610b53293114f7a5a90))
* **restoreParty:** fix restore party if failed to get member connect status ([0f602b7](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/0f602b75fc3a7b75601fcebc15b13001b0012d2e))
* room cache race condition ([23fb5db](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/23fb5db769edc2c01ed02fb3f878e85816a09a24))
* wrong presence state when querying user presence ([dbeebad](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/dbeebad1d1f798a44a5fc57fa94157b04e73781d))


### Documentations

* implement script for automated build ([a694fff](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/a694fff576918f0c5302f923ee00734c5e8e6365))


### Refactors

* Add UniqueNetId param in FOnlineChatAccelByte::RegisterChatDelegates and change accessablility it to PACKAGE_SCOPE ([437a4e7](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/437a4e77480efec9b4fc4b35e261eac099c9f7d1))
* move chat connect to chat interface ([38b193a](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/38b193ae7c921907c4dcc79dde3025d1488ebb34))
* rename ConnectAccelByteChat to Connect ([5686906](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/5686906beee11673d48f029e6e2b23b1b3d26d5a))

### [0.8.1](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.8.1%0D0.8.0) (2022-12-09)


### Bug Fixes

* **MPv2:** update create session and send game session invite calls to allow servers to make them ([29b8408](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/29b84085923b4bfc2fa3631d52403a06ec6f4be6))

## [0.8.0](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.8.0%0D0.7.3) (2022-12-05)


### ⚠ BREAKING CHANGES

* some methods are removed from the OSS Interfaces in Unreal Engine 5.1

### Features

* Bulk query specified stats from specified users ([5eeffe2](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/5eeffe239f618a677653d428fc267dd9c1cfc984))
* Get cached stats from a specified user ([bce2fca](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/bce2fca364acc3f171bdcb791c67b7aba4caf035))


### Bug Fixes

* fix analytics renaming post merge ([0282205](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/02822056d4a19d2fa307723a0b0baa8ce4d65ead))


### Refactors

* change naming convention in analytics interface ([0d6f3ea](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/0d6f3ea5f5e7d913671a429566974df182aa640c))


* ﻿feat: update implementation to support Unreal Engine 5.1 ([b970957](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/b970957bfe2fb3647bda2342704f34d72c8779d2))

### [0.7.3](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.7.3%0D0.7.2) (2022-12-01)


### Features

* add chat interface and connect chat on login ([ac170dc](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/ac170dc7ec1831869ff1c18039c59b554537258a))
* add GetFromSubsystem and GetFromWorld in FOnlineChatAccelByte ([18659a7](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/18659a7f4545db9fe2b71563bd0237ff86811e90))
* add TextChat option on session ([1362a54](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/1362a548f7d26323c573cfc903bed4bb9510af80))
* chat interface functions and events ([1c337dc](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/1c337dc97183eb4afe018e8d4220914e5af4710b))
* load last message for private message ([8699e1e](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/8699e1ee78719ddb8672be859946a3d652750c3f))


### Bug Fixes

* add query room data after user successfully join a chat room ([7304822](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/730482274a18f561328fc04939ac7e975d3cad73))
* chat interface include ([01acb89](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/01acb89acedc4766bbad6baa9e89263c3454d7be))
* **chat:** fix fstring initialization compile failure with ue5 ([1e2e888](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/1e2e88897c3d81e008b50eeaa9b060adbc169d2d))
* implement GetChatInterface override method in OnlineSubsystemAccelByte ([4073e57](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/4073e572c430f54291bf3b23be8352167b18b6c7))
* room cache race condition ([b6b4017](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/b6b4017c3f094c4003f338bb58aaf91a951ef398))


### Refactors

* Add UniqueNetId param in FOnlineChatAccelByte::RegisterChatDelegates and change accessablility it to PACKAGE_SCOPE ([a03c038](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/a03c0381ff08ccc21a4656e3989b21ec1bda9906))
* move chat connect to chat interface ([b4bca6e](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/b4bca6e19ff3947b285f72efed20f7ed5ed945b0))
* rename ConnectAccelByteChat to Connect ([8f8c3b5](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/8f8c3b5bec52698077072d734cd8949f878d7435))

### [0.7.2](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.7.2%0D0.7.1) (2022-11-23)

### [0.7.1](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.7.1%0D0.7.0) (2022-11-21)


### Features

* add analytics interface ([a01641d](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/a01641d3d627337d3c5fef1dd713e7319cdfd333))
* implement send telemetry in analytics interface ([ca8cb4b](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/ca8cb4b3a0b736a2c1296f5ea61ede0aa6ff2bbc))
* Query all stats from a specified user & fix Bulk query specified stats from specified users ([4ca7808](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/4ca78084d89ee94002984555e4e7146be8053f11))
* Query specified stats from a specified user ([f699068](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/f6990680237bd3107cc525e7087676ad5d685600))
* **time:** add time interface ([f9f781d](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/f9f781dd4e44fa08f0a231bc6662993ca3f39e95))


### Bug Fixes

* **login:** add condition to login with ps4 ([48d2eed](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/48d2eedd1332d047a5bf3a25224e3d94952c9e49))
* update both session min and max players if either has a new value ([fe67736](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/fe6773659e5254dc6f6a43a905b5384672db4313))

## [0.7.0](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.7.0%0D0.6.6) (2022-11-07)


### ⚠ BREAKING CHANGES

* use session type in SDK, change EOnlineSessionTypeAccelByte to EAccelByteV2SessionType

### Features

* add cloud save interface ([760649d](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/760649dbe5065d0114e8e0c1dbb050d7d9c4aedc))
* add match pool field to game session create and update ([a7058de](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/a7058dea500d865017ee641dfa4db9319a0f1153))
* **sessionV2:** add delegate to notify if a session failed to get a DS ([e0a38b8](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/e0a38b8856a19feb3d3530c8e25b93eb760151f4))
* **sessionV2:** trigger OnSessionServerUpdate Delegate when get join session response with a DS ready. ([32da370](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/32da370dc0b276a27e4aec8a84b62b9a1c9af891))


### Bug Fixes

* can not set max player ([d54917a](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/d54917a91d9f702c4b07b7af3316e5744895fb6a))
* crash when static casting session model ([a5fe09a](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/a5fe09a2ff4e8aa4c30afed94b7799102a7f247b))
* double dereference of a string in Logging method ([93b8427](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/93b842784f0dbf416cb7aeb3f73f574a2fb00045))
* double include headers in StoreV2 interface ([47ec4e5](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/47ec4e508e688e5e725ce9893b79a8f042b6b44d))
* GetPlayerNickname crashes when failing to find a player ([c1c9608](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/c1c96083c61d95f501777388b31e0a369618db32))
* handle trigger session join complete correctly ([ad0f307](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/ad0f307a652c7eadae4a4ae13f7c2be54af66c95))
* remove encriptionkey from oss ([9d8b0e0](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/9d8b0e00c5e9181c6c2b15d02929e1739feb8e6d))


### Refactors

* **SessionV2:** Rework V2 session updates ([aec1298](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/aec1298b4e525b142aaa31ed646a2ec7e4501b7d)), closes [#123](https://accelbyte.atlassian.net/browse/123)

### [0.6.6](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.6.6%0D0.6.5) (2022-10-24)


### Features

* allow to override serverIP by using -serverip=xxx.xx.xx.xx commandline argument ([a998102](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/a9981025eaceb0abfc35901f92d78d175dab553f))
* **party:** add join to party using party code ([5243d27](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/5243d273abe1c34a70c060ec763b71f2d45dcf76))
* **Presence:** presence listening to lobby notif ([009653f](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/009653fe46260c6d91c14c82b898766b24c27b67))


### Bug Fixes

* add some missing headers ([8168adc](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/8168adc46021b67148a8a531a41ed349e3cd69b2))
* **log:** change '%s' to '%d' ([56ae459](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/56ae4592928536e7991a777a44276c99b1ab05b1))

### [0.6.5](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.6.5%0D0.6.4) (2022-10-10)


### Features

* **sdk:** update compatible sdk to ver 16.2.1 ([cdc5964](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/cdc59645e18e247b3e7933367cae99974036b3ce))
* set outgoing HTTP header to indicated the OSS version ([5ea4d2c](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/5ea4d2cea12327d4c4d48274610107f055495157))


### Bug Fixes

* fix v2 session deployment setting being sent to backend ([351bc81](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/351bc81429df00d3c01eb97d705bb9d6ea803d0c))
* **log:** change '%s' to '%d' ([0da7747](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/0da7747df66e4f7681abf941406861c7b71b7630))

### [0.6.4](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.6.4%0D0.6.3) (2022-09-27)


### Features

* add update session and OnRTCClosed delegate bringback from eville ([723dc18](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/723dc18e96d0ed422f08b04d5e7af517f9e2d8fc))
* **friends:** trigger some delegates on invite friend feature ([3b95c95](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/3b95c954814b72c5cd711449360086921700246d))
* **store:** add query child categories ([9b814ae](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/9b814aedd323c2c4171d91a4274ac1bbb3ba9887))
* **store:** add query item by sku ([18e7b22](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/18e7b22dc50c513da0861863c86f1d3ae90fe4fb))
* **store:** add query item's dynamic data ([2f512b8](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/2f512b8ab206239b1b9e2e8e8c618dc7ccf73b70))


### Bug Fixes

* apiclient always null ([e3fd85f](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/e3fd85fe7cf027378628d6dbc9637fac56a09815))
* fix UpdateGameSession and UpdateGameSessionSetting async to target session interface v1 ([f8c8772](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/f8c8772fd82abaffb92527791b8f642410d19518))


### Refactors

* move and rename UpdateGameSession & UpdateGameSettings async task ([f800bba](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/f800bba3260107de090c6c19bbd68b099a97f52f))

### [0.6.3](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.6.3%0D0.6.2) (2022-09-19)


### Bug Fixes

* implicit conversion in CreateUniquePlayerId causes an issue ([b613e9a](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/b613e9a38cc338ffbdf771a694514e451a4e12a0))

### [0.6.2](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.6.2%0D0.6.1) (2022-09-15)

### [0.6.1](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.6.1%0D0.6.0) (2022-09-12)


### Features

* Implements Multiplayer v2's Session ([14464c7](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/14464c74248f857ea358e4796112b97f7584708c)), closes [#94](https://accelbyte.atlassian.net/browse/94)

## 0.6.0 (2022-08-30)

### Features

* **store v2:** implement Online Store V2 Interfaces which handle store categories and items that can be purchased using in-game currency
* **entitlements:** implement Online Entitlements Interfaces which handle entitlement and IAP purchases
* **purchase:** implement Online Purchase Interfaces which handle purchase items using in-game currency

### Bug fixes

* **ue5:** remove deprecation warnings with FUniqueNetId derived class

## 0.5.0 (2022-08-22)

### Bug fixes

* **session:** joinable session not working due to enqueue session is not implemented
* **identity:** lobby websocket not disconnected during logout

## 0.4.1 (2022-08-05)

### Bug fixes

* **oss:** fix racing condition when the same user is logging in and out consecutively in short period of time

## 0.4.0 (2022-08-02)

### ⚠ BREAKING CHANGES

* **oss:** remove singleton instance for several Online Interfaces to run in PIE mode
* **oss:** move SetLocalUserNumCached, GetLocalUserNumCached and GetApiClient from Online Identity interface to the Subsystem class 

### Features

* **identity:** add new config bMultipleLocalUsersEnabled to enable or disable multiple local users support

## 0.3.1 (2022-07-21)

### Bug fixes

* **identity:** fix login with refresh token type

## 0.3.0 (2022-07-18)

### Features

* **wallet:** implement Online Wallet Interfaces which handle AccelByte Wallet services
* **oss:** now supports UE5
* **session:** trigger delegates on failed matchmaking

### Bug fixes

* **session:** fix End Session to support matchmaking and p2p game session
* **session:** fix Register and Unregister player to session
* **session:** fix joinable game session
* **party:** fix Online Party Interfaces implementation

## 0.2.2 (2022-07-12)

### Bug fixes

* **linux build:** fix linux build, move static struct outside the online agreement class

## 0.2.1 (2022-07-05)

### Bug fixes

* **lobby:** fix trigger on connect lobby always return false

## 0.2.0 (2022-07-04)

### Features

* **agreement:** implement Online Agreement Interfaces which handle AccelByte Agreement services

### Bug fixes

* **presence:** player unable to set presence status

## 0.1.0 (2022-06-06)

### Features

* **identity:** implement Online Identity Interfaces such as login, logout and player profiles
* **party:** implement Online Party Interfaces such as create party, kick member, etc. 
* **friends:** implement Online Friend Interfaces such as add and remove friends
* **presence:** implement Online Presence Interfaces such as set presence status
* **session:** implement Onlinbe Session Interfaces such as start matchmaking, create session, find sessions, join session, etc. 
