#include "AdvancedEditorBPLibrary.h"

#include "AssumeChecked.h"
#include "ContentBrowserModule.h"
#include "DataTableEditorUtils.h"
#include "IContentBrowserSingleton.h"
#include "LevelEditor.h"
#include "Dialogs/Dialogs.h"
#include "Dialogs/DlgPickAssetPath.h"
#include "Dialogs/DlgPickPath.h"
#include "Factories/DataTableFactory.h"
#include "Kismet/Public/SSCSEditor.h"

#include "LevelEditorViewport.h"
#include "SLevelViewport.h"
#include "CinematicCamera/Public/CineCameraComponent.h"
#include "Engine/Selection.h"
#include "Framework/Notifications/NotificationManager.h"
#include "UMGEditor/Public/Components/DetailsView.h"
#include "UObject/MetaData.h"
#include "UnrealEd/Public/EditorViewportClient.h"
#include "Widgets/Notifications/SNotificationList.h"



namespace InternalEditorLevelLibrary
{
template <class T>
bool IsEditorLevelActor(T * Actor)
{
	bool bResult = false;
	if (Actor && !Actor->IsPendingKill( ))
	{
		const UWorld * World = Actor->GetWorld( );
		if (World && World->WorldType == EWorldType::Editor)
		{
			bResult = true;
		}
	}
	return bResult;
}

UWorld * GetEditorWorld( )
{
	return GEditor ? GEditor->GetEditorWorldContext(false).World( ) : nullptr;
}

UWorld * GetGameWorld( )
{
	if (GEditor)
	{
		if (const FWorldContext * WorldContext = GEditor->GetPIEWorldContext( ))
		{
			return WorldContext->World( );
		}

		return nullptr;
	}

	return GWorld;
}

template <class T>
TArray<T *> GetAllLoadedObjects( )
{
	TArray<T *> Result;

	if (!EditorScriptingUtils::CheckIfInEditorAndPIE( ))
	{
		return Result;
	}

	constexpr EObjectFlags ExcludeFlags = RF_ClassDefaultObject;
	for (TObjectIterator<T> It(ExcludeFlags, true, EInternalObjectFlags::PendingKill); It; ++It)
	{
		T * Obj = *It;
		if (InternalEditorLevelLibrary::IsEditorLevelActor(Obj))
		{
			Result.Add(Obj);
		}
	}

	return Result;
}
}



UAdvancedToolsEditorBPLibrary::UAdvancedToolsEditorBPLibrary(
	const FObjectInitializer & ObjectInitializer
)
	: Super(ObjectInitializer)
{}

UScriptStruct * UAdvancedToolsEditorBPLibrary::FindScriptStructByName(const FString ScriptStructName)
{
	for (TObjectIterator<UScriptStruct> It; It; ++It)
	{
		UScriptStruct * Struct = *It;

		if (Struct->GetName( ) == ScriptStructName)
			return Struct;
	}

	return nullptr;
}

void UAdvancedToolsEditorBPLibrary::SetForceHiddenPropertyVisibility(UDetailsView * DetailsView, const bool Value)
{
	if (DetailsView)
	{
		DetailsView->bForceHiddenPropertyVisibility = Value;
	}
}

TMap<UObject *, FObjectMetaData> UAdvancedToolsEditorBPLibrary::GetMetaData(const UPackage * Package)
{
	TMap<UObject *, FObjectMetaData> Result;

	bool bHasMetaData;
#if ENGINE_MAJOR_VERSION >= 5
	bHasMetaData = Package->HasMetaData( );
#else
	bHasMetaData = !!Package->MetaData;
#endif

	if (!Package || !bHasMetaData)
		return Result;

	for (const auto & Entry : Package->MetaData->ObjectMetaDataMap)
	{
		if (Entry.Key.IsValid( ))
		{
			Result.Add(Entry.Key.Get( ), FObjectMetaData{ Entry.Value });
		}
	}

	return Result;
}

