# Changelog

All notable changes to this project will be documented in this file. See [standard-version](https://github.com/conventional-changelog/standard-version) for commit guidelines.

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
