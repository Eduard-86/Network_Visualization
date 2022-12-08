#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
// Required as include for FHitResult for UHT
// #include "Engine/EngineTypes.h"
#include "MovieScene/Public/MovieScene.h"

#include "AdvancedRuntimeBPLibrary.generated.h"

class UMovieSceneNameableTrack;
class UMovieSceneSequencePlayer;
struct FMovieSceneMarkedFrame;
class UMovieSceneTrack;
class ULevelSequencePlayer;
class UActorComponent;

DECLARE_DYNAMIC_DELEGATE(FSimpleRunnableSignature);


UENUM(BlueprintType, meta = (ScriptName = "WorldType"))
enum EBPWorldType
{
	/** An untyped world, in most cases this will be the vestigial worlds of streamed in sub-levels */
	None = 0,

	/** The game world */
	Game = EWorldType::Game,

	/** A world being edited in the editor */
	Editor = EWorldType::Editor,

	/** A Play In Editor world */
	PIE = EWorldType::PIE,

	/** A preview world for an editor tool */
	EditorPreview = EWorldType::EditorPreview,

	/** A preview world for a game */
	GamePreview = EWorldType::GamePreview,

	/** A minimal RPC world for a game */
	GameRPC = EWorldType::GameRPC,

	/** An editor world that was loaded but not currently being edited in the level editor */
	Inactive = EWorldType::Inactive
};


static_assert(EBPWorldType::None == EWorldType::None, "Enum wrapping internal err");

//  //
/*************************************************************************************************/

UENUM(BlueprintType, meta = (ScriptName = "FieldType"))
enum EFieldType
{
	Other = 0,

	Boolean,
	Integer8,
	Integer16,
	Integer32,
	Integer64,
	Float,
	Double,
	Name,
	String,
	Text,
	Delegate,
	Structure,
	Object,
};


USTRUCT(BlueprintType, DisplayName="PropertyData")
struct FBPPropertyData
{
	GENERATED_BODY( )

	UPROPERTY(BlueprintReadOnly)
	TEnumAsByte<EFieldType> FieldType = EFieldType::Other;

	UPROPERTY(BlueprintReadOnly)
	FName PropertyName = NAME_None;

	/** If set, then Value fields are all invalid */
	UPROPERTY(BlueprintReadOnly)
	bool bIsArray = false;

	UPROPERTY(BlueprintReadOnly)
	bool bIsExposedToEditor = false;

	UPROPERTY(BlueprintReadOnly, AdvancedDisplay)
	bool bIsSignedNumber = false;

	UPROPERTY(BlueprintReadOnly, AdvancedDisplay)
	FName StructTypeName = NAME_None;

	UPROPERTY(BlueprintReadOnly, AdvancedDisplay)
	UClass * ObjectType = nullptr;

	//

	UPROPERTY(BlueprintReadOnly, AdvancedDisplay)
	bool BooleanValue = false;

	UPROPERTY(BlueprintReadOnly, AdvancedDisplay)
	uint8 UnsignedInteger8Value = 0;

	UPROPERTY(BlueprintReadOnly, AdvancedDisplay)
	int32 Integer32Value = 0;

	UPROPERTY(BlueprintReadOnly, AdvancedDisplay)
	int64 Integer64Value = 0;

	UPROPERTY(BlueprintReadOnly, AdvancedDisplay)
	float FloatValue = 0;

	UPROPERTY(BlueprintReadOnly, AdvancedDisplay)
	FName NameValue = NAME_None;

	UPROPERTY(BlueprintReadOnly, AdvancedDisplay)
	FString StringValue;

	UPROPERTY(BlueprintReadOnly, AdvancedDisplay)
	FText TextValue;

	UPROPERTY(BlueprintReadOnly, AdvancedDisplay)
	UObject * ObjectValue = nullptr;

	FProperty * Property = nullptr;
};


//  //
/*************************************************************************************************/

USTRUCT(BlueprintType)
struct FBPReferencerInformation
{
	GENERATED_BODY( )

	UPROPERTY(BlueprintReadOnly)
	bool bIsExternalReference;

