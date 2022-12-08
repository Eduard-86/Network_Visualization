#include "AdvancedRuntimeBPLibrary.h"

#include "CombinedLog.h"
#include "MovieSceneNameableTrack.h"
#include "AdvancedToolsRuntime/Private/KismetTraceUtils.h"
#include "GameFramework/Actor.h"
#include "HAL/Runnable.h"
#include "HAL/RunnableThread.h"
#include "Interfaces/Interface_AssetUserData.h"
#include "LevelSequence/Public/LevelSequencePlayer.h"
#include "Misc/FrameRate.h"
#include "Windows/WindowsPlatformApplicationMisc.h"

UAdvancedToolsRuntimeBPLibrary::UAdvancedToolsRuntimeBPLibrary(
	const FObjectInitializer & ObjectInitializer
)
	: Super(ObjectInitializer)
{}

UObject * UAdvancedToolsRuntimeBPLibrary::DuplicateObject(
	const UObject * SourceObject,
	UObject *       Outer,
	const FName     Name
)
{
	return ::DuplicateObject<UObject>(SourceObject, Outer, Name);
}

UActorComponent * UAdvancedToolsRuntimeBPLibrary::DuplicateComponentTo(
	UActorComponent * OriginalComponent,
	AActor *          ToActor,
	const FTransform  RelativeTransform,
	const FName       Name,
	const bool        bManualAttachment
)
{
	UActorComponent * Result = ToActor->CreateComponentFromTemplate(OriginalComponent, Name);
	ToActor->FinishAddComponent(Result, bManualAttachment, RelativeTransform);
	return Result;
}

float UAdvancedToolsRuntimeBPLibrary::GetPenetrationDepth(const FHitResult & Hit)
{
	return Hit.PenetrationDepth;
}

UObject * UAdvancedToolsRuntimeBPLibrary::GetClassDefaultObject(const TSubclassOf<UObject> Class)
{
	return Class.Get( ) ? Class->ClassDefaultObject : nullptr;
}

UObject * UAdvancedToolsRuntimeBPLibrary::CastToProvidedClass(UObject * Object, const TSubclassOf<UObject> To)
{
	if (!Object || !To.Get( ))
		return nullptr;

	return To.Get( )->HasAnyClassFlags(CLASS_Interface)
	       ? (UObject *)(Object->GetInterfaceAddress(To.Get( )))
	       : (Object->IsA(To.Get( )) ? Object : nullptr);
}

EBPWorldType UAdvancedToolsRuntimeBPLibrary::GetWorldType(UObject * WorldContext)
{
	const UWorld * World = WorldContext ? WorldContext->GetWorld( ) : nullptr;
	return EBPWorldType(World ? World->WorldType.GetValue( ) : EBPWorldType::None);
}

UPackage * UAdvancedToolsRuntimeBPLibrary::GetPackage(const UObject * Object)
{
	return Object ? Object->GetPackage( ) : nullptr;
}

UPackage * UAdvancedToolsRuntimeBPLibrary::GetTransientPackageBP( )
{
	return GetTransientPackage( );
}

bool UAdvancedToolsRuntimeBPLibrary::MarkPackageDirty(UObject * Object)
{
	if (Object)
		return Object->MarkPackageDirty( );

	return false;
}

void UAdvancedToolsRuntimeBPLibrary::SetDirtyFlag(const UPackage * Package, const bool Value)
{
	if (Package)
		Package->GetPackage( )->SetDirtyFlag(Value);
}

UObject * UAdvancedToolsRuntimeBPLibrary::FindObject(
	const TSubclassOf<UObject> Class,
	UObject *                  Outer,
	const FString              Name,
	const bool                 bExactClass
)
{
	if (!Class.Get( ) || !Outer)
		return nullptr;

	return StaticFindObjectSafe(Class.Get( ), Outer, *Name, bExactClass);
}

UObject * UAdvancedToolsRuntimeBPLibrary::FindObjectFast(
	const TSubclassOf<UObject> Class,
	UObject *                  Outer,
	const FName                Name,
	const bool                 bExactClass,
	const bool                 bAnyPackage
)
{
	if (!Class.Get( ) || !Outer)
		return nullptr;

	return StaticFindObjectFastSafe(Class.Get( ), Outer, Name, bExactClass, bAnyPackage);
}

TArray<UObject *> UAdvancedToolsRuntimeBPLibrary::GetObjectsWithOuterBP(
	const UObject * Outer,
	const bool      bIncludeNestedObjects
)
{
	TArray<UObject *> Result;
	if (Outer)
	{
		GetObjectsWithOuter(Outer, Result, bIncludeNestedObjects);
	}

	return Result;
}