TArray<FObjectMetaData> UAdvancedToolsEditorBPLibrary::GetMetaDataForUnreachableObjects(const UPackage * Package)
{
	TArray<FObjectMetaData> Result;

	if (!Package)
		return Result;

	for (const auto & Entry : Package->MetaData->ObjectMetaDataMap)
	{
		if (!Entry.Key.IsValid( ))
		{
			Result.Emplace(FObjectMetaData{ Entry.Value });
		}
	}

	return Result;
}

FORCEINLINE UMetaData * MetaDataGetter(const UObject * Object)
{
#if ENGINE_MAJOR_VERSION >= 5
	return Object->GetPackage( )->GetMetaData( );
#else
	return Object->GetPackage()->MetaData;
#endif
}

void UAdvancedToolsEditorBPLibrary::SetMetaData(const UObject * Object, const FName Key, const FString Value)
{
	if (!Object || Key.IsNone( )) return;

	MetaDataGetter(Object)->SetValue(Object, Key, *Value);
}

void UAdvancedToolsEditorBPLibrary::RemoveMetaData(const UObject * Object, const FName Key)
{
	if (!Object || Key.IsNone( )) return;

	MetaDataGetter(Object)->RemoveValue(Object, Key);
}

void UAdvancedToolsEditorBPLibrary::InvokeTab(const FName TabName)
{
	FGlobalTabmanager::Get( )->TryInvokeTab(TabName);
}

UActorComponent * UAdvancedToolsEditorBPLibrary::AddComponent(
	AActor *  ToActor,
	UClass *  NewComponentClass,
	UObject * AssetToAssignTo
)
{
	const auto SCSEditor = SNew(SSCSEditor)
		.EditorMode(EComponentEditorMode::ActorInstance)
		.AllowEditing(true)
		.ActorContext(ToActor);

	return SCSEditor->AddNewComponent(NewComponentClass, AssetToAssignTo);
}

void UAdvancedToolsEditorBPLibrary::SystemPathDialog(
	const FString & Message,
	const FString & DefaultPath,
	bool &          Selected,
	FString &       SelectedDirectory
)
{
	Selected = PromptUserForDirectory(SelectedDirectory, Message, DefaultPath);
}

FString UAdvancedToolsEditorBPLibrary::PathDialog(
	const FText Title,
	const FText DefaultPath
)
{
	const auto PickContentPathDlg =
			SNew(SDlgPickPath)
		.Title(Title)
		.DefaultPath(DefaultPath);
	PickContentPathDlg->ShowModal( );

	return PickContentPathDlg->GetPath( ).ToString( );
}

FString UAdvancedToolsEditorBPLibrary::AssetPathDialog(
	const FText Title,
	const FText DefaultPath,
	const bool  bAllowReadonlyFolders
)
{
	const auto PickAssetPathWidget =
			SNew(SDlgPickAssetPath)
		.Title(Title)
		.DefaultAssetPath(DefaultPath)
		.AllowReadOnlyFolders(bAllowReadonlyFolders);
	PickAssetPathWidget->ShowModal( );

	return PickAssetPathWidget->GetFullAssetPath( ).ToString( );
}

