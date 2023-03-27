// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "OnlineSessionSettings.h"

enum class ESessionSettingsAccelByteArrayFieldType : uint8
{
	INVALID = 0,
	STRINGS,
	DOUBLES
};

class ONLINESUBSYSTEMACCELBYTE_API FOnlineSearchSettingsAccelByte : public FOnlineSearchSettings
{
public:
	using FOnlineSearchSettings::Set;
	using FOnlineSearchSettings::Get;

	/**
	 * Sets a key value pair combination that defines a search parameter
	 *
	 * @param Key key for the setting
	 * @param Value value of the setting
	 * @param InType type of comparison
	 * @param ID ID of comparison
	 */
	void Set(FName Key, const TArray<FString>& Value, EOnlineComparisonOp::Type InType, int32 ID);

	/**
	 * Sets a key value pair combination that defines a search parameter
	 *
	 * @param Key key for the setting
	 * @param Value value of the setting
	 * @param InType type of comparison
	 * @param ID ID of comparison
	 */
	void Set(FName Key, const TArray<double>& Value, EOnlineComparisonOp::Type InType, int32 ID);

	/**
	 * Sets a key value pair combination that defines a search parameter
	 *
	 * @param Key key for the setting
	 * @param Value value of the setting
	 * @param InType type of comparison
	 */
	void Set(FName Key, const TArray<FString>& Value, EOnlineComparisonOp::Type InType);

	/**
	 * Sets a key value pair combination that defines a search parameter
	 *
	 * @param Key key for the setting
	 * @param Value value of the setting
	 * @param InType type of comparison
	 */
	void Set(FName Key, const TArray<double>& Value, EOnlineComparisonOp::Type InType);

	/**
	 * Sets a key value pair combination that defines a search parameter
	 *
	 * @param SearchSettings search settings to apply to search parameter to
	 * @param Key key for the setting
	 * @param Value value of the setting
	 * @param InType type of comparison
	 */
	static void Set(FOnlineSearchSettings& SearchSettings, FName Key, const TArray<FString>& Value, EOnlineComparisonOp::Type InType);

	/**
	 * Sets a key value pair combination that defines a search parameter
	 *
	 * @param SearchSettings search settings to apply to search parameter to
	 * @param Key key for the setting
	 * @param Value value of the setting
	 * @param InType type of comparison
	 */
	static void Set(FOnlineSearchSettings& SearchSettings, FName Key, const TArray<double>& Value, EOnlineComparisonOp::Type InType);

	/**
	 * Sets a key value pair combination that defines a search parameter
	 *
	 * @param SearchSettings search settings to apply to search parameter to
	 * @param Key key for the setting
	 * @param Value value of the setting
	 * @param InType type of comparison
	 */
	static void Set(FOnlineSearchSettings& SearchSettings, FName Key, const TArray<FString>& Value, EOnlineComparisonOp::Type InType, int32 ID);

	/**
	 * Sets a key value pair combination that defines a search parameter
	 *
	 * @param SearchSettings search settings to apply to search parameter to
	 * @param Key key for the setting
	 * @param Value value of the setting
	 * @param InType type of comparison
	 */
	static void Set(FOnlineSearchSettings& SearchSettings, FName Key, const TArray<double>& Value, EOnlineComparisonOp::Type InType, int32 ID);

	/**
	 * Gets a key value pair combination that defines a search parameter
	 *
	 * @param Key key for the setting
	 * @param Value value of the setting
	 *
	 * @return true if found, false otherwise
	 */
	bool Get(FName Key, TArray<FString>& Value) const;

	/**
	 * Gets a key value pair combination that defines a search parameter
	 *
	 * @param Key key for the setting
	 * @param Value value of the setting
	 *
	 * @return true if found, false otherwise
	 */
	bool Get(FName Key, TArray<double>& Value) const;

	/**
	 * Gets a key value pair combination that defines a search parameter
	 *
	 * @param SearchSettings search settings to fetch setting from
	 * @param Key key for the setting
	 * @param Value value of the setting
	 *
	 * @return true if found, false otherwise
	 */
	static bool Get(const FOnlineSearchSettings& SearchSettings, FName Key, TArray<FString>& Value);