UObject * UAdvancedToolsRuntimeBPLibrary::FindObjectWithOuterBP(
	const UObject *            Outer,
	const TSubclassOf<UObject> ClassToLookFor,
	const FName                NameToLookFor
)
{
	if (Outer)
	{
		UObjectBase * const Result = FindObjectWithOuter(Outer, ClassToLookFor.Get( ), NameToLookFor);
		const UClass *      Class  = Result->GetClass( );

		return (Class && Class->IsChildOf(UObject::StaticClass( )))
		       ? static_cast<UObject *>(Result)
		       : nullptr;
	}

	return nullptr;
}

TArray<UObject *> UAdvancedToolsRuntimeBPLibrary::GetObjectsWithPackageBP(
	const UPackage * Package,
	const bool       bIncludeNestedObjects
)
{
	TArray<UObject *> Result;

	if (Package)
	{
		GetObjectsWithPackage(Package, Result, bIncludeNestedObjects);
	}

	return Result;
}


TArray<FBPPropertyData> UAdvancedToolsRuntimeBPLibrary::GetProperties(
	const UObject * Object,
	const bool      bIncludeSuper,
	const bool      bIncludeDeprecated
)
{
	TArray<FBPPropertyData> Result;

	if (!Object)
		return Result;

	const UClass *                             Class           = Object->GetClass( );
	const EFieldIteratorFlags::SuperClassFlags SuperClassFlags =
			bIncludeSuper
			? EFieldIteratorFlags::SuperClassFlags::IncludeSuper
			: EFieldIteratorFlags::SuperClassFlags::ExcludeSuper;
	const EFieldIteratorFlags::DeprecatedPropertyFlags DeprecatedPropertyFlags =
			bIncludeDeprecated
			? EFieldIteratorFlags::DeprecatedPropertyFlags::IncludeDeprecated
			: EFieldIteratorFlags::DeprecatedPropertyFlags::ExcludeDeprecated;

	for (TFieldIterator<FProperty> FieldIterator(Class, SuperClassFlags, DeprecatedPropertyFlags); FieldIterator; ++
	     FieldIterator)
	{
		FProperty * Field = *FieldIterator;

		FBPPropertyData & Item  = Result.Emplace_GetRef( );
		Item.Property           = Field;
		Item.PropertyName       = Field->NamePrivate;
		Item.bIsExposedToEditor = Field->HasAllPropertyFlags(CPF_Edit);
		if (const auto ArrayField = CastField<FArrayProperty>(Field))
		{
			Item.bIsArray = true;
			Field         = ArrayField->Inner;
		}
		else
		{
			Item.bIsArray = false;
		}


		if (false) {}
		else if (const auto BoolField = CastField<FBoolProperty>(Field))
		{
			Item.FieldType = Boolean;
			if (!Item.bIsArray)
				Item.BooleanValue = *BoolField->ContainerPtrToValuePtr<bool>(Object);
		}
#pragma region Numerical properties
		else if (const auto ByteField = CastField<FByteProperty>(Field))
		{
			Item.FieldType       = Integer8;
			Item.bIsSignedNumber = false;
			if (!Item.bIsArray)
				Item.UnsignedInteger8Value = *ByteField->ContainerPtrToValuePtr<uint8>(Object);
		}
		else if (const auto Int8Field = CastField<FInt8Property>(Field))
		{
			Item.FieldType       = Integer8;
			Item.bIsSignedNumber = true;
		}
		else if (const auto UInt16Field = CastField<FUInt16Property>(Field))
		{
			Item.FieldType       = Integer16;
			Item.bIsSignedNumber = false;
		}
		else if (const auto Int16Field = CastField<FInt16Property>(Field))
		{
			Item.FieldType       = Integer16;
			Item.bIsSignedNumber = true;
		}
		else if (const auto UInt32Field = CastField<FUInt32Property>(Field))
		{
			Item.FieldType       = Integer32;
			Item.bIsSignedNumber = false;
		}
		else if (const auto Int32Field = CastField<FIntProperty>(Field))
		{
			Item.FieldType       = Integer32;
			Item.bIsSignedNumber = true;
			if (!Item.bIsArray)
				Item.Integer32Value = *Int32Field->ContainerPtrToValuePtr<int32>(Object);
		}
		else if (const auto UInt64Field = CastField<FUInt64Property>(Field))
		{
			Item.FieldType       = Integer64;
			Item.bIsSignedNumber = false;
		}
		else if (const auto Int64Field = CastField<FInt64Property>(Field))
		{
			Item.FieldType       = Integer64;
			Item.bIsSignedNumber = true;
			if (!Item.bIsArray)
				Item.Integer64Value = *Int64Field->ContainerPtrToValuePtr<int64>(Object);
		}
		else if (const auto FloatField = CastField<FFloatProperty>(Field))
		{
			Item.FieldType = Float;
			if (!Item.bIsArray)
				Item.FloatValue = *FloatField->ContainerPtrToValuePtr<float>(Object);
		}
		else if (const auto DoubleField = CastField<FDoubleProperty>(Field))
		{
			Item.FieldType = Double;
		}
#pragma endregion ~ Numerical properties
		else if (const auto NameField = CastField<FNameProperty>(Field))
		{
			Item.FieldType = Name;
			if (!Item.bIsArray)
				Item.NameValue = *NameField->ContainerPtrToValuePtr<FName>(Object);
		}
		else if (const auto StringField = CastField<FStrProperty>(Field))
		{
			Item.FieldType = String;
			if (!Item.bIsArray)
				Item.StringValue = *StringField->ContainerPtrToValuePtr<FString>(Object);
		}
		else if (const auto TextField = CastField<FTextProperty>(Field))
		{
			Item.FieldType = Text;
			if (!Item.bIsArray)
				Item.TextValue = *TextField->ContainerPtrToValuePtr<FText>(Object);
		}
		else if (const auto DelegateField = CastField<FDelegateProperty>(Field))
		{
			Item.FieldType = Delegate;
		}
		else if (const auto StructField = CastField<FStructProperty>(Field))
		{
			Item.FieldType      = Structure;
			Item.StructTypeName = StructField->Struct->GetFName( );
		}
		else if (const auto ObjectField = CastField<FObjectProperty>(Field))
		{
			Item.FieldType  = EFieldType::Object;
			Item.ObjectType = ObjectField->PropertyClass;
			if (!Item.bIsArray)
				Item.ObjectValue = *ObjectField->ContainerPtrToValuePtr<UObject *>(Object);
		}
	}

	return Result;
}

