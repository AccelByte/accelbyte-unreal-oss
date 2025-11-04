// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

/**
 * Header to be used internally by the AccelByte OSS housing macros and other such definitions helpful to the
 * development of the OSS. This header also houses the definitions for the trace logging macros used in each interface.
 */
#pragma once

#include "OnlineSubsystemAccelByteLog.h"

/**
 * Quick macro to be used in UE_LOG to output a boolean condition as a string for "true" or "false"
 * 
 * @param Condition condition to evaluate as true or false for text construction
 */
#define LOG_BOOL_FORMAT(Condition) ((Condition) ? TEXT("true") : TEXT("false"))

/**
  * Simple macro for logging a trace for when an interface method begins. Should only be called on interfaces with the parent
  * subsystem as a member named 'AccelByteSubsystem'.
  *
  * @param Verbosity Log verbosity for this trace ending, corresponds to the verbosity for UE_LOG
  * @param Format Same as a format string passed into UE_LOG, and as such must be wrapped with a TEXT macro.
  * @param Args Corresponds to the types set in Format, just like in UE_LOG
  */
#define AB_OSS_INTERFACE_TRACE_BEGIN_VERBOSITY(Verbosity, Format, ...) UE_LOG_AB(Verbosity, TEXT(">>> %s (%s) was called. Args: ") Format, *FString(__func__), *AccelByteSubsystem->GetInstanceName().ToString(), ##__VA_ARGS__)
#define AB_OSS_PTR_INTERFACE_TRACE_BEGIN_VERBOSITY(Verbosity, Format, ...) \
	UE_LOG_AB(Verbosity, TEXT(">>> %s (%s) was called. Args: ") Format \
		, *FString(__func__) \
		, AccelByteSubsystem.Pin().IsValid() ? *AccelByteSubsystem.Pin()->GetInstanceName().ToString() : TEXT("INVALID") \
		, ##__VA_ARGS__)

/**
 * Simple macro for logging a trace for when an interface method begins. Should only be called on interfaces with the parent
 * subsystem as a member named 'AccelByteSubsystem'. Same as AB_OSS_INTERFACE_TRACE_BEGIN_VERBOSITY, except this will
 * always use the Verbose verbosity.
 *
 * @param Format Same as a format string passed into UE_LOG, and as such must be wrapped with a TEXT macro.
 * @param Args Corresponds to the types set in Format, just like in UE_LOG
 */
#define AB_OSS_INTERFACE_TRACE_BEGIN(Format, ...) AB_OSS_INTERFACE_TRACE_BEGIN_VERBOSITY(Verbose, Format, ##__VA_ARGS__)
#define AB_OSS_PTR_INTERFACE_TRACE_BEGIN(Format, ...) AB_OSS_PTR_INTERFACE_TRACE_BEGIN_VERBOSITY(Verbose, Format, ##__VA_ARGS__)

/**
 * Simple macro for logging a trace for when an interface method has finished execution. Should only be called on interfaces with
 * the parent subsystem as a member named 'AccelByteSubsystem'.
 *
 * @param Verbosity Log verbosity for this trace ending, corresponds to the verbosity for UE_LOG
 * @param Format Same as a format string passed into UE_LOG, and as such must be wrapped with a TEXT macro.
 * @param Args Corresponds to the types set in Format, just like in UE_LOG
 */
#define AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Verbosity, Format, ...) UE_LOG_AB(Verbosity, TEXT("<<< %s (%s) has finished execution. ") Format, *FString(__func__), *AccelByteSubsystem->GetInstanceName().ToString(), ##__VA_ARGS__)
#define AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Verbosity, Format, ...) \
	UE_LOG_AB(Verbosity, TEXT("<<< %s (%s) has finished execution. ") Format \
		, *FString(__func__) \
		, AccelByteSubsystem.Pin().IsValid() ? *AccelByteSubsystem.Pin()->GetInstanceName().ToString() : TEXT("INVALID") \
		, ##__VA_ARGS__)

/**
 * Macro for logging a trace when an interface method has finished execution. Same as AB_OSS_INTERFACE_TRACE_END_VERBOSITY, except this will
 * always use the Verbose verbosity. Should only be called on interfaces with the parent subsystem as a member named
 * 'AccelByteSubsystem'.
 *
 * @param Format Same as a format string passed into UE_LOG, and as such must be wrapped with a TEXT macro.
 * @param Args Corresponds to the types set in Format, just like in UE_LOG
 */
#define AB_OSS_INTERFACE_TRACE_END(Format, ...) AB_OSS_INTERFACE_TRACE_END_VERBOSITY(Verbose, Format, ##__VA_ARGS__)
#define AB_OSS_PTR_INTERFACE_TRACE_END(Format, ...) AB_OSS_PTR_INTERFACE_TRACE_END_VERBOSITY(Verbose, Format, ##__VA_ARGS__)

/**
  * Simple macro for logging a trace for when a method begins in a class that doesn't have a subsystem instance attached.
  *
  * @param Verbosity Log verbosity for this trace ending, corresponds to the verbosity for UE_LOG
  * @param Format Same as a format string passed into UE_LOG, and as such must be wrapped with a TEXT macro.
  * @param Args Corresponds to the types set in Format, just like in UE_LOG
  */
#define AB_OSS_GENERIC_TRACE_BEGIN_VERBOSITY(Verbosity, Format, ...) UE_LOG_AB(Verbosity, TEXT(">>> %s was called. Args: ") Format, *FString(__func__), ##__VA_ARGS__)

/**
 * Simple macro for logging a trace for when an method begins in a class that doesn't have a subsystem instance attached.
 * Same as AB_OSS_GENERIC_TRACE_BEGIN_VERBOSITY, except this will always use the Verbose verbosity.
 *
 * @param Format Same as a format string passed into UE_LOG, and as such must be wrapped with a TEXT macro.
 * @param Args Corresponds to the types set in Format, just like in UE_LOG
 */
#define AB_OSS_GENERIC_TRACE_BEGIN(Format, ...) AB_OSS_GENERIC_TRACE_BEGIN_VERBOSITY(Verbose, Format, ##__VA_ARGS__)

/**
 * Simple macro for logging a trace for when a method has finished execution inside of a class without a subsystem
 * instance attached.
 *
 * @param Verbosity Log verbosity for this trace ending, corresponds to the verbosity for UE_LOG
 * @param Format Same as a format string passed into UE_LOG, and as such must be wrapped with a TEXT macro.
 * @param Args Corresponds to the types set in Format, just like in UE_LOG
 */
#define AB_OSS_GENERIC_TRACE_END_VERBOSITY(Verbosity, Format, ...) UE_LOG_AB(Verbosity, TEXT("<<< %s has finished execution. ") Format, *FString(__func__), ##__VA_ARGS__)

/**
 * Macro for logging a trace when a method has finished execution in a class without a subsystem instance attached.
 * Same as AB_OSS_GENERIC_TRACE_END_VERBOSITY, except this will always use the Verbose verbosity.
 *
 * @param Format Same as a format string passed into UE_LOG, and as such must be wrapped with a TEXT macro.
 * @param Args Corresponds to the types set in Format, just like in UE_LOG
 */
#define AB_OSS_GENERIC_TRACE_END(Format, ...) AB_OSS_GENERIC_TRACE_END_VERBOSITY(Verbose, Format, ##__VA_ARGS__)
