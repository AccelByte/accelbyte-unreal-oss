// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.

#include "Blueprints/OnlineSubsystemAccelByteBlueprint.h"

#include "OnlineSubsystemAccelByte.h"
#include "OnlineSubsystemUtils.h"

UAccelByteInstance* UOnlineSubsystemAccelByteBlueprint::GetAccelByteInstance(UObject* WorldContextObject)
{
	if(WorldContextObject == nullptr)
	{
		return  nullptr;
	}

	const UWorld* World = WorldContextObject->GetWorld();
	if(World == nullptr)
	{
		return nullptr;
	}

	IOnlineSubsystem* Subsystem = Online::GetSubsystem(World);
	FOnlineSubsystemAccelByte* SubsystemAccelByte = static_cast<FOnlineSubsystemAccelByte*>(Subsystem);
	
	UAccelByteInstance* InstancePtr = NewObject<UAccelByteInstance>();
	InstancePtr->SetAccelByteInstance(SubsystemAccelByte->GetAccelByteInstance().Pin());

	return InstancePtr;
}