TArray<UObject *> UAdvancedToolsRuntimeBPLibrary::GetObjectArrayProperty(
	UObject *         Object,
	FBPPropertyData & PropertyData,
	bool &            bSuccess
)
{
	TArray<UObject *> Result;

	if (false
		|| !Object
		|| PropertyData.FieldType != EFieldType::Object
		|| !PropertyData.bIsArray
	)
	{
		bSuccess = false;
		return Result;
	}

	if (ensure(PropertyData.Property != nullptr))
	{
		const auto Array = CastField<FArrayProperty>(PropertyData.Property);
		if (ensure(Array))
		{
			const TArray<UObject *> * ArrayPtr = Array->ContainerPtrToValuePtr<TArray<UObject *>>(Object);
			Result                             = *ArrayPtr;
			bSuccess                           = true;
		}
	}

	return Result;
}

bool UAdvancedToolsRuntimeBPLibrary::AssignObjectProperty(
	UObject *               Object,
	const FBPPropertyData & PropertyData,
	UObject *               Value
)
{
	if (false
		|| !Object
		|| PropertyData.FieldType != EFieldType::Object
		|| (Value && Value->GetClass( ) != PropertyData.ObjectType)
	)
		return false;

	bool Result = false;
	if (ensure(PropertyData.Property != nullptr))
	{
		UObject ** FieldPtr = PropertyData.Property->ContainerPtrToValuePtr<UObject *>(Object);
		*FieldPtr           = Value;
		Result              = true;
	}

	return Result;
}

bool UAdvancedToolsRuntimeBPLibrary::AssignObjectPropertyByName(
	UObject *   Object,
	const FName FieldName,
	UObject *   Value
)
{
	if (!Object || FieldName.IsNone( ))
		return false;

	bool Result = false;
	const UClass * Class = Object->GetClass( );
	constexpr EFieldIteratorFlags::SuperClassFlags SuperClassFlags = EFieldIteratorFlags::SuperClassFlags::IncludeSuper;
	constexpr EFieldIteratorFlags::DeprecatedPropertyFlags DeprecatedPropertyFlags =
			EFieldIteratorFlags::DeprecatedPropertyFlags::ExcludeDeprecated;

	for (TFieldIterator<FProperty> Field(Class, SuperClassFlags, DeprecatedPropertyFlags); Field; ++Field)
	{
		if (Field->NamePrivate == FieldName)
		{
			if (const auto ObjField = CastField<FObjectProperty>(*Field))
			{
				UObject ** FieldPtr = ObjField->ContainerPtrToValuePtr<UObject *>(Object);
				*FieldPtr           = Value;
				Result              = true;
			}
			break;
		}
	}

	return Result;
}

