// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ThreadUtils/ScannerNotificationProxy.h"
#include "Modules/ModuleManager.h"
#include "ThreadUtils/FProcWorkerThread.hpp"

class FResScannerEditorModule : public IModuleInterface
{
public:
	static FResScannerEditorModule& Get();
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	void PluginButtonClicked();
	TSharedRef<class SDockTab> OnSpawnPluginTab(const class FSpawnTabArgs& InSpawnTabArgs);
	void OnTabClosed(TSharedRef<SDockTab> InTab);
	void AddMenuExtension(FMenuBuilder& Builder);
	void AddToolbarExtension(FToolBarBuilder& Builder);
	void RunProcMission(const FString& Bin, const FString& Command, const FString& MissionName);
private:
	mutable TSharedPtr<FProcWorkerThread> mProcWorkingThread;
	UScannerNotificationProxy* MissionNotifyProay;
	TSharedPtr<class FUICommandList> PluginCommands;
	TSharedPtr<SDockTab> DockTab;
};
