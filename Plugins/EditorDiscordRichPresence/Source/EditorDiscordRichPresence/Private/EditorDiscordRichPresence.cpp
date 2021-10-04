// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "EditorDiscordRichPresence.h"
#include "discord-files/discord.h"
#include "Runtime/Launch/Resources/Version.h"
#if WITH_EDITOR
#include "UnrealEd.h"
#include "Toolkits/AssetEditorManager.h"
#endif


#define LOCTEXT_NAMESPACE "FEditorDiscordRichPresenceModule"

discord::Core* fcore{};

void FEditorDiscordRichPresenceModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	// Get the base directory of this plugin
#if WITH_EDITOR
	FString BaseDir = IPluginManager::Get().FindPlugin("EditorDiscordRichPresence")->GetBaseDir();

	// Add on the relative location of the third party dll and load it
	FString LibraryPath;
#if PLATFORM_WINDOWS
	LibraryPath = FPaths::Combine(*BaseDir, TEXT("ThirdParty/Discord/x64/discord_game_sdk.dll"));
	DLLHandle = FPlatformProcess::GetDllHandle(*LibraryPath);
#endif // PLATFORM_WINDOWS

#endif

	/*
		Grab that Client ID from earlier
		Discord.CreateFlags.Default will require Discord to be running for the game to work
		If Discord is not running, it will:
		1. Close your game
		2. Open Discord
		3. Attempt to re-open your game
		Step 3 will fail when running directly from the Unity editor
		Therefore, always keep Discord running during tests, or use Discord.CreateFlags.NoRequireDiscord
	*/

#if WITH_EDITOR
	auto result = discord::Core::Create(634091660615221258, DiscordCreateFlags_NoRequireDiscord, &fcore);
	//discord::Activity activity{};
	activity.SetType(discord::ActivityType::Streaming);

	FString ProjectName;
	GConfig->GetString(
		TEXT("/Script/EngineSettings.GeneralProjectSettings"),
		TEXT("ProjectName"),
		ProjectName,
		GGameIni
	);

	FString v = "Engine Version: ";
	v.AppendInt(ENGINE_MAJOR_VERSION);
	v.Append(".").AppendInt(ENGINE_MINOR_VERSION);
	v.Append(".").AppendInt(ENGINE_PATCH_VERSION);

	activity.SetState(TCHAR_TO_ANSI(*v));

	FString Status = ProjectName;// +" - " + "in Blueprints";
	activity.SetDetails(TCHAR_TO_ANSI(*Status));

	discord::Timestamp time = FDateTime::UtcNow().ToUnixTimestamp();
	activity.GetTimestamps().SetStart(time);

	activity.GetAssets().SetLargeImage("default");

	if (fcore)
	{
		fcore->ActivityManager().UpdateActivity(activity, [](discord::Result result)
		{
			
		});
	}



	//Tick for Discord update
	TickDelegate = FTickerDelegate::CreateRaw(this, &FEditorDiscordRichPresenceModule::Tick);
	TickDelegateHandle = FTicker::GetCoreTicker().AddTicker(TickDelegate);
#endif
}

void FEditorDiscordRichPresenceModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

#if WITH_EDITOR
	FTicker::GetCoreTicker().RemoveTicker(TickDelegateHandle);
	FPlatformProcess::FreeDllHandle(DLLHandle);
#endif
}

bool FEditorDiscordRichPresenceModule::Tick(float DeltaTime)
{
	//FEditorDiscordRichPresenceModule::Update(DeltaTime);
	UpdateFocusedWindowStatus();
	if (::fcore)
	{
		::fcore->RunCallbacks();
	}


	return true;
}

void FEditorDiscordRichPresenceModule::UpdateFocusedWindowStatus()
{
#if WITH_EDITOR
	FAssetEditorManager assetEditorManager = FAssetEditorManager::Get();
	TArray<UObject*> editedAssets = assetEditorManager.GetAllEditedAssets();
	TArray<IAssetEditorInstance*> openedEditors;

		for (UObject* editedAsset : editedAssets)
		{
			openedEditors.Add(assetEditorManager.FindEditorForAsset(editedAsset, false));
		}

		IAssetEditorInstance* focusedEditor = nullptr;
		double maxLastActivationTime = 0.0;

		for (IAssetEditorInstance* openedEditor : openedEditors)
		{
			if (openedEditor && openedEditor->GetLastActivationTime() > maxLastActivationTime)
			{
				maxLastActivationTime = openedEditor->GetLastActivationTime();
				focusedEditor = openedEditor;
			}
		}


		if (focusedEditor)
		{
			FString ProjectName;
			GConfig->GetString(
				TEXT("/Script/EngineSettings.GeneralProjectSettings"),
				TEXT("ProjectName"),
				ProjectName,
				GGameIni
			);

			FString EditorName = ProjectName + " - in " + GetFriendlyEditorName(focusedEditor->GetEditorName());
			activity.SetDetails(TCHAR_TO_ANSI(*EditorName));
			
			if (fcore)
			{
				fcore->ActivityManager().UpdateActivity(activity, [](discord::Result result)
					{

					});
			}

		}
		else
		{
			FString ProjectName;
			GConfig->GetString(
				TEXT("/Script/EngineSettings.GeneralProjectSettings"),
				TEXT("ProjectName"),
				ProjectName,
				GGameIni
			);

			FString EditorName = ProjectName + " - in " + "Level Editor";
			activity.SetDetails(TCHAR_TO_ANSI(*EditorName));

			if (fcore)
			{
				fcore->ActivityManager().UpdateActivity(activity, [](discord::Result result)
					{

					});
			}

		}
#endif
}

FString FEditorDiscordRichPresenceModule::GetFriendlyEditorName(FName name)
{
	if (FriendlyEditorNameMap.Contains(name))
	{
		return FriendlyEditorNameMap[name];
	}

	return FString("Working...");
}


#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FEditorDiscordRichPresenceModule, EditorDiscordRichPresence)