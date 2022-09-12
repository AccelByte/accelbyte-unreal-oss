// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineSessionSettingsAccelByte.h"

#include "OnlineSubsystemAccelByte.h"

constexpr auto DATA_OFFSET = sizeof(uint8);

#pragma region Conversion utility functions

static void ConvertArrayToBytes(const TArray<FString>& InArray, TArray<uint8>& OutArray)
{
	// Prefix the output bytes with the data type
	OutArray.Add(StaticCast<uint8>(ESessionSettingsAccelByteArrayFieldType::STRINGS));

	TArray<uint8>::SizeType ByteIndex = DATA_OFFSET;
	for(const auto& String : InArray)
	{
		const auto Bytes = String.Len() * sizeof(TCHAR);

		// Copy the string to the output bytes array
		OutArray.AddUninitialized(Bytes + sizeof(TCHAR));
		FMemory::Memcpy(OutArray.GetData() + ByteIndex, *String, Bytes);
		ByteIndex += Bytes;

		// Insert a TCHAR with value 0 to delineate the end of each string in the array 
		FMemory::Memset(OutArray.GetData() + ByteIndex, 0, sizeof(TCHAR));
		ByteIndex += sizeof(TCHAR);
	}
}

static void ConvertArrayToBytes(const TArray<double>& InArray, TArray<uint8>& OutArray)
{
	// Prefix the output bytes with the data type
	OutArray.Add(StaticCast<uint8>(ESessionSettingsAccelByteArrayFieldType::DOUBLES));
	// Add enough fields to serialize all of the values in InArray
	OutArray.AddUninitialized( sizeof(double) * InArray.Num());

	TArray<uint8>::SizeType ByteIndex = DATA_OFFSET;
	for(const auto& Number : InArray)
	{
		FMemory::Memcpy(OutArray.GetData() + ByteIndex, &Number, sizeof(double));
		ByteIndex += sizeof(double);
	}
}

static bool ConvertBytesToArray(const TArray<uint8>& InArray, TArray<FString>& OutArray)
{
	if(InArray.Num() < DATA_OFFSET)
	{
		return false;
	}

	const auto DataType = StaticCast<ESessionSettingsAccelByteArrayFieldType>(InArray[0]);
	const auto DataSize = InArray.Num() - DATA_OFFSET;

	// Ensure that the data type field is correct, and that the number of bytes is evenly divisible by the number of
	// bytes in a TCHAR to avoid reading outside of the bounds of InArray
	if(DataType != ESessionSettingsAccelByteArrayFieldType::STRINGS || DataSize % sizeof(TCHAR) != 0)
	{
		return false;
	}

	FString String = TEXT("");
	for(TArray<uint8>::SizeType i = DATA_OFFSET; i < DataSize + DATA_OFFSET; i += sizeof(TCHAR))
	{
		TCHAR Char;
		FMemory::Memcpy(&Char, InArray.GetData() + i, sizeof(TCHAR));

		// When serializing, we inserted a TCHAR with value 0 to separate the array items
		if(Char == 0)
		{
			OutArray.Add(String);
			String = TEXT("");
		}
		else
		{
			String.AppendChar(Char);
		}
	}

	return true;
}

static bool ConvertBytesToArray(const TArray<uint8>& InArray, TArray<double>& OutArray)
{
	if(InArray.Num() < DATA_OFFSET)
	{
		return false;
	}

	const auto DataType = StaticCast<ESessionSettingsAccelByteArrayFieldType>(InArray[0]);
	const auto DataSize = InArray.Num() - DATA_OFFSET;

	// Ensure that the data type field is correct, and that the number of bytes is evenly divisible by the number of
	// bytes in a double to avoid reading outside of the bounds of InArray
	if(DataType != ESessionSettingsAccelByteArrayFieldType::DOUBLES || DataSize % sizeof(double) != 0)
	{
		return false;
	}

	for(TArray<uint8>::SizeType i = DATA_OFFSET; i < DataSize + DATA_OFFSET; i += sizeof(double))
	{
		double Number;
		FMemory::Memcpy(&Number, InArray.GetData() + i, sizeof(double));
		OutArray.Add(Number);
	}

	return true;
}

#pragma endregion

void FOnlineSearchSettingsAccelByte::Set(FName Key, const TArray<FString>& Value, EOnlineComparisonOp::Type InType, int32 ID)
{
	Set(*this, Key, Value, InType, ID);
}

void FOnlineSearchSettingsAccelByte::Set(FName Key, const TArray<double>& Value, EOnlineComparisonOp::Type InType, int32 ID)
{
	Set(*this, Key, Value, InType, ID);
}

