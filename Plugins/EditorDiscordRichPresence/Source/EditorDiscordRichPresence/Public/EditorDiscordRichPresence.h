// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Containers/Ticker.h"
#include "discord-files/discord.h"
#if WITH_EDITORONLY_DATA
#include "Interfaces/IPluginManager.h"
#endif

class FEditorDiscordRichPresenceModule : public IModuleInterface
{
public:

	FTickerDelegate TickDelegate;
	FDelegateHandle TickDelegateHandle;

	TMap<FName, FString> FriendlyEditorNameMap =
	{
		{"GenericAssetEditor", "Editing..."},
		{"StaticMeshEditor", "StaticMesh Editor"},
		{"SkeletalMeshEditor", "SkeletalMesh Editor"},
		{"SkeletonEditor", "Skeleton Editor"},
		{"BlueprintEditor", "Blueprint Editor"},
		{"MaterialEditor", "Material Editor"},
		{"TextureEditor", "Texture Editor"},
		{"SoundCueEditor", "Sound Cue Editor"},
		{"MaterialEditor", "Material Editor"},
		{"AnimationEditor", "Animation Editor"},
		{"AnimationBlueprintEditor", "Animation BP Editor"},
		{"FlipbookEditor", "Flipbook Editor"},
		{"SpriteEditor", "Sprite Editor"},
		{"FontEditor", "Font Editor"},
		{"MaterialInstanceEditor", "Material Inst Editor"},
	};

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	virtual bool Tick(float DeltaTime);

private:
	/** Handle to the test dll we will load */
	void* DLLHandle;
	void UpdateFocusedWindowStatus();
	FString GetFriendlyEditorName(FName name);

	discord::Activity activity{};
};