	/**
	 * Gets a key value pair combination that defines a search parameter
	 *
	 * @param SearchSettings search settings to fetch setting from
	 * @param Key key for the setting
	 * @param Value value of the setting
	 *
	 * @return true if found, false otherwise
	 */
	static bool Get(const FOnlineSearchSettings& SearchSettings, FName Key, TArray<double>& Value);

	/**
	 * Gets an array-typed search setting value from variant data
	 *
	 * @param Data variant data to get the value from
	 * @param Value value of the setting
	 *
	 * @return a boolean indicating whether the value was retrieved
	 */
	static bool Get(const FVariantData& Data, TArray<FString>& Value);

	/**
	 * Gets an array-typed search setting value from variant data
	 *
	 * @param Data variant data to get the value from
	 * @param Value value of the setting
	 *
	 * @return a boolean indicating whether the value was retrieved
	 */
	static bool Get(const FVariantData& Data, TArray<double>& Value);

	/**
	 * Get a search setting double value as an integer. Search settings across the OSS are stored as a double. However,
	 * due to limitations with FVariantData, integers cannot be retrieved from these double values, even if they are
	 * within range.
	 *
	 * @param SearchSettings search settings object to fetch the setting from
	 * @param Key setting key that we want to read
	 * @param Value output value that will be returned to the caller
	 * @return boolean that is true if an integer could be grabbed, or false otherwise.
	 */
	template<typename T>
	static bool GetInt(const FOnlineSearchSettings& SearchSettings, FName Key, T& OutValue);

	/**
	 * Get the ESessionSettingsAccelByteArrayFieldType for a field
	 *
	 * @param SearchSettings search settings to get the type from
	 * @param Key the key for the setting
	 *
	 * @return an ESessionSettingsAccelByteArrayFieldType enum value
	 */
	static ESessionSettingsAccelByteArrayFieldType GetArrayFieldType(const FOnlineSearchSettings& SearchSettings, FName Key);

	/**
	 * Get the ESessionSettingsAccelByteArrayFieldType for a VariantData field
	 *
	 * @param Data variant data object to get the value from
	 *
	 * @return an ESessionSettingsAccelByteArrayFieldType enum value
	 */
	static ESessionSettingsAccelByteArrayFieldType GetArrayFieldType(const FVariantData& Data);
};

class ONLINESUBSYSTEMACCELBYTE_API FOnlineSessionSettingsAccelByte : public FOnlineSessionSettings
{
public:
	using FOnlineSessionSettings::Set;
	using FOnlineSessionSettings::Get;

	/**
	 * Sets a key value pair combination that defines a session setting with an ID
	 *
	 * @param Key key for the setting
	 * @param Value value of the setting
	 * @param InType type of online advertisement
	 * @param InID ID for this session setting
	 */
	void Set(FName Key, const TArray<FString>& Value, EOnlineDataAdvertisementType::Type InType, int32 InID);

	/**
	 * Sets a key value pair combination that defines a session setting with an ID
	 *
	 * @param Key key for the setting
	 * @param Value value of the setting
	 * @param InType type of online advertisement
	 * @param InID ID for this session setting
	 */
	void Set(FName Key, const TArray<double>& Value, EOnlineDataAdvertisementType::Type InType, int32 InID);

	/**
	 * Sets a key value pair combination that defines a session setting
	 *
	 * @param Key key for the setting
	 * @param Value value of the setting
	 * @param InType type of online advertisement
	 */
	void Set(FName Key, const TArray<FString>& Value, EOnlineDataAdvertisementType::Type InType);

	/**
	 * Sets a key value pair combination that defines a session setting
	 *
	 * @param Key key for the setting
	 * @param Value value of the setting
	 * @param InType type of online advertisement
	 */
	void Set(FName Key, const TArray<double>& Value, EOnlineDataAdvertisementType::Type InType);

	/**
	 * Sets a key value pair combination that defines a session setting
	 *
	 * @param Key key for the setting
	 * @param Value setting value
	 */
	void Set(FName Key, const TArray<FString>& Value);

	/**
	 * Sets a key value pair combination that defines a session setting
	 *
	 * @param Key key for the setting
	 * @param Value setting value
	 */
	void Set(FName Key, const TArray<double>& Value);

