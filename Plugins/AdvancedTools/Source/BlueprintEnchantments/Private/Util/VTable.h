#pragma once

#include "Templates/UnrealTypeTraits.h"

/*
 * Principle tested with MSVC, GCC, Clang
 * May be a good idea to mark all non-const methods volatile
 */

template <class Class, class ParentClass = void>
class FVTable
{
	static_assert(
		TIsDerivedFrom<Class, ParentClass>::Value
		|| TIsSame<ParentClass, void>::Value == true,
		"Can't cast provided Class to ParentClass"
	);

	// Subtypes
public:
#pragma region Subtypes

	enum class FVTablePtr : UPTRINT;


	/**
	 * Simple TGuardValue, but returnable
	 * Could construct a TGuardValue by output ref probably, but not as convenient
	 */
	template <class OneOfParentClasses>
	struct FTemporaryPatch
	{
		FORCEINLINE FTemporaryPatch(
			FVTable<Class, ParentClass> &                    PatchedTable,
			const FVTable<OneOfParentClasses, ParentClass> & NewTable
		);

		FORCEINLINE ~FTemporaryPatch( );

	private:
		UPTRINT & ValueContainer;
		UPTRINT   OldValue;
	};


	template <class OneOfParentClasses>
	friend struct FTemporaryPatch;

#pragma endregion ~ Subtypes

	// Initialization
public:
	FORCEINLINE FVTable(const Class & Object);

	bool IsValid( ) const { return VTable > nullptr; }

	operator bool( ) const { return IsValid( ); }

	// Public interface
public:
	/** Overwrites this VTable with provided */
	template <class OneOfParentClasses>
	void Overwrite(const FVTable<OneOfParentClasses, ParentClass> & NewTable);

	/** Overwrites this VTable with provided. Reverts once returned patch expires */
	template <class OneOfParentClasses>
	FTemporaryPatch<OneOfParentClasses> CreateTemporaryPatch(
		const FVTable<OneOfParentClasses, ParentClass> & NewTable
	);

	/*
	 * ParamTypes and FuncParamTypes are different parameter packs because they are deduced differently
	 * ParamTypes are decayed and looses reference if any, when substituted as function params
	 * FuncParamTypes are acquired through something like decltype(MethodPointer)
	 *
	 * ParamTypes are forwarded to avoid reference types being decayed to value types
	 * Excessive references are then removed by lambda expressions, calling the provided method pointer
	 *
	 * VTable lookup is forced with calling via pointer
	 */

	/** Non-const version */
	template <class ParentSampleClass, typename ReturnType, typename ... ParamTypes, typename ... FuncParamTypes>
	FORCEINLINE static decltype(auto) CallParent(
		Class &                   This,
		const ParentSampleClass & ParentSample,
		ReturnType (Class::*      MethodPointer)(FuncParamTypes ...),
		ParamTypes && ...         Params
	);

	/** Const version */
	template <class ParentSampleClass, typename ReturnType, typename ... ParamTypes, typename ... FuncParamTypes>
	FORCEINLINE static decltype(auto) CallParent(
		const Class &             This,
		const ParentSampleClass & ParentSample,
		ReturnType (Class::*      MethodPointer)(FuncParamTypes ...) const,
		ParamTypes && ...         Params
	);

	// Public macro
public:

	/** VTable of this */
#define FThisVTable(...)

	/** Short version of CallParent for UObjects */
#define FVTable__CallSuper(FunctionName, ...)

	/**
	 * Short version of CallParent for UObjects, but you provide method pointer manually.
	 * Likely via SelectOverload(FunctionName, ParamTypes...)
	 */
#define FVTable__CallSuperOverload(Overload, ...)

	/** Short version of CallParent for UObjects with Interface specification */
#define FVTable__CallSuperInterface(FunctionName, Interface, ...)

	/**
	 * Short version of CallParent for UObjects with Interface specification, but you provide method pointer manually.
	 * Likely via SelectOverload(FunctionName, ParamTypes...)
	 */
#define FVTable__CallSuperInterfaceOverload(Overload, Interface, ...)

