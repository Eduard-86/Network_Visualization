#include "Enchantments.h"

#include "K2Node_CallFunction.h"
#include "K2Node_Composite.h"
#include "BlueprintGraph/Classes/K2Node.h"

/*
 * Do not use FParse for search operations, as it fails when params are named like "NewParam, NewParam1"
 * Only acceptable for token extraction
 */

// @formatter:off
const FEnchantments::FKey FEnchantments::Key_CallableWithoutWorldContext = FKey::Register(TEXT("CallableWithoutWorldContext"));
const FEnchantments::FKey FEnchantments::Key_DevelopmentOnly = FKey::Register(TEXT("DevelopmentOnly"));
const FEnchantments::FKey FEnchantments::Key_EditorOnly = FKey::Register(TEXT("EditorOnly"));
const FEnchantments::FKey FEnchantments::Key_CommutativeAssociativeBinaryOperator = FKey::Register(TEXT("CommutativeAssociativeBinaryOperator"));
const FEnchantments::FKey FEnchantments::Key_PropertyFlags = FKey::Register(TEXT("PropertyFlags"));
const FEnchantments::FKey FEnchantments::Key_AutoCreateRefTerm = FKey::Register(TEXT("AutoCreateRefTerm"));
// @formatter:on

const FEnchantments::FKey FEnchantments::Key_Timestamp = FKey::Register(TEXT("EnchantmentsVersion"));

#define ForEachGeneratedFunction(...)\
	ForEachGeneratedFunction(\
		FunctionGraph,\
		[&](UFunction& Function)\
		{\
			__VA_ARGS__\
		}\
	)

FEnchantments::FListKey::FListKey(const TCHAR * Value)
	: Value(HandleDelimiters(FString(Value)))
{}

FEnchantments::FListKey::FListKey(const FString & Value)
	: Value(HandleDelimiters(FString(Value)))
{}

FEnchantments::FListKey::FListKey(const FName & Value)
	: Value(HandleDelimiters(Value.ToString( )))
{}

FString && FEnchantments::FListKey::HandleDelimiters(FString && Value)
{
	Value.ReplaceCharInline(' ', '\1', ESearchCase::CaseSensitive);
	return MoveTemp(Value);
}

bool FEnchantments::AreAvailableFor(const UK2Node & Node)
{
	if (const UK2Node_CallFunction * CallFunctionNode = Cast<UK2Node_CallFunction>(&Node))
	{
		return AreAvailableFor(*CallFunctionNode);
	}

	return false;
}

bool FEnchantments::AreAvailableFor(const UK2Node_CallFunction & CallFunctionNode)
{
	UEdGraph * FunctionGraph = GetFunctionGraph(CallFunctionNode);
	if (!FunctionGraph)
		return false;

	const FEnchantments Enchantments(*FunctionGraph);
	return Enchantments.AreAvailable( );
}

void FEnchantments::EnchantGeneratedFunctionsStatic(UEdGraph & FunctionGraph)
{
	FEnchantments Enchantments(FunctionGraph);
	Enchantments.EnchantGeneratedFunctions(FunctionGraph);
}

void FEnchantments::EnchantGeneratedFunctionsStatic(UK2Node_CallFunction & CallFunctionNode)
{
	if (UEdGraph * FunctionGraph = GetFunctionGraph(CallFunctionNode))
	{
		EnchantGeneratedFunctionsStatic(*FunctionGraph);
	}
}

FEnchantments::FEnchantments(UEdGraph & Object)
	: MetaData(Object)
{}

FEnchantments::FEnchantments(const UK2Node_CallFunction & CallFunctionNode)
{
	if (UEdGraph * Graph = GetFunctionGraph(CallFunctionNode))
	{
		new(this) FEnchantments(*Graph);
	}
}

bool FEnchantments::AreAvailable( ) const
{
	return
			MetaData.Object
			&& (Has(Key_PropertyFlags)
				|| Has(Key_CallableWithoutWorldContext)
				|| Has(Key_DevelopmentOnly)
				|| Has(Key_EditorOnly)
				|| Has(Key_CommutativeAssociativeBinaryOperator));
}

