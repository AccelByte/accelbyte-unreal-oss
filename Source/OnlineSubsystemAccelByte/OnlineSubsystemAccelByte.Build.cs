// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

using UnrealBuildTool;
using System.IO;
using Tools.DotNETCommon;

public class OnlineSubsystemAccelByte : ModuleRules
{
	private static bool IsPlatformEqual(UnrealTargetPlatform TargetPlatform, string Platform)
	{
		return TargetPlatform.ToString().Equals(Platform);
	}
	
	public OnlineSubsystemAccelByte(ReadOnlyTargetRules Target) : base(Target)
	{
		PrivateDefinitions.Add("ONLINESUBSYSTEMACCELBYTE_PACKAGE=1");
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		bAllowConfidentialPlatformDefines = true;
		
		PublicDependencyModuleNames.AddRange(new string[] {
			"OnlineSubsystemUtils", "AccelByteUe4Sdk"
		});
		
		PrivateDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Projects",
			"WebSockets",
			"NetCore",
			"Engine",
			"Sockets",
			"OnlineSubsystem",
			"PacketHandler",
			"Json",
			"JsonUtilities",
			"AccelByteUe4Sdk",
			"AccelByteNetworkUtilities",
			"Http"
		});
	}
}
