// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "JsonObjectWrapper.h"

/**
 * @brief Represents a user's linked platform information, typically used for caching.
 *
 * Contains essential information about a user from a linked external platform.
 */
struct FAccelByteLinkedUserInfo
{
public:

	/**
	 * @brief Composite ID representation for this user, platform information may be blank if we cannot retrieve these values.
	 */
	TSharedPtr<const FUniqueNetIdAccelByteUser> Id{ nullptr };

	/**
	 * @brief The display name of the user on the linked external platform.
	 */
	FString DisplayName{};

	/**
	 * @brief Enum representing the type of platform that this link is for
	 */
	EAccelBytePlatformType PlatformType { EAccelBytePlatformType::None };

	/**
	 * @brief The unique identifier (PlatformId) of the user on the linked external platform.
	 */
	FString PlatformId{};

	/**
	 * @brief The avatar of the user on the linked external platform.
	 */
	FString AvatarUrl{};
};