UClass * UAdvancedToolsEditorBPLibrary::ClassDialog(
	UClass *                        BasicClass,
	const TArray<UClass *>          CommonClasses,
	const FText                     Title,
	const EBPClassViewerDisplayMode DisplayMode,
	const bool                      bActorsOnly,
	const bool                      bPlaceableOnly,
	const bool                      bBlueprintableOnly,
	const bool                      bShowUnloadedBlueprints,
	const bool                      bShowNoneOption,
	const bool                      bShowUObjectClass,
	const bool                      bExpandRootByDefault,
	const bool                      bEnableClassDynamicLoading,
	EBPClassViewerNameTypeToDisplay NameTypeToDisplay,
	const FText                     ViewerTitleString,
	const bool                      bAllowViewOptions,
	const bool                      bShowBackgroundBorder,
	const bool                      bEditorClassesOnly,
	UClass *                        InitiallySelectedClass
)
{
	UClass * Result = nullptr;

	FClassViewerInitializationOptions Options;
	Options.Mode                       = EClassViewerMode::ClassPicker;
	Options.DisplayMode                = EClassViewerDisplayMode::Type(DisplayMode);
	Options.bIsActorsOnly              = bActorsOnly;
	Options.bIsPlaceableOnly           = bPlaceableOnly;
	Options.bIsBlueprintBaseOnly       = bBlueprintableOnly;
	Options.bShowUnloadedBlueprints    = bShowUnloadedBlueprints;
	Options.bShowNoneOption            = bShowNoneOption;
	Options.bShowObjectRootClass       = bShowUObjectClass;
	Options.bExpandRootNodes           = bExpandRootByDefault;
	Options.bEnableClassDynamicLoading = bEnableClassDynamicLoading;
	Options.NameTypeToDisplay          = EClassViewerNameTypeToDisplay(NameTypeToDisplay);
	Options.ViewerTitleString          = ViewerTitleString;
	Options.bAllowViewOptions          = bAllowViewOptions;
	Options.bShowBackgroundBorder      = bShowBackgroundBorder;
	Options.ExtraPickerCommonClasses   = CommonClasses;
	Options.bEditorClassesOnly         = bEditorClassesOnly;
	Options.InitiallySelectedClass     = InitiallySelectedClass;

	SClassPickerDialog::PickClass(Title, Options, Result, BasicClass);
	return Result;
}

const TCHAR * IconTypeToBrushName(const ENotificationIcon Icon)
{
	switch (Icon)
	{
		case ENotificationIcon::Note:
			return TEXT("MessageLog.Note");
		case ENotificationIcon::Warning:
			return TEXT("MessageLog.Warning");
		case ENotificationIcon::Error:
			return TEXT("MessageLog.Error");
		case ENotificationIcon::Save:
			return TEXT("MainFrame.AutoSaveImage");
		case ENotificationIcon::None:
		default:
			return nullptr;
	}
}

void UAdvancedToolsEditorBPLibrary::ShowNotification(
	const FText             Message,
	const ENotificationIcon Icon,
	const float             FadeInDuration,
	const bool              bUseLargeFont,
	const bool              bUseThrobber,
	const float             FadeOutDuration,
	const float             WidthOverride,
	const float             ExpireDuration,
	const bool              bAllowThrottleWhenFrameRateIsLow
)
{
	const TCHAR * BrushName = IconTypeToBrushName(Icon);

	FNotificationInfo NotificationInfo(Message);
	NotificationInfo.bFireAndForget                   = true;
	NotificationInfo.FadeInDuration                   = FadeInDuration;
	NotificationInfo.FadeOutDuration                  = FadeOutDuration;
	NotificationInfo.ExpireDuration                   = ExpireDuration;
	NotificationInfo.bUseThrobber                     = bUseThrobber;
	NotificationInfo.bUseSuccessFailIcons             = BrushName != nullptr;
	NotificationInfo.bUseLargeFont                    = bUseLargeFont;
	NotificationInfo.bAllowThrottleWhenFrameRateIsLow = bAllowThrottleWhenFrameRateIsLow;

	if (BrushName)
		NotificationInfo.Image = FCoreStyle::Get( ).GetBrush(BrushName);
	if (WidthOverride > 0)
		NotificationInfo.WidthOverride = WidthOverride;

	FSlateNotificationManager::Get( ).AddNotification(NotificationInfo);
}

