// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "OnlineSubsystemAccelByteModule.h"
#include "OnlineSubsystemModule.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineSubsystemAccelByteDefines.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"

DEFINE_LOG_CATEGORY(LogAccelByteOSS);
DEFINE_LOG_CATEGORY(LogAccelByteOSSParty);

IMPLEMENT_MODULE(FOnlineSubsystemAccelByteModule, OnlineSubsystemAccelByte);

/**
 * Subclass of IOnlineFactory used for creating instances of FOnlineSubsystemAccelByte. Registered with the
 * OnlineSubsystem module for the engine to handle.
 */
class FOnlineFactoryAccelByte : public IOnlineFactory
{
public:

	virtual ~FOnlineFactoryAccelByte() = default;

	virtual IOnlineSubsystemPtr CreateSubsystem(FName InstanceName) override
	{
		FOnlineSubsystemAccelBytePtr OnlineSubPtr = MakeShared<FOnlineSubsystemAccelByte, ESPMode::ThreadSafe>(InstanceName);
		
		if (OnlineSubPtr->IsEnabled())
		{
			if (!OnlineSubPtr->Init())
			{
				UE_LOG_AB(Error, TEXT("AccelByte API failed to initialize!"));
				OnlineSubPtr->Shutdown();
				OnlineSubPtr.Reset();
			}
			else
			{
				UE_LOG_AB(Log, TEXT("AccelByte API initialized!"));
			}
		}
		else
		{
			UE_LOG_AB(Warning, TEXT("AccelByte API disabled!"));
			OnlineSubPtr->Shutdown();
			OnlineSubPtr.Reset();
		}

		return OnlineSubPtr;
	}

};

void FOnlineSubsystemAccelByteModule::StartupModule()
{
	UE_LOG_AB(Log, TEXT("Starting OnlineSubsystemAccelByte module!"));

	// Create an instance of the subsystem instance creation factory and register it with the OnlineSubsystem module
	AccelByteFactory = MakeUnique<FOnlineFactoryAccelByte>();
	FOnlineSubsystemModule& OSS = FModuleManager::GetModuleChecked<FOnlineSubsystemModule>("OnlineSubsystem");
	OSS.RegisterPlatformService(ACCELBYTE_SUBSYSTEM, AccelByteFactory.Get());
}

void FOnlineSubsystemAccelByteModule::ShutdownModule()
{
	UE_LOG_AB(Log, TEXT("Shutting down OnlineSubsystemAccelByte module!"));

	// Unregister our subsystem factory from the OnlineSubsystem module and reset our instance
	FOnlineSubsystemModule& OSS = FModuleManager::GetModuleChecked<FOnlineSubsystemModule>("OnlineSubsystem");
	OSS.UnregisterPlatformService(ACCELBYTE_SUBSYSTEM);
	AccelByteFactory.Reset();
}