TArray<FBPReferencerInformation> UAdvancedToolsRuntimeBPLibrary::GetReferences(
	UObject *  Object,
	const bool bCheckSubObjects
)
{
	TArray<FBPReferencerInformation> Result;

	if (Object && !Object->IsUnreachable( ))
	{
		FReferencerInformationList ReferenceList;
		if (IsReferenced(Object, RF_NoFlags, EInternalObjectFlags::None, bCheckSubObjects, &ReferenceList))
		{
			for (const FReferencerInformation & Reference : ReferenceList.InternalReferences)
			{
				auto & Item               = Result.Emplace_GetRef( );
				Item.bIsExternalReference = false;
				Item.Referencer           = Reference.Referencer;
				Item.TotalReferences      = Reference.TotalReferences;
				for (const FProperty * Property : Reference.ReferencingProperties)
				{
					Item.ReferencingProperties.Push(Property->NamePrivate);
				}
			}
			for (const FReferencerInformation & Reference : ReferenceList.ExternalReferences)
			{
				auto & Item               = Result.Emplace_GetRef( );
				Item.bIsExternalReference = true;
				Item.Referencer           = Reference.Referencer;
				Item.TotalReferences      = Reference.TotalReferences;
				for (const FProperty * Property : Reference.ReferencingProperties)
				{
					Item.ReferencingProperties.Push(Property->NamePrivate);
				}
			}
		}
	}

	return Result;
}

bool UAdvancedToolsRuntimeBPLibrary::RenameObject(
	UObject *     Object,
	const int32   RenameFlags,
	const FString NewName,
	UObject *     NewOuter
)
{
	return Object
	       ? Object->Rename(
		       NewName.IsEmpty( ) ? nullptr : *NewName,
		       NewOuter,
		       ERenameFlags(RenameFlags)
	       )
	       : false;
}

void UAdvancedToolsRuntimeBPLibrary::CopyToClipboard(const FString String)
{
	FPlatformApplicationMisc::ClipboardCopy(*String);
}

FString UAdvancedToolsRuntimeBPLibrary::PasteFromClipboard( )
{
	FString Result;
	FPlatformApplicationMisc::ClipboardPaste(Result);
	return Result;
}

UAnimMontage * UAdvancedToolsRuntimeBPLibrary::PlaySlotAnimationAsDynamicMontage(
	UAnimInstance *            Target,
	TArray<FSlotAnimationData> Slots,
	const float                BlendInTime,
	const float                BlendOutTime,
	const float                InPlayRate,
	const int32                LoopCount,
	const float                BlendOutTriggerTime,
	const float                InTimeToStartMontageAt
)
{
	// Checks
	if (!Target || Slots.Num( ) == 0)
		return nullptr;

	for (const FSlotAnimationData & Slot : Slots)
	{
		if (false
			|| !Slot.Asset
			|| Slot.SlotNodeName.IsNone( )
			|| !Target->CurrentSkeleton->IsCompatible(Slot.Asset->GetSkeleton( ))
			|| !Slot.Asset->CanBeUsedInComposition( )
			|| Slot.Asset->IsA<UAnimMontage>( )
		)
			return nullptr;
	}

	// Create montage
	UAnimMontage * NewMontage = NewObject<UAnimMontage>( );
	NewMontage->SetSkeleton(Slots[0].Asset->GetSkeleton( ));

	// Add tracks
	NewMontage->SlotAnimTracks.Empty( );
	for (const FSlotAnimationData & Slot : Slots)
	{
		FSlotAnimationTrack & Track = NewMontage->AddSlot(Slot.SlotNodeName);
		Track.SlotName              = Slot.SlotNodeName;
		FAnimSegment NewSegment;
		NewSegment.AnimReference = Slot.Asset;
		NewSegment.AnimStartTime = 0.f;

		float SequenceLength;
#if ENGINE_MAJOR_VERSION >= 5
		SequenceLength = Slot.Asset->GetPlayLength( );
#else
		SequenceLength = Slot.Asset->SequenceLength;
#endif

		NewSegment.AnimEndTime  = SequenceLength;
		NewSegment.AnimPlayRate = 1.f;
		NewSegment.StartPos     = 0.f;
		NewSegment.LoopingCount = LoopCount;
		// Check the original code for reference lol
		PRAGMA_DISABLE_DEPRECATION_WARNINGS
		NewMontage->SequenceLength = NewSegment.GetLength( );
		PRAGMA_ENABLE_DEPRECATION_WARNINGS
		Track.AnimTrack.AnimSegments.Add(NewSegment);

		FCompositeSection NewSection;
		NewSection.SectionName = TEXT("Default");
		NewSection.LinkSequence(Slot.Asset, SequenceLength);
		NewSection.SetTime(0.0f);

		// add new section
		NewMontage->CompositeSections.Add(NewSection);
		NewMontage->BlendIn.SetBlendTime(BlendInTime);
		NewMontage->BlendOut.SetBlendTime(BlendOutTime);
		NewMontage->BlendOutTriggerTime = BlendOutTriggerTime;
	}

	// Play montage
	if (NewMontage)
	{
		const float PlayTime = Target->Montage_Play(
			NewMontage,
			InPlayRate,
			EMontagePlayReturnType::MontageLength,
			InTimeToStartMontageAt
		);
		return PlayTime > 0.0f ? NewMontage : nullptr;
	}

	return nullptr;
}