	// These are expected to be public but are of no use now
private:
	const FVTablePtr Get( ) const { return FVTablePtr(*VTable); }
	void             Set(const FVTablePtr Ptr) { *VTable = UPTRINT(Ptr); }

	// Real private stuff
private:
	FORCEINLINE static UPTRINT * GetVTablePtr(const Class & Object);

	UPTRINT * VTable;
};


// Utilities //
/*************************************************************************************************/

#pragma region Utilities

/** Template for acquiring class method return type */
template <class Class, typename ... ParamTypes, typename ReturnType>
ReturnType MethodReturnType(ReturnType (Class::*MethodPtr)(ParamTypes ...));

/** Type of class, owning the method using this macro. Convenient to use within other macro */
#define __PureThisType__ TRemoveReference<TRemovePointer<TRemoveCV<decltype(this)>::Type>::Type>::Type

/** Creates a pointer to desired overloaded method based on provided param types */
#define SelectOverload(FunctionName, ...)\
	(\
		/* Return type  */decltype(MethodReturnType<__PureThisType__, __VA_ARGS__>(&__PureThisType__::FunctionName))\
		/* Owning class */(__PureThisType__::*)\
		/* Param  types */(__VA_ARGS__)\
	) &__PureThisType__::FunctionName

#pragma endregion ~ Utilities

// Template implementations //
/*************************************************************************************************/

#pragma region Template implementations

#pragma region Temporary patch

template<class Class, class ParentClass>

template <class OneOfParentClasses>
FVTable<Class, ParentClass>::FTemporaryPatch<OneOfParentClasses>::FTemporaryPatch(
	FVTable<Class, ParentClass> &                    PatchedTable,
	const FVTable<OneOfParentClasses, ParentClass> & NewTable
)
	: ValueContainer(*PatchedTable.VTable)
	, OldValue(*PatchedTable.VTable)
{
	// Okaaay, its easier to cast then befriend
	*PatchedTable.VTable = *((FVTable<Class, ParentClass> &)NewTable).VTable;
}

template <class Class, class ParentClass>
template <class OneOfParentClasses>
FVTable<Class, ParentClass>::FTemporaryPatch<OneOfParentClasses>::~FTemporaryPatch( )
{
	ValueContainer = OldValue;
}

#pragma endregion ~ Temporary patch

template <class Class, class ParentClass>
FVTable<Class, ParentClass>::FVTable(const Class & Object)
	: VTable(GetVTablePtr(Object))
{}

template <class Class, class ParentClass>
template <class OneOfParentClasses>
void FVTable<Class, ParentClass>::Overwrite(const FVTable<OneOfParentClasses, ParentClass> & NewTable)
{
	static_assert(
		TIsDerivedFrom<OneOfParentClasses, ParentClass>::Value,
		"OneOfParentClasses, whos VTable is used for overwriting, must be lower in inheritance chain then ParentClass"
	);
	*VTable = *(((decltype(this))(&NewTable))->VTable);
}

template <class Class, class ParentClass>
template <class OneOfParentClasses>
typename FVTable<Class, ParentClass>::template FTemporaryPatch<OneOfParentClasses>
FVTable<Class, ParentClass>::CreateTemporaryPatch(const FVTable<OneOfParentClasses, ParentClass> & NewTable)
{
	static_assert(
		TIsDerivedFrom<OneOfParentClasses, ParentClass>::Value,
		"OneOfParentClasses, whos VTable is used for overwriting, must be lower in inheritance chain then ParentClass"
	);
	return FTemporaryPatch<OneOfParentClasses>(*this, NewTable);
}

