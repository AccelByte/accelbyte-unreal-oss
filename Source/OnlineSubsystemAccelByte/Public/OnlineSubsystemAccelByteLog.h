// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "CoreMinimal.h"

/** Log category for any AccelByte OSS logs, including traces */
DECLARE_LOG_CATEGORY_EXTERN(LogAccelByteOSS, Warning, All);

/** Convenience UE_LOG macro that will automatically log to LogAccelByteOSS with the specified Verbosity and Format. See UE_LOG for usage. */
#define UE_LOG_AB(Verbosity, Format, ...) UE_LOG(LogAccelByteOSS, Verbosity, Format, ##__VA_ARGS__)