void FEnchantments::EnchantGeneratedFunctions(const UEdGraph & FunctionGraph) const
{
	const UBlueprint * Blueprint = Cast<UBlueprint>(FunctionGraph.GetOuter( ));
	if (!Blueprint) return;

	const FName   FunctionFName = FunctionGraph.GetFName( );
	const FString FunctionName  = FunctionFName.ToString( );
	const bool    bIsAnimGraph  = FunctionName.EndsWith(TEXT("__AnimFunc"));
	if (bIsAnimGraph) return;

	auto Functions = GetGeneratedFunctions(FunctionGraph);
	EnchantFunctions(Functions.Get<0>( ), Functions.Get<1>( ));
}

TTuple<UFunction *, UFunction *> FEnchantments::GetGeneratedFunctions(const UEdGraph & FunctionGraph)
{
	using RetType = TTuple<UFunction *, UFunction *>;

	const UBlueprint * Blueprint = Cast<UBlueprint>(FunctionGraph.GetOuter( ));
	if (!Blueprint) return RetType(nullptr, nullptr);

	const FName   FunctionFName = FunctionGraph.GetFName( );
	const FString FunctionName  = FunctionFName.ToString( );
	const bool    bIsAnimGraph  = FunctionName.EndsWith(TEXT("__AnimFunc"));
	if (bIsAnimGraph) return RetType(nullptr, nullptr);

	UFunction * Function =
			Blueprint->GeneratedClass
			? Blueprint->GeneratedClass->FindFunctionByName(FunctionFName, EIncludeSuperFlag::ExcludeSuper)
			: nullptr;
	UFunction * FunctionSkeleton =
			Blueprint->SkeletonGeneratedClass
			? Blueprint->SkeletonGeneratedClass->FindFunctionByName(FunctionFName, EIncludeSuperFlag::ExcludeSuper)
			: nullptr;

	return RetType(Function, FunctionSkeleton);
}

bool FEnchantments::ModifyFunction(UFunction & Function)
{
	Function.SetFlags(RF_Transactional);
	return Function.Modify( );
}

void FEnchantments::Modify( )
{
	// Meta data is not transactional by default
	// Exact point to mark it transactional is non-obvious, so easier to mark it right when needed
	MetaData.MetaData.GetMetaData( )->SetFlags(RF_Transactional);
	MetaData.MetaData.GetMetaData( )->Modify( );
}

void FEnchantments::EnchantFunctionsUnsafe(int32 NumArgs, UFunction * Function, ...) const
{
	va_list Functions;
	va_start(Functions, Function);

	while (true)
	{
		if (Function)
			break;

		if (NumArgs == 1)
			return;

		Function = va_arg(Functions, UFunction*);
		NumArgs--;
	}

	const FMetaData Meta(*Function);

	while (NumArgs > 0)
	{
		if (Function)
		{
			FFunctionMetaData FunctionMetaData(*Function, Meta);
			// @formatter:off
			SyncMeta(FunctionMetaData, FEnchantments::Key_DevelopmentOnly);
			SyncMeta(FunctionMetaData, FEnchantments::Key_CallableWithoutWorldContext);
			SyncMeta(FunctionMetaData, FEnchantments::Key_CommutativeAssociativeBinaryOperator);
			SyncMetaValue(FunctionMetaData, FEnchantments::Key_AutoCreateRefTerm);
			SyncFlag(*Function, FEnchantments::Key_EditorOnly, FUNC_EditorOnly);
			SyncPropertyFlags(Function);
			//Function.SyncMetaValue(*this, Key_Timestamp);
			// @formatter:on
		}

		Function = va_arg(Functions, UFunction*);
		NumArgs--;
	}
	va_end(Functions);
}

void FEnchantments::Add(const FEnchantments::FKey & Key, const FEnchantments::FValue & Value)
{
	MetaData.Add(Key, Value);
	IncrementTimeStamp( );
}

void FEnchantments::Add(const FKey & Key, const UEdGraph & FunctionGraph, bool bModify, const FValue & Value)
{
	Add(Key, Value);
	const auto GeneratedFunctions = GetGeneratedFunctions(FunctionGraph);
	ForEachGeneratedFunction(
		if (bModify) ModifyFunction(Function);

		FFunctionMetaData FunctionMeta(Function);
		FunctionMeta.Add(Key, Value);
	);
}

