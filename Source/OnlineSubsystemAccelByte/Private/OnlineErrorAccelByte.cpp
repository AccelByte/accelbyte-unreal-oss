// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineErrorAccelByte.h"
#include "OnlineSubsystemAccelByteUtils.h"
#include "Internationalization/Text.h"
#include "Internationalization/StringTableRegistry.h"
#include "Interfaces/IPluginManager.h"
#include "Internationalization/Internationalization.h"
#include "Internationalization/Culture.h"
#include "Internationalization/StringTableCore.h"
#include "Misc/Paths.h"

#define ACCELBYTE_ERROR_CODE_TABLE_ID "ErrorCode"
#define ACCELBYTE_ERROR_CODE_TABLE_NAMESPACE "ErrorCodeStringTable"
#define ACCELBYTE_ERROR_KEY_TABLE_ID "ErrorKey"
#define ACCELBYTE_ERROR_KEY_TABLE_NAMESPACE "ErrorKeyStringTable"
#define ACCELBYTE_DEFAULT_ERROR_KEY_TABLE_ID "DefaultErrorKey"
#define ACCELBYTE_DEFAULT_ERROR_KEY_TABLE_NAMESPACE "DefaultErrorKeyStringTable"

/** Creates and registers a new string table instance, loading strings from the given file (the path is relative to the AccelByteOSS plugin's content directory). */
#define LOCTABLE_FROMFILE_ACCELBYTE(ID, NAMESPACE, FILEPATH) \
	FStringTableRegistry::Get().Internal_LocTableFromFile(TEXT(ID), TEXT(NAMESPACE), FILEPATH, IPluginManager::Get().FindPlugin("OnlineSubsystemAccelByte")->GetContentDir())

FOnlineErrorAccelByte::FOnlineErrorAccelByte(EOnlineErrorResult InResult, const FString& InErrorCode, const FText& InErrorMessage)
{
	Result = InResult;
	SetFromErrorCode(Result, InErrorCode, InErrorMessage);
}

bool FOnlineErrorAccelByte::bIsTablesRegistered = false;
TCHAR const* FOnlineErrorAccelByte::DefaultLanguage = TEXT("en");
FString FOnlineErrorAccelByte::Language = TEXT("");

FOnlineErrorAccelByte FOnlineErrorAccelByte::CreateError(const FString& ErrorNamespace, const int32 ErrorCode, EOnlineErrorResult Result)
{
	FString ErrorKey = GetErrorKey(ErrorCode);
	if (ErrorKey.Equals(*FStringTableEntry::GetPlaceholderSourceString()))
	{
		ErrorKey = DefaultErrorCode(Result);
	}

	return CreateError(ErrorNamespace, ErrorKey, Result);
}

FOnlineErrorAccelByte FOnlineErrorAccelByte::CreateError(const FString& ErrorNamespace, const FString& ErrorKey, EOnlineErrorResult Result)
{
	FText ErrorMessage = GetErrorText(ErrorKey);
	if (ErrorMessage.EqualTo(FText::FromString(*FStringTableEntry::GetPlaceholderSourceString())))
	{
		ErrorMessage = DefaultErrorMsg(Result);
	}

	FOnlineErrorAccelByte Error(Result, ErrorKey, ErrorMessage);
	if (!Error.GetErrorCode().IsEmpty() && !Error.GetErrorCode().Contains(ErrorNamespace + TEXT(".")))
	{
		FString Namespace(ErrorNamespace.IsEmpty() ? GetDefaultErrorNamespace() : ErrorNamespace);
		Namespace.Append(TEXT("."));

		// namespace the error code if not using a backend error
		Error.SetErrorCode(Namespace + Error.GetErrorCode());
	}

	return Error;
}