void FOnlineSearchSettingsAccelByte::Set(FName Key, const TArray<FString>& Value, EOnlineComparisonOp::Type InType)
{
	Set(*this, Key, Value, InType);
}

void FOnlineSearchSettingsAccelByte::Set(FName Key, const TArray<double>& Value, EOnlineComparisonOp::Type InType)
{
	Set(*this, Key, Value, InType);
}

void FOnlineSearchSettingsAccelByte::Set(FOnlineSearchSettings& SearchSettings, FName Key, const TArray<FString>& Value, EOnlineComparisonOp::Type InType)
{
	TArray<uint8> ResultArray;
	ConvertArrayToBytes(Value, ResultArray);
	SearchSettings.Set(Key, ResultArray, InType);
}

void FOnlineSearchSettingsAccelByte::Set(FOnlineSearchSettings& SearchSettings, FName Key, const TArray<double>& Value,	EOnlineComparisonOp::Type InType)
{
	TArray<uint8> ResultArray;
	ConvertArrayToBytes(Value, ResultArray);
	SearchSettings.Set(Key, ResultArray, InType);
}

void FOnlineSearchSettingsAccelByte::Set(FOnlineSearchSettings& SearchSettings, FName Key, const TArray<FString>& Value, EOnlineComparisonOp::Type InType, int32 ID)
{
	TArray<uint8> ResultArray;
	ConvertArrayToBytes(Value, ResultArray);
	SearchSettings.Set(Key, ResultArray, InType, ID);
}

void FOnlineSearchSettingsAccelByte::Set(FOnlineSearchSettings& SearchSettings, FName Key, const TArray<double>& Value,	EOnlineComparisonOp::Type InType, int32 ID)
{
	TArray<uint8> ResultArray;
	ConvertArrayToBytes(Value, ResultArray);
	SearchSettings.Set(Key, ResultArray, InType, ID);
}

bool FOnlineSearchSettingsAccelByte::Get(FName Key, TArray<FString>& Value) const
{
	return Get(*this, Key, Value);
}

bool FOnlineSearchSettingsAccelByte::Get(FName Key, TArray<double>& Value) const
{
	return Get(*this, Key, Value);
}

bool FOnlineSearchSettingsAccelByte::Get(const FOnlineSearchSettings& SearchSettings, FName Key, TArray<FString>& Value)
{
	TArray<uint8> RawArray;
	if(!SearchSettings.Get(Key, RawArray))
	{
		return false;
	}

	return ConvertBytesToArray(RawArray, Value);
}

bool FOnlineSearchSettingsAccelByte::Get(const FOnlineSearchSettings& SearchSettings, FName Key, TArray<double>& Value)
{
	TArray<uint8> RawArray;
	if(!SearchSettings.Get(Key, RawArray))
	{
		return false;
	}

	return ConvertBytesToArray(RawArray, Value);
}

bool FOnlineSearchSettingsAccelByte::Get(const FVariantData& Data, TArray<FString>& Value)
{
	TArray<uint8> RawArray;
	Data.GetValue(RawArray);
	return ConvertBytesToArray(RawArray, Value);
}

bool FOnlineSearchSettingsAccelByte::Get(const FVariantData& Data, TArray<double>& Value)
{
	TArray<uint8> RawArray;
	Data.GetValue(RawArray);
	return ConvertBytesToArray(RawArray, Value);
}

ESessionSettingsAccelByteArrayFieldType FOnlineSearchSettingsAccelByte::GetArrayFieldType(const FOnlineSearchSettings& SearchSettings, FName Key)
{
	TArray<uint8> RawArray;
	if(!SearchSettings.Get(Key, RawArray) || RawArray.Num() < DATA_OFFSET)
	{
		return ESessionSettingsAccelByteArrayFieldType::INVALID;
	}

	return StaticCast<ESessionSettingsAccelByteArrayFieldType>(RawArray[0]);
}

ESessionSettingsAccelByteArrayFieldType FOnlineSearchSettingsAccelByte::GetArrayFieldType(const FVariantData& Data)
{
	TArray<uint8> RawArray;
	Data.GetValue(RawArray);
	if(RawArray.Num() < DATA_OFFSET)
	{
		return ESessionSettingsAccelByteArrayFieldType::INVALID;
	}

	return StaticCast<ESessionSettingsAccelByteArrayFieldType>(RawArray[0]);
}

void FOnlineSessionSettingsAccelByte::Set(FName Key, const TArray<FString>& Value, EOnlineDataAdvertisementType::Type InType, int32 InID)
{
	return Set(*this, Key, Value, InType, InID);
}