	/** The object that is referencing the target */
	UPROPERTY(BlueprintReadOnly)
	UObject * Referencer;

	/** The total number of references from Referencer to the target */
	UPROPERTY(BlueprintReadOnly)
	int32 TotalReferences;

	/** The array of UProperties in Referencer which hold references to target */
	UPROPERTY(BlueprintReadOnly)
	TArray<FName> ReferencingProperties;
};


//  //
/*************************************************************************************************/

UENUM(BlueprintType, Flags, meta = (ScriptName = "RenameFlags"))
enum class EBPRenameFlags : uint8
{
	/** Default rename behavior */
	None = 0,
	/** Rename won't call ResetLoaders or flush async loading. You should pass this if you are renaming a deep subobject and do not need to reset loading for the outer package */
	ForceNoResetLoaders = (0x0001),
	/** Just test to make sure that the rename is guaranteed to succeed if an non test rename immediately follows */
	Test = (0x0002),
	/** Indicates that the object (and new outer) should not be dirtied */
	DoNotDirty = (0x0004),
	/** Don't create an object redirector, even if the class is marked RF_Public */
	DontCreateRedirectors = (0x0010),
	/** Don't call Modify() on the objects, so they won't be stored in the transaction buffer */
	NonTransactional = (0x0020),
	/** Force unique names across all packages not just within the scope of the new outer */
	ForceGlobalUnique = (0x0040),
	/** Prevent renaming of any child generated classes and CDO's in blueprints */
	SkipGeneratedClasses = (0x0080),
};


//  //
/*************************************************************************************************/

USTRUCT(BlueprintType, DisplayName="FMovieSceneBinding")
struct FBPMovieSceneBinding
{
	GENERATED_BODY( )

	UPROPERTY(BlueprintReadOnly)
	FGuid ObjectGuid;

	UPROPERTY(BlueprintReadOnly)
	FString BindingName;

	UPROPERTY(BlueprintReadOnly)
	TArray<UMovieSceneTrack *> Tracks;
};


//  //
/*************************************************************************************************/

USTRUCT(BlueprintType)
struct FSlotAnimationData
{
	GENERATED_BODY( )

	UPROPERTY(BlueprintReadWrite)
	UAnimSequenceBase * Asset;

	UPROPERTY(BlueprintReadWrite)
	FName SlotNodeName;
};


//  //
/*************************************************************************************************/

UCLASS( )
class UAdvancedToolsRuntimeBPLibrary final : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY( )
	// Shift right/left //
	/**************************************************************************************************/

#pragma region Shift right/left

	UFUNCTION(
		BlueprintCallable,
		BlueprintPure,
		Category = "Math | Integer",
		meta = (
			Keywords = "simona shr >> int int32 integer",
			DisplayName = "ShiftRight (integer)",
			CompactNodeTitle = ">>"
		)
	)
	static FORCEINLINE int32 ShiftRight_Int32(const int32 Value, const int32 Shift) { return Value >> Shift; }

	UFUNCTION(
		BlueprintCallable,
		BlueprintPure,
		Category = "Math | Integer",
		meta = (
			Keywords = "simona shl << int int32 integer",
			DisplayName = "ShiftLeft (integer)",
			CompactNodeTitle = "<<"
		)
	)
	static FORCEINLINE int32 ShiftLeft_Int32(const int32 Value, const int32 Shift) { return Value << Shift; }

	UFUNCTION(
		BlueprintCallable,
		BlueprintPure,
		Category = "Math | Byte",
		meta = (
			Keywords = "simona shr >> byte uint8",
			DisplayName = "ShiftRight (byte)",
			CompactNodeTitle = ">>"
		)
	)
	static FORCEINLINE uint8 ShiftRight_Uint8(const uint8 Value, const int32 Shift) { return Value >> Shift; }

	UFUNCTION(
		BlueprintCallable,
		BlueprintPure,
		Category = "Math | Integer",
		meta = (
			Keywords = "simona shl << byte uint8",
			DisplayName = "ShiftLeft (byte)",
			CompactNodeTitle = "<<"
		)
	)
	static FORCEINLINE uint8 ShiftLeft_Uint8(const uint8 Value, const int32 Shift) { return Value << Shift; }

