#pragma once

#include "AssetRegistry/AssetData.h"
#include "Delegates/Delegate.h"
#include "Engine/DataTable.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Kismet2/SClassPickerDialog.h"

#include "AdvancedEditorBPLibrary.generated.h"

class UCineCameraComponent;
class UActorComponent;
class UDataTableFactory;
class UDetailsView;
struct FTableRowBase;

//  //
/*************************************************************************************************/

UENUM(BlueprintType, meta = (ScriptName = "ClassViewerDisplayMode"))
enum EBPClassViewerDisplayMode
{
	/** Default will choose what view mode based on if in Viewer or Picker mode. */
	DefaultView = 0,

	/** Displays all classes as a tree. */
	TreeView = EClassViewerDisplayMode::TreeView,

	/** Displays all classes as a list. */
	ListView = EClassViewerDisplayMode::ListView,
};


static_assert(
	EBPClassViewerDisplayMode::DefaultView == EClassViewerDisplayMode::DefaultView,
	"Enum wrapping internal err"
);

//  //
/*************************************************************************************************/

UENUM(BlueprintType, meta = (ScriptName = "ClassViewerNameTypeToDisplay"))
enum EBPClassViewerNameTypeToDisplay
{
	/** Display both the display name and class name if they're available and different. */
	Dynamic = 0,

	/** Always use the display name */
	DisplayName = EClassViewerNameTypeToDisplay::DisplayName,

	/** Always use the class name */
	ClassName = EClassViewerNameTypeToDisplay::ClassName,
};


static_assert(
	uint8(EBPClassViewerNameTypeToDisplay::Dynamic) == uint8(EClassViewerNameTypeToDisplay::Dynamic),
	"Enum wrapping internal err"
);

//  //
/*************************************************************************************************/

UENUM(BlueprintType)
enum class ENotificationIcon : uint8
{
	None,
	Note,
	Warning,
	Error,
	Save,
};


DECLARE_DYNAMIC_DELEGATE(FOnNotificationHyperlinkClick);


//  //
/*************************************************************************************************/

USTRUCT(BlueprintType)
struct FObjectMetaData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	TMap<FName, FString> MetaData;
};

//  //
/*************************************************************************************************/

UCLASS( )
class UAdvancedToolsEditorBPLibrary final : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY( )
	// Generic //
	/**************************************************************************************************/

#pragma region Generic

	UFUNCTION(
		BlueprintCallable,
		BlueprintPure,
		Category = "EditorScripting|Utilities",
		meta = (Keywords = "simona")
	)
	static UScriptStruct * FindScriptStructByName(FString ScriptStructName);

	UFUNCTION(
		BlueprintCallable,
		Category = "EditorScripting|Utilities",
		meta = (Keywords = "simona")
	)
	static void SetForceHiddenPropertyVisibility(UDetailsView * DetailsView, bool Value);

	UFUNCTION(
		BlueprintCallable,
		Category = "EditorScripting|Utilities",
		meta = (Keywords = "simona")
	)
	static TMap< UObject*, FObjectMetaData> GetMetaData(const UPackage* Package);

	UFUNCTION(
		BlueprintCallable,
		Category = "EditorScripting|Utilities",
		meta = (Keywords = "simona")
	)
	static TArray<FObjectMetaData> GetMetaDataForUnreachableObjects(const UPackage* Package);

	UFUNCTION(
		BlueprintCallable,
		Category = "EditorScripting|Utilities",
		meta = (Keywords = "simona")
	)
	static void SetMetaData(const UObject* Object, FName Key, FString Value);

	UFUNCTION(
		BlueprintCallable,
		Category = "EditorScripting|Utilities",
		meta = (Keywords = "simona")
	)
	static void RemoveMetaData(const UObject* Object, FName Key);

	UFUNCTION(
		BlueprintCallable,
		Category = "EditorScripting|Utilities",
		meta = (Keywords = "simona open summon")
	)
	static void InvokeTab(FName TabName);

#pragma endregion ~ Generic

	// Level editor //
	/**************************************************************************************************/

