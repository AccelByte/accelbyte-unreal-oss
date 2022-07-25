// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#if ENGINE_MAJOR_VERSION >= 5
#include "Online/CoreOnline.h"
#else
#include "UObject/CoreOnline.h"
#endif

#if WITH_DEV_AUTOMATION_TESTS

/** Index of the user that any exec test OSS calls are using for testing */
#define TEST_USER_INDEX 0

/**
 * Simple base class for setting up a Exec console command test for OSS functions.
 * 
 * Should be subclassed by the test case you are trying to construct, grabbing the proper subsystem through the World
 * instance and SubsystemName.
 * 
 * Keep in mind that for simplicity, there is no passing in a local user identifier such as a LocalUserNum or a UserId,
 * for any operations that require these identifiers, zero is used as the player index.
 */
class FExecTestBase
{
public:

	/** Whether or not this exec test has completed its work */
	bool bIsComplete = false;

	/**
	 * Base constructor for any exec test, sets up the world and subsystem instances
	 *
	 * @param InWorld World instance that we want to use to get the subsystem instance from
	 * @param SubsystemName Name of the subsystem that we want to get from the world, usually will be ACCELBYTE_SUBSYSTEM
	 */
	FExecTestBase(UWorld* InWorld, const FName& InSubsystemName)
		: World(InWorld)
		, SubsystemName(InSubsystemName)
	{
	}

	virtual ~FExecTestBase() = default;

	/**
	 * Fires off all OSS calls that relate to this test case.
	 *
	 * @return a boolean that is true when OSS tasks have been fired and delegates are awaiting, or false if there was an
	 * issue and no tasks were fired
	 */
	virtual bool Run() { return true; };

protected:

	/** World associated with this exec test */
	UWorld* World;

	/** Name of the subsystem we want to use with this test */
	FName SubsystemName;

};

#endif
