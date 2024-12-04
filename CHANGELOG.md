# Changelog

All notable changes to this project will be documented in this file. See [standard-version](https://github.com/conventional-changelog/standard-version) for commit guidelines.

### [0.12.28](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.12.28%0D0.12.27) (2024-12-04)


### Features

* add api to sync platform user friends using platform handler ([fb6b11d](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/fb6b11de49b0bb30ddc3ce661230bf01c971d31a))
* **chat:** Expose chat and topic ids to FAccelByteChatMessage. Add ReportChatMessage method to FOnlineChatAccelByte interface. ([da5c3a0](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/da5c3a0b43b3ecdf5183a9bb1a813520f959a9dd))
* **CloudSave:** add adminGameRecord endpoints ([de06c32](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/de06c3222cd767a12db5bdfac2c79108cbe66dd8))
* **cloudsave:** add bulk get and replace user record for server ([bb31d14](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/bb31d1414cfa277f25ddad1dd7a0975d8bda2e2d))
* **EntitlementInterface:** add support for metaQuest IAP ([66cdd8a](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/66cdd8a048d73c0099ca0c08809bfef7580f50d3))
* **friends:** handle error case when requester is blocked by requestee ([04266be](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/04266be37cc640243fb711659102f59ff76beb16))
* **identity:** add oculus login support using platform handler ([190f127](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/190f12778d077f3fd0e80ef8606a2698e8cf7871))
* implement delete record ttl config on cloud save interface ([b69b1b4](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/b69b1b4864971f63a15dd1b4ea982bbebd824d25))
* implement delete record ttl config on cloud save interface ([8a92cd8](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/8a92cd8eeb1552fe483f4a5f90f5bcd3c24f3f3e))
* **login queue:** add manual claim login queue ([a55db29](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/a55db292990cc3f721f2b324267f1d76e13e39f3))
* server partial backfill acceptance adjustment ([c074cf8](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/c074cf8123f31a8b662e81aa2e0b78fc4fa01b65))
* **server:** allow server to accept backfill proposal partially ([326fe42](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/326fe4239efe5d0eda485986224de0b078e60c05))
* **session:** sync and exclude player's past session from matchmaking backfill ([ca9c606](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/ca9c6066227d157fd4f93dd9f95744cd152bf4ef))


### Bug Fixes

* compile error 4.27 ([a81ed86](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/a81ed86e0eb55b99389d2b28ec4d490703fcf1e1))
* reset IdentityInterface last on subsystem shutdown ([37d461b](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/37d461be18c65a7c7dae640a2e163903a502db67))
* SessionInterface holds ptr to IdentityInterface so session will be destroyed first ([25df245](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/25df245ccf808e8101a4ff19e47ae671e8a031e7))
* **session:** remove duplicate trigger delegate in kick member v2 for leader ([a34b839](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/a34b839dc215466ea0ad154231191de7a21f6039))
* unbind lobby delegate in OnlineSubsystemAccelByte ([c69be28](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/c69be28dd31932154f35f369fcb3bc52d22b36b2))
* **User:** PostLoginBulkGetUserProfileCompleted clear wrong delegate ([8c6fb19](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/8c6fb19db659afa5180d89c2b00111b4a030564d))
* **user:** update user info without override previous cache. ([640b08d](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/640b08dd55fd5f9ff227de4c039667fb72c68c8f))

### [0.12.27](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.12.27%0D0.12.26) (2024-11-07)

### [0.12.26](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.12.26%0D0.12.25) (2024-10-30)


### Features

* **IAP sync:** improve the SyncPlatformPurchase to sync mobile subscription & add function to query mobile subcription ([1d81a68](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/1d81a68cb665c202e23734968d5652db621875fc))
* **iapsync:** modify to able to accept subscription sync ([58ba62f](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/58ba62fa1331f7c38b14234ae89dc986e5a2ca96))
* **qos:** add flag to not send metric automatically ([741f3db](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/741f3db03b25c7483dcaf9e00d7aff66937f54bc))
* **sessionv2:** add OnPartySessionCreated online delegate ([76411df](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/76411df9642151f716f9e1ad83d4fa2816fb4bfd))
* **websocket:** add ability to configure the reconnection for Lobby, Chat, AMS, and DSHub service ([2893026](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/2893026ec67da59fa577729c7f8b9fc4fb9f2ff9))


### Bug Fixes

* **Analytics:** change AddRaw to AddThreadSafeSP, pass SharedPtr instead of raw object pointer ([154e8da](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/154e8da275317788078115d59312142edd0de08d))
* fix incorrect bIsOnline presence ([2d3a1b5](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/2d3a1b58b318fc3acb9b5ca5851ea8638abe69b0))
* fix localusernum not cached when using simultaneous login ([acb27f4](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/acb27f4be2b2aa15601142cce72c5038f1359197))
* fix user platform overwritten from presence status ([7eea7fe](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/7eea7fe7ddf963acb02713ed67b85fd7a64748a5))
* **querysubscription:** add online player delegate for query subscription ([3e2df7f](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/3e2df7fb0ba7a92fec3fa571037dae8504e6e28f))
* remove ensure checker on places that don't need it ([62b86bd](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/62b86bd0dd6c0891578355675284f57e4c934100))
* reset and shutdown voice interface if initialized ([eb3467c](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/eb3467c7bfc34942c756f85be463ed9664062056))
* update local session storage only when receiving OnStorageChanged notification and not other notifications ([15f5ce8](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/15f5ce8bc24bec861d7c098977a7585d15fc74bb))

### [0.12.25](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.12.25%0D0.12.24) (2024-09-26)


### Features

* add lock key in AsyncTask ([4bc0547](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/4bc0547290df8712d993a4ff71998a3b2faf98da))
* implement lock key in lobby notification processing ([f875075](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/f875075e4b72f3b782a615037e31be01072d9eaa))
* read matchmaking feature flag in cancel to expect notification ([1cea9cc](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/1cea9cc8f73309a0eefdc73cfd0d980de86a1f51))
* trigger cancel notification from match ticket details polling ([718ee7d](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/718ee7dbcce6e68f340f78b18bbd0d850fcd42a0))


### Bug Fixes

* cleanup on pre exit ([66c4787](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/66c4787b4657fc0cbbda9a3925862056ef28de0d))
* fix error code in check matchmaking ticket details for ticket not found ([ab158ec](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/ab158ecd5d9b29204145b4e2b920f55127d2bdd6))
* matchmaking ticket keep polling if expecting notification after cancel success. ([43d0ac4](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/43d0ac4b4c76a1dab199ce79b4a7f64f984b285e))
* stop session invite polling when join session success ([8856a10](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/8856a103c46b8397221f8cd6f9e172fb4436a45f))
* trigger OnMatchmakingCanceled and OnMatchmakingComplete when receiving matchmaking cancel response and not expecting cancel notification ([5420c11](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/5420c11d40ee1a36231590cd77d48b4fa219e211))

### [0.12.24](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.12.24%0D0.12.23) (2024-09-19)


### Features

* **asyncTask:** apply the pinned subsystem to entire async task ([148802f](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/148802f7686a901afec3688a6af61c0859a9faf3))
* **asyncTask:** introduce a macro to pin the subsystem & check the validity before usage ([3be9950](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/3be9950eba25deae6f0750919b5df3fa11c48b5d))
* **entitlement:** add SyncPlatformPurchase for googleplay ([920f468](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/920f468e249cfd3b0ed1b6c6de12b8b24fc5ed2e))
* **identity:** add method for updatepassword, enable MFA and disable MFA ([2371a75](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/2371a75848ed89769ca1540709dc7d438be079bd))
* implement server auto shutdown after receive AMS drain signal ([4c68981](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/4c689814359e2d7daae2336befa3aca237bd7294))
* **Login:** add additional optional parameter to allow serviceLabel for authentication ([114b3ce](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/114b3ceaee6349454f51e8830318344726d25abe))
* **session history:** add a new endpoint to get game session history ([5079669](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/5079669adedfce2a2e3bbf7980a142eb101bc531))
* **session:** handle auto join session logic in OnMatchFound internal notification handler ([359e025](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/359e025b341da2b71ed55bed61255cd4779cce6a))
* **session:** implement internal bypass of session join API call if already marked as joined ([6e4ce93](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/6e4ce93c5fc98bef3105321324e8e75db5ee346b))
* **session:** implement OnGameSessionJoined handler for auto join game sessions ([ee7f009](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/ee7f009d7a05bcb58e4a4d94b6c7542a81db3278))
* **sessionv2:** add API to kick player from a game session ([46b2b39](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/46b2b39b8089dba414b9b8099b5ad33b071a47e9))
* **UserAccountCredential:** improve the access token storage from FUserOnlineAccountAccelByte by using a the credentials reference as the source ([b622fbd](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/b622fbd91c0df47332bad3ba0458728d1550ac7a))


### Bug Fixes

* **AMS:** Remove disconnect from AMS and DSHub on receive AMS drain signal ([c88c20a](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/c88c20aba11ff1fea68c58bf87f884bcdf629975))
* change CredentialsRef using FBaseCredentialsWPtr instead ([3840eb4](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/3840eb4e8f5bc66cb716ea30364bc29568157674))
* check TicketID is the same when receive matchmaking canceled notification ([fbddc53](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/fbddc53f12df86964d6d792463209264353a6b1a))
* **PartySession:** remove reset handle after leave party complete ([d44d464](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/d44d46403b4e751020acbad9dafe40500ca97588))
* **ReadFriendList:** prevent async task timeout if it queries multiple page ([c8604a2](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/c8604a2a5e093b09eb74fbd0aa5c75b50b9dc701))
* ReadLeaderboardsAroundRank properly return item amount when pivot rank is below range ([7d45543](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/7d45543783d508d9b3cc980dd50182420fbe53d4))
* refresh sessions after reconnected. ([16d607a](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/16d607a146ba027ca8264e7c7f4d281da2e9b760))
* **session:** change status usage to statusv2 ([1c87f13](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/1c87f13624f8790b585cecbacc819010d84fe8e6))


### Reverts

* shutdown DS ticker on AMS drain ([6cacdb7](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/6cacdb7cc2333bc8d2d20c46ab3e91a864d9afff))

### [0.12.23](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.12.23%0D0.12.22) (2024-08-21)


### Bug Fixes

* check TicketID is the same when receive matchmaking canceled notification ([181ee23](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/181ee2321c3cfe20fa201fb8f2cb59e26677c1d5))
* crash when trying to fetch NativeSubsystem during OnLoginSuccess ([131960c](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/131960caa2d131cb465db604a206524bc5415ac5))
* crashes when Logging out during shutdown due to accessing object that has been destroyed ([5f02515](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/5f02515fee71bdf5b75f0a8971bc00293a957ab3))
* fix race condition causing OnMatchmakingComplete triggered 2x ([b3b217b](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/b3b217b7e0628b9417b445648dce9f13a5905321))
* fix race condition causing session invite triggers 2x ([2ab3f0e](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/2ab3f0e4ab5935ae7b5be4712298127c6d11e730))
* login queue ticket updated triggered with wrong success state ([2ae3ab6](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/2ae3ab6c1f914a05639f4ef1049adb0f574132c0))
* **user:** add guard for invalid interface and local user num before dispatching query user task ([1177e26](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/1177e26bc56ce850c52a29eab674522210b1bf17))
* **UserCache:** check platform user info if accelbyte id is match from requested user ([920023b](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/920023bffd79c7ebb3f81eeb73ca4369f72ca60a))

### [0.12.22](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.12.22%0D0.12.21) (2024-08-12)


### Features

* **chatv2:** add API to get and set user chat configuration ([4d1e99c](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/4d1e99c367499037cfcbb1f826a1c4d854fc4ecb))
* expose SKU and backend entitlement data in FOnlineEntitlementAccelByte ([7b2c474](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/7b2c47477ace2e5ecf2171d457e0609c12d64fdb))
* **friends:** add paged readFriendsList ([4224f6b](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/4224f6bbaddec2416eab3acf0b72404b57338555))
* **Login:** add additional optional parameter to allow serviceLabel for authentication ([37a2824](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/37a28248d2dc0380044c2216b509ccb5524d998e))
* **session:** add set server timeout and reset server timeout ([3267a86](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/3267a86cf8bbb606bbfcbc57b23b44543bc5033b))
* **user:** use v2 endpoints for query users and add basic query ([d7eb8b8](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/d7eb8b885342d3e6890078028f08e188243d78fc))


### Bug Fixes

* add platform info in presence status properties in ReadFriendList and Presence received ([872038c](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/872038c43b85c0619c0d640c3546a9dd879c8f75))
* **identity:** add error details in case of login task timeout ([ce39874](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/ce398743e2e327fec9334bdef20705f351720bb7))
* **purchase:** change order price and discountedPrice to match item quantity ([0cc19bb](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/0cc19bb0bfdeaaf3dfc6b114db9f1d46357bf363))


### Refactors

* implement BulkGetUserByOtherPlatformUserIdsV4 when query user by platform user ID ([710a84c](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/710a84c269efd3f683bc64c14b5d99ab6425db18))

### [0.12.21](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.12.21%0D0.12.20) (2024-07-18)


### Bug Fixes

* crashes due to IdentityInterface is null ([fc142b2](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/fc142b2ca162cfa8d628734ce0e8c776566c51d7))
* miss precision calculation in the Expired SessionInvite time ([7cd1b1b](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/7cd1b1b9ff67388a3c7faecdd83c2092fc3d6dfc))

### [0.12.20](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.12.20%0D0.12.19) (2024-07-10)


### Features

* **sessionv2:** trigger FOnSessionParticipantJoined and FOnSessionParticipantLeft starting from UE 5.2 ([a1a42b4](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/a1a42b49e658a2b4dbe63eeb19fe0ade04838824))

### [0.12.19](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.12.19%0D0.12.18) (2024-07-05)


### Features

* add new delegate OnMatchmakingCanceledReason ([75d553b](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/75d553b26ba7cd09815c58b6b876a3c35c768e73))
* add session name when starting matchmaking and get session name in storage when auto join session and matchmaking started ([dfcfff0](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/dfcfff0d9ff774b918c5d6a9952ef9feeed7aed5))
* add storage parameter in start matchmaking ([8fe28a6](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/8fe28a6d2b555ceaa441b408dc3e3a8d83c47ecc))
* insert session name to leader storage when creating game session ([feff30b](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/feff30bc8e75724cf2833cbba0af5ea69a41e8c6))
* **login:** add support to login with Apple on iOS ([6d0feaa](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/6d0feaa1134e65f0fd53d2a77887bc9faef9e64c))
* **mpv2:** add session invite cancelation ([385bb90](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/385bb900c491956f1a2d33d3937c0664c6019d3c))
* **turn:** get turn server latencies when matchmaking with P2P server ([46af355](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/46af35591d17e02aa8b9f5bbb41b5168c46b4b57))


### Bug Fixes

* add session info valid check before stopping invite check poll ([c11ef7f](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/c11ef7f2087e1cf2f0fec7ee6646c5a47be2482e))
* add session storage when constructing from backend model ([87288b4](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/87288b47fb467f62e36fd901ceffdaad01b1cc79))
* **asyncTask:** Apple LoginType switch case AsyncTaskLogin ([1718f24](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/1718f240bdee9e8ed5d7dee1553469449e2217bb))
* **chat:** handle chat connection closed in chat interface instead of async task ([b5e73b1](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/b5e73b1d49ef1ee5cd6f8cfdc8dd620cff512b3e))
* **ecommerce:** complete query store front async task as success if item mapping not found ([f71d1d9](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/f71d1d93c849a0969c539ca8dc608e0ea2b5d506))
* **login:** add a checker for the steam module existency. ([ddbcae9](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/ddbcae9e78b51d0d30ff4de24c4294e460061611))
* **session:** fix refresh session not triggering OnSessionUpdateReceived after successful refresh ([4ed224f](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/4ed224f98a160a899a984dc70f91fb9a4c01bbc3))

### [0.12.18](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.12.18%0D0.12.17) (2024-06-30)


### Features

* **login:** add support to login with Apple on iOS ([bf95fb0](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/bf95fb079fce2676ee5de5b8bf8c65226d92a75b))


### Bug Fixes

* **session:** fix refresh session not triggering OnSessionUpdateReceived after successful refresh ([78c2a05](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/78c2a05b811567ef344456c73f31ff9928b57f88))

### [0.12.17](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.12.17%0D0.12.16) (2024-06-17)


### Features

* **OnlineFriend:** add UserInfo to OnlineFriend to able to get info like friend's platform's displayName ([82708d8](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/82708d8ebd5a717c910e21f0360e00556fd91488))
* **usercache:** change how to update userInfo, copy the new value to existing data instead of overwrite it ([e2cda5f](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/e2cda5ffd913c2948b677c10421e6b4f36f24855))
* **usercache:** implement concept of 'stale' cached user data to allow for refresh when older than configured time ([e2089e5](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/e2089e578f20ac3ac5c48b88b7a3027bf9ebf18e))


### Bug Fixes

* incorrect display name assigned for other users data ([4afb6af](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/4afb6af87760135a2f0cd0c7ac8e0b9d3346895c))
* incorrect unique display name assigned to the user info data ([bc8c593](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/bc8c59308434d7c0dbd2f065eecac9b23545f8a5))
* missing ESPMode::ThreadSafe for TSharedRef<FAccelByteUserInfo> ([4a227f0](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/4a227f09d7521a3be3d13ca0a52cd82d51461f11))
* **SessionInvite:** improve the expiration calculation for the timeout invitation ([519a528](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/519a52803b3d59919c193b4bd5c5d1aa43387fa3))

### [0.12.16](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.12.16%0D0.12.15) (2024-06-05)


### Features

* **logout:** clean up local cached session data when user logs out ([473b44e](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/473b44e425b341a5817133a4f4b49ad0c47d3971))
* **NewProfanityFilter:** add new endpoint for validate user input ([a89358e](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/a89358ef7d28b46f3bc5b66a9833d02a2abf7e32))
* **user:** use logout instead of revoke ([fff67fe](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/fff67fe0e85a9917f4e6300b533f7de513e769d7))


### Bug Fixes

* notifications were triggered by the recovery system even when the user has logged out ([f9d4a60](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/f9d4a604963702e9bd86069b3d0881b782bf724a))

### [0.12.15](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.12.15%0D0.12.14) (2024-05-22)


### Features

* add roles to the player session attribute ([6611f74](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/6611f748ea5072ca88132683639ca9c06fed93c4))
* changes to support unreal 5.4 ([64ca45b](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/64ca45b0be7fd414ea87b2d5075e42ce975a400a))
* **chat:** check the max length of the message if auto check flag is active ([6c2cead](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/6c2ceadf8de313823327808d77c187e5aef89fc0))
* **MMv2:** MM start notification for cross play ([25172b1](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/25172b1925b57656992d0f54b8ec6adfec7033a8))


### Bug Fixes

* **asyncTask:** update the AsyncTask to respect the bShouldUseTimeout configuration correctly ([3067c20](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/3067c2092b57f8803e137449ce31864d738b4d40))
* **lobby:** add a reconnected delegate fire after reconnected to lobby ([b6c7b14](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/b6c7b147bbbdeed2c037028c953d59b12b94a950))
* **session:** bubble DS error delegate to auto join sessions ([1ea0432](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/1ea0432927e9657b38c35a41c5d88beefb1828a8))

### [0.12.14](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.12.14%0D0.12.13) (2024-05-13)


### Features

* **analytics:** user segmentation data quality check ([a6fe68f](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/a6fe68f19d1807137725e4a9f8569943a014957b))
* **config:** allow for runtime override of native and secondary platform subsystems ([654aa56](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/654aa56e64c4a0ed21e5ec70d2f72655c813fe2f))
* **entitlement:** expose public get user entitlement history ([32c2eb2](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/32c2eb2e967c41d72a197687919868ff36a82376))


### Bug Fixes

* login queue keep polling after ticket is canceled ([6a7a1db](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/6a7a1db6ce0dbbfadc2d9c5dec1ee69a263751ce))
* **session:** add API_CLIENT_CHECKGUARD in get recent player async task ([adb845e](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/adb845e52fae1d665cb4746ef736363bb952ec65))

### [0.12.13](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.12.13%0D0.12.12) (2024-05-07)


### Bug Fixes

* crashes when dereferencing invalid TSharedPtr ([9aca17c](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/9aca17ca528ee309205636ea96ab3abc03d94d2d))

### [0.12.12](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.12.12%0D0.12.11) (2024-05-04)


### Features

* **asyncTask:** add simple checker to prevent an invalid ApiClient call from AsyncTask ([3eef498](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/3eef49819d3fffe1b22773bc3a55c302346b445b))


### Bug Fixes

* **asyncTask:** swap private variable name ([5dab7db](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/5dab7db7dd44881d114abbc7199467bccb559d7d))

### [0.12.11](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.12.11%0D0.12.10) (2024-05-01)


### Features

* **entitlement:** expose get user entitlement history ([b7942f9](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/b7942f9692800f75d362aecf0d7fa6d909acfe5f))
* **invitation:** add expiration information for session invitation model ([ef1d0c7](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/ef1d0c712270944dd77b118878654d5a2e180561))
* **notification:** add a listener and delegate to listen when a Party or GameSession invitation for invitee is timeout ([5c10b62](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/5c10b62192e052f38c2bd0f3b1fc93179ab4a2fd))


### Bug Fixes

* **identity:** ensuring to reset the local player logging in status to false first even the user id is not valid after login complete. ([e757fe5](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/e757fe5cbf60579a806de4922f4c0f6e881dbb94))

### [0.12.10](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.12.10%0D0.12.9) (2024-04-26)


### Features

* **achievement:** implement method to call psn event sync endpoint from servers ([c9b120e](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/c9b120ee948c2b4a74e53181ad71dc87299c55cb))
* Add OnV2SessionEndedNotification delegate ([58ad76c](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/58ad76c8e357d1802e5e8b483207ef38dad507d8))
* **login-queue:** add jitter in login queue polling delay ([0e9c013](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/0e9c0137e3344275511655b2a872ebef88ddb8e0))
* **session:** add recent player endpoint on high level oss for client and server ([0f8de9f](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/0f8de9f1e9979f00ca750320c6745d1a20f7e938))
* **sync:** add endpoint for syncing blocked players from third party platform ([857dbae](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/857dbaef90fe6e2515b8aa52717eabdd5f91c18e))


### Bug Fixes

* **achievement:** update QueryUserAchievements to use global sort by ([a68a6f7](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/a68a6f7f151e7b331771bb330db61131af1e1f26))
* **Login:** move steamCallback to a class, only create when needed, and destroy immediately after used ([fb0ea49](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/fb0ea49b42a3ff38c4d33c1cfde155b62723f605))
* poller set delay below minDelay to minDelay instead of returning ([ce1eb93](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/ce1eb93fb96e210a09585a89d2bdb96f515f9fe4))
* **session:** add guard to UpdateSession to prevent removing vital session settings ([af00748](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/af00748aa78971dbc103e0d798c4fe744351abfd))
* **Session:** Change DS claimed check on register success by register DS result. ([61e0dba](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/61e0dba6681457356dc2c61d55e2660499f0efc4))

### [0.12.9](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.12.9%0D0.12.8) (2024-04-05)


### Features

* **ecommerce:** add query active sections and store front ([fbbdc5d](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/fbbdc5dd20952056c378dd61c1a59a3e25655ec5))
* **lobby:** add entitlement token generation and check to lobby connection ([79888e8](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/79888e883536b32f2aec1bc0b36c0ed0c4caa3a3))


### Bug Fixes

* **cast:** fix unique net cast check for non AB ([5ff056b](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/5ff056bfa8276c17990856032c3579ca56c02769))
* **cast:** fix unique net cast check for non AB ([8d6290c](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/8d6290cf2fd1b7882ac9ba85a6512a163aa0d7c6))
* **chore:** update submodule ([9e9ace3](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/9e9ace3635608a4b7dbc987de4ac36c66cabcd8b))
* **lobby:** fix compile issue both windows and linux ([e7a4d88](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/e7a4d881a82ec373450e3e233071e71bc53be8b0))
* **session:** update party member setting with a correct session setting pointer ([34fbf86](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/34fbf86d1cef869a65fdfbbc17f5e80f0551b821))

### [0.12.8](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.12.8%0D0.12.7) (2024-03-28)


### Features

* **login:** store simultaneous platform user information in user account ([280c9be](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/280c9be57b7d48a9e1f6678a304217e906115679))
* **User:** add 3rd party platform info to user cache ([908d827](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/908d827585ad21b652e785b15dcb65faa8933f7d))


### Bug Fixes

* **lobby:** suppress ConnectLobby delegate firing when session lobby serial tasks succeed ([51939ed](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/51939edc7ce63766eced9fcd0e353a08a21d1a3d))

### [0.12.7](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.12.7%0D0.12.6) (2024-03-24)


### Features

* **Login:** add new Login Type to support OIDC login methods ([9083f41](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/9083f4108d9d5948974e1d5d2f5605a859327cbc))


### Bug Fixes

* automatic native platform refresh triggered on non-native platform login ([b771445](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/b771445d915ab9e431e0d8a4f51a46e0cd79882e))
* **MPv2:** make sure OnMatchmakingCompleteDelegate triggered once in matchmaking process ([6ab404e](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/6ab404eab49bcc71ffa5abff585a8d5082d4a457))
* **SessionInterface:** incorrect array removal used in matchmaking polling ([dded824](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/dded82466e5c717374320b9f3db3f27023344c23))
* **setting:** make sure all overriden settings are goes to accelbyte flag as well ([0aed076](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/0aed07609f42d94576a7a4e2647b8d8b5cae1467))
* **TTL:** expose time to live config on several endpoints ([0406170](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/04061708bffa6cab0ea740352e9f37a17bc40918))

### [0.12.6](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.12.6%0D0.12.5) (2024-03-08)


### Features

* **backfill:** add new delegate and set notification when backfill ticket is expired ([16fc905](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/16fc90515a00e8731ad59178e93b52eae513ab71))
* Implement login queue ([fecea37](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/fecea37833ab137c4a609dc32c93cd9244c4c890))
* **session:** Add method for retrieving party or game session information ([58368d1](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/58368d1c3d1c2f1ff82a61a4934d39d5e337654b))


### Bug Fixes

* **AccelBytePoller:** change FTSTicker to FTickerAlias and fix UE 4.27 compilation ([ada090f](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/ada090f5ebe4a6157ca93500c504c2ae6e71ae55))
* add PS5 and XSX dependencies in build script ([a5204c4](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/a5204c4b576e64a1cb15433bd514e69a11bd50e6))
* change poller delegate creation to use ThreadSafeSP ([f42f1d5](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/f42f1d59cc801fae532b682317c9288474ee3e84))
* change validation to compare platform id from backend with platform type ([60aefcf](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/60aefcf6a5d5ebb85c3031b5056b4944b7711155))
* fix crashing issue when login queue poller initializing ([5d367eb](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/5d367ebc841ecdcdd456d2d4d10100ae0b77ffad))
* **login:** implement per-local user flag to determine if that player is currently in the process of logging in ([f1a5793](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/f1a57934411120e861b1bf65b1e3ed490e28a577))
* refactor simultaneous login using v4 endpoint ([86b4534](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/86b4534ad3b977ae6cf1f7264d53025a59144dc8))
* split large basic user info query to smaller multiple request to support query more than 100 users ([04a5e84](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/04a5e84eb8c777505e2e703759ab49c1ba379d53))
* **submodule:** update submodule ([7e36549](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/7e36549dd54066cde7ba21996968a4ba3f4fb608))

### [0.12.5](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.12.5%0D0.12.4) (2024-02-21)


### Features

* add new autoCalcEstimatedPrice Param ([91a1ac5](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/91a1ac5b0dc93bd48ff3c46c76d058683da849e5))
* change friend interaction from websocket API to REST API ([ae1bd95](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/ae1bd950f33901817c4b72ec7d1c4a2edb98fedf))
* expose get 3rd party platform information for current user ([3987065](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/39870655ec3af435a63befc8b980f46789a01f78))
* implement matchmaking notifications missing mitigations ([ea0ed60](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/ea0ed60e5fe88455ffdcaabfa480644c77c54692))
* **ugc:** add some public api that can be used without login ([1b62ee7](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/1b62ee7e611f5cc999d3cc4ba82d4037109c1dd4))


### Bug Fixes

* **account:** account profile revamp ([0cd7c61](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/0cd7c614e4c8812d6835b02484e12f60ae677f8d))
* crash when ApiClient is not valid during FindV2GameSessionById async task ([340da00](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/340da006132c07e70f03f0e7ad8d2afbd2f36f27))
* **DisplayName:** on query user IDs success, provide the default displayname & avatar URL first, before overriding it using platform infomration ([f2188b7](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/f2188b7a4496205ed277cf8385cf57ab6e6fe2cf))
* PerformLogin accidentally triggered by Steam Callback ([e7ad70f](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/e7ad70f39ff909483295c057c0f84db491753b53))
* premature credentials removal when Lobby connection was closed ([d8c83f8](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/d8c83f8a458466d348fd66e6f5fd87a3ca93cfa6))
* resolve conflict merge ([77c1ad6](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/77c1ad6347356c54d10dc6054645c75eedf6b800))
* **session:** omit empty fields before send player attributes ([511433b](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/511433b39974e5f60a6f0bbb2d4cdd3c4e313f30))
* **session:** Restore session should always attempt to rejoin rather than just requerying data ([2bda5be](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/2bda5bef1b62eb5515cfba5103b1a2cd7cebcc71))
* submodule update ([6f96752](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/6f96752188af8abe0c1e39ff4d7f81769a0dd84f))
* **UniqueNetID:** improve the UniqueNetID for 32-digit UUID input, to reduce the unnecessary decode & deserialization ([f385936](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/f3859361d3d0d86ca9be3ebe1b0f8f1982adea11))


### Refactors

* update with renamed function on PlatformHandler ([74c972b](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/74c972b9c6ca0067fa61932654239194d9b8a3bc))

### [0.12.4](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.12.4%0D0.12.3) (2024-02-05)


### Features

* **lobby:** replace instead of add delegate on notification message ([a334ae0](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/a334ae07632d0cc16d1feaf0d71ab64f8138eb07))
* **presence:** add steam callback to update presence status when changed from platform ([b8e8f1b](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/b8e8f1b1656c867fa8f7ed0e59c56969c43fa580))
* replace the get bulk user information with a newer endpoint ([c871d9a](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/c871d9a75544e53fb7eae210a891405edf5681de))

### [0.12.3](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.12.3%0D0.12.2) (2024-01-18)


### Features

* add new delegate for lobby connection closed and reconnecting on OSS ([aafeec5](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/aafeec51747f9433c434f9309e7d45b0c586f5ad))
* add platform information in presence interface ([9541bce](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/9541bce7dedaeb69ebc0bec77ad28922d7aa2fd0))
* add QueryRecentPlayer when using session v2 ([1ddf886](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/1ddf886d53281c12bf84a02877d28e7bf0b0701a))
* add user country/region to their account identity ([d504a26](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/d504a26abc4e5290e4271508b300855924fcac0f))
* ensure lobby is connected before start matchmaking ([d060e10](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/d060e102800068a48432e8f5bff6ea1dbfcd4d27))
* **Login:** simultaneous login using two platform accounts ([4cba08d](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/4cba08dea467c645c9e50effd4ff073d1e373b06))
* **mpv2:** ensure lobby connected before calling mpv2 api ([4e11a8b](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/4e11a8b327ce6e35c53e9f97418ea851c348f317))
* sdk adjustment to support returning explicit 3rd party platform info ([4b332b2](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/4b332b2379721f4675c4ae5b853a4b1d3a46953f))


### Bug Fixes

* complete lobby connection task properly when skipping success callback ([cd32d01](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/cd32d01d8d29bcbdb28c487ce78cf4b3dfe68a11))
* include OnlineSubsystemAccelBytePackage.h to avoid PACKAGE_SCOPE issue in unity build ([09f4f8f](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/09f4f8f3ccce7758a834008f10e1e7b3213d86ad))
* **native platform:** missing native platform converter ([872cfcb](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/872cfcbd3408cae8cbae5a1be7585143a4981a45))

### [0.12.2](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.12.2%0D0.12.1) (2024-01-09)


### Features

* **gameTelemetry:** add SetTelemetryCriticalEventList to set some criticalEvents that need to be cached ([8c46e4a](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/8c46e4a34dd4dd57a84b74417ce50fb7081cd254))
* **IAM:** refresh native platform token ([b152758](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/b152758d2efbdbe80b0ca6600b7e7565cd5363b9))
* new lobby notification delegate for UE OSS ([699bd11](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/699bd115f019a1261f47d54877fa16428e5c0948))


### Bug Fixes

* **cloudsave:** delegate return empty model when the request is failed ([14c9b73](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/14c9b739ff9a5849ef275828d3cdf958907bb8b4))
* **EOS:** add empty token prevention ([1845de8](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/1845de8aabd69d710d5a7700c1ff4ebf9b290cbd))
* log UniqueNetId with ToDebugString ([ee82d49](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/ee82d49cc4c05bebb8b9057cd3e45d3e360ed62f))
* **mpv2:** remove teams field when updating game session with closed joinability ([a9a77ab](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/a9a77ab312450dbb89dccf732af70c922dad09ac))


### Refactors

* **ServerTime:** move ServerTime local calculation to use TimeManager ([9094dca](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/9094dca1c70e6a26a85fd65ab6d1a2b230d0b481))

### [0.12.1](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.12.1%0D0.12.0) (2024-01-02)


### Bug Fixes

* add a new validation of target user id parameter to denied an access if several condition is invalid. ([9448ecb](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/9448ecbf0f2e91604b9a52cd046d66e7b2c78422))
* add default value on initialized variable ([ce73d04](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/ce73d040d4d364222121a713350a9485176b57dc))
* change the error response from AccessDenied to NotImplemented ([f74893f](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/f74893fe2bd536507d823a0338c7dc99398f36ea))
* hotfix for supporting dedicated server to call cloud save interface ([7d2337e](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/7d2337e5b258cfcba50ac55633bc24f94fe3dec7))
* reorder the initilize variable on construct function and change the parameter type to accelbyte unique net id instead of FString. ([6b5009a](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/6b5009a97a59adddc136581cb9a60cd363d0c052))

## [0.12.0](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.12.0%0D0.11.28) (2023-12-14)


### ⚠ BREAKING CHANGES

* **sessionV2:** pass MatchmakingHandler in MatchmakingExpired delegate

### Features

* Adding/Expose Extension Json FObjectFOnlineStoreOfferAccelByte model ([3cbb669](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/3cbb6699c14c45dcae5848f98580e86c900af406))
* **analytics:** change naming for cached analytics event model ([e8608d0](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/e8608d0d6fab11dfce5d6ccdca54cc26991795f7))
* **chatv2:** add system message inbox and transient message ([4a99507](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/4a99507bfed12c53c06751823500ee1f145974b3))
* **entitlement:** add consume entitlement feature ([8083d4f](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/8083d4ffcfd4b9b5bd2f1c1a6c1c1eaf6882938f))
* **mpv2:** add online error details for create match ticket ([42de476](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/42de4769270810f3afde725d1db25c0ada975c04))
* **tracing:** create base class for analytics-related interface ([a19c408](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/a19c408b229ece553a07f3436cbb2ee77fe26292))


### Bug Fixes

* only success get session storage when jsonObject is valid ([7897bea](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/7897bea83cfa11b2f531da430adf8d7997f46481))
* **sessionv2:** change all lambda captured variable from reference to copy value ([189f587](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/189f58744fb7e577404286d4b2292ef80e5ab341))
* **sessionV2:** trigger matchmaking expired delegate when receiving matchmaking expired notification ([28aaea1](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/28aaea146912cce944c141257636555de05d4707))
* unbind delegates on RestoreAllV2Session async task timeout ([88afd6d](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/88afd6d88e9e9c7703cfcda9508a9652234ca579))

### [0.11.28](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.11.28%0D0.11.27) (2023-11-28)


### Features

* add predefined events inside storage scope endpoints ([a8073e4](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/a8073e4923588b19b08f6f642426d76f26e997c6))
* **Login:** add CreateHeadless param ([4110102](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/4110102315aa0744ce5d3eef84b0dac149c2d172))
* **login:** add setter for autoLogin create headless ([63fa44b](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/63fa44bc5ddfe2b0e68d2aa045992141343e312b))
* **Login:** login using code that is generated from a publisher namespace token ([b471ce8](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/b471ce8c4b9930909973ff2519bd52d669abfcfe))


### Bug Fixes

* fixed cache token ([1cba87d](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/1cba87d2b8b765d299b094d5be0e1819740b906a))
* fixed member settings were not being handled correctly ([fda0f46](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/fda0f4619ad4c922408b6662ae0a75f9869e7058))


### Refactors

* **CloudSave:** refine conditional check and logging ([9c9733e](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/9c9733ebea00fb56c414b781aa23204eaf0bbee1))

### [0.11.27](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.11.27%0D0.11.26) (2023-11-13)


### Features

* add bulk query user presence ([fb61d9d](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/fb61d9d5a5a9e99ce1dc21dd77aaeca19d0b4f57))
* check user availability ([21311d4](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/21311d4ecfc83c6a76dceb4aa3ef1bcd7fb49dc9))
* **MPv2:** listen to V2GameSessionRejectedNotif and trigger OnSessionInviteRejected delegates ([13bab24](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/13bab2496295ce24034ccb28278f046b9b8f4af8))
* **predefinedEvent:** add some implementations on Engagement Leaderboard, Seasonpass, and Reward scope ([b8abde8](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/b8abde8a89393bea9998b1f1a04068486c4ffe85))
* **predefinedEvent:** trigger events for chatv2 scope ([50bd442](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/50bd442c638811c4f490230215e3230e5e83a691))
* **SessionInterface:** separate the function to SendServerReady ([031880e](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/031880e6cdea83068aeddf5ff199f326b4241a8a))
* **SessionV2:** add a reminder to prevent DS that forgot to set server as ready ([14c2e41](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/14c2e412d4c77a10e630d580654aa574b219e046))


### Bug Fixes

* fixed cache token ([037a150](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/037a15054bd7f956ae544c9e7a9263a9ae04a639))
* missing undef online error / #undef ONLINE_ERROR_NAMESPACE in some ecommerce async classes ([7b4c326](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/7b4c3260ae6d851f5481f46ce5461b1045f792e5))
* **MPv2:** return session leader id for game and party session ([c6a6338](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/c6a6338fc090cc6bf6eabee870c23482b6dbe769))
* **predefinedEvent:** fix preDefinedEventName for cached event ([b09a863](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/b09a863bdce2370aad7d68aff5743572ba825a7b))
* **predefinedEvent:** move predefinedEvent name inside the payload ([6cb30b6](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/6cb30b6d1c07ca637a4260e2a09f7d3221eb76f6))
* **SessionV2:** change on the RegisterServer. Either do nothing, or fast forward to SendServerFunction. ConnectToDSHub() is automatically handled by each async task on Finalize() ([2d65317](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/2d65317a09f563ae3dcc1c626c190a17b399d603))
* **SessionV2:** flip the default behavior, set server ready is manual and recommended to disabled. (previous: manual set server ready is required) ([ecd1e53](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/ecd1e53c3a482b846c60d817f0086f46f8a5f1c8))
* trigger OnBlockListChangeDelegates only if the list changed ([3b28f41](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/3b28f4123626f5cf8eceabe46a12d2ecc5345db1))

### [0.11.26](https://bitbucket.org/accelbyte/justice-ue4-oss/branches/compare/0.11.26%0D0.11.25) (2023-10-30)


### Features

* Adding Support for Multiple Currencies by using Search Item in Store Offer ([7af6980](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/7af69807b1eb51f69480109ddff42110f808cd78))
* **ecommerce:** Flexible bundle pricing ([8c5f909](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/8c5f90997c67558663ec8fec2bf1f77ba4d00153))
* epic games sync dlc ([3af5262](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/3af5262df6b6a9886829375946cdf80de8f72479))
* **MPv2:** add method to set DS session ready manually ([da7a603](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/da7a6036fec9063b640c887c33abb2cde1350ba7))
* **predefinedEvent:** add some models on Engagement Achievement scope ([4cf5f06](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/4cf5f0696b2966a30b558d78be698f5983825da7))
* **predefinedEvent:** integrate some models on Engagement Group scope ([f32d7d5](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/f32d7d55c12b46c43087f9ae9c677b4074e12669))
* **PredefinedEvent:** send event payloads for Play scope ([9ab83e6](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/9ab83e658e3f2f74e9dcda03bb6db65d19991cfb))


### Bug Fixes

* fix DS unable to update session with empty teams ([edfae97](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/edfae978a6467acbf02276ef980bddc5dffe6aca))
* Incorrect include paths ([9abe1a4](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/9abe1a400cb6021ca7683534ebf53e76813b817b))
* missing undef online error / #undef ONLINE_ERROR_NAMESPACE in some ecommerce async classes ([1a28117](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/1a281170103d0f820f5f3fa9cf0619a40bc9b881))
* **MPv2:** clear CurrentMatchmakingHandle on leave session ([13ab409](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/13ab4093a68122187f4c49f4dd7444b791fdbe4e))
* **MPv2:** store delegates as member variables to keep its lifetime as long as the async task alive ([2f7356e](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/2f7356ea6d03e72499cee58ecac9a6ff0e5d6ded))
* **predefinedEvent:** fix preDefinedEventName for cached event ([5a6a171](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/5a6a17189dd2283cfadbdd7f41d0dbf6cc8af3c5))
* **predefinedEvent:** move predefinedEvent name inside the payload ([317e60a](https://bitbucket.org/accelbyte/justice-ue4-oss/commits/317e60abc3b863a34f3ea94fe4b5f372268949f8))

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