FOnlineErrorAccelByte FOnlineErrorAccelByte::CreateError(const FString& ErrorNamespace, FOnlineError ErrorInfo)
{
	TSharedPtr<FOnlineErrorAccelByte> Error = nullptr;
	if (ErrorInfo.GetErrorCode().IsNumeric())
	{
		const int32 ErrorCode = FCString::Atoi(*ErrorInfo.GetErrorCode());
		Error = MakeShared<FOnlineErrorAccelByte>(CreateError(ErrorNamespace, ErrorCode, ErrorInfo.GetErrorResult()));
	}
	else
	{
		Error = MakeShared<FOnlineErrorAccelByte>(CreateError(ErrorNamespace, ErrorInfo.GetErrorCode(), ErrorInfo.GetErrorResult()));
	}

	if (Error->GetErrorMessage().IsEmptyOrWhitespace())
	{
		Error->ErrorMessage = ErrorInfo.GetErrorMessage();
	}

	return *Error.Get();
}

FString FOnlineErrorAccelByte::PublicGetErrorKey(const int32 ErrorCode, const FString& DefaultErrorKey)
{
	FString ErrorKey = GetErrorKey(ErrorCode);
	if (ErrorKey.Equals(*FStringTableEntry::GetPlaceholderSourceString()))
	{
		ErrorKey = DefaultErrorKey.IsEmpty()? DefaultErrorCode(EOnlineErrorResult::Unknown) : DefaultErrorKey;
	}
	return ErrorKey;
}

FText FOnlineErrorAccelByte::GetErrorText(const FString& ErrorKey)
{
	RegisterTables();
	FText ErrorText = FText::FromStringTable(ACCELBYTE_ERROR_KEY_TABLE_ID, ErrorKey);
	if (Language != DefaultLanguage)
	{
		if (ErrorText.EqualTo(FText::FromString(*FStringTableEntry::GetPlaceholderSourceString())))
		{
			ErrorText = FText::FromStringTable(ACCELBYTE_DEFAULT_ERROR_KEY_TABLE_ID, ErrorKey);
		}
	}
	return ErrorText;
}

FString FOnlineErrorAccelByte::GetErrorKey(const int32 ErrorCode)
{
	RegisterTables();
	FString ErrorKey = FOnlineSubsystemAccelByteUtils::GetStringFromStringTable(ACCELBYTE_ERROR_CODE_TABLE_ID, ErrorCode);
	return ErrorKey; 
}

void FOnlineErrorAccelByte::RegisterTables()
{
	if (!bIsTablesRegistered)
	{
		const FString ContentDir = IPluginManager::Get().FindPlugin("OnlineSubsystemAccelByte")->GetContentDir();
		Language = FInternationalization::Get().GetCurrentLanguage()->GetTwoLetterISOLanguageName();
		if (!FPaths::FileExists(FString::Printf(TEXT("%s/Localization/AccelByteErrorMessages_%s.csv"), *ContentDir, *Language)))
		{
			Language = DefaultLanguage;
		}
		const FString LocalizationPath = FString::Printf(TEXT("Localization/AccelByteErrorMessages_%s.csv"), *Language);
		const FString DefaultLangPath = FString::Printf(TEXT("Localization/AccelByteErrorMessages_%s.csv"), DefaultLanguage);

		LOCTABLE_FROMFILE_ACCELBYTE(ACCELBYTE_DEFAULT_ERROR_KEY_TABLE_ID, ACCELBYTE_DEFAULT_ERROR_KEY_TABLE_NAMESPACE, DefaultLangPath);
		LOCTABLE_FROMFILE_ACCELBYTE(ACCELBYTE_ERROR_KEY_TABLE_ID, ACCELBYTE_ERROR_KEY_TABLE_NAMESPACE, LocalizationPath);
		LOCTABLE_FROMFILE_ACCELBYTE(ACCELBYTE_ERROR_CODE_TABLE_ID, ACCELBYTE_ERROR_CODE_TABLE_NAMESPACE, TEXT("AccelByteErrorCodes.csv"));
		bIsTablesRegistered = true;
	}
}
