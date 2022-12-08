#pragma once

#include "EdGraph/EdGraph.h"
#include "Util/MetaData.h"

class UK2Node_CallFunction;
class UK2Node;

// Could wrap calls in a macro instead, but it leads to naming conflicts
#define ImplementSafeWrappers(MethodName, MethodConstQualifier, ParamType, ParamName)\
	FORCEINLINE auto MethodName(ParamType ParamName)							MethodConstQualifier { return MethodName##Unsafe(1, ParamName); }\
	FORCEINLINE auto MethodName(ParamType ParamName##1, ParamType ParamName##2) MethodConstQualifier { return MethodName##Unsafe(2, ParamName##1, ParamName##2); }

typedef TObjectMetaData<UFunction> FFunctionMetaData;


/** A wrapper for whatever actually holds the data */
class FEnchantments
{
public:
	using FKey = FMetaData::FKey;
	using FValue = FMetaData::FValue;


	struct FListKey
	{
		FListKey(const TCHAR * Value);
		FListKey(const FString & Value);
		FListKey(const FName & Value);

		const TCHAR *   ToChar( ) const { return *Value; }
		const FString & ToString( ) const { return Value; }

		operator const TCHAR *( ) const { return *Value; }

	private:
		static FString && HandleDelimiters(FString && Value);
		const FString     Value;
	};


public:
	static const FKey Key_CallableWithoutWorldContext;
	static const FKey Key_DevelopmentOnly;
	static const FKey Key_EditorOnly;
	static const FKey Key_CommutativeAssociativeBinaryOperator;
	static const FKey Key_PropertyFlags;
	static const FKey Key_AutoCreateRefTerm;

	static const FKey Key_Timestamp;

public:
	static bool AreAvailableFor(const UK2Node & Node);
	static bool AreAvailableFor(const UK2Node_CallFunction & CallFunctionNode);
	static void EnchantGeneratedFunctionsStatic(UEdGraph & FunctionGraph);
	static void EnchantGeneratedFunctionsStatic(UK2Node_CallFunction & CallFunctionNode);

public:
	FEnchantments(UEdGraph & Object);
	FEnchantments(const UK2Node_CallFunction & CallFunctionNode);

	bool AreAvailable( ) const;
	operator bool( ) const { return AreAvailable( ); }

	void Modify( );
	void EnchantFunctionsUnsafe(int32 NumArgs, UFunction * Function ...) const;
	ImplementSafeWrappers(EnchantFunctions, const, UFunction*, Function);
	void EnchantGeneratedFunctions(const UEdGraph & FunctionGraph) const;

public:
	bool            Has(const FKey & Key) const { return MetaData.Has(Key); }
	const FString & Get(const FKey & Key) const { return MetaData.Get(Key); }
	void            Add(const FKey & Key, const FValue & Value = FMetaData::Value_DUMMY);
	void            Add
	(
		const FKey &     Key,
		const UEdGraph & FunctionGraph,
		bool             bModify = false,
		const FValue &   Value   = FMetaData::Value_DUMMY
	);
	void Add(const FKey & FlagName, EFunctionFlags FlagValue, const UEdGraph & FunctionGraph, bool bModify = false);
	void Remove(const FKey & Key) { MetaData.Remove(Key); }
	void Remove(const FKey & Key, const UEdGraph & FunctionGraph, bool bModify = false);
	void Remove(const FKey & FlagName, EFunctionFlags FlagValue, const UEdGraph & FunctionGraph, bool bModify = false);
	void MarkDirty( ) { MetaData.MarkDirty( ); }

	bool HasListItem(const FKey & Key, const FListKey & Item) const { return FindListItem(Get(Key), Item); }
	FString AddListItem(const FKey & Key, const FListKey & Item);
	void AddListItem(const FKey & Key, const FListKey & Item, const UEdGraph & FunctionGraph, bool bModify = false);
	FString RemoveListItem(const FKey & Key, const FListKey & Item);
	void RemoveListItem(const FKey & Key, const FListKey & Item, const UEdGraph & FunctionGraph, bool bModify = false);
	FString RenameListItem(const FKey & Key, const FListKey & OldItemName, const FListKey & NewItemName);
	void RenameListItem
	(
		const FKey &     Key,
		const FListKey & OldItemName,
		const FListKey & NewItemName,
		const UEdGraph & FunctionGraph,
		bool             bModify = false
	);


	void AddPropertyFlag(const FListKey & PropertyName, EPropertyFlags Flag);
	void RemovePropertyFlag(const FListKey & PropertyName, EPropertyFlags Flag);
	void UpdatePropertyName(const FListKey & OldPropertyName, const FListKey & NewPropertyName);
	void DropPropertyFlagsOnPropertyDestruction(const FListKey & PropertyName);
	// @formatter:off
	void AddPropertyFlag(const FListKey & PropertyName, EPropertyFlags Flag, const UEdGraph & FunctionGraph, bool bModify = false);
	void RemovePropertyFlag(const FListKey & PropertyName, EPropertyFlags Flag, const UEdGraph & FunctionGraph, bool bModify = false);
	// @formatter:on
	EPropertyFlags              LoadPropertyFlags(const FListKey & PropertyName) const;
	TMap<FName, EPropertyFlags> LoadAllPropertiesFlags( ) const;

	void SyncMeta(FFunctionMetaData & FunctionMetaData, const FKey & Key) const;
	void SyncMetaValue(FFunctionMetaData & FunctionMetaData, const FKey & Key) const;
	void SyncFlag(UFunction & Function, const FKey & FlagName, EFunctionFlags FlagValue) const;
	void SyncPropertyFlagsUnsafe(int32 NumArgs, UFunction * Function ...) const;
	ImplementSafeWrappers(SyncPropertyFlags, const, UFunction*, Function);

private:
	// Property flags
	EPropertyFlags RetrievePropertyFlags(const FListKey & PropertyName) const;
	void           SavePropertyFlags(const FListKey & PropertyName, EPropertyFlags PropertyFlags);

	// Timestamp
	int32 RetrieveTimestamp( ) const;
	void  IncrementTimeStamp( );


	// Lists
	struct FListItemReference
	{
		int32 Position;
		int32 NameLength;
		/**
		 * [NameLength + ", "] for basic lists
		 * [NameLength + " " + ValueLength or infinity if property occupies the ending of the list] for property lists
		 */
		int32 TotalLength;

		int32 PropertyValueLength( ) const;
		int32 PropertyValuePos( ) const;
		void  IncludePrecedingSpace( );

		operator bool( ) const { return Position > INDEX_NONE; }
	};


	static FListItemReference FindListItem(const FString & List, const FListKey & Item);
	static FListItemReference FindPropertyListItem(const FString & List, const FListKey & PropertyName);

	//
	static UEdGraph *                       GetFunctionGraph(const UK2Node_CallFunction & CallFunctionNode);
	static TTuple<UFunction *, UFunction *> GetGeneratedFunctions(const UEdGraph & FunctionGraph);
	static bool                             ModifyFunction(UFunction & Function);

	template <class LambdaType>
	FORCEINLINE static void ForEachGeneratedFunction(const UEdGraph & FunctionGraph, const LambdaType & Lambda)
	{
		const auto GeneratedFunctions = GetGeneratedFunctions(FunctionGraph);
		if (UFunction * Function = GeneratedFunctions.Get<0>( )) { Lambda(*Function); }
		if (UFunction * Function = GeneratedFunctions.Get<1>( )) { Lambda(*Function); }
	}

private:
	TObjectMetaData<UEdGraph> MetaData;
};

#undef ImplementSafeWrappers
