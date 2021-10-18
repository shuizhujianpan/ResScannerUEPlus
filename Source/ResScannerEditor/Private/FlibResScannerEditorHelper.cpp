// Fill out your copyright notice in the Description page of Project Settings.


#include "FlibResScannerEditorHelper.h"
#include "DesktopPlatformModule.h"
#include "IDesktopPlatform.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Kismet/KismetSystemLibrary.h"
#define LOCTEXT_NAMESPACE "UFlibResScannerEditorHelper"

TArray<FString> UFlibResScannerEditorHelper::OpenFileDialog()
{
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	TArray<FString> SelectedFiles;
	
	if (DesktopPlatform)
	{
		const bool bOpened = DesktopPlatform->OpenFileDialog(
			nullptr,
			LOCTEXT("OpenHotPatchConfigDialog", "Open .json").ToString(),
			FString(TEXT("")),
			TEXT(""),
			TEXT("HotPatcher json (*.json)|*.json"),
			EFileDialogFlags::None,
			SelectedFiles
		);
	}
	return SelectedFiles;
}

TArray<FString> UFlibResScannerEditorHelper::SaveFileDialog()
{
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();

	TArray<FString> SaveFilenames;
	if (DesktopPlatform)
	{
		
		const bool bOpened = DesktopPlatform->SaveFileDialog(
			nullptr,
			LOCTEXT("SvedHotPatcherConfig", "Save .json").ToString(),
			FString(TEXT("")),
			TEXT(""),
			TEXT("HotPatcher json (*.json)|*.json"),
			EFileDialogFlags::None,
			SaveFilenames
		);
	}
	return SaveFilenames;
}


FString UFlibResScannerEditorHelper::GetUECmdBinary()
{
	FString Binary;
#if ENGINE_MAJOR_VERSION > 4
	Binary = TEXT("UnrealEditor");
#else
	Binary = TEXT("UE4Editor");
#endif

	
#if PLATFORM_WINDOWS
	return FPaths::Combine(
		FPaths::ConvertRelativePathToFull(FPaths::EngineDir()),
		TEXT("Binaries"),
#if PLATFORM_64BITS	
		TEXT("Win64"),
#else
		TEXT("Win32"),
#endif
#ifdef WITH_HOTPATCHER_DEBUGGAME
	#if PLATFORM_64BITS
			FString::Printf(TEXT("%s-Win64-DebugGame-Cmd.exe"),*Binary)
			// TEXT("UE4Editor-Win64-DebugGame-Cmd.exe")
	#else
			FString::Printf(TEXT("%s-Win32-DebugGame-Cmd.exe"),*Binary)
			// TEXT("UE4Editor-Win32-DebugGame-Cmd.exe")
	#endif
#else
		FString::Printf(TEXT("%s-Cmd.exe"),*Binary)
		// TEXT("UE4Editor-Cmd.exe")
#endif
	);
#endif
#if PLATFORM_MAC
	return FPaths::Combine(
		FPaths::ConvertRelativePathToFull(FPaths::EngineDir()),
		TEXT("Binaries"),
		TEXT("Mac"),
		FString::Printf(TEXT("%s-Cmd"),*Binary)
		//TEXT("UE4Editor-Cmd")
	);
#endif
	return TEXT("");
}

FString UFlibResScannerEditorHelper::GetProjectFilePath()
{
	FString ProjectFilePath;
	{
		FString ProjectPath = UKismetSystemLibrary::GetProjectDirectory();
		FString ProjectName = FString(FApp::GetProjectName()).Append(TEXT(".uproject"));
		ProjectFilePath =  FPaths::Combine(ProjectPath, ProjectName);
	}
	return ProjectFilePath;
}

#undef LOCTEXT_NAMESPACE