#pragma region Level editor

	/**
 * Adds a component to an actor in level editor. To refresh details view, reset and reassign
 * level editor actor selection (nodes "GetSelectionSet", "ClearActorSelectionSet",
 * "SetSelectedLevelActors")
 *
 * @param ToActor           Actor to add component to
 * @param NewComponentClass Class of component to create
 * @param AssetToAssignTo   If component can use provided asset, it will be assigned (e.g. StaticMesh to StaticMeshComponent)
 *
 * @return Created component if succeeded
*/
	UFUNCTION(
		BlueprintCallable,
		meta = (Keywords = "simona create spawn"),
		Category = "EditorScripting|LevelUtility"
	)
	static UPARAM(DisplayName = "New Component") UActorComponent * AddComponent(
		AActor *  ToActor,
		UClass *  NewComponentClass,
		UObject * AssetToAssignTo
	);

	/** Focuses level editor tab and and its viewport tab, summoning it if necessary */
	UFUNCTION(
		BlueprintCallable,
		meta = (Keywords = "simona"),
		Category = "EditorScripting"
	)
	static void FocusEditorViewport( );

#pragma endregion ~ Level editor

	// Path utilities //
	/*************************************************************************************************/

#pragma region Path utilities

	/** Returns the name of the package referred to by the specified object path */
	UFUNCTION(
		BlueprintCallable,
		BlueprintPure,
		meta = (Keywords = "simona"),
		Category = "Utilities|Paths"
	)
	static FString ObjectPathToPackageName(const FString & ObjectPath);

	/** Returns the name of the package referred to by the specified object path, splitted into AssetName and PackagePath */
	UFUNCTION(
		BlueprintCallable,
		BlueprintPure,
		meta = (Keywords = "simona"),
		Category = "Utilities|Paths"
	)
	static void ObjectPathToPackageNameSplitted(
		const FString & ObjectPath,
		FString &       AssetName,
		FString &       PackagePath
	);

	/** Splits PackageName into AssetName and PackagePath */
	UFUNCTION(
		BlueprintCallable,
		BlueprintPure,
		meta = (Keywords = "simona"),
		Category = "Utilities|Paths"
	)
	static void SplitPackageName(
		const FString & PackageName,
		FString &       AssetName,
		FString &       PackagePath
	);

#pragma endregion ~ Path utilities

	// Editor dialogs //
	/*************************************************************************************************/

#pragma region UnrealEd dialogs

	UFUNCTION(
		BlueprintCallable,
		meta = (Keywords = "simona"),
		Category = "EditorScripting|Dialogs"
	)
	static void SystemPathDialog(
		const FString & Message,
		const FString & DefaultPath,
		bool &          Selected,
		FString &       SelectedDirectory
	);

	/** @return Empty if user canceled dialog */
	UFUNCTION(
		BlueprintCallable,
		meta = (Keywords = "simona"),
		Category = "EditorScripting|Dialogs"
	)
	static UPARAM(DisplayName = "Path") FString PathDialog(
		FText Title       = NSLOCTEXT("Editor dialogs", "Select Folder", "Select Folder"),
		FText DefaultPath = NSLOCTEXT("", "", "")
	);

	/** @return Empty if user canceled dialog */
	UFUNCTION(
		BlueprintCallable,
		meta = (Keywords = "simona"),
		Category = "EditorScripting|Dialogs"
	)
	static UPARAM(DisplayName = "Object Path") FString AssetPathDialog(
		FText Title                 = NSLOCTEXT("Editor dialogs", "Select Asset", "Select Asset"),
		FText DefaultPath           = NSLOCTEXT("", "", ""),
		bool  bAllowReadonlyFolders = false
	);

	UFUNCTION(
		BlueprintCallable,
		Category = "EditorScripting|Dialogs",
		meta = (Keywords = "simona", AutoCreateRefTerm = "CommonClasses", AdvancedDisplay = 1)

	)
	static UClass * ClassDialog(
		UClass *                        BasicClass,
		TArray<UClass *>                CommonClasses,
		FText                           Title = NSLOCTEXT("Editor dialogs", "Select Class", "Select Class"),
		EBPClassViewerDisplayMode       DisplayMode = DefaultView,
		bool                            bActorsOnly = false,
		bool                            bPlaceableOnly = false,
		bool                            bBlueprintableOnly = false,
		bool                            bShowUnloadedBlueprints = true,
		bool                            bShowNoneOption = false,
		bool                            bShowUObjectClass = false,
		bool                            bExpandRootByDefault = true,
		bool                            bEnableClassDynamicLoading = true,
		EBPClassViewerNameTypeToDisplay NameTypeToDisplay = Dynamic,
		FText                           ViewerTitleString = NSLOCTEXT("", "", ""),
		bool                            bAllowViewOptions = true,
		bool                            bShowBackgroundBorder = true,
		bool                            bEditorClassesOnly = false,
		UClass *                        InitiallySelectedClass = nullptr
	);

	UFUNCTION(
		BlueprintCallable,
		Category = "EditorScripting|Dialogs",
		meta = (Keywords = "simona", AdvancedDisplay = 1)
	)
	static void ShowNotification(
		FText             Message,
		ENotificationIcon Icon                             = ENotificationIcon::Note,
		float             FadeInDuration                   = 0.5f,
		bool              bUseLargeFont                    = true,
		bool              bUseThrobber                     = true,
		float             FadeOutDuration                  = 2.0f,
		float             WidthOverride                    = -1,
		float             ExpireDuration                   = 1.0f,
		bool              bAllowThrottleWhenFrameRateIsLow = true
	);

	UFUNCTION(
		BlueprintCallable,
		Category = "EditorScripting|Dialogs",
		meta = (Keywords = "simona", AdvancedDisplay = 3, HyperlinkText = "Details")
	)
	static void ShowNotificationWithHyperLink(
		FText                         Message,
		FOnNotificationHyperlinkClick Hyperlink,
		FText                         HyperlinkText,
		ENotificationIcon             Icon                             = ENotificationIcon::Note,
		float                         FadeInDuration                   = 0.5f,
		bool                          bUseLargeFont                    = true,
		bool                          bUseThrobber                     = true,
		float                         FadeOutDuration                  = 2.0f,
		float                         WidthOverride                    = -1,
		float                         ExpireDuration                   = 1.0f,
		bool                          bAllowThrottleWhenFrameRateIsLow = true
	);