#pragma endregion ~ Shift right/left

	// Async //
	/**************************************************************************************************/

#pragma region Async

	UFUNCTION(
		BlueprintCallable,
		meta = (Keywords = "simona thread"),
		Category = "Performance|Threading"
	)
	static void RunAsync(
		const FSimpleRunnableSignature & Function,
		const FString &                  ThreadName = "AsyncTaskRunner"
	);

#pragma endregion ~ Async

	// Build flags //
	/**************************************************************************************************/

#pragma region Build flags

	UFUNCTION(
		BlueprintCallable,
		BlueprintPure,
		meta = (
			Keywords = "simona",
			CompactNodeTitle = "IsDebugBuild",
			DeprecatedFunction,
			DeprecationMessage = "Use BuildTypeSwitch instead"
		),
		Category = "Build"
	)
	static FORCEINLINE bool IsDebugBuild( ) { return UE_BUILD_DEBUG; }

	UFUNCTION(
		BlueprintCallable,
		BlueprintPure,
		meta = (
			Keywords = "simona",
			CompactNodeTitle = "IsDevelopmentBuild",
			DeprecatedFunction,
			DeprecationMessage = "Use BuildTypeSwitch instead"
		),
		Category = "Build"
	)
	static FORCEINLINE bool IsDevelopmentBuild( ) { return UE_BUILD_DEVELOPMENT; }

	UFUNCTION(
		BlueprintCallable,
		BlueprintPure,
		meta = (
			Keywords = "simona",
			CompactNodeTitle = "IsShippingBuild",
			DeprecatedFunction,
			DeprecationMessage = "Use BuildTypeSwitch instead"
		),
		Category = "Build"
	)
	static FORCEINLINE bool IsShippingBuild( ) { return UE_BUILD_SHIPPING; }

	UFUNCTION(
		BlueprintCallable,
		BlueprintPure,
		meta = (
			Keywords = "simona",
			CompactNodeTitle = "IsWithEditorBuild"
		),
		Category = "Build"
	)
	static FORCEINLINE bool IsWithEditorBuild( ) { return WITH_EDITOR; }

#pragma endregion ~ Build flags

	// Sequencer //
	/**************************************************************************************************/

