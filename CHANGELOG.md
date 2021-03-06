# Changelog

All notable changes to this project will be documented in this file. See [standard-version](https://github.com/conventional-changelog/standard-version) for commit guidelines.

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