struct ADVANCEDTOOLSRUNTIME_API FSimpleRunnable final : public FRunnable
{
private:
	FSimpleRunnableSignature Actions;
	FCriticalSection         SyncSection;
	FRunnableThread *        Thread;

public:
	FSimpleRunnable(const FSimpleRunnableSignature & InActions, const FString & ThreadName)
		: Actions(InActions)
	{
		{
			FScopeLock Lock(&SyncSection);
			Thread = FRunnableThread::Create(this, *ThreadName);
		}

		if (UNLIKELY(!Thread))
		{
			delete this;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("\"%s\" thread has been initialized"), *ThreadName);
		}
	}

	// bool Init( ) override { }

	virtual uint32 Run( ) override
	{
		Actions.ExecuteIfBound( );
		return 0;
	}

	virtual void Exit( ) override
	{
		UE_LOG(LogTemp, Display, TEXT("RunAsync: Thread \"%s\" completed"), *this->Thread->GetThreadName());
		delete this;
	}

	virtual ~FSimpleRunnable( ) override
	{
		FScopeLock Lock(&SyncSection);
		delete Thread;
	}
};


void UAdvancedToolsRuntimeBPLibrary::RunAsync(
	const FSimpleRunnableSignature & Function,
	const FString &                  ThreadName
)
{
	new FSimpleRunnable(Function, ThreadName);
}

// Sequencer //
/*************************************************************************************************/

int32 UAdvancedToolsRuntimeBPLibrary::GetCurrentMovieSceneFrame(const UMovieSceneSequencePlayer * SequencePlayer)
{
	if (!SequencePlayer)
		return -1;

	const FFrameRate PlayerFrameRate      = SequencePlayer->GetFrameRate( );
	const FFrameRate TargetTrackFrameRate = SequencePlayer->GetSequence( )
	                                                      ->GetMovieScene( )
	                                                      ->GetTickResolution( );
	// Optimized proportion calculation
	const int32 PlayerTime   = SequencePlayer->GetCurrentTime( ).Time.FrameNumber.Value;
	const int32 CurrentFrame =
			PlayerTime * PlayerFrameRate.Denominator * TargetTrackFrameRate.Numerator
			/ PlayerFrameRate.Numerator / TargetTrackFrameRate.Denominator;

	return CurrentFrame;
}

int32 UAdvancedToolsRuntimeBPLibrary::SequencePlayerFrameToMovieSceneFrame(
	const FFrameNumber                FrameNumber,
	const UMovieSceneSequencePlayer * SequencePlayer
)
{
	if (!SequencePlayer)
		return -1;

	const FFrameRate PlayerFrameRate      = SequencePlayer->GetFrameRate( );
	const FFrameRate TargetTrackFrameRate = SequencePlayer->GetSequence( )
	                                                      ->GetMovieScene( )
	                                                      ->GetTickResolution( );

	// Optimized proportion calculation
	const int32 CurrentFrame =
			FrameNumber.Value * PlayerFrameRate.Denominator * TargetTrackFrameRate.Numerator
			/ PlayerFrameRate.Numerator / TargetTrackFrameRate.Denominator;

	return CurrentFrame;
}

FFrameTime UAdvancedToolsRuntimeBPLibrary::MovieSceneFrameToSequencePlayerFrame(
	const FFrameNumber           FrameNumber,
	const ULevelSequencePlayer * SequencePlayer
)
{
	if (!SequencePlayer)
		return -1;

	const FFrameRate PlayerFrameRate      = SequencePlayer->GetFrameRate( );
	const FFrameRate TargetTrackFrameRate = SequencePlayer->GetSequence( )
	                                                      ->GetMovieScene( )
	                                                      ->GetTickResolution( );

	const FFrameTime FrameTime =
			FrameNumber.Value * PlayerFrameRate.Numerator * TargetTrackFrameRate.Denominator
			/ PlayerFrameRate.Denominator / TargetTrackFrameRate.Numerator;

	return FrameTime;
}