#pragma region Sequencer

	/**
	 * Converts Player time to underlying MovieScene time
	 */
	// UFUNCTION(
	// 	BlueprintCallable,
	// 	BlueprintPure,
	// 	Category = "Game|Cinematic",
	// 	meta = (KeyWords = "simona level sequence sequencer")
	// )
	static int32 GetCurrentMovieSceneFrame(const UMovieSceneSequencePlayer * SequencePlayer);

	/**
     * Converts SequencePlayer frame to underlying MovieScene frame
     */
	UFUNCTION(
		BlueprintCallable,
		BlueprintPure,
		Category = "Game|Cinematic",
		meta = (KeyWords = "simona level sequence sequencer")
	)
	static int32 SequencePlayerFrameToMovieSceneFrame(
		FFrameNumber                      FrameNumber,
		const UMovieSceneSequencePlayer * SequencePlayer
	);

	/**
	 * Converts underlying MovieScene frame to SequencePlayer frame
	 */
	UFUNCTION(
		BlueprintCallable,
		BlueprintPure,
		Category = "Game|Cinematic",
		meta = (KeyWords = "simona level sequence sequencer")
	)
	static FFrameTime MovieSceneFrameToSequencePlayerFrame(
		FFrameNumber                 FrameNumber,
		const ULevelSequencePlayer * SequencePlayer
	);

	/** @return Nullptr on failure */
	static UMovieSceneTrack * GetMasterTrack(
		const ULevelSequencePlayer & SequencePlayer,
		const FString &              MasterTrackType,
		int32                        TrackIndex
	);

	UFUNCTION(
		BlueprintCallable,
		Category = "Game|Cinematic",
		meta = (
			AdvancedDisplay = "trackIndex",
			KeyWords = "simona level sequence sequencer master track"
		)
	)
	static void SkipToEndOfCurrentSection(
		ULevelSequencePlayer * SequencePlayer,
		const FString &        MasterTrackType = "Audio",
		int32                  TrackIndex      = 0
	);

	/**
	 * @brief
	 * @param SequencePlayer
	 * @param MasterTrackType
	 * @param TrackIndex
	 * @return If no section is currently playing, then -1
	 */
	UFUNCTION(
		BlueprintCallable,
		BlueprintPure,
		Category = "Game|Cinematic",
		meta = (
			AdvancedDisplay = "TrackIndex",
			KeyWords = "simona level sequence sequencer master track"
		)
	)
	static int32 GetCurrentSectionIndex(
		ULevelSequencePlayer * SequencePlayer,
		const FString &        MasterTrackType = "Audio",
		int32                  TrackIndex      = 0
	);

	UFUNCTION(
		BlueprintCallable,
		Category = "Game|Cinematic",
		meta = (
			KeyWords = "simona level sequence sequencer",
			AdvancedDisplay = "Subsequence"
		)
	)
	static UObject * GetOrCreateDirector(ULevelSequencePlayer * SequencePlayer, const int32 Subsequence = 0);

	UFUNCTION(
		BlueprintCallable,
		BlueprintPure,
		Category = "Game|Cinematic",
		meta = (
			KeyWords = "simona level sequence sequencer"
		)
	)
	static UMovieScene * GetMovieScene(const UMovieSceneSequence * Sequence);

	UFUNCTION(
		BlueprintCallable,
		Category = "Game|Cinematic",
		meta = (
			KeyWords = "simona level sequence sequencer"
		)
	)
	static void SortMarkedFrames(UMovieScene * MovieScene);

	UFUNCTION(
		BlueprintCallable,
		Category = "Game|Cinematic",
		meta = (
			KeyWords = "simona level sequence sequencer"
		)
	)
	static TArray<FMovieSceneMarkedFrame> GetMarkedFrames(const UMovieScene * MovieScene);

	UFUNCTION(
		BlueprintCallable,
		BlueprintPure,
		Category = "Game|Cinematic",
		meta = (
			KeyWords = "simona level sequence sequencer"
		)
	)
	static UMovieSceneSequence * GetSequence(const UMovieSceneSequencePlayer * Player);

	UFUNCTION(
		BlueprintCallable,
		BlueprintPure,
		Category = "Game|Cinematic",
		meta = (
			DevelopmentOnly,
			KeyWords = "simona level sequence sequencer"
		)
	)
	static FText GetTrackDisplayName(const UMovieSceneNameableTrack * Track);

	UFUNCTION(
		BlueprintCallable,
		Category = "Game|Cinematic",
		meta = (KeyWords = "simona level sequence sequencer")
	)
	static TArray<UMovieSceneTrack *> GetMasterTracks(const UMovieScene * MovieScene);

	UFUNCTION(
		BlueprintCallable,
		Category = "Game|Cinematic",
		meta = (KeyWords = "simona level sequence sequencer")
	)
	static TArray<FBPMovieSceneBinding> GetBindings(const UMovieScene * MovieScene);

	UFUNCTION(
		BlueprintCallable,
		Category = "Game|Cinematic",
		meta = (KeyWords = "simona level sequence sequencer")
	)
	static TArray<UMovieSceneSection *> GetSections(const UMovieSceneTrack * Track);

	UFUNCTION(
		BlueprintCallable,
		BlueprintPure,
		Category = "Game|Cinematic",
		meta = (KeyWords = "simona level sequence sequencer")
	)
	static FFrameNumberRange GetRange(const UMovieSceneSection * Section);

	UFUNCTION(
		BlueprintCallable,
		BlueprintPure,
		Category = "Game|Cinematic",
		meta = (
			KeyWords = "simona level sequence sequencer frame playback position",
			DeprecatedFunction,
			DeprecationMessage = "MovieSceneSequencePlayer already has this method"
		)
	)
	static FQualifiedFrameTime GetCurrentTime(const ULevelSequencePlayer * SequencePlayer);

	UFUNCTION(
		BlueprintCallable,
		BlueprintPure,
		Category = "Game|Cinematic",
		meta = (KeyWords = "simona level sequence sequencer")
	)
	static FFrameNumberRange GetPlaybackRange(const UMovieScene * MovieScene);