void UAdvancedToolsEditorBPLibrary::ShowNotificationWithHyperLink(
	const FText                   Message,
	FOnNotificationHyperlinkClick Hyperlink,
	const FText                   HyperlinkText,
	const ENotificationIcon       Icon,
	const float                   FadeInDuration,
	const bool                    bUseLargeFont,
	const bool                    bUseThrobber,
	const float                   FadeOutDuration,
	const float                   WidthOverride,
	const float                   ExpireDuration,
	const bool                    bAllowThrottleWhenFrameRateIsLow
)
{
	const TCHAR * BrushName = IconTypeToBrushName(Icon);

	FNotificationInfo NotificationInfo(Message);
	NotificationInfo.bFireAndForget = true;
	NotificationInfo.FadeInDuration = FadeInDuration;
	NotificationInfo.FadeOutDuration = FadeOutDuration;
	NotificationInfo.ExpireDuration = ExpireDuration;
	NotificationInfo.bUseThrobber = bUseThrobber;
	NotificationInfo.bUseSuccessFailIcons = BrushName != nullptr;
	NotificationInfo.bUseLargeFont = bUseLargeFont;
	NotificationInfo.bAllowThrottleWhenFrameRateIsLow = bAllowThrottleWhenFrameRateIsLow;
	NotificationInfo.HyperlinkText = HyperlinkText;
	NotificationInfo.Hyperlink = FSimpleDelegate::CreateLambda([Hyperlink]( ) { Hyperlink.ExecuteIfBound( ); });

	if (BrushName)
		NotificationInfo.Image = FCoreStyle::Get( ).GetBrush(BrushName);
	if (WidthOverride > 0)
		NotificationInfo.WidthOverride = WidthOverride;

	FSlateNotificationManager::Get( ).AddNotification(NotificationInfo);
}

void UAdvancedToolsEditorBPLibrary::FocusEditorViewport( )
{
	const FLevelEditorModule & LevelEditor =
			FModuleManager::GetModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));

	// LevelEditor.h::LevelEditorTabIds is inaccessible as not exported
	// Thus, raw tab names a copied

	LevelEditor.GetLevelEditorTabManager( )->TryInvokeTab(FName("LevelEditorViewport"));
}

FString UAdvancedToolsEditorBPLibrary::ObjectPathToPackageName(const FString & ObjectPath)
{
	return FPackageName::ObjectPathToPackageName(ObjectPath);
}

void UAdvancedToolsEditorBPLibrary::ObjectPathToPackageNameSplitted(
	const FString & ObjectPath,
	FString &       AssetName,
	FString &       PackagePath
)
{
	const FString PackageName = FPackageName::ObjectPathToPackageName(ObjectPath);
	AssetName                 = FPaths::GetBaseFilename(PackageName);
	PackagePath               = FPaths::GetPath(PackageName);
}

void UAdvancedToolsEditorBPLibrary::SplitPackageName(
	const FString & PackageName,
	FString &       AssetName,
	FString &       PackagePath
)
{
	AssetName   = FPaths::GetBaseFilename(PackageName);
	PackagePath = FPaths::GetPath(PackageName);
}

int64 UAdvancedToolsEditorBPLibrary::GetContentBrowserModuleID( )
{
	return (int64)&FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser").Get( );
}

FString UAdvancedToolsEditorBPLibrary::ContentBrowserSaveAssetDialog(
	const FString & DefaultPath,
	const FString & DefaultAssetName,
	const FText     Title,
	const bool      bDisallowOverwriting,
	int64           ContentBrowserModuleID
)
{
	if (!ContentBrowserModuleID)
		ContentBrowserModuleID = GetContentBrowserModuleID( );

	IContentBrowserSingleton * ContentBrowser = (IContentBrowserSingleton *)ContentBrowserModuleID;

	FSaveAssetDialogConfig DialogConfig;
	DialogConfig.DialogTitleOverride = Title;
	DialogConfig.DefaultPath         = DefaultPath;
	DialogConfig.DefaultAssetName    = DefaultAssetName;
	DialogConfig.ExistingAssetPolicy = bDisallowOverwriting
	                                   ? ESaveAssetDialogExistingAssetPolicy::Disallow
	                                   : ESaveAssetDialogExistingAssetPolicy::AllowButWarn;

	return ContentBrowser->CreateModalSaveAssetDialog(DialogConfig);
}

