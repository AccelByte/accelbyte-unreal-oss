// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "JsonObjectWrapper.h"

/**
 * @brief Contains optional parameters to request specific achievement descriptions from backend.
 */
struct FAccelByteQueryAchievementDescriptionParameters
{
public:

	/**
	 * @brief The language to display the appropriate achievement's name and description. If it is empty, it will use its 
	 * default language. If the achievement does not have the expected language, it will use its default language.
	 */
	FString Language = TEXT("");

	/**
	 * @brief Sorting method for the result of achievement items.
	 * Default value: Created At Descending.
	 */
	EAccelByteAchievementListSortBy SortBy = EAccelByteAchievementListSortBy::CREATED_AT_DESC;

	/**
	 * @brief Number of content per page to retrieve and page to retrieve. Default value start (offset) : 0, count (limit) : 100
	 */
	FPagedQuery Page = FPagedQuery();

	/**
	 * @brief Filter the achievement items result by available tags.
	 * Default value: Empty.
	 */
	FString Tag = TEXT("");

	/**
	 * @brief Flag to display the configuration of global achievement items.
	 * Default value: false.
	 */
	bool bGlobalAchievement = false;
};

/**
 * @brief Contains optional parameters to request specific achievement items from backend.
 */
struct FAccelByteQueryAchievementsParameters
{
public:

	/**
	 * @brief Sorting method for the result of achievement items.
	 * Default value: Created At Descending.
	 */
	EAccelByteGlobalAchievementListSortBy SortBy = EAccelByteGlobalAchievementListSortBy::CREATED_AT_DESC;

	/**
	 * @brief Number of content per page to retrieve and page to retrieve. Default value start (offset) : 0, count (limit) : 100
	 */
	FPagedQuery Page = FPagedQuery();

	/**
	 * @brief Filter the achievement items result by available tags.
	 * Default value: Empty.
	 */
	FString Tag = TEXT("");

	/**
	 * @brief Flag to display the unlocked achievement items.
	 * Default value: true.
	 */
	bool bUnlocked = true;
};