#pragma endregion ~ Sequencer

	// Tracing //
	/**************************************************************************************************/

#pragma region Tracing

	/**
*  Sweep a capsule against the world and return all initial overlaps using a specific profile, then overlapping hits and then first blocking hit
*  Results are sorted, so a blocking hit (if found) will be the last element of the array
*  Only the single closest blocking result will be generated, no tests will be done after that
*
* @param WorldContextObject	World context
* @param Start			Start of line segment
* @param End			End of line segment
* @param Rotation		Orientation of swept shape
* @param Radius			Radius of the capsule to sweep
* @param HalfHeight		Distance from center of capsule to tip of hemisphere endcap.
* @param ProfileName	The 'profile' used to determine which components to hit
* @param bTraceComplex	True to test against complex collision, false to test against simplified collision.
* @param OutHits		A list of hits, sorted along the trace from start to finish.  The blocking hit will be the last hit, if there was one.
* @return				True if there was a blocking hit, false otherwise.
*/
	UFUNCTION(
		BlueprintCallable,
		Category = "Collision",
		meta = (
			bIgnoreSelf = "true",
			WorldContext = "WorldContextObject",
			AutoCreateRefTerm = "ActorsToIgnore",
			DisplayName = "MultiCapsuleTraceByProfile (Advanced)",
			AdvancedDisplay = "TraceColor, TraceHitColor, DrawTime",
			Keywords = "sweep"
		)
	)
	static bool CapsuleTraceMultiByProfile(
		const UObject *          WorldContextObject,
		const FVector            Start,
		const FVector            End,
		const FRotator           Rotation,
		float                    Radius,
		float                    HalfHeight,
		FName                    ProfileName,
		bool                     bTraceComplex,
		const TArray<AActor *> & ActorsToIgnore,
		EDrawDebugTrace::Type    DrawDebugType,
		TArray<FHitResult> &     OutHits,
		bool                     bIgnoreSelf,
		FLinearColor             TraceColor    = FLinearColor::Red,
		FLinearColor             TraceHitColor = FLinearColor::Green,
		float                    DrawTime      = 5.0f
	);

#pragma endregion ~ Tracing

	// Asset user data //
	/*************************************************************************************************/

#pragma region Asset user data

	UFUNCTION(
		BlueprintCallable,
		BlueprintPure,
		Category = "AssetUserData",
		meta = (Keywords = "simona")
	)
	static bool IsAssetUserDataContainer(const UObject * Object);

	UFUNCTION(
		BlueprintCallable,
		Category = "AssetUserData",
		meta = (Keywords = "simona")
	)
	static void AddAssetUserData(UObject * AssetUserDataContainer, UAssetUserData * InUserData);

	UFUNCTION(
		BlueprintCallable,
		BlueprintPure,
		Category = "AssetUserData",
		meta = (
			DeterminesOutputType = "UserDataClass",
			Keywords = "simona"
		)
	)
	static UAssetUserData * GetAssetUserData(
		UObject *                   AssetUserDataContainer,
		TSubclassOf<UAssetUserData> UserDataClass
	);

	/**
	 * Get existing data of provided class, or create new
	 */
	UFUNCTION(
		BlueprintCallable,
		Category = "AssetUserData",
		meta = (
			DeterminesOutputType = "UserDataClass",
			Keywords = "simona"
		)
	)
	static UAssetUserData * RetrieveAssetUserData(
		UObject *                         AssetUserDataContainer,
		const TSubclassOf<UAssetUserData> UserDataClass,
		UObject *                         Outer
	);

	UFUNCTION(
		BlueprintCallable,
		Category = "AssetUserData",
		meta = (Keywords = "simona")
	)
	static void RemoveAssetUserData(UObject * AssetUserDataContainer, TSubclassOf<UAssetUserData> UserDataClass);

	UFUNCTION(
		BlueprintCallable,
		Category = "AssetUserData",
		meta = (Keywords = "simona")
	)
	static TArray<UAssetUserData *> GetAssetUserDataArray(const UObject * AssetUserDataContainer);