void FEnchantments::Add(const FKey & FlagName, EFunctionFlags FlagValue, const UEdGraph & FunctionGraph, bool bModify)
{
	Add(FlagName);
	ForEachGeneratedFunction(
		if (bModify) ModifyFunction(Function);

		EnumAddFlags(Function.FunctionFlags, FlagValue);
	);
}

void FEnchantments::Remove(const FKey & Key, const UEdGraph & FunctionGraph, bool bModify)
{
	Remove(Key);
	ForEachGeneratedFunction(
		if (bModify) ModifyFunction(Function);

		FFunctionMetaData FunctionMeta(Function);
		FunctionMeta.Remove(Key);
	);
}

void FEnchantments::Remove
(
	const FKey &     FlagName,
	EFunctionFlags   FlagValue,
	const UEdGraph & FunctionGraph,
	bool             bModify
)
{
	Remove(FlagName);
	ForEachGeneratedFunction(
		if (bModify) ModifyFunction(Function);

		EnumRemoveFlags(Function.FunctionFlags, FlagValue);
	);
}

FEnchantments::FListItemReference FEnchantments::FindListItem
(const FString & List, const FEnchantments::FListKey & Item)
{
	FListItemReference Result;
	Result.NameLength  = FCString::Strlen(Item);
	Result.TotalLength = Result.NameLength;
	Result.Position    = 0;
	check(Result.NameLength);
	// Handle cases when params named like "NewParam, NewParam1"
	while (true)
	{
		Result.Position = List.Find(Item, ESearchCase::CaseSensitive, ESearchDir::FromStart, Result.Position);
		if (Result.Position == INDEX_NONE)
			break;

		const bool bNoSymbolsAfter = (Result.Position + Result.NameLength) == List.Len( );
		if (bNoSymbolsAfter)
			break;

		const bool bCommaAfter = List[Result.Position + Result.NameLength] == TCHAR(',');
		if (bCommaAfter)
		{
			assumeChecked(List[Result.Position + Result.NameLength + 1] == TCHAR(' '));
			Result.TotalLength += 2;
			break;
		}

		const bool bSpaceAfter = List[Result.Position + Result.NameLength] == TCHAR(' ');
		if (bSpaceAfter)
		{
			assumeChecked(TChar<TCHAR>::IsAlnum(List[Result.Position + Result.NameLength + 1]));
			Result.TotalLength += 1;
			break;
		}

		Result.Position += Result.NameLength - 1;
	}

	assumeChecked(
		Result.Position == INDEX_NONE
		|| FindListItem(List.Mid(Result.Position + Result.TotalLength - 1), Item) == false
	);
	return Result;
}

FEnchantments::FListItemReference FEnchantments::FindPropertyListItem
(
	const FString &                 List,
	const FEnchantments::FListKey & PropertyName
)
{
	FListItemReference Result = FindListItem(List, PropertyName);
	if (Result)
	{
		const int32 ValuePos = Result.Position + Result.TotalLength;
		check(List.Len() > ValuePos);
		const int32 CharAfterValuePos = List.Find(
			TEXT(" "),
			ESearchCase::CaseSensitive,
			ESearchDir::FromStart,
			ValuePos
		);

		Result.TotalLength =
				CharAfterValuePos == INDEX_NONE
				? INT32_MAX
				: CharAfterValuePos - Result.Position + 1; //Take the ' ' too
	}

	return Result;
}

FString FEnchantments::AddListItem(const FEnchantments::FKey & Key, const FEnchantments::FListKey & Item)
{
	FString UpdatedList(Get(Key));
	if (!UpdatedList.IsEmpty( ))
		UpdatedList.Append(TEXT(", "));
	UpdatedList.Append(Item.ToChar( ));
	Add(Key, UpdatedList);

	return UpdatedList;
}

void FEnchantments::AddListItem
(const FEnchantments::FKey & Key, const FEnchantments::FListKey & Item, const UEdGraph & FunctionGraph, bool bModify)
{
	const FString UpdatedList = AddListItem(Key, Item);
	ForEachGeneratedFunction(
		if (bModify) ModifyFunction(Function);

		FFunctionMetaData MetaData(Function);
		MetaData.Add(Key, UpdatedList);
	);
}

