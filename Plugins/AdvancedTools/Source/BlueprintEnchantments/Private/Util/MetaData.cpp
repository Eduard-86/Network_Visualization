#include "MetaData.h"

const FMetaData::FValue FMetaData::Value_DUMMY = TEXT("");

FMetaData::FKey::FKey(const TCHAR * Value)
	: Value(Value, FNAME_Find)
{}

FMetaData::FKey::FKey(const FString & Value)
	: Value(*Value)
{}

FMetaData::FKey::FKey(const FName & Value)
	: Value(Value)
{}

FMetaData::FKey FMetaData::FKey::Register(const TCHAR * Value)
{
	return FKey(FName(Value, FNAME_Add));
}

FMetaData::FValue::FValue(const TCHAR * Value)
	: Value(Value)
{}

FMetaData::FValue::FValue(const FString & Value)
	: Value(Value)
{
}

FMetaData::FValue::FValue(const FName & Value)
	: Value(Value.ToString( ))
{}

FMetaData::FMetaData(const UObject & Object)
	: Package(Object.GetOutermost( ))
  , MetaData(Package->GetMetaData( ))
{
	check(Package);
	check(MetaData);
}

void FMetaData::Add(const UObject & Object, const FMetaData::FKey & Key, const FMetaData::FValue & Value)
{
	assumeChecked(MetaData);
	assumeChecked(Package);
	MetaData->SetValue(&Object, Key, Value);
}

void FMetaData::Remove(const UObject & Object, const FMetaData::FKey & Key)
{
	assumeChecked(MetaData);
	assumeChecked(Package);
	MetaData->RemoveValue(&Object, Key);
}

bool FMetaData::Has(const UObject & Object, const FMetaData::FKey & Key) const
{
	assumeChecked(MetaData);
	return MetaData->HasValue(&Object, Key);
}

const FString & FMetaData::Get(const UObject & Object, const FMetaData::FKey & Key) const
{
	assumeChecked(MetaData);
	return MetaData->GetValue(&Object, Key);
}