	/**
	 * Sets a key value pair combination that defines a session setting with an ID
	 *
	 * @param SessionSettings settings object to set the value in
	 * @param Key key for the setting
	 * @param Value value of the setting
	 * @param InType type of online advertisement
	 * @param InID ID for this session setting
	 */
	static void Set(FOnlineSessionSettings& SessionSettings, FName Key, const TArray<FString>& Value, EOnlineDataAdvertisementType::Type InType, int32 InID);

	/**
	 * Sets a key value pair combination that defines a session setting with an ID
	 *
	 * @param SessionSettings settings object to set the value in
	 * @param Key key for the setting
	 * @param Value value of the setting
	 * @param InType type of online advertisement
	 * @param InID ID for this session setting
	 */
	static void Set(FOnlineSessionSettings& SessionSettings, FName Key, const TArray<double>& Value, EOnlineDataAdvertisementType::Type InType, int32 InID);

	/**
	 * Sets a key value pair combination that defines a session setting
	 *
	 * @param SessionSettings settings object to set the value in
	 * @param Key key for the setting
	 * @param Value value of the setting
	 * @param InType type of online advertisement
	 */
	static void Set(FOnlineSessionSettings& SessionSettings, FName Key, const TArray<FString>& Value, EOnlineDataAdvertisementType::Type InType);

	/**
	 * Sets a key value pair combination that defines a session setting
	 *
	 * @param SessionSettings settings object to set the value in
	 * @param Key key for the setting
	 * @param Value value of the setting
	 * @param InType type of online advertisement
	 */
	static void Set(FOnlineSessionSettings& SessionSettings, FName Key, const TArray<double>& Value, EOnlineDataAdvertisementType::Type InType);

	/**
	 * Sets a key value pair combination that defines a session setting
	 *
	 * @param SessionSettings settings object to set the value in
	 * @param Key key for the setting
	 * @param Value setting value
	 */
	static void Set(FOnlineSessionSettings& SessionSettings, FName Key, const TArray<FString>& Value);

	/**
	 * Sets a key value pair combination that defines a session setting
	 *
	 * @param SessionSettings settings object to set the value in
	 * @param Key key for the setting
	 * @param Value setting value
	 */
	static void Set(FOnlineSessionSettings& SessionSettings, FName Key, const TArray<double>& Value);

	/**
	 * Gets a setting value by its name
	 *
	 * @param Key key for the setting
	 * @param OutValue setting value
	 *
	 * @return an boolean indicating whether the value was retrieved
	 */
	bool Get(FName Key, TArray<FString>& OutValue) const;

	/**
	 * Gets a setting value by its name
	 *
	 * @param Key key for the setting
	 * @param OutValue setting value
	 *
	 * @return an boolean indicating whether the value was retrieved
	 */
	bool Get(FName Key, TArray<double>& OutValue) const;

	/**
	 * Gets a setting value by its name
	 *
	 * @param SessionSettings settings object to get the value from
	 * @param Key key for the setting
	 * @param OutValue setting value
	 *
	 * @return an boolean indicating whether the value was retrieved
	 */
	static bool Get(const FOnlineSessionSettings& SessionSettings, FName Key, TArray<FString>& OutValue);

	/**
	 * Gets a setting value by its name
	 *
	 * @param SessionSettings settings object to get the value from
	 * @param Key key for the setting
	 * @param OutValue setting value
	 *
	 * @return an boolean indicating whether the value was retrieved
	 */
	static bool Get(const FOnlineSessionSettings& SessionSettings, FName Key, TArray<double>& OutValue);

	/**
	 * Get a session setting double value as an integer. Session settings across the OSS are stored as a double. However,
	 * due to limitations with FVariantData, integers cannot be retrieved from these double values, even if they are
	 * within range.
	 *
	 * @param SessionSettings session settings object to fetch the setting from
	 * @param Key setting key that we want to read
	 * @param Value output value that will be returned to the caller
	 * @return boolean that is true if an integer could be grabbed, or false otherwise.
	 */
	template<typename T>
	static bool GetInt(const FOnlineSessionSettings& SessionSettings, FName Key, T& OutValue);

	/**
	 * Get the ESessionSettingsAccelByteArrayFieldType for a field
	 *
	 * @param SessionSettings settings to get the type from
	 * @param Key the key for the setting
	 *
	 * @return an ESessionSettingsAccelByteArrayFieldType enum value
	 */
	static ESessionSettingsAccelByteArrayFieldType GetArrayFieldType(const FOnlineSessionSettings& SessionSettings, FName Key);
};