TArray<FAssetData> UAdvancedToolsEditorBPLibrary::ContentBrowserOpenAssetDialog(
	const FString &     DefaultPath,
	const TArray<FName> AssetClassNames,
	const FText         Title,
	const bool          bAllowMultipleSelection,
	int64               ContentBrowserModuleID
)
{
	if (!ContentBrowserModuleID)
		ContentBrowserModuleID = GetContentBrowserModuleID( );

	IContentBrowserSingleton * ContentBrowser = (IContentBrowserSingleton *)ContentBrowserModuleID;

	FOpenAssetDialogConfig DialogConfig;
	DialogConfig.DialogTitleOverride     = Title;
	DialogConfig.DefaultPath             = DefaultPath;
	DialogConfig.AssetClassNames         = AssetClassNames;
	DialogConfig.bAllowMultipleSelection = bAllowMultipleSelection;

	return ContentBrowser->CreateModalOpenAssetDialog(DialogConfig);
}

void UAdvancedToolsEditorBPLibrary::FocusPrimaryContentBrowser(
	const bool bFocusSearch,
	int64      ContentBrowserModuleID
)
{
	if (!ContentBrowserModuleID)
		ContentBrowserModuleID = GetContentBrowserModuleID( );

	IContentBrowserSingleton * ContentBrowser = (IContentBrowserSingleton *)ContentBrowserModuleID;

	ContentBrowser->FocusPrimaryContentBrowser(bFocusSearch);
}

FName UAdvancedToolsEditorBPLibrary::GetDataTableRowStructName(const UDataTable * DataTable)
{
	if (DataTable)
	{
		return DataTable->GetRowStructName( );
	}
	else
	{
		return NAME_None;
	}
}

void UAdvancedToolsEditorBPLibrary::UpdateDataTableRowByName(
	UDataTable *          DataTable,
	FName                 RowName,
	const FTableRowBase & RowData
)
{
	// We should never hit this!  stubs to avoid NoExport on the class.
	// Check CustomStructureParam at https://docs.unrealengine.com/4.27/en-US/ProgrammingAndScripting/GameplayArchitecture/Metadata/
	check(0);
}

void UAdvancedToolsEditorBPLibrary::Generic_UpdateDataTableRowByName(
	UDataTable *          DataTable,
	const FName           RowName,
	const FTableRowBase & RowData
)
{
	assumeChecked(DataTable);
	assumeChecked(RowName.IsValid());

	// No need for transaction here as it may be a single action in a series actions of a transaction
	// const FScopedTransaction Transaction(LOCTEXT("UpdateDataTableRow", "Update Data Table Row"));

	FDataTableEditorUtils::EDataTableChangeInfo ChangeType =
			FDataTableEditorUtils::EDataTableChangeInfo::RowData;
	if (DataTable->FindRowUnchecked(RowName))
	{
		ChangeType = FDataTableEditorUtils::EDataTableChangeInfo::RowList;
	}

	FDataTableEditorUtils::BroadcastPreChange(DataTable, ChangeType);
	assumeChecked(DataTable->CanModify());
	DataTable->Modify( );

	DataTable->AddRow(RowName, RowData);

	// This manually notifies all GetDataTableRow BP nodes. Gotta do the same if necessary.
	FDataTableEditorUtils::BroadcastPostChange(DataTable, ChangeType);
}

void UAdvancedToolsEditorBPLibrary::RemoveDataTableRowByName(
	UDataTable * DataTable,
	const FName  RowName
)
{
	if (!DataTable || RowName.IsNone( ))
		return;

	constexpr FDataTableEditorUtils::EDataTableChangeInfo ChangeType =
			FDataTableEditorUtils::EDataTableChangeInfo::RowList;

	FDataTableEditorUtils::BroadcastPreChange(DataTable, ChangeType);
	assumeChecked(DataTable->CanModify());
	DataTable->Modify( );

	DataTable->RemoveRow(RowName);

	FDataTableEditorUtils::BroadcastPostChange(DataTable, ChangeType);
}

UDataTableFactory * UAdvancedToolsEditorBPLibrary::GetDataTableFactory( )
{
	return (UDataTableFactory *)UDataTableFactory::StaticClass( )->GetDefaultObject( );
}