FString FEnchantments::RemoveListItem(const FEnchantments::FKey & Key, const FEnchantments::FListKey & Item)
{
	const FString & List = Get(Key);
	if (const FListItemReference SearchResult = FindListItem(List, Item))
	{
		FString UpdatedList(List);
		UpdatedList.RemoveAt(SearchResult.Position, SearchResult.TotalLength, false);
		Add(Key, UpdatedList);

		return UpdatedList;
	}

	return FString( );
}

void FEnchantments::RemoveListItem
(const FEnchantments::FKey & Key, const FEnchantments::FListKey & Item, const UEdGraph & FunctionGraph, bool bModify)
{
	const FString UpdatedList = RemoveListItem(Key, Item);

	if (UpdatedList.IsEmpty( ))
	{
		ForEachGeneratedFunction(
			if (bModify) ModifyFunction(Function);

			FFunctionMetaData MetaData(Function);
			MetaData.Remove(Key);
		);
	}
	else
	{
		ForEachGeneratedFunction(
			if (bModify) ModifyFunction(Function);

			FFunctionMetaData MetaData(Function);
			MetaData.Add(Key, UpdatedList);
		);
	}
}

FString FEnchantments::RenameListItem
(
	const FEnchantments::FKey &     Key,
	const FEnchantments::FListKey & OldItemName,
	const FEnchantments::FListKey & NewItemName
)
{
	const FString & List = Get(Key);
	if (const FListItemReference SearchResult = FindListItem(List, OldItemName))
	{
		FString UpdatedList(Get(Key));
		UpdatedList.RemoveAt(SearchResult.Position, SearchResult.NameLength, false);
		UpdatedList.InsertAt(SearchResult.Position, NewItemName.ToString( ));
		Add(Key, UpdatedList);

		return UpdatedList;
	}

	return List;
}

void FEnchantments::RenameListItem
(
	const FEnchantments::FKey &     Key,
	const FEnchantments::FListKey & OldItemName,
	const FEnchantments::FListKey & NewItemName,
	const UEdGraph &                FunctionGraph,
	bool                            bModify
)
{
	const FString NewList = RenameListItem(Key, OldItemName, NewItemName);
	if (NewList.IsEmpty( ))
		return;

	ForEachGeneratedFunction(
		if (bModify) ModifyFunction(Function);

		FFunctionMetaData MetaData(Function);
		MetaData.Add(Key, NewList);
	);
}

void FEnchantments::AddPropertyFlag(const FEnchantments::FListKey & PropertyName, EPropertyFlags Flag)
{
	SavePropertyFlags(PropertyName, RetrievePropertyFlags(PropertyName) | Flag);
	IncrementTimeStamp( );
}

void FEnchantments::AddPropertyFlag
(
	const FEnchantments::FListKey & PropertyName,
	EPropertyFlags                  Flag,
	const UEdGraph &                FunctionGraph,
	bool                            bModify
)
{
	AddPropertyFlag(PropertyName, Flag);
	ForEachGeneratedFunction(
		if (bModify) ModifyFunction(Function);

		// @formatter:off
		for (TFieldIterator<FProperty> PropertyIterator(&Function); PropertyIterator; ++PropertyIterator)
		{
			if (PropertyIterator->NamePrivate == PropertyName)
			{
				EnumAddFlags(PropertyIterator->PropertyFlags, Flag);
				break;
			}
		}
		// @formatter:on
	);
}

void FEnchantments::RemovePropertyFlag(const FEnchantments::FListKey & PropertyName, EPropertyFlags Flag)
{
	EPropertyFlags Flags = RetrievePropertyFlags(PropertyName);
	EnumRemoveFlags(Flags, Flag);
	SavePropertyFlags(PropertyName, Flags);
	IncrementTimeStamp( );
}

void FEnchantments::RemovePropertyFlag
(
	const FEnchantments::FListKey & PropertyName,
	EPropertyFlags                  Flag,
	const UEdGraph &                FunctionGraph,
	bool                            bModify
)
{
	RemovePropertyFlag(PropertyName, Flag);
	ForEachGeneratedFunction(
		if (bModify) ModifyFunction(Function);

		// @formatter:off
		for (TFieldIterator<FProperty> PropertyIterator(&Function); PropertyIterator; ++PropertyIterator)
			{
			if (PropertyIterator->NamePrivate == PropertyName)
			{
				EnumRemoveFlags(PropertyIterator->PropertyFlags, Flag);
				break;
			}
		}
		// @formatter:on
	);
}

