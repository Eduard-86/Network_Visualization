#include "GeneratedFunction.h"
#include "EdGraph/EdGraph.h"
#include "Engine/Blueprint.h"
#include "Inspector/Enchantments.h"

//Keep in sync with AnimGraph/Public/IAnimBlueprintCompilationContext.h
#define ANIM_FUNC_DECORATOR	TEXT("__AnimFunc")

FGeneratedFunction::FGeneratedFunction(const UBlueprint & Blueprint, const UEdGraph & FunctionGraph)
{
	const FName FunctionName = GetFunctionName(FunctionGraph);

	const EBlueprintStatus BPStatus = Blueprint.Status;
	// These guys are processed separately
	const bool bIsAnimGraph               = FunctionName.ToString( ).EndsWith(ANIM_FUNC_DECORATOR);
	const bool bHasGeneratedClass         = (bool)Blueprint.GeneratedClass;
	const bool bHasGeneratedClassSkeleton = (bool)Blueprint.SkeletonGeneratedClass;
	bool       bHasFunction               = bHasGeneratedClass;
	bool       bHasFunctionSkeleton       = bHasGeneratedClassSkeleton;

	if (BPStatus == BS_Dirty && bHasGeneratedClass && bHasGeneratedClassSkeleton)
	{
		bHasFunction         = (bool)Blueprint.GeneratedClass->FindFunctionByName(FunctionName);
		bHasFunctionSkeleton = (bool)Blueprint.SkeletonGeneratedClass->FindFunctionByName(FunctionName);
	}

	if (Blueprint.Status != BS_Error && bHasFunction && bHasFunctionSkeleton && !bIsAnimGraph)
	{
		GeneratedFunction         = Blueprint.GeneratedClass->FindFunctionByName(FunctionName);
		GeneratedFunctionSkeleton = Blueprint.SkeletonGeneratedClass->FindFunctionByName(FunctionName);
		//check(GeneratedFunction);
		//check(GeneratedFunctionSkeleton);
		if (GeneratedFunction && GeneratedFunctionSkeleton)
		{
			new(&MetaData) FMetaData(*GeneratedFunction);
			return;
		}
	}
	GeneratedFunction = GeneratedFunctionSkeleton = nullptr;
}

FGeneratedFunction::FGeneratedFunction(const UEdGraph & FunctionGraph)
{
	if (const UBlueprint * Blueprint = Cast<UBlueprint>(FunctionGraph.GetOuter( )))
	{
		new(this) FGeneratedFunction(*Blueprint, FunctionGraph);
	}
	else
	{
		GeneratedFunction = GeneratedFunctionSkeleton = nullptr;
	}
}

bool FGeneratedFunction::IsValid( ) const
{
	return (bool)GeneratedFunction;
}

void FGeneratedFunction::AddMeta(const TCHAR * Key, const TCHAR * Value)
{
	assumeChecked(GeneratedFunction);
	assumeChecked(GeneratedFunctionSkeleton);
	MetaData.Add(*GeneratedFunction, Key, Value);
	MetaData.Add(*GeneratedFunctionSkeleton, Key, Value);
}

void FGeneratedFunction::RemoveMeta(const TCHAR * Key)
{
	assumeChecked(GeneratedFunction);
	assumeChecked(GeneratedFunctionSkeleton);
	MetaData.Remove(*GeneratedFunction, Key);
	MetaData.Remove(*GeneratedFunctionSkeleton, Key);
}

void FGeneratedFunction::AddFlag(const TCHAR * FlagName, EFunctionFlags FlagValue)
{
	assumeChecked(GeneratedFunction);
	assumeChecked(GeneratedFunctionSkeleton);
	AddMeta(FlagName);
	EnumAddFlags(GeneratedFunction->FunctionFlags, FlagValue);
	EnumAddFlags(GeneratedFunctionSkeleton->FunctionFlags, FlagValue);
}

void FGeneratedFunction::RemoveFlag(const TCHAR * FlagName, EFunctionFlags FlagValue)
{
	assumeChecked(GeneratedFunction);
	assumeChecked(GeneratedFunctionSkeleton);
	RemoveMeta(FlagName);
	EnumRemoveFlags(GeneratedFunction->FunctionFlags, FlagValue);
	EnumRemoveFlags(GeneratedFunctionSkeleton->FunctionFlags, FlagValue);
}

void FGeneratedFunction::AddPropertyFlag(FName PropertyName, EPropertyFlags Flag)
{
	for (TFieldIterator<FProperty> PropertyIterator(GeneratedFunction); PropertyIterator; ++PropertyIterator)
	{
		if (PropertyIterator->NamePrivate == PropertyName)
		{
			EnumAddFlags(PropertyIterator->PropertyFlags, Flag);
			break;
		}
	}
	for (TFieldIterator<FProperty> PropertyIterator(GeneratedFunctionSkeleton); PropertyIterator; ++PropertyIterator)
	{
		if (PropertyIterator->NamePrivate == PropertyName)
		{
			EnumAddFlags(PropertyIterator->PropertyFlags, Flag);
			break;
		}
	}
}

void FGeneratedFunction::RemovePropertyFlag(FName PropertyName, EPropertyFlags Flag)
{
	for (TFieldIterator<FProperty> PropertyIterator(GeneratedFunction); PropertyIterator; ++PropertyIterator)
	{
		if (PropertyIterator->NamePrivate == PropertyName)
		{
			EnumRemoveFlags(PropertyIterator->PropertyFlags, Flag);
			break;
		}
	}
	for (TFieldIterator<FProperty> PropertyIterator(GeneratedFunctionSkeleton); PropertyIterator; ++PropertyIterator)
	{
		if (PropertyIterator->NamePrivate == PropertyName)
		{
			EnumRemoveFlags(PropertyIterator->PropertyFlags, Flag);
			break;
		}
	}
}

void FGeneratedFunction::SyncMeta(const FEnchantments & Data, const TCHAR * Key)
{
	if (Data.Has(Key))
	{
		AddMeta(Key);
	}
	else
	{
		RemoveMeta(Key);
	}
}

void FGeneratedFunction::SyncMetaValue(const FEnchantments & Data, const TCHAR * Key)
{
	const FString & MetaValue = Data.Get(Key);
	if (MetaValue.IsEmpty( ) == false)
	{
		AddMeta(Key, *MetaValue);
	}
	else
	{
		RemoveMeta(Key);
	}
}

void FGeneratedFunction::SyncFlag(const FEnchantments & Data, const TCHAR * FlagName, EFunctionFlags FlagValue)
{
	if (Data.Has(FlagName))
	{
		AddFlag(FlagName, FlagValue);
	}
	else
	{
		RemoveFlag(FlagName, FlagValue);
	}
}

void FGeneratedFunction::SyncPropertyFlags(const FEnchantments & Data)
{
	Data.SyncPropertyFlags(GeneratedFunction, GeneratedFunctionSkeleton);
}

const FString & FGeneratedFunction::GetTimestamp( ) const
{
	return MetaData.Get(*GeneratedFunctionSkeleton, FEnchantments::Key_Timestamp);
}

void FGeneratedFunction::Modify( )
{
	GeneratedFunction->SetFlags(RF_Transactional);
	GeneratedFunctionSkeleton->SetFlags(RF_Transactional);

	GeneratedFunction->Modify( );
	GeneratedFunctionSkeleton->Modify( );
}

FName FGeneratedFunction::GetFunctionName(const UEdGraph & Graph)
{
	return Graph.GetFName( );
}

#undef ANIM_FUNC_DECORATOR