UMovieSceneTrack * UAdvancedToolsRuntimeBPLibrary::GetMasterTrack(
	const ULevelSequencePlayer & SequencePlayer,
	const FString &              MasterTrackType,
	const int32                  TrackIndex
)
{
	// Get target track or notify one doesn't exist
	FString                   TrackIndexString = FString::FromInt(TrackIndex);
	UMovieSceneTrack * const* TargetTrackPtr   =
			SequencePlayer.GetSequence( )->GetMovieScene( )->GetMasterTracks( ).FindByPredicate(
				[&MasterTrackType, TrackIndexString](const UMovieSceneTrack * Track)
				{
					// track->GetTrackName() only works WITH_EDITORONLY_DATA
					const FString Name     = Track->GetFName( ).ToString( );
					const int32   FoundPos = Name.Find(MasterTrackType);
					if (FoundPos > 0)
					{
						return Name.Find(
							TrackIndexString,
							ESearchCase::IgnoreCase,
							ESearchDir::FromEnd
						) > 0;
					}
					return false;
				}
			);

	if (TargetTrackPtr == nullptr)
	{
		COMBO_LOG_FUNC(
			LogTemp,
			Error,
			"LevelSequence has no \"%s_%s\" track!",
			*MasterTrackType,
			*TrackIndexString
		);
		return nullptr;
	}

	return *TargetTrackPtr;
}

void UAdvancedToolsRuntimeBPLibrary::SkipToEndOfCurrentSection(
	ULevelSequencePlayer * SequencePlayer,
	const FString &        MasterTrackType,
	int32                  TrackIndex
)
{
	if (!SequencePlayer)
	{
		COMBO_LOG_FUNC(LogTemp, Error, "no level sequence player provided!");
		return;
	}
	if (MasterTrackType == "")
	{
		COMBO_LOG_FUNC(LogTemp, Error, "track name must not be empty!");
		return;
	}
	if (TrackIndex < 0)
	{
		COMBO_LOG_FUNC(LogTemp, Error, "invalid track index: %i!", TrackIndex);
		return;
	}
	// When trying to skip and sequence isn't currently playing, sequence is
	// entered and, seemingly, paused
	if (!SequencePlayer->IsPlaying( ))
	{
		COMBO_LOG_FUNC(LogTemp, Verbose, "level sequence is not playing");
		return;
	}

	//
	// Get target track
	//
	UMovieSceneTrack * TargetTrack = GetMasterTrack(*SequencePlayer, MasterTrackType, TrackIndex);
	if (!TargetTrack)
	{
		COMBO_LOG_FUNC(
			LogTemp,
			Warning,
			"couldn't find track %s...%i!",
			*MasterTrackType,
			TrackIndex
		);
		return;
	}

	//
	// Check track is compatible
	//
	const TArray<UMovieSceneSection *> & Sections = TargetTrack->GetAllSections( );
	if (Sections.Num( ) == 0)
	{
		COMBO_LOG_FUNC(LogTemp, Warning, "provided track has no sections!");
		return;
	}
	if (!Sections[0]->HasEndFrame( ))
	{
		COMBO_LOG_FUNC(LogTemp, Warning, "provided track is not supported!");
		return;
	}

	// Find the first segment, that's not ended yet
	// Convert its time back to player time
	// Skip to to this time
	int32 CurrentFrame = GetCurrentMovieSceneFrame(SequencePlayer);
	for (auto Section : TargetTrack->GetAllSections( ))
	{
		FFrameNumber FirstFrameAfterSection = Section->GetExclusiveEndFrame( );
		if (FirstFrameAfterSection > CurrentFrame)
		{
			FMovieSceneSequencePlaybackParams Params;
			Params.PositionType = EMovieScenePositionType::Frame;
			Params.Frame        = MovieSceneFrameToSequencePlayerFrame(FirstFrameAfterSection, SequencePlayer);
			SequencePlayer->SetPlaybackPosition(Params);
			return;
		}
	}

	COMBO_LOG_FUNC(LogTemp, Verbose, "no section to skip");
}

int32 UAdvancedToolsRuntimeBPLibrary::GetCurrentSectionIndex(
	ULevelSequencePlayer * SequencePlayer,
	const FString &        MasterTrackType,
	int32                  TrackIndex
)
{
	constexpr int InvalidIndex = -1;

	if (SequencePlayer == nullptr)
	{
		COMBO_LOG_FUNC(LogTemp, Error, "no level sequence player provided!");
		return InvalidIndex;
	}
	if (MasterTrackType == "")
	{
		COMBO_LOG_FUNC(LogTemp, Error, "track name must not be empty!");
		return InvalidIndex;
	}
	if (TrackIndex < 0)
	{
		COMBO_LOG_FUNC(LogTemp, Error, "invalid track index: %i!", TrackIndex);
		return InvalidIndex;
	}

	//
	// Get target track
	//
	UMovieSceneTrack * TargetTrack = GetMasterTrack(*SequencePlayer, MasterTrackType, TrackIndex);
	if (!TargetTrack)
	{
		COMBO_LOG_FUNC(
			LogTemp,
			Warning,
			"couldn't find track %s...%i!",
			*MasterTrackType,
			TrackIndex
		);
		return InvalidIndex;
	}

	//
	// Check track is compatible
	//
	const TArray<UMovieSceneSection *> & Sections = TargetTrack->GetAllSections( );
	if (Sections.Num( ) == 0)
	{
		COMBO_LOG_FUNC(LogTemp, Warning, "provided track has no sections!");
		return InvalidIndex;
	}
	if (!Sections[0]->HasEndFrame( ))
	{
		COMBO_LOG_FUNC(LogTemp, Warning, "provided track is not supported!");
		return InvalidIndex;
	}

	// Find current section
	int32 Frame        = GetCurrentMovieSceneFrame(SequencePlayer);
	int32 SectionIndex = 0;
	for (auto Section : Sections)
	{
		if (Section->GetExclusiveEndFrame( ) < Frame)
		{
			SectionIndex++;
			continue;
		}

		if (Section->GetInclusiveStartFrame( ) <= Frame)
		{
			return SectionIndex;
		}
		else
		{
			return InvalidIndex;
		}
	}

	return InvalidIndex;
}