void FEnchantments::UpdatePropertyName
(const FEnchantments::FListKey & OldPropertyName, const FEnchantments::FListKey & NewPropertyName)
{
	const FString & Data = Get(Key_PropertyFlags);
	if (const FListItemReference SearchResult = FindListItem(Data, OldPropertyName.ToString( )))
	{
		FString NewData(Data);
		NewData.RemoveAt(SearchResult.Position, SearchResult.NameLength);
		NewData.InsertAt(SearchResult.Position, NewPropertyName.ToString( ));
		Add(Key_PropertyFlags, NewData);
	}
}

EPropertyFlags FEnchantments::LoadPropertyFlags(const FEnchantments::FListKey & PropertyName) const
{
	const FString & Data = Get(Key_PropertyFlags);
	if (Data.IsEmpty( ))
		return CPF_None;

	uint64 Result = 0;
	if (FListItemReference SearchResult = FindPropertyListItem(Data, PropertyName))
	{
		Result = FCString::Strtoui64(&Data[SearchResult.PropertyValuePos( )], nullptr, 10);
	}

	return EPropertyFlags(Result);
}

TMap<FName, EPropertyFlags> FEnchantments::LoadAllPropertiesFlags( ) const
{
	TMap<FName, EPropertyFlags> Result;

	const FString & AllParamsFlags = Get(FEnchantments::Key_PropertyFlags);
	const TCHAR *   Stream         = *AllParamsFlags;
	FString         ParamName;
	FString         ParamFlagsString;
	while (FParse::Token(Stream, ParamName, false))
	{
		const bool bHasValue = FParse::Token(Stream, ParamFlagsString, false);
		assumeChecked(bHasValue);
		int64 ParamFlags = FCString::Atoi64(*ParamFlagsString);
		Result.Add(FName(ParamName), EPropertyFlags(ParamFlags));

		// Clean containers
		ParamName.RemoveAt(0, MAX_int32, false);
		ParamFlagsString.RemoveAt(0, MAX_int32, false);
	}

	return Result;
}

void FEnchantments::DropPropertyFlagsOnPropertyDestruction(const FEnchantments::FListKey & PropertyName)
{
	const FString & Data = Get(Key_PropertyFlags);
	if (FListItemReference SearchResult = FindPropertyListItem(Data, PropertyName))
	{
		FString NewData(Data);
		SearchResult.IncludePrecedingSpace( );
		NewData.RemoveAt(SearchResult.Position, SearchResult.TotalLength, false);
		Add(Key_PropertyFlags, NewData);
	}
}

void FEnchantments::SyncMeta(FFunctionMetaData & FunctionMetaData, const FKey & Key) const
{
	if (Has(Key))
	{
		FunctionMetaData.Add(Key);
	}
	else
	{
		FunctionMetaData.Remove(Key);
	}
}

void FEnchantments::SyncMetaValue(FFunctionMetaData & FunctionMetaData, const FEnchantments::FKey & Key) const
{
	const FString & MetaValue = Get(Key);
	if (Has(Key))
	{
		FunctionMetaData.Add(Key, MetaValue);
	}
	else
	{
		FunctionMetaData.Remove(Key);
	}
}

void FEnchantments::SyncFlag(UFunction & Function, const FEnchantments::FKey & FlagName, EFunctionFlags FlagValue) const
{
	if (Has(FlagName))
	{
		EnumAddFlags(Function.FunctionFlags, FlagValue);
	}
	else
	{
		EnumRemoveFlags(Function.FunctionFlags, FlagValue);
	}
}

void FEnchantments::SyncPropertyFlagsUnsafe(int32 NumArgs, UFunction * Function ...) const
{
	// Load flags to be added
	const TMap<FName, EPropertyFlags> PropertyFlags = LoadAllPropertiesFlags( );
	if (PropertyFlags.Num( ) == 0)
		return;

	va_list Functions;
	va_start(Functions, Function);

	while (NumArgs > 0)
	{
		if (Function)
		{
			// Add flags to valid properties
			TMap<FName, FProperty * RESTRICT> Properties;
			for (TFieldIterator<FProperty> Property(Function); Property; ++Property)
			{
				if (const EPropertyFlags * Flags = PropertyFlags.Find(Property->NamePrivate))
				{
					EnumAddFlags(Property->PropertyFlags, *Flags);
				}
			}
		}

		Function = va_arg(Functions, UFunction*);
		NumArgs--;
	}

	va_end(Functions);
}

