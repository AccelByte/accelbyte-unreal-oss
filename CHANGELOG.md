# Changelog

All notable changes to this project will be documented in this file. See [standard-version](https://github.com/conventional-changelog/standard-version) for commit guidelines.

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
