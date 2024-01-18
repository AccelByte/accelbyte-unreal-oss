// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "CoreMinimal.h"

#pragma once

class FOnlineAccountCredentialsAccelByte : public FOnlineAccountCredentials
{
public:
	EAccelByteLoginType LoginType;
	bool bCreateHeadlessAccount = true;

	FOnlineAccountCredentialsAccelByte(EAccelByteLoginType InType
		, const FString& InId
		, const FString& InToken
		, bool bInCreateHeadlessAccount = true)
		: FOnlineAccountCredentials(FAccelByteUtilities::GetUEnumValueAsString(InType), InId, InToken)
		, LoginType{ InType }
		, bCreateHeadlessAccount(bInCreateHeadlessAccount)
	{}

	FOnlineAccountCredentialsAccelByte(FOnlineAccountCredentials AccountCredentials
		, bool bInCreateHeadlessAccount = true)
		: FOnlineAccountCredentials(AccountCredentials.Type, AccountCredentials.Id, AccountCredentials.Token)
		, LoginType{ FAccelByteUtilities::GetUEnumValueFromString<EAccelByteLoginType>(AccountCredentials.Type) }
		, bCreateHeadlessAccount(bInCreateHeadlessAccount)
	{}

	FOnlineAccountCredentialsAccelByte(bool bInCreateHeadlessAccount) //Login with native oss
		: FOnlineAccountCredentials()
		, bCreateHeadlessAccount(bInCreateHeadlessAccount)
	{}
};

/**
 * @brief Enum representing the result from simultaneous login attempt which might fail in the middle of the process
 */
enum class ESimultaneousLoginResult : uint32
{
	Unknown = 0,
	Success,
	Uninitialized,
	NativePlatformLoginFail,
	SecondaryPlatformLoginFail,
	InvalidTicket,
	ConflictAccountLinking,

	/**
	 * @brief HTTP Response 400 
	 * 
	 * Misconfiguration appID in the client or admin dashboard might trigger HTTP response 400.
	 */
	ServiceMisconfiguration,

	/**
	 * @brief 10215
	 * 
	 * Simultaneous ticket is required
	 */
	LinkingFailure_SecondaryPlatformTicketRequired = 10215,

	/**
	 * @brief 10220
	 * 
	 * Native ticket's account linked AGS account is different with the one which simultaneous ticket's linked to
	 * Please provide either the Native or Secondary platform account, that hasn't tied to different AccelByte Game Service account.
	 */
	LinkingFailure_BothNativeAndSecondaryPlatformAlreadyLinkedToDifferentAccelByteAccount = 10220,

	/**
	 * @brief 10219
	 * 
	 * Native ticket's account linked AGS is already linked simultaneous but different with the input simultaneous ticket's
	 * Please provide the correct SecondaryPlatformAccount's ticket.
	 */
	LinkingFailure_AccountAlreadyLinkedButSecondaryTicketMismatch = 10219,

	/**
	 * @brief 10217
	 * 
	 * Native ticket's account linked AGS account has different linking history with input simultaneous ticket's
	 * Please use the previously-linked SecondaryPlatformAccount and provide the ticket.
	 */
	LinkingFailure_NativePlatformAccountPreviouslyLinkedToAnotherAccount = 10217,

	/**
	 * @brief 10221
	 * 
	 * Simultaneous ticket's account linked AGS is already linked native but different with the input native ticket's
	 * Please provide the correct NativePlatformAccount's ticket.
	 */
	LinkingFailure_AccountAlreadyLinkedButNativeTicketMismatch = 10221,

	/**
	 * @brief 10218
	 * 
	 * Simultaneous ticket's account linked AGS account has different linking history with input native ticket's
	 * Please use the previously-linked NativePlatformAccount and provide the ticket.
	 */
	LinkingFailure_SecondaryPlatformAccountPreviouslyLinkedToAnotherAccount = 10218
};