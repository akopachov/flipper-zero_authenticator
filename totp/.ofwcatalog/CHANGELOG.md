# Changelog

## v5.18.0 - Apr 29 2025

* fix: tokens longer than 64 bytes are leading to incorrect codes ([#260](https://github.com/akopachov/flipper-zero_authenticator/issues/260))

## v5.17.3 - Apr 11 2025

* fix: compatibility with SDK version 1.3 f7 ([#256](https://github.com/akopachov/flipper-zero_authenticator/issues/256))

## v5.17.1 - 26 Mar 2025

* fix: HOTP counter is not increasing when "OK" button long-pressed ([#253](https://github.com/akopachov/flipper-zero_authenticator/issues/253))

## v5.17.0 - 27 Feb 2025

* feat: Extended valid UTC offset range to be from -12 to +14 ([#249](https://github.com/akopachov/flipper-zero_authenticator/issues/249))

## v5.16.1 - 22 Oct 2024

* Fixed compatibility with latest FW changes ([#243](https://github.com/akopachov/flipper-zero_authenticator/issues/243))

## v5.16.0 - 29 Aug 2024

* feat: confirm totp export on the device ([#240](https://github.com/akopachov/flipper-zero_authenticator/issues/240))

## v5.15.1 - 21 Aug 2024

* refactor: removed all the references to Xtreme FW as it is dead
* fix: BT key files name format is wrong

## v5.15.0 - 10 Jul 2024

* feat: added support of multiple BT profiles ([#237](https://github.com/akopachov/flipper-zero_authenticator/issues/237))

## v5.14.1 - 29 May 2024

* fix: totp export command uses the wrong format to print url-encoded symbol.

## v5.14.0 - 23 Apr 2024

* fix: unable to add new token if type token secret for a longer than a minute ([#231](https://github.com/akopachov/flipper-zero_authenticator/issues/231))

## v5.13.0 - 11 Mar 2024

* fix: HOTP counter increment is not working ([#226](https://github.com/akopachov/flipper-zero_authenticator/issues/226))
* feat: updated wolfSSL library to the latest version [5.6.6](https://github.com/wolfSSL/wolfssl/releases/tag/v5.6.6-stable)
* feat: updated code to make it compatible with latest FW changes (BT refactoring)

## v5.11.0 - 5 Feb 2024

* feat: implemented standard navigation patterns ([#223](https://github.com/akopachov/flipper-zero_authenticator/issues/223))

## v5.10.0 - 8 Jan 2024

* Implemented export functionality ([#219](https://github.com/akopachov/flipper-zero_authenticator/issues/219))

## v5.9.1 - 23 Nov 2023

* Added checks for unsupported crypto methods
* Added release artifacts supporting old crypto methods

## v5.9.0 - 22 Nov 2023

* Implemented [#217](https://github.com/akopachov/flipper-zero_authenticator/issues/217)

## v5.8.0 - 20 Nov 2023

* Implemented [#215](https://github.com/akopachov/flipper-zero_authenticator/issues/215)

## v5.7.0 - 3 Nov 2023

* Updated WolfSSL to [v5.6.4](https://github.com/wolfSSL/wolfssl/releases/tag/v5.6.4-stable)
* Cosmetic fix to "totp help" CLI command output text

## v5.6.0 - 30 Oct 2023

* Fixed [#212](https://github.com/akopachov/flipper-zero_authenticator/issues/212)

## v5.5.0 - 26 Sep 2023

* Implemented counter-based tokens (HOTP) ([#205](https://github.com/akopachov/flipper-zero_authenticator/issues/205))
* Migrated build pipelines to uFBT ([#203](https://github.com/akopachov/flipper-zero_authenticator/issues/203))

## v5.0.0 - 13 Sep 2023

* Implemented [#198](https://github.com/akopachov/flipper-zero_authenticator/issues/198)
* Implemented [#200](https://github.com/akopachov/flipper-zero_authenticator/issues/200)

## v4.1.0 - 28 Aug 2023

* Implemented [#195](https://github.com/akopachov/flipper-zero_authenticator/issues/195)
* Added "totp version" CLI command

## v4.0.0 - 9 Aug 2023

* Dropped homebrew hashing algo implementations and replaced them with [WolfSSL](https://github.com/wolfSSL/wolfssl/) library ([#185](https://github.com/akopachov/flipper-zero_authenticator/issues/185))
* Updated crypto algorithms to use PBKDF2 instead of plain HMAC to generate IV.
* Fixed CLI bugs when dealing with conf file and folders

## v3.2.0 - 2 Aug 2023

* Implemented [#177](https://github.com/akopachov/flipper-zero_authenticator/issues/177)
* Fixed [#178](https://github.com/akopachov/flipper-zero_authenticator/issues/178)
* Refactoring

## v3.1.0 - 31 Jul 2023

* Implemented [#171](https://github.com/akopachov/flipper-zero_authenticator/issues/171)

## v3.0.2 - 28 Jul 2023

* Fixed [#169](https://github.com/akopachov/flipper-zero_authenticator/issues/169)
* Fixed [#172](https://github.com/akopachov/flipper-zero_authenticator/issues/172)

## v3.0.0 - 26 Jul 2023

* Implemented better encryption [#167](https://github.com/akopachov/flipper-zero_authenticator/issues/167)

## v2.3.0 - 18 Jul 2023

* Implemented [#165](https://github.com/akopachov/flipper-zero_authenticator/issues/165)
* Just a bit of refactoring


## v2.2.3 - 10 Jul 2023

* Refactoring [#162](https://github.com/akopachov/flipper-zero_authenticator/issues/158) 
* Updated repo to match [OFW catalog](https://github.com/flipperdevices/flipper-application-catalog/) requirements


## v2.2.2 - 19 Jun 2023

* Fixed [#158](https://github.com/akopachov/flipper-zero_authenticator/issues/158) 
* Cosmetic refactoring



## v2.2.1 - 06 Jun 2023

* Fixed [#155](https://github.com/akopachov/flipper-zero_authenticator/issues/155) 
* Refactoring



## v2.2.0 - 05 Jun 2023

* Fixed [#153](https://github.com/akopachov/flipper-zero_authenticator/issues/153) 
* Updated firmware submodules



## v2.1.1 - 30 May 2023

* Updated firmware submodules
* Added some new fonts for supporters 



## v2.1.0 - 24 May 2023

* Implemented [#148](https://github.com/akopachov/flipper-zero_authenticator/issues/148) 
* 🧹 Refactoring
* ** Added nice custom fonts for supporters **



## v2.0.3 - 29 Apr 2023

* Fixed [#146](https://github.com/akopachov/flipper-zero_authenticator/issues/146) 
* Updated firmware references



## v2.0.2 - 28 Apr 2023

* Refactoring & minor improvements



## v2.0.1 - 26 Apr 2023

* Fixed few bugs I found during day 1 of v2.0.0
* Refactoring
* Updated firmware submodules



## v2.0.0 - 25 Apr 2023

* Reworked token list to make app work smooth with bigger lists of tokens. So right now you can add as much tokens as you need.
* Improved token list operations to make them more reliable
* Refactoring
* Updated firmware submodules



## v1.9.2 - 13 Apr 2023

* Updated firmware submodules



## v1.9.1 - 13 Apr 2023

* Improved TOTP code rendering method
* Added "--clean" flag to custom FBT



## v1.9.0 - 12 Apr 2023

* Moved token generation into separate thread
* Refactoring and code cleanup



## v1.8.8 - 11 Apr 2023

* Xtreme: fixed BT name length according to latest XFW changes (by [@Willy-JL](https://github.com/Willy-JL))
* Updated firmware submodules



## v1.8.7 - 07 Apr 2023

* Backup conf file before running "totp pin set" and "totp pin remove" CLI commands ([#123](https://github.com/akopachov/flipper-zero_authenticator/issues/123))
* Some refactoring ([#120](https://github.com/akopachov/flipper-zero_authenticator/issues/120))



## v1.8.6 - 06 Apr 2023

* Updated firmware submodules [#118](https://github.com/akopachov/flipper-zero_authenticator/issues/118) 



## v1.8.5 - 05 Apr 2023

* Fixed [#116](https://github.com/akopachov/flipper-zero_authenticator/issues/116) 



## v1.8.4 - 05 Apr 2023

* Added Steam guard token support ([#111](https://github.com/akopachov/flipper-zero_authenticator/issues/111))
* Added Base64-encoded token support
* Custom font for token
* Refactoring



## v1.8.3 - 03 Apr 2023

* Build artifact for [Xtreme](https://github.com/ClaraCrazy/Flipper-Xtreme) firmware ([#113](https://github.com/akopachov/flipper-zero_authenticator/issues/113))
* Xtreme: custom BT name & MAC which should resolve [#98](https://github.com/akopachov/flipper-zero_authenticator/issues/98)



## v1.8.2 - 31 Mar 2023

* Implemented [#106](https://github.com/akopachov/flipper-zero_authenticator/issues/106) 



## v1.8.1 - 31 Mar 2023

* Implemented [#102](https://github.com/akopachov/flipper-zero_authenticator/issues/102) 
* Implemented [#103](https://github.com/akopachov/flipper-zero_authenticator/issues/103) 
* Refactoring
* Minor fixes



## v1.8.0 - 30 Mar 2023

* Implemented [#95](https://github.com/akopachov/flipper-zero_authenticator/issues/95) 
* New CLI commands "totp update", "totp lsattr"
* Refactoring



## v1.7.1 - 22 Mar 2023

* Updated firmware submodules



## v1.7.0 - 21 Mar 2023

* Implemented [#89](https://github.com/akopachov/flipper-zero_authenticator/issues/89) 
* Refactoring



## v1.6.5 - 09 Mar 2023

* Updated firmware references
* Updated code to make it compatible with latest firmware changes



## v1.6.4 - 01 Mar 2023

* Updated firmware references



## v1.6.3 - 16 Feb 2023

* Fixed [#76](https://github.com/akopachov/flipper-zero_authenticator/issues/76) 
* Fixed [#77](https://github.com/akopachov/flipper-zero_authenticator/issues/77)
* Made CLI console output colorful
* Few refactoring



## v1.6.2 - 13 Feb 2023

* Updated submodules to make app compatible with latest firmware
* Cosmetic code changes



## v1.6.1 - 07 Feb 2023

* Implemented [#70](https://github.com/akopachov/flipper-zero_authenticator/issues/70) 



## v1.6.0 - 18 Jan 2023

* Implemented [#65](https://github.com/akopachov/flipper-zero_authenticator/issues/65) 



## v1.5.6 - 09 Jan 2023

* Updated firmwares



## v1.5.5 - 04 Jan 2023

* Implemented [#58](https://github.com/akopachov/flipper-zero_authenticator/issues/58) 
* A bit of refactoring



## v1.5.4 - 02 Jan 2023

* Fixed [#53](https://github.com/akopachov/flipper-zero_authenticator/issues/53) 



## v1.5.3 - 30 Dec 2022

* Fixed [#51](https://github.com/akopachov/flipper-zero_authenticator/issues/51) 



## v1.5.2 - 21 Dec 2022

* Config file moved to "/ext/authenticator/totp.conf"
* Improved the way how config file is getting handled
* Improved user notification if on error during config file open\read\update



## v1.5.1 - 01 Dec 2022

* Implemented [#44](https://github.com/akopachov/flipper-zero_authenticator/issues/44) to be compatible with latest API changes



## v1.5.0 - 23 Nov 2022

* Implemented [#36](https://github.com/akopachov/flipper-zero_authenticator/issues/36) 
* Refactoring



## v1.4.0 - 21 Nov 2022

* Implemented [#10](https://github.com/akopachov/flipper-zero_authenticator/issues/10) 
* Refactoring



## v1.3.1 - 18 Nov 2022

* Implemented [#30](https://github.com/akopachov/flipper-zero_authenticator/issues/30) 
* Cosmetic improvements



## v1.3.0 - 17 Nov 2022

* Implemented [#26](https://github.com/akopachov/flipper-zero_authenticator/issues/26) 
* Implemented [#28](https://github.com/akopachov/flipper-zero_authenticator/issues/28) 
* Improved HID worker code
* Improved navigation using "long press" event
* Refactoring



## v1.2.1 - 10 Nov 2022

* Fixed [#24](https://github.com/akopachov/flipper-zero_authenticator/issues/24)
* Small improvement to the USB mode restore code



## v1.2.0 - 10 Nov 2022

* Implemented [#21](https://github.com/akopachov/flipper-zero_authenticator/issues/21) 



## v1.1.2 - 07 Nov 2022

* Integrated PVS Studio scanner and fixed all its complains
* Updated firmwares
* Refactoring
* Dead code eliminated to fix "COMPACT=1 DEBUG=0" build
* Build artifacts are done with "COMPACT=1 DEBUG=0" build settings



## v1.1.1 - 02 Nov 2022

* Latest firmwares
* Refactoring
* Sonar scan issues fixed



## v1.1.0 - 26 Oct 2022

* CLI added (thanks to [@br0ziliy](https://github.com/br0ziliy)). [#11](https://github.com/akopachov/flipper-zero_authenticator/issues/11) 
* Updated firmware submodules to latest
* Refactoring



## v1.0.1 - 21 Oct 2022

* Fixed issue [#8](https://github.com/akopachov/flipper-zero_authenticator/issues/8) 
* Refactoring



## v1.0.0 - 17 Oct 2022

* Made PIN optional
* Submodules updated
* Refactoring
* Few fixes



## v0.0.6 - 13 Oct 2022

* Added timezone configuration UI
* Removed CLI app POC
* Minor fixes



## v0.0.5 - 10 Oct 2022

* Separate builds for Official and Unleashed firmwares
* Updated code to FuriString



## v0.0.4 - 05 Oct 2022

* Moved TOTP app to separate folder to segregate it from flipper_firmware code and make it clearer
* Code refactoring
* Added SHA1, SHA256 and SHA512 hashing algos
* Minor improvements


## v0.0.2

Ability to add more than one token

## v0.0.1

First POC. SHA1 hashing algorithm and one token.
