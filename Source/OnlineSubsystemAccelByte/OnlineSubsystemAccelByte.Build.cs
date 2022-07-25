// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

using UnrealBuildTool;
using System.IO;

#if UE_5_0_OR_LATER
using EpicGames.Core;
#else
using Tools.DotNETCommon;
#endif

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
			"OnlineSubsystemUtils",
			"AccelByteUe4Sdk"
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
			"HTTP"
		});

		bool bEnableV2Sessions = false;
		GetBoolFromEngineConfig("OnlineSubsystemAccelByte", "bEnableV2Sessions", out bEnableV2Sessions);
		PublicDefinitions.Add(string.Format("AB_USE_V2_SESSIONS={0}", bEnableV2Sessions ? 1 : 0));
    }

	private bool GetBoolFromEngineConfig(string Section, string Key, out bool Value)
	{
		ConfigHierarchy EngineConfig = ConfigCache.ReadHierarchy(ConfigHierarchyType.Engine,
			DirectoryReference.FromFile(Target.ProjectFile),
			Target.Platform);

		if (EngineConfig == null)
		{
            Value = false;
            return false;
		}

		bool TempValue = false;
		if (!EngineConfig.TryGetValue(Section, Key, out TempValue))
		{
			Value = false;
			return false;
		}

		Value = TempValue;
		return true;
	}
}
