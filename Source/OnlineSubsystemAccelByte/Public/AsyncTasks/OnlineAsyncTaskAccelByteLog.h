// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystemAccelByteLog.h"

#define AB_OSS_ASYNC_TASK_TRACE_BEGIN_VERBOSITY(Verbosity, Format, ...) UE_LOG_AB(Verbosity, TEXT(">>> %s::%s (AsyncTask method) was called. Args: ") Format, *GetTaskName(), *FString(__func__), ##__VA_ARGS__)
#define AB_OSS_ASYNC_TASK_TRACE_BEGIN(Format, ...) AB_OSS_ASYNC_TASK_TRACE_BEGIN_VERBOSITY(Verbose, Format, ##__VA_ARGS__)
#define AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Verbosity, Format, ...) UE_LOG_AB(Verbosity, TEXT("<<< %s::%s (AsyncTask method) has finished execution. ") Format, *GetTaskName(), *FString(__func__), ##__VA_ARGS__)
#define AB_OSS_ASYNC_TASK_TRACE_END(Format, ...) AB_OSS_ASYNC_TASK_TRACE_END_VERBOSITY(Verbose, Format, ##__VA_ARGS__)