template <class Class, class ParentClass>
template <class ParentSampleClass, typename ReturnType, typename ... ParamTypes, typename ... FuncParamTypes>
decltype(auto) FVTable<Class, ParentClass>::CallParent(
	Class &                   This,
	const ParentSampleClass & ParentSample,
	ReturnType (Class::*      MethodPointer)(FuncParamTypes ...),
	ParamTypes && ...         Params
)
{
	FVTable<Class, ParentClass>                   MyVTable(This);
	const FVTable<ParentSampleClass, ParentClass> ParentVTable(ParentSample);

	const auto Patch(MyVTable.CreateTemporaryPatch(ParentVTable));
	return [&This, &MethodPointer](ParamTypes ...LambdaParams)
	{
		return (This.*MethodPointer)(LambdaParams...);
	}(Params...);
}

template <class Class, class ParentClass>
template <class ParentSampleClass, typename ReturnType, typename ... ParamTypes, typename ... FuncParamTypes>
decltype(auto) FVTable<Class, ParentClass>::CallParent(
	const Class &             This,
	const ParentSampleClass & ParentSample,
	ReturnType (Class::*      MethodPointer)(FuncParamTypes ...) const,
	ParamTypes && ...         Params
)
{
	FVTable<Class, ParentClass>                   MyVTable(This);
	const FVTable<ParentSampleClass, ParentClass> ParentVTable(ParentSample);

	const auto Patch(MyVTable.CreateTemporaryPatch(ParentVTable));
	return [&This, &MethodPointer](ParamTypes ...LambdaParams)
	{
		return (This.*MethodPointer)(LambdaParams...);
	}(Params...);
}

template <class Class, class ParentClass>
UPTRINT * FVTable<Class, ParentClass>::GetVTablePtr(const Class & Object)
{
	return (UPTRINT *)(
		UPTRINT(&Object)
		// This _IS_ constexpr, but It's not allowed to be declared so
		// 1 because any casts from 0 always returns 0
		+ (UPTRINT)((ParentClass *)((Class *)1)) - 1
	);
}

#pragma endregion ~ Template implementations

// Macro implementations //
/*************************************************************************************************/

#pragma region Macro implementations

#undef FThisVTable
#undef FVTable__CallSuper
#undef FVTable__CallSuperOverload
#undef FVTable__CallSuperInterface
#undef FVTable__CallSuperInterfaceOverload

/** VTable of this */
#define FThisVTable(...) FVTable<__PureThisType__, __VA_ARGS__>

/** Short version of CallParent for UObjects */
#define FVTable__CallSuper(FunctionName, ...)\
	FThisVTable()::CallParent(*this, *GetDefault<Super>(), &__PureThisType__::FunctionName, __VA_ARGS__)
/**
 * Short version of CallParent for UObjects, but you provide method pointer manually.
 * Likely via SelectOverload(FunctionName, ParamTypes...)
 */
#define FVTable__CallSuperOverload(Overload, ...)\
	FThisVTable()::CallParent(*this, *GetDefault<Super>(), Overload, __VA_ARGS__)

/** Short version of CallParent for UObjects with Interface specification */
#define FVTable__CallSuperInterface(FunctionName, Interface, ...)\
	FThisVTable(Interface)::CallParent(*this, *GetDefault<Super>(), &__PureThisType__::FunctionName, __VA_ARGS__)

/**
 * Short version of CallParent for UObjects with Interface specification, but you provide method pointer manually.
 * Likely via SelectOverload(FunctionName, ParamTypes...)
 */
#define FVTable__CallSuperInterfaceOverload(Overload, Interface, ...)\
	FThisVTable(Interface)::CallParent(*this, *GetDefault<Super>(), Overload, __VA_ARGS__)

#pragma endregion ~ Macro implementations

// Protection //
/*************************************************************************************************/

#pragma region Protection

// Better then nothing
// ReSharper disable once CppPolymorphicClassWithNonVirtualPublicDestructor
struct FVTableLocationChecker
{
	virtual void foo( ) { };
	int          Value;
};


static_assert(offsetof(FVTableLocationChecker, Value) != 0, "VTable location detection failure");

#pragma endregion ~ Protection