#pragma endregion ~ Asset user data

	// Reflection //
	/*************************************************************************************************/

#pragma region Reflection

	UFUNCTION(
		BlueprintCallable,
		Category = "Core|Reflection",
		meta = (
			AdvancedDisplay = 1,
			Keywords = "simona"
		)
	)
	static TArray<FBPPropertyData> GetProperties(
		const UObject * Object,
		bool            bIncludeSuper      = true,
		bool            bIncludeDeprecated = false
	);


	UFUNCTION(
		BlueprintCallable,
		Category = "Core|Reflection",
		meta = (
			Keywords = "simona"
		)
	)
	static TArray<UObject *> GetObjectArrayProperty(
		UObject *                     Object,
		UPARAM(ref) FBPPropertyData & PropertyData,
		bool &                        bSuccess
	);

	/** @return True on success */
	UFUNCTION(
		BlueprintCallable,
		Category = "Core|Reflection",
		meta = (Keywords = "simona")
	)
	static bool AssignObjectProperty(UObject * Object, const FBPPropertyData & PropertyData, UObject * Value);

	/** @return True on success */
	UFUNCTION(
		BlueprintCallable,
		Category = "Core|Reflection",
		meta = (Keywords = "simona")
	)
	static bool AssignObjectPropertyByName(UObject * Object, FName FieldName, UObject * Value);

#pragma endregion ~ Reflection

	// Utils //
	/*************************************************************************************************/

