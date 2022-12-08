#pragma once

#include "AssumeChecked.h"
#include "UObject/MetaData.h"
#include "UObject/Package.h"


class FMetaData
{
public:
	/** */
	struct FKey
	{
		FKey(const TCHAR * Value);
		FKey(const FString & Value);
		FKey(const FName & Value);

		static FKey Register(const TCHAR * Value);

		const TCHAR * ToChar( ) const { return *Value.ToString( ); }

		operator const FName &( ) const { return Value; }

	private:
		const FName Value;
	};


	/** */
	struct FValue
	{
		FValue(const TCHAR * Value);
		FValue(const FString & Value);
		FValue(const FName & Value);

		const TCHAR *   ToChar( ) const { return *Value; }
		const FString & ToString( ) const { return Value; }

		operator const TCHAR *( ) const { return *Value; }

	private:
		const FString Value;
	};


public:
	static const FValue Value_DUMMY;

public:
	FMetaData( ) = default;
	FMetaData(const UObject & Object);

	UPackage *  GetPackage( ) const { return Package; }
	UMetaData * GetMetaData( ) const { return MetaData; }

	void            Add(const UObject & Object, const FKey & Key, const FValue & Value = Value_DUMMY);
	void            Remove(const UObject & Object, const FKey & Key);
	bool            Has(const UObject & Object, const FKey & Key) const;
	const FString & Get(const UObject & Object, const FKey & Key) const;

private:
	UPackage *  Package  = nullptr;
	UMetaData * MetaData = nullptr;
};


/** */
template <class ObjectType>
class TObjectMetaData
{
	static_assert(TIsDerivedFrom<ObjectType, UObject>::IsDerived, "ObjectType must be a UObject subclass");

public:
	using FKey = FMetaData::FKey;
	using FValue = FMetaData::FValue;

	TObjectMetaData( ) = default;

	TObjectMetaData(ObjectType & Object)
		: Object(&Object)
	  , MetaData(Object)
	{ }

	TObjectMetaData(ObjectType & Object, const FMetaData & MetaData)
		: Object(&Object)
	  , MetaData(MetaData)
	{
		assumeChecked(MetaData.GetPackage() == Object.GetPackage());
	}

	const UPackage &  GetPackage( ) const { return *MetaData.GetPackage( ); }
	const UMetaData & GetMetaData( ) const { return *MetaData.GetMetaData( ); }
	bool              MarkDirty( ) { return MetaData.GetPackage( )->MarkPackageDirty( ); }

	void Add(const FKey & Key, const FValue & Value = FMetaData::Value_DUMMY)
	{
		assumeChecked(Object);
		MetaData.Add(*Object, Key, Value);
	}

	void Remove(const FKey & Key)
	{
		assumeChecked(Object);
		MetaData.Remove(*Object, Key);
	}

	bool Has(const FKey & Key) const
	{
		assumeChecked(Object);
		return MetaData.Has(*Object, Key);
	}

	const FString & Get(const FKey & Key) const
	{
		assumeChecked(Object);
		return MetaData.Get(*Object, Key);
	}

public:
	ObjectType * const Object = nullptr;
	FMetaData          MetaData;
};