void UAdvancedToolsEditorBPLibrary::SetSelectedLevelComponent(
	const TArray<class UActorComponent *> & ActorComponentToSelect
)
{
	TGuardValue<bool> UnattendedScriptGuard(GIsRunningUnattendedScript, true);

	TArray<UActorComponent *> Result;
	/*
	if (!EditorScriptingUtils::CheckIfInEditorAndPIE())
	{
		return;
	}*/

	if (GEdSelectionLock)
	{
		//UE_LOG(LogEditorScripting, Warning, TEXT("SetSelectedLevelActors. The editor selection is currently locked."));
		return;
	}

	GEditor->GetSelectedComponents( )->Modify( );
	if (ActorComponentToSelect.Num( ) > 0)
	{
		GEditor->SelectNone(false, true, false);
		for (UActorComponent * ActorComponent : ActorComponentToSelect)
		{
			if (InternalEditorLevelLibrary::IsEditorLevelActor(ActorComponent))
			{
				/*
				if (!GEditor->CanSelectActor(ActorComponent, true))
				{
					UE_LOG(LogEditorScripting, Warning, TEXT("SetSelectedLevelActors. Can't select actor '%s'."), *ActorComponent->GetName());
					continue;
				}
				*/

				GEditor->SelectActor(ActorComponent->GetOwner( ), true, false);
				GEditor->SelectComponent(ActorComponent, true, false);
			}
		}
		GEditor->NoteSelectionChange( );
	}
	else
	{
		GEditor->SelectNone(true, true, false);
	}
}

void UAdvancedToolsEditorBPLibrary::EjectPilotLevelActorCorrectZoom( )
{
	FLevelEditorModule & LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>("LevelEditor");

	const TSharedPtr<SLevelViewport> ActiveLevelViewport = LevelEditorModule.GetFirstActiveLevelViewport( );
	if (ActiveLevelViewport.IsValid( ))
	{
		FLevelEditorViewportClient & LevelViewportClient = ActiveLevelViewport->GetLevelViewportClient( );


		if (AActor * LockedActor = LevelViewportClient.GetActiveActorLock( ).Get( ))
		{
			//// Check to see if the locked actor was previously overriding the camera settings
			//if (CanGetCameraInformationFromActor(LockedActor))
			//{
			//	// Reset the settings
			//	LevelViewportClient.ViewFOV = LevelViewportClient.FOVAngle;
			//}

			LevelViewportClient.ViewFOV = LevelViewportClient.FOVAngle;

			LevelViewportClient.SetActorLock(nullptr);

			//LevelViewportClient.ViewFOV = 41.3f;

			// remove roll and pitch from camera when unbinding from actors
			GEditor->RemovePerspectiveViewRotation(true, true, false);
		}
	}
}

void UAdvancedToolsEditorBPLibrary::LockActorTransform(AActor * Target, const bool IsLock)
{
	if (!Target)
		return;

#if ENGINE_MAJOR_VERSION >= 5
	Target->SetLockLocation(IsLock);
#else
	Target->bLockLocation = IsLock;
#endif
}


void UAdvancedToolsEditorBPLibrary::DrawDebugFocusPlaceOnCineCamera(UCineCameraComponent * CineCamera, bool IsLock)
{
	if (IsValid(CineCamera))
	{
		CineCamera->FocusSettings.bDrawDebugFocusPlane = IsLock;
	}
	else
		GEngine->AddOnScreenDebugMessage(0, 100, FColor::Red, "Camera not Camera");
}

void UAdvancedToolsEditorBPLibrary::SetCurrentFocusDistance(UCineCameraComponent * CineCamera, float Value)
{
	if (IsValid(CineCamera))
	{
		CineCamera->FocusSettings.ManualFocusDistance = Value;
		CineCamera->CurrentFocusDistance              = Value;
	}
	else
		GEngine->AddOnScreenDebugMessage(0, 100, FColor::Red, "Camera not Camera");
}