#pragma region Utils

	/**
	* This only duplicates object but doesn't register it anywhere if it have to. Never use it
	* until you know what you doing for sure.
	*
	* @param SourceObject the object being copied
	* @param Outer the outer to use for the object
	* @param Name the optional name of the object
	*
	* @return the copied object or null if it failed for some reason
	*/
	UFUNCTION(
		BlueprintCallable,
		meta = (
			DeterminesOutputType = "SourceObject",
			Keywords = "simona copy clone create spawn construct"
		),
		Category = "Core"
	)
	static UObject * DuplicateObject(
		const UObject * SourceObject,
		UObject *       Outer,
		const FName     Name = NAME_None
	);

	UFUNCTION(
		BlueprintCallable,
		meta = (Keywords = "simona copy clone create spawn construct"),
		Category = "Game"
	)
	static UActorComponent * DuplicateComponentTo(
		UActorComponent * OriginalComponent,
		AActor *          ToActor,
		FTransform        RelativeTransform,
		const FName       Name              = NAME_None,
		bool              bManualAttachment = false
	);

	UFUNCTION(
		BlueprintCallable,
		BlueprintPure,
		meta = (Keywords = "simona"),
		Category = "Collision"
	)
	static float GetPenetrationDepth(const FHitResult & Hit);

	UFUNCTION(
		BlueprintCallable,
		BlueprintPure,
		meta = (
			DeterminesOutputType = "UserDataClass",
			CompactNodeTitle = "DefaultObject",
			Keywords = "simona cdo"
		),
		Category = "Core"
	)
	static UObject * GetClassDefaultObject(const TSubclassOf<UObject> Class);

	UFUNCTION(
		BlueprintCallable,
		BlueprintPure,
		meta = (
			DeterminesOutputType = "To",
			DisplayName = "Cast",
			CompactNodeTitle = "Cast",
			Keywords = "simona"
		),
		Category = "Core"
	)
	static UObject * CastToProvidedClass(UObject * Object, const TSubclassOf<UObject> To);

	UFUNCTION(
		BlueprintCallable,
		BlueprintPure,
		CustomThunk,
		meta = (
			Keywords = "simona",
			CompactNodeTitle = "Deprecate",
			CustomStructureParam = "Dummy",
			DeprecatedFunction,
			DeprecationMessage = "Use MakeCompilerMessage node instead"
		),
		Category = "Development"
	)
	static void DeprecateMacro(UObject *& Dummy, FString DeprecationMessage)
	{
		check(0);
	}

	DECLARE_FUNCTION(execDeprecateMacro)
	{
		P_GET_OBJECT_REF(UObject, Dummy);
		P_GET_PROPERTY(FStrProperty, DeprecationMessage);
		P_FINISH;

		Dummy = nullptr;

		FBlueprintExceptionInfo ExceptionInfo(
			EBlueprintExceptionType::AccessViolation,
			DeprecationMessage.IsEmpty( )
			? NSLOCTEXT(
				"AdvancedRuntimBPLibrary",
				"Macro is deprecated",
				"This macro is deprecated. Please, check comments inside it for replacement."
			)
			: FText::FromString(DeprecationMessage)
		);
		FBlueprintCoreDelegates::ThrowScriptException(P_THIS, Stack, ExceptionInfo);
	}

	UFUNCTION(
		BlueprintCallable,
		BlueprintPure,
		Category = "Core",
		meta = (
			WorldContext = "WorldContext",
			Keywords = "simona"
		)
	)
	static EBPWorldType GetWorldType(UObject * WorldContext);

	/**
	 * Walks up the list of outers until it finds a package directly associated with the object.
	 *
	 * @return the package the object is in.
	 */
	UFUNCTION(
		BlueprintCallable,
		BlueprintPure,
		Category = "Core",
		meta = (Keywords = "simona")
	)
	static UPackage * GetPackage(const UObject * Object);

	/** Returns the transient top-level package, which is useful for temporarily storing objects that should never be saved */
	UFUNCTION(
		BlueprintCallable,
		BlueprintPure,
		Category = "Core",
		meta = (
			DisplayName = "GetTransientPackage",
			Keywords = "simona"
		)
	)
	static UPackage * GetTransientPackageBP( );

	/**
	 * Finds the outermost package and marks it dirty.
	 * The editor suppresses this behavior during load as it is against policy to dirty packages simply by loading them.
	 *
	 * @return false if the request to mark the package dirty was suppressed by the editor and true otherwise.
	 */
	UFUNCTION(
		BlueprintCallable,
		Category = "Core",
		meta = (Keywords = "simona")
	)
	static bool MarkPackageDirty(UObject * Object);

	/**
	 * Marks/Unmarks the package's bDirty flag, save the package to the transaction buffer if a transaction is ongoing
	 */
	UFUNCTION(
		BlueprintCallable,
		Category = "Core",
		meta = (Keywords = "simona")
	)
	static void SetDirtyFlag(const UPackage * Package, bool Value);

	/**
	 * Tries to find an object in memory. This will handle fully qualified paths of the form /path/packagename.object:subobject and resolve references for you.
	 *
	 * @param	Class			The to be found object's class
	 * @param	Outer			Outer object to look inside. If this is ANY_PACKAGE it will search all in memory packages, if this is null then InName should start with a package name
	 * @param	Name			The object path to search for an object, relative to InOuter
	 * @param	bExactClass		Whether to require an exact match with the passed in class
	 *
	 * @return	Returns a pointer to the found object or nullptr if none could be found
	 */
	UFUNCTION(
		BlueprintCallable,
		Category = "Core",
		meta = (
			DeterminesOutputType = "Class",
			Keywords = "simona"
		)
	)
	static UObject * FindObject(
		const TSubclassOf<UObject> Class,
		UObject *                  Outer,
		const FString              Name        = TEXT(""),
		const bool                 bExactClass = false
	);

	UFUNCTION(
		BlueprintCallable,
		Category = "Core",
		meta = (
			DeterminesOutputType = "Class",
			Keywords = "simona"
		)
	)
	static UObject * FindObjectFast(
		const TSubclassOf<UObject> Class,
		UObject *                  Outer,
		const FName                Name,
		const bool                 bExactClass = false,
		const bool                 bAnyPackage = false
	);

	UFUNCTION(
		BlueprintCallable,
		Category = "Core",
		meta = (
			DisplayName = "GetObjectsWithOuter",
			Keywords = "simona"
		)
	)
	static TArray<UObject *> GetObjectsWithOuterBP(const UObject * Outer, bool bIncludeNestedObjects = true);

	UFUNCTION(
		BlueprintCallable,
		Category = "Core",
		meta = (
			DeterminesOutputType = "ClassToLookFor",
			DisplayName = "FindObjectWithOuter",
			Keywords = "simona"
		)
	)
	static UObject * FindObjectWithOuterBP(
		const UObject *            Outer,
		const TSubclassOf<UObject> ClassToLookFor,
		FName                      NameToLookFor = NAME_None
	);

	UFUNCTION(
		BlueprintCallable,
		Category = "Core",
		meta = (
			DeterminesOutputType = "GetObjectsWithPackage",
			Keywords = "simona"
		)
	)
	static TArray<UObject *> GetObjectsWithPackageBP(const UPackage * Package, bool bIncludeNestedObjects = true);

	/**
	 * Returns whether an object is referenced, not counting references from itself
	 * Warning - is a heavy operation involving scanning of all UObjects
	 *
	 * @param	Object			Object to check
	 * @param	bCheckSubObjects	Treat subobjects as if they are the same as passed in object
	 */
	UFUNCTION(
		BlueprintCallable,
		Category = "Core",
		meta = (Keywords = "simona")
	)
	static TArray<FBPReferencerInformation> GetReferences(UObject * Object, bool bCheckSubObjects = false);

	UFUNCTION(
		BlueprintCallable,
		Category = "Core",
		meta = (Keywords = "simona")
	)
	static bool RenameObject(UObject * Object, int32 RenameFlags, FString NewName, UObject * NewOuter);

	UFUNCTION(
		BlueprintCallable,
		Category = "Core",
		meta = (Keywords = "simona")
	)
	static void CopyToClipboard(FString String);

	UFUNCTION(
		BlueprintCallable,
		Category = "Core",
		meta = (Keywords = "simona")
	)
	static FString PasteFromClipboard( );

	/** Returns Self if its available within current context */
	UFUNCTION(
		BlueprintCallable,
		BlueprintPure,
		meta = (
			DevelopmentOnly,
			HidePin = "Context",
			DefaultToSelf = "Context",
			Keywords = "simona"
		)
	)
	static UObject * OptionalSelf(UObject * Context) { return Context; }

