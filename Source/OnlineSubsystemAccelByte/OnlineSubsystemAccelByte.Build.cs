// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

using UnrealBuildTool;
using System;
using System.IO;
using System.Diagnostics;

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

		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
#if UE_5_2_OR_LATER
		IWYUSupport = IWYUSupport.Full;
#else
		bEnforceIWYU = true;
#endif
		bAllowConfidentialPlatformDefines = true;

		PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private"));

		PublicDependencyModuleNames.AddRange(new string[] {
			"OnlineSubsystemUtils",
			"AccelByteUe4Sdk",
			"AccelByteNetworkUtilities"
		});

        if ((
#if !UE_5_0_OR_LATER
            Target.Platform == UnrealTargetPlatform.Win32 ||
#endif
            Target.Platform == UnrealTargetPlatform.Win64 ||
            Target.Platform == UnrealTargetPlatform.Linux ||
            Target.Platform == UnrealTargetPlatform.Mac)
			&& Target.Type != TargetType.Server)
        {
            PublicDependencyModuleNames.Add("Steamworks");
        }

        PrivateDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Projects",
			"WebSockets",
			"NetCore",
			"Engine",
			"Sockets",
			"Voice",
			"VoiceChat",
			"OnlineSubsystem",
			"PacketHandler",
			"Json",
			"JsonUtilities",
			"AccelByteUe4Sdk",
			"AccelByteNetworkUtilities",
			"HTTP"
		});

#if UE_5_1_OR_LATER
		PrivateDependencyModuleNames.AddRange(new string[] {
			"OnlineBase"
		});
#endif

		bool bEnableV2Sessions = false;
		GetBoolFromEngineConfig("OnlineSubsystemAccelByte", "bEnableV2Sessions", out bEnableV2Sessions);
		PublicDefinitions.Add(string.Format("AB_USE_V2_SESSIONS={0}", bEnableV2Sessions ? 1 : 0));

		string TargetPlatformName = Target.Platform.ToString().ToUpper();
		if (TargetPlatformName == "PS5")
		{
			PrivateDependencyModuleNames.AddRange(new string[] {
				"OnlineSubsystemPS5",
			});
		}

		if (TargetPlatformName == "XSX")
		{
			PrivateDependencyModuleNames.AddRange(new string[] {
				"OnlineSubsystemGDK",
			});
		}

		if (string.IsNullOrEmpty(Environment.GetEnvironmentVariable("BuildDocs"))) return;

		if (BuildDocs(ModuleDirectory))
		{
			Console.WriteLine("AccelByteDocsBuilder: documentation built successfully");
		}
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

		private static bool BuildDocs(string ModuleDirectory)
	{
		try
		{
			using (var Process = new Process())
			{
				string ScriptPath = Path.Combine(ModuleDirectory, @"..\..\..\Doxygen\docs-builder\accelbyte_docs_builder.py");
				Process.StartInfo.FileName = "python.exe";
				Process.StartInfo.Arguments = string.Format("{0} internal", ScriptPath);
				Process.StartInfo.UseShellExecute = false;

				Process.Start();
				if (!Process.HasExited) Process.WaitForExit();

				return Process.ExitCode == 0;
			}
		}
		catch (Exception E)
		{
			Console.WriteLine("Docs Builder Error: {0}", E.Message);
			return false;
		}
	}
}