UObject * UAdvancedToolsRuntimeBPLibrary::GetOrCreateDirector(
	ULevelSequencePlayer * SequencePlayer,
	const int32            Subsequence
)
{
	if (!SequencePlayer)
		return nullptr;

	// Fails because simply creates a director that is not associated with anything
	//SequencePlayer->GetSequence()->CreateDirectorInstance(*SequencePlayer);

	return static_cast<IMovieScenePlayer *>(SequencePlayer)
	       ->GetEvaluationTemplate( )
	       .GetOrCreateDirectorInstance(
		       FMovieSceneSequenceID(Subsequence),
		       *SequencePlayer
	       );
}

UMovieScene * UAdvancedToolsRuntimeBPLibrary::GetMovieScene(const UMovieSceneSequence * Sequence)
{
	return Sequence ? Sequence->GetMovieScene( ) : nullptr;
}

void UAdvancedToolsRuntimeBPLibrary::SortMarkedFrames(UMovieScene * MovieScene)
{
	if (MovieScene)
		MovieScene->SortMarkedFrames( );
}

TArray<FMovieSceneMarkedFrame> UAdvancedToolsRuntimeBPLibrary::GetMarkedFrames(
	const UMovieScene * MovieScene
)
{
	return MovieScene ? MovieScene->GetMarkedFrames( ) : TArray<FMovieSceneMarkedFrame>( );
}

UMovieSceneSequence * UAdvancedToolsRuntimeBPLibrary::GetSequence(const UMovieSceneSequencePlayer * Player)
{
	return Player ? Player->GetSequence( ) : nullptr;
}

FText UAdvancedToolsRuntimeBPLibrary::GetTrackDisplayName(const UMovieSceneNameableTrack * Track)
{
	return
#if WITH_EDITORONLY_DATA
			Track
			? Track->GetDisplayName( )
			:
#endif
			FText::GetEmpty( );
}

TArray<UMovieSceneTrack *> UAdvancedToolsRuntimeBPLibrary::GetMasterTracks(const UMovieScene * MovieScene)
{
	if (!MovieScene)
		return { };

	return MovieScene->GetMasterTracks( );
}

TArray<FBPMovieSceneBinding> UAdvancedToolsRuntimeBPLibrary::GetBindings(const UMovieScene * MovieScene)
{
	if (!MovieScene)
		return { };

	TArray<FBPMovieSceneBinding> Result;
	for (const FMovieSceneBinding & Binding : MovieScene->GetBindings( ))
	{
		FBPMovieSceneBinding & Item = Result.Emplace_GetRef( );
		Item.BindingName            = Binding.GetName( );
		Item.ObjectGuid             = Binding.GetObjectGuid( );
		Item.Tracks                 = Binding.GetTracks( );
	}

	return Result;
}

TArray<UMovieSceneSection *> UAdvancedToolsRuntimeBPLibrary::GetSections(const UMovieSceneTrack * Track)
{
	return Track ? Track->GetAllSections( ) : TArray<UMovieSceneSection *>( );
}

FFrameNumberRange UAdvancedToolsRuntimeBPLibrary::GetRange(const UMovieSceneSection * Section)
{
	return Section ? Section->GetRange( ) : TRange<FFrameNumber>( );
}

FQualifiedFrameTime UAdvancedToolsRuntimeBPLibrary::GetCurrentTime(const ULevelSequencePlayer * SequencePlayer)
{
	return SequencePlayer ? SequencePlayer->GetCurrentTime( ) : FQualifiedFrameTime( );
}

FFrameNumberRange UAdvancedToolsRuntimeBPLibrary::GetPlaybackRange(const UMovieScene * MovieScene)
{
	return MovieScene ? MovieScene->GetPlaybackRange( ) : FFrameNumberRange( );
}