#pragma endregion ~ Utils

	UFUNCTION(
		BlueprintCallable,
		Category="Animation",
		meta = (
			Keywords = "simona",
			DefaultToSelf = "Target",
			DisplayName = "Play Slot Animation as Dynamic Montage (Array)"
		)
	)
	static UAnimMontage * PlaySlotAnimationAsDynamicMontage(
		UAnimInstance *            Target,
		TArray<FSlotAnimationData> Slots,
		float                      BlendInTime            = 0.25,
		float                      BlendOutTime           = 0.25,
		float                      InPlayRate             = 1.0,
		int32                      LoopCount              = 1,
		float                      BlendOutTriggerTime    = -1.0,
		float                      InTimeToStartMontageAt = 0.0
	);
};


// Not sure how it shall be used, so not exposing it to BP yet
//
// UFUNCTION(
// 	BlueprintCallable,
// 	Category = "Game",
// 	meta = (Keywords = "simona")
// )
// static void BeginDestruction(UObject* Object);

// Doesn't work due to internal BP pointer checks
//
// UFUNCTION(
// 	BlueprintCallable,
// 	BlueprintPure,
// 	CustomThunk,
// 	meta = (
// 		Keywords = "simona",
// 		CompactNodeTitle = "NULL",
// 		CustomStructureParam = "OutNullptr"
// 	),
// 	Category = "Game"
// )
// static void GetNullptr(UPARAM(DisplayName = "Nullptr") UObject*& OutNullptr) { check(0); }
//
// DECLARE_FUNCTION(execGetNullptr)
// {
// 	// Unreal actually nulls memory fine without any additional effort
// 	// Moreover, this value won't be read anyway
//
// 	P_GET_OBJECT(UObject, OutNullptr);
// 	P_FINISH;
// 	
// 	*(&OutNullptr) = StaticClass()->GetDefaultObject();
// }