EPropertyFlags FEnchantments::RetrievePropertyFlags(const FEnchantments::FListKey & PropertyName) const
{
	const FString & PropertyFlags = Get(Key_PropertyFlags);
	if (PropertyFlags.IsEmpty( ))
		return CPF_None;

	int64 Result = 0;
	if (const FListItemReference SearchResult = FindPropertyListItem(PropertyFlags, PropertyName))
	{
		Result = FCString::Strtoi64(&PropertyFlags[SearchResult.PropertyValuePos( )], nullptr, 10);
	}

	return EPropertyFlags(Result);
}

void FEnchantments::SavePropertyFlags(const FEnchantments::FListKey & PropertyName, EPropertyFlags PropertyFlags)
{
	if (PropertyFlags == CPF_None)
	{
		DropPropertyFlagsOnPropertyDestruction(PropertyName);
		return;
	}

	const FString FlagsValue(FString::Printf(TEXT("%lld"), PropertyFlags));
	FString       Data(Get(Key_PropertyFlags));
	if (const FListItemReference SearchResult = FindPropertyListItem(Data, PropertyName))
	{
		Data.RemoveAt(SearchResult.PropertyValuePos( ), SearchResult.PropertyValueLength( ), false);
		Data.InsertAt(SearchResult.PropertyValuePos( ), FlagsValue);
	}
	else
	{
		Data
				.AppendChar(' ')
				.Append(PropertyName.ToString( ))
				.AppendChar(' ')
				.Append(FlagsValue);
	}

	Add(Key_PropertyFlags, Data);
}

int32 FEnchantments::RetrieveTimestamp( ) const
{
	const FString & Timestamp = Get(Key_Timestamp);
	if (Timestamp.IsEmpty( ))
		return 0;

	return FCString::Strtoi(*Timestamp, nullptr, 10);
}

void FEnchantments::IncrementTimeStamp( )
{
	// Not used atm

	//MetaData.Add(Key_Timestamp, FString::FromInt(RetrieveTimestamp() + 1));
}

int32 FEnchantments::FListItemReference::PropertyValueLength( ) const
{
	return TotalLength == INT32_MAX ? INT32_MAX : TotalLength - NameLength - 1;
}

int32 FEnchantments::FListItemReference::PropertyValuePos( ) const
{
	return Position + NameLength + 1;
}

void FEnchantments::FListItemReference::IncludePrecedingSpace( )
{
	if (Position > 0)
	{
		Position--;
		if (TotalLength < INT32_MAX)
			TotalLength++;
	}
}

UEdGraph * FEnchantments::GetFunctionGraph(const UK2Node_CallFunction & CallFunctionNode)
{
	const UBlueprint * Blueprint = nullptr;

	if (CallFunctionNode.FunctionReference.IsSelfContext( ))
	{
		UEdGraph * Graph = CallFunctionNode.GetGraph( );
		assumeChecked(Graph);

		const UObject * MayBeBP = Graph->GetOuter( );
		while (MayBeBP->IsA<UBlueprint>( ) == false)
		{
			MayBeBP = MayBeBP->GetOuter( );
		}

		Blueprint = (UBlueprint *)MayBeBP;
	}
	// Node may be damaged - then reference will be dead 
	else if (const UClass * OwningClass = CallFunctionNode.FunctionReference.GetMemberParentClass( ))
	{
		Blueprint = Cast<UBlueprint>(OwningClass->ClassGeneratedBy);
	}

	if (Blueprint)
	{
		const FName FunctionName = CallFunctionNode.FunctionReference.GetMemberName( );
		for (UEdGraph * FunctionGraph : Blueprint->FunctionGraphs)
		{
			if (FunctionGraph->GetFName( ) == FunctionName)
				return FunctionGraph;
		}
	}

	return nullptr;
}

#undef ForEachGeneratedFunction