#pragma endregion ~ Editor dialogs

	// Content browser dialogs //
	/*************************************************************************************************/

#pragma region Content browser dialogs

	UFUNCTION(
		BlueprintCallable,
		BlueprintPure,
		Category = "EditorScripting|Dialogs|Content Browser",
		meta = (
			Keywords = "simona",
			CompactNodeTitle = "Content Browser ID"
		)
	)
	static int64 GetContentBrowserModuleID( );

	UFUNCTION(
		BlueprintCallable,
		Category = "EditorScripting|Dialogs|Content Browser",
		meta = (
			Keywords = "simona",
			DisplayName = "Save Asset Dialog",
			AdvancedDisplay = "ContentBrowserModuleID"
		)
	)
	static UPARAM(DisplayName = "Object Path") FString ContentBrowserSaveAssetDialog(
		const FString & DefaultPath,
		const FString & DefaultAssetName,
		FText           Title                  = NSLOCTEXT("Editor dialogs", "Save asset", "Save asset"),
		bool            bDisallowOverwriting   = true,
		int64           ContentBrowserModuleID = 0
	);

	UFUNCTION(
		BlueprintCallable,
		Category = "EditorScripting|Dialogs|Content Browser",
		meta = (
			Keywords = "simona",
			DisplayName = "Open Assets Dialog",
			AdvancedDisplay = "ContentBrowserModuleID"
		)
	)
	static UPARAM(DisplayName = "Selected Assets") TArray<FAssetData> ContentBrowserOpenAssetDialog(
		const FString & DefaultPath,
		TArray<FName>   AssetClassNames,
		FText           Title                   = NSLOCTEXT("Editor dialogs,", "Open Asset", "Open Asset"),
		bool            bAllowMultipleSelection = false,
		int64           ContentBrowserModuleID  = 0
	);

	UFUNCTION(
		BlueprintCallable,
		Category = "EditorScripting|Dialogs|Content Browser",
		meta = (
			Keywords = "simona",
			AdvancedDisplay = "ContentBrowserModuleID"
		)
	)
	static void FocusPrimaryContentBrowser(
		bool  bFocusSearch           = false,
		int64 ContentBrowserModuleID = 0
	);

#pragma endregion ~ Content browser dialogs

	// Data table //
	/**************************************************************************************************/

#pragma region Data table

	UFUNCTION(
		BlueprintCallable,
		BlueprintPure,
		Category = "EditorScripting|DataTable",
		meta = (Keywords = "simona")
	)
	static FName GetDataTableRowStructName(const UDataTable * DataTable);

	/**
	 * Get a Row from a DataTable given a RowName
	 * @note This calls Modify on the table, so you don't need a TransactObject node when building
	 *       a transaction
	 */
	UFUNCTION(
		BlueprintCallable,
		CustomThunk,
		Category = "EditorScripting|DataTable",
		meta = (
			CustomStructureParam = "RowData",
			BlueprintInternalUseOnly = "true"
		)
	)
	static void UpdateDataTableRowByName(
		UDataTable *          DataTable,
		FName                 RowName,
		const FTableRowBase & RowData
	);

	static void Generic_UpdateDataTableRowByName(
		UDataTable *          DataTable,
		FName                 RowName,
		const FTableRowBase & RowData
	);