// Tracing //
/**************************************************************************************************/

#pragma region Tracing

bool UAdvancedToolsRuntimeBPLibrary::CapsuleTraceMultiByProfile(
	const UObject *             WorldContextObject,
	const FVector               Start,
	const FVector               End,
	const FRotator              Rotation,
	const float                 Radius,
	const float                 HalfHeight,
	const FName                 ProfileName,
	const bool                  bTraceComplex,
	const TArray<AActor *> &    ActorsToIgnore,
	const EDrawDebugTrace::Type DrawDebugType,
	TArray<FHitResult> &        OutHits,
	const bool                  bIgnoreSelf,
	const FLinearColor          TraceColor,
	const FLinearColor          TraceHitColor,
	const float                 DrawTime
)
{
	static const FName          CapsuleTraceMultiName(TEXT("CapsuleTraceMultiByProfile (Advanced)"));
	const FCollisionQueryParams Params = Copied::ConfigureCollisionParams(
		CapsuleTraceMultiName,
		bTraceComplex,
		ActorsToIgnore,
		bIgnoreSelf,
		WorldContextObject
	);

	const FQuat    Rot(Rotation);
	const UWorld * World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	const bool     bHit  = World
	                       ? World->SweepMultiByProfile(
		                       OutHits,
		                       Start,
		                       End,
		                       Rot,
		                       ProfileName,
		                       FCollisionShape::MakeCapsule(Radius, HalfHeight),
		                       Params
	                       )
	                       : false;

#if ENABLE_DRAW_DEBUG
	Copied::DrawDebugCapsuleTraceMulti(
		World,
		Start,
		End,
		Rot,
		Radius,
		HalfHeight,
		DrawDebugType,
		bHit,
		OutHits,
		TraceColor,
		TraceHitColor,
		DrawTime
	);
#endif

	return bHit;
}

#pragma endregion ~ Tracing

// Asset user data //
/*************************************************************************************************/

#pragma region Asset user data

bool UAdvancedToolsRuntimeBPLibrary::IsAssetUserDataContainer(const UObject * Object)
{
	return Cast<IInterface_AssetUserData>(Object) != nullptr;
}

void UAdvancedToolsRuntimeBPLibrary::AddAssetUserData(UObject * AssetUserDataContainer, UAssetUserData * InUserData)
{
	IInterface_AssetUserData * Container = Cast<IInterface_AssetUserData>(AssetUserDataContainer);

	if (InUserData && Container)
	{
		Container->AddAssetUserData(InUserData);
	}
}

UAssetUserData * UAdvancedToolsRuntimeBPLibrary::GetAssetUserData(
	UObject *                         AssetUserDataContainer,
	const TSubclassOf<UAssetUserData> UserDataClass
)
{
	UAssetUserData * Result    = nullptr;
	const auto       Container = Cast<IInterface_AssetUserData>(AssetUserDataContainer);

	if (UserDataClass.Get( ) && Container)
	{
		Result = Container->GetAssetUserDataOfClass(UserDataClass);
	}

	return Result;
}

UAssetUserData * UAdvancedToolsRuntimeBPLibrary::RetrieveAssetUserData(
	UObject *                         AssetUserDataContainer,
	const TSubclassOf<UAssetUserData> UserDataClass,
	UObject *                         Outer
)
{
	UAssetUserData * Result    = nullptr;
	const auto       Container = Cast<IInterface_AssetUserData>(AssetUserDataContainer);

	if (Container && UserDataClass.Get( ))
	{
		Result = Container->GetAssetUserDataOfClass(UserDataClass);
		if (!Result)
		{
			Result = NewObject<UAssetUserData>(Outer, UserDataClass.Get( ));
			Container->AddAssetUserData(Result);
		}
	}

	return Result;
}

void UAdvancedToolsRuntimeBPLibrary::RemoveAssetUserData(
	UObject *                         AssetUserDataContainer,
	const TSubclassOf<UAssetUserData> UserDataClass
)
{
	const auto Container = Cast<IInterface_AssetUserData>(AssetUserDataContainer);

	if (UserDataClass.Get( ) && Container)
	{
		Container->RemoveUserDataOfClass(UserDataClass);
	}
}

TArray<UAssetUserData *> UAdvancedToolsRuntimeBPLibrary::GetAssetUserDataArray(const UObject * AssetUserDataContainer)
{
	const auto Container = Cast<IInterface_AssetUserData>(AssetUserDataContainer);

	return Container ? *Container->GetAssetUserDataArray( ) : TArray<UAssetUserData *>( );
}

#pragma endregion ~ Asset user data


// void UAdvancedToolsRuntimeBPLibrary::BeginDestruction(UObject * Object)
// {
// 	if (Object)
// 		Object->ConditionalBeginDestroy( );
//
//
// }
