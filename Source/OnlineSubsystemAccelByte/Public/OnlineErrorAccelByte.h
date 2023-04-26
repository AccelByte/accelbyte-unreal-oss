// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "OnlineError.h"

class ONLINESUBSYSTEMACCELBYTE_API FOnlineErrorAccelByte :
    public FOnlineError
{
public:

    /**
     * @brief Create factory for FOnlineErrorAccelByte with specified ErrorKey and to get the Localized ErrorMessage as well
     */
    static FOnlineErrorAccelByte CreateError(const FString& ErrorNamespace, const FString& ErrorKey, EOnlineErrorResult Result = EOnlineErrorResult::RequestFailure);

    /**
     * @brief Create factory for FOnlineErrorAccelByte with specified ErrorCode and to get the Localized ErrorMessage as well
     */
    static FOnlineErrorAccelByte CreateError(const FString& ErrorNamespace, const int32 ErrorCode, EOnlineErrorResult Result = EOnlineErrorResult::RequestFailure);

    /**
     * @brief Create factory for FOnlineErrorAccelByte using FOnlineError object and to get the Localized ErrorMessage as well
     */
    static FOnlineErrorAccelByte CreateError(const FString& ErrorNamespace, FOnlineError ErrorInfo);

private:
    explicit FOnlineErrorAccelByte(EOnlineErrorResult InResult, const FString& InErrorCode, const FText& InErrorMessage);

    /**
     * @brief Get ErrorText from the Localization ErrorMessages String Table
     * 
     * @param ErrorKey the ErrorKey to search on the Table
     */
    static FText GetErrorText(const FString& ErrorKey);

    /**
     * @brief Get ErrorKey from the ErrorCodes String Table
     * 
     * @param ErrorCode the ErrorCode(usually from BE) to search on the Table
     */
    static FString GetErrorKey(const int32 ErrorCode);

    /**
     * @brief Register the String Tables, import from the existing csv files, only register the tables once every runtime
     */
    static void RegisterTables();

    /*
     * @brief flag to indicate whether the String Tables already registered or not, only register the tables once every runtime
     */
    static bool bIsTablesRegistered;

};

/** must be defined to a valid namespace for using ONLINE_ERROR factory macro */
#undef ONLINE_ERROR_NAMESPACE
#define ONLINE_ERROR_ACCELBYTE(...) FOnlineErrorAccelByte::CreateError(TEXT(ONLINE_ERROR_NAMESPACE), __VA_ARGS__)