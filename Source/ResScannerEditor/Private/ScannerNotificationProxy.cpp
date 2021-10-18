// Fill out your copyright notice in the Description page of Project Settings.


#include "ThreadUtils/ScannerNotificationProxy.h"

#include "Async/Async.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
DEFINE_LOG_CATEGORY_STATIC(LogScannerNotificationProxy, All, All);

#define LOCTEXT_NAMESPACE "MissionNotificationPorxy"

UScannerNotificationProxy::UScannerNotificationProxy(const FObjectInitializer& Initializer):Super(Initializer)
{}


void UScannerNotificationProxy::SetMissionName(FName NewMissionName)
{
	MissionName = NewMissionName; // TEXT("Cook");
}

void UScannerNotificationProxy::SetMissionNotifyText(const FText& RunningText, const FText& CancelText,
    const FText& SuccessedText, const FText& FaildText)
{
	// running
	RunningNotifyText = RunningText; // LOCTEXT("CookNotificationInProgress", "Cook in progress");
	// running cancel
	RunningNofityCancelText = CancelText; // LOCTEXT("RunningCookNotificationCancelButton", "Cancel");
	// mission successed
	MissionSuccessedNotifyText = SuccessedText; // LOCTEXT("CookSuccessedNotification", "Cook Finished!");
	// mission failed
	MissionFailedNotifyText = FaildText; // LOCTEXT("CookFaildNotification", "Cook Faild!");
}

void UScannerNotificationProxy::ReceiveOutputMsg(const FString& InMsg)
{
	FString FindItem(TEXT("Display:"));
	int32 Index = InMsg.Len() - InMsg.Find(FindItem) - FindItem.Len();

	if (InMsg.Contains(TEXT("Error:")))
	{
		UE_LOG(LogScannerNotificationProxy, Error, TEXT("%s"), *InMsg);
	}
	else if (InMsg.Contains(TEXT("Warning:")))
	{
		UE_LOG(LogScannerNotificationProxy, Warning, TEXT("%s"), *InMsg.Right(Index));
	}
	else
	{
		UE_LOG(LogScannerNotificationProxy, Display, TEXT("%s"), *InMsg);
	}
}

void UScannerNotificationProxy::SpawnRuningMissionNotification()
{
	UScannerNotificationProxy* MissionProxy=this;
	AsyncTask(ENamedThreads::GameThread, [MissionProxy]()
    {
        if (MissionProxy->PendingProgressPtr.IsValid())
        {
            MissionProxy->PendingProgressPtr.Pin()->ExpireAndFadeout();
        }
        FNotificationInfo Info(MissionProxy->RunningNotifyText);

        Info.bFireAndForget = false;
		Info.Hyperlink = FSimpleDelegate::CreateStatic([](){ FGlobalTabmanager::Get()->InvokeTab(FName("OutputLog")); });
        Info.HyperlinkText = LOCTEXT("ShowOutputLogHyperlink", "Show Output Log");
        Info.ButtonDetails.Add(FNotificationButtonInfo(MissionProxy->RunningNofityCancelText, FText(),
            FSimpleDelegate::CreateLambda([MissionProxy]() {MissionProxy->CancelMission(); }),
            SNotificationItem::CS_Pending
        ));

        MissionProxy->PendingProgressPtr = FSlateNotificationManager::Get().AddNotification(Info);

        MissionProxy->PendingProgressPtr.Pin()->SetCompletionState(SNotificationItem::CS_Pending);
        MissionProxy->bRunning = true;
    });
}

void UScannerNotificationProxy::SpawnMissionSuccessedNotification()
{
	UScannerNotificationProxy* MissionProxy=this;
	AsyncTask(ENamedThreads::GameThread, [MissionProxy]() {
        TSharedPtr<SNotificationItem> NotificationItem = MissionProxy->PendingProgressPtr.Pin();

        if (NotificationItem.IsValid())
        {
            NotificationItem->SetText(MissionProxy->MissionSuccessedNotifyText);
            NotificationItem->SetCompletionState(SNotificationItem::CS_Success);
            NotificationItem->ExpireAndFadeout();

            MissionProxy->PendingProgressPtr.Reset();
        }

        MissionProxy->bRunning = false;
        UE_LOG(LogScannerNotificationProxy, Log, TEXT("The %s Mission is Successfuly."),*MissionProxy->MissionName.ToString());
	});
}

void UScannerNotificationProxy::SpawnMissionFaildNotification()
{
	UScannerNotificationProxy* MissionProxy = this;
	AsyncTask(ENamedThreads::GameThread, [MissionProxy]() {
        TSharedPtr<SNotificationItem> NotificationItem = MissionProxy->PendingProgressPtr.Pin();

        if (NotificationItem.IsValid())
        {
            NotificationItem->SetText(MissionProxy->MissionFailedNotifyText);
            NotificationItem->SetCompletionState(SNotificationItem::CS_Fail);
            NotificationItem->ExpireAndFadeout();

            MissionProxy->PendingProgressPtr.Reset();
            MissionProxy->bRunning = false;
        	UE_LOG(LogScannerNotificationProxy, Error, TEXT("The %s Mission is faild."),*MissionProxy->MissionName.ToString())
        }
        
	});
}

void UScannerNotificationProxy::CancelMission()
{
	MissionCanceled.Broadcast();
}



#undef LOCTEXT_NAMESPACE
