#pragma once

#include "Util/MetaData.h"

class UBlueprint;
class UEdGraph;
class FEnchantments;

class FGeneratedFunction
{
public:
	FGeneratedFunction(const UBlueprint & Blueprint, const UEdGraph & FunctionGraph);
	FGeneratedFunction(const UEdGraph & FunctionGraph);
	bool IsValid( ) const;
	operator bool() const { return IsValid(); }

	void AddMeta(const TCHAR * Key, const TCHAR * Value = FMetaData::Value_DUMMY);
	void RemoveMeta(const TCHAR * Key);
	void AddFlag(const TCHAR * FlagName, EFunctionFlags FlagValue);
	void RemoveFlag(const TCHAR * FlagName, EFunctionFlags FlagValue);
	void AddPropertyFlag(FName PropertyName, EPropertyFlags Flag);
	void RemovePropertyFlag(FName PropertyName, EPropertyFlags Flag);

	void            SyncMeta(const FEnchantments & Data, const TCHAR * Key);
	void            SyncMetaValue(const FEnchantments & Data, const TCHAR * Key);
	void            SyncFlag(const FEnchantments & Data, const TCHAR * FlagName, EFunctionFlags FlagValue);
	void            SyncPropertyFlags(const FEnchantments & Data);
	const FString & GetTimestamp( ) const;

	void Modify();
	bool MarkDirty( ) { return MetaData.GetPackage()->MarkPackageDirty(); }

private:
	static FName GetFunctionName(const UEdGraph & Graph);

private:
	UFunction * GeneratedFunction;
	UFunction * GeneratedFunctionSkeleton;
	FMetaData   MetaData;
};