void FOnlineSessionSettingsAccelByte::Set(FName Key, const TArray<double>& Value, EOnlineDataAdvertisementType::Type InType, int32 InID)
{
	return Set(*this, Key, Value, InType, InID);
}

void FOnlineSessionSettingsAccelByte::Set(FName Key, const TArray<FString>& Value, EOnlineDataAdvertisementType::Type InType)
{
	return Set(*this, Key, Value, InType);
}

void FOnlineSessionSettingsAccelByte::Set(FName Key, const TArray<double>& Value, EOnlineDataAdvertisementType::Type InType)
{
	return Set(*this, Key, Value, InType);
}

void FOnlineSessionSettingsAccelByte::Set(FName Key, const TArray<FString>& Value)
{
	Set(*this, Key, Value);
}

void FOnlineSessionSettingsAccelByte::Set(FName Key, const TArray<double>& Value)
{
	Set(*this, Key, Value);
}

void FOnlineSessionSettingsAccelByte::Set(FOnlineSessionSettings& SessionSettings, FName Key, const TArray<FString>& Value, EOnlineDataAdvertisementType::Type InType, int32 InID)
{
	TArray<uint8> ResultArray;
	ConvertArrayToBytes(Value, ResultArray);
	SessionSettings.Set(Key, ResultArray, InType, InID);
}

void FOnlineSessionSettingsAccelByte::Set(FOnlineSessionSettings& SessionSettings, FName Key, const TArray<double>& Value, EOnlineDataAdvertisementType::Type InType, int32 InID)
{
	TArray<uint8> ResultArray;
	ConvertArrayToBytes(Value, ResultArray);
	SessionSettings.Set(Key, ResultArray, InType, InID);
}

void FOnlineSessionSettingsAccelByte::Set(FOnlineSessionSettings& SessionSettings, FName Key, const TArray<FString>& Value, EOnlineDataAdvertisementType::Type InType)
{
	TArray<uint8> ResultArray;
	ConvertArrayToBytes(Value, ResultArray);
	SessionSettings.Set(Key, ResultArray, InType);
}

void FOnlineSessionSettingsAccelByte::Set(FOnlineSessionSettings& SessionSettings, FName Key, const TArray<double>& Value, EOnlineDataAdvertisementType::Type InType)
{
	TArray<uint8> ResultArray;
	ConvertArrayToBytes(Value, ResultArray);
	SessionSettings.Set(Key, ResultArray, InType);
}

void FOnlineSessionSettingsAccelByte::Set(FOnlineSessionSettings& SessionSettings, FName Key, const TArray<FString>& Value)
{
	TArray<uint8> ResultArray;
	ConvertArrayToBytes(Value, ResultArray);
	SessionSettings.Set(Key, ResultArray);
}

void FOnlineSessionSettingsAccelByte::Set(FOnlineSessionSettings& SessionSettings, FName Key, const TArray<double>& Value)
{
	TArray<uint8> RawArray;
	ConvertArrayToBytes(Value, RawArray);
	SessionSettings.Set(Key, RawArray);
}

bool FOnlineSessionSettingsAccelByte::Get(FName Key, TArray<FString>& OutValue) const
{
	return Get(*this, Key, OutValue);
}

bool FOnlineSessionSettingsAccelByte::Get(FName Key, TArray<double>& OutValue) const
{
	return Get(*this, Key, OutValue);
}

bool FOnlineSessionSettingsAccelByte::Get(const FOnlineSessionSettings& SessionSettings, FName Key, TArray<FString>& OutValue)
{
	TArray<uint8> RawArray;
	if(!SessionSettings.Get(Key, RawArray))
	{
		return false;
	}

	return ConvertBytesToArray(RawArray, OutValue);
}

bool FOnlineSessionSettingsAccelByte::Get(const FOnlineSessionSettings& SessionSettings, FName Key, TArray<double>& OutValue)
{
	TArray<uint8> RawArray;
	if(!SessionSettings.Get(Key, RawArray))
	{
		return false;
	}

	return ConvertBytesToArray(RawArray, OutValue);
}

ESessionSettingsAccelByteArrayFieldType FOnlineSessionSettingsAccelByte::GetArrayFieldType(const FOnlineSessionSettings& SessionSettings, FName Key)
{
	TArray<uint8> RawArray;
	if(!SessionSettings.Get(Key, RawArray) || RawArray.Num() < DATA_OFFSET)
	{
		return ESessionSettingsAccelByteArrayFieldType::INVALID;
	}

	return StaticCast<ESessionSettingsAccelByteArrayFieldType>(RawArray[0]);
}
