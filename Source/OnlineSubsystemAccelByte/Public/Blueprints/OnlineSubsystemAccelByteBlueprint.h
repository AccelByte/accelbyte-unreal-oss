// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Blueprints/AccelByteInstanceBlueprints.h"
#include "UObject/Object.h"

#include "OnlineSubsystemAccelByteBlueprint.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class ONLINESUBSYSTEMACCELBYTE_API UOnlineSubsystemAccelByteBlueprint : public UObject
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "AccelByte | OnlineSubsystemAccelByte", meta = (WorldContext="WorldContextObject"))
	static UAccelByteInstance* GetAccelByteInstance(UObject* WorldContextObject);
};