#pragma region CustomThunk

	/** Based on UDataTableFunctionLibrary::GetDataTableRow */
	DECLARE_FUNCTION(execUpdateDataTableRowByName)
	{
		P_GET_OBJECT(UDataTable, Table);
		P_GET_PROPERTY(FNameProperty, RowName);

		// May be not fully correct, as it crashes if no struct is provided at all
		// This argument is guaranteed by custom node at the moment
		// Though as far as I see previous getters gonna crash as well
		Stack.StepCompiledIn<FStructProperty>(NULL);
		FTableRowBase * RowDataPtr = (FTableRowBase *)(Stack.MostRecentPropertyAddress);

		P_FINISH;

		FStructProperty * StructProp = CastField<FStructProperty>(Stack.MostRecentProperty);
		if (!Table)
		{
			FBlueprintExceptionInfo ExceptionInfo(
				EBlueprintExceptionType::AccessViolation,
				NSLOCTEXT(
					"UpdateDataTableRow",
					"MissingTableInput",
					"Failed to resolve the table input. Be sure the DataTable is valid."
				)
			);
			FBlueprintCoreDelegates::ThrowScriptException(P_THIS, Stack, ExceptionInfo);
		}
		else if (StructProp && RowDataPtr)
		{
			UScriptStruct *       InputType = StructProp->Struct;
			const UScriptStruct * TableType = Table->GetRowStruct( );

			const bool bCompatible = (InputType == TableType) ||
			(InputType->IsChildOf(TableType) &&
				FStructUtils::TheSameLayout(InputType, TableType));
			if (bCompatible)
			{
				P_NATIVE_BEGIN;
					Generic_UpdateDataTableRowByName(Table, RowName, *RowDataPtr);
				P_NATIVE_END;
			}
			else
			{
				FBlueprintExceptionInfo ExceptionInfo(
					EBlueprintExceptionType::AccessViolation,
					NSLOCTEXT(
						"UpdateDataTableRow",
						"IncompatibleRowDataType",
						"Incompatible row data parameter; the data table's type is not the same as the row type."
					)
				);
				FBlueprintCoreDelegates::ThrowScriptException(P_THIS, Stack, ExceptionInfo);
			}
		}
		else
		{
			FBlueprintExceptionInfo ExceptionInfo(
				EBlueprintExceptionType::AccessViolation,
				NSLOCTEXT(
					"UpdateDataTableRow",
					"MissingRowDataInput",
					"Failed to resolve row type for UpdateDataTableRow."
				)
			);
			FBlueprintCoreDelegates::ThrowScriptException(P_THIS, Stack, ExceptionInfo);
		}
	}

#pragma endregion ~ CustomThunk

	/**
	 * @note This calls Modify on the table, so you don't need a TransactObject node when building
	 *       a transaction
	*/
	UFUNCTION(
		BlueprintCallable,
		Category = "EditorScripting|DataTable",
		meta = (Keywords = "simona")
	)
	static void RemoveDataTableRowByName(UDataTable * DataTable, FName RowName);

	UFUNCTION(
		BlueprintCallable,
		BlueprintPure,
		Category = "EditorScripting|DataTable",
		meta = (
			Keywords = "simona",
			CompactNodeTitle = "Data Table Factory"
		)
	)
	static UDataTableFactory * GetDataTableFactory( );

#pragma endregion ~ Data table

#pragma region xXxEduardoxXx

	//BlueprintPure,
	UFUNCTION(
		BlueprintCallable,

		Category = "EditorScripting|Utilities",
		meta = (Keywords = "simona")
	)
	static void SetSelectedLevelComponent(const TArray<class UActorComponent *> & ActorComponentToSelect);

	UFUNCTION(BlueprintCallable, Category = "EditorScripting|Utilities", meta = (Keywords = "simona"))
	static void EjectPilotLevelActorCorrectZoom( );

	UFUNCTION(BlueprintCallable, Category = "EditorScripting|Utilities", meta = (Keywords = "simona"))
	static void LockActorTransform(AActor * Target, bool IsLock);

	UFUNCTION(BlueprintCallable, Category = "EditorScripting|Utilities", meta = (Keywords = "simona"))
	static void DrawDebugFocusPlaceOnCineCamera(UCineCameraComponent * CineCamera, bool IsLock);

	UFUNCTION(BlueprintCallable, Category = "EditorScripting|Utilities", meta = (Keywords = "simona"))
	static void SetCurrentFocusDistance(UCineCameraComponent * CineCamera, float Value);

#pragma endregion ~ Something nodes
};
