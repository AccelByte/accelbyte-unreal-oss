// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "OnlineSubsystem.h"

/**
 * Module for the AccelByte Online Subsystem. Handles creating a new IOnlineFactory instance for the AccelByte
 * subsystem and registering and unregistering the factory with the global online subsystem module.
 */
class ONLINESUBSYSTEMACCELBYTE_API FOnlineSubsystemAccelByteModule : public IModuleInterface
{
private:

	/** Class responsible for creating instance(s) of the subsystem */
	TUniquePtr<IOnlineFactory> AccelByteFactory;

public:

	FOnlineSubsystemAccelByteModule() :
		AccelByteFactory(nullptr)
	{
	}

	virtual ~FOnlineSubsystemAccelByteModule() = default;

	//~ Begin IModuleInterface
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	virtual bool SupportsDynamicReloading() override
	{
		return false;
	}

	virtual bool SupportsAutomaticShutdown() override
	{
		return false;
	}
	//~ End IModuleInterface
};
