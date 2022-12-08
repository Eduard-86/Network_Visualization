#pragma once

#include "Enchantments.h"
#include "IDetailCustomNodeBuilder.h"
#include "IDetailCustomization.h"
#include "Layout/Visibility.h"
#include "PropertyEditor/Public/PropertyEditorDelegates.h"
#include "Styling/SlateTypes.h"

class UK2Node;
class UK2Node_EditablePinBase;
class UEdGraph;
class FBlueprintGraphArgumentGroupLayout;
class FBaseBlueprintGraphActionDetails;
class FBlueprintGraphArgumentLayout;
class IDetailCategoryBuilder;

//  //
/*************************************************************************************************/

#define DeclareProperty_Bool(PropertyName)\
		ECheckBoxState bIs##PropertyName = ECheckBoxState::Undetermined;\
		\
		ECheckBoxState Is##PropertyName( ) const;\
		void           OnIs##PropertyName##Modified(const ECheckBoxState NewCheckedState)


//  //
/*************************************************************************************************/

class FKismetInspectorEnchantments : public IDetailCustomization
{
public:
	FKismetInspectorEnchantments(const FOnGetDetailCustomizationInstance & OriginalCustomizerProvider);

	// Where the action goes
public:
	virtual void CustomizeDetails(IDetailLayoutBuilder & DetailBuilder) override;
private:
	void Inject(IDetailCategoryBuilder & Category);

	// Utils
private:
#pragma region Utils

	void         SyncExtraData( );
	UBlueprint & GetSelectedBlueprint( ) const;
	UFunction &  GetSelectedFunction( ) const;
	UFunction &  GetSelectedFunctionSkeleton( ) const;
	void         AddSelectedFunctionMetaData(const FName & Key, const TCHAR * Value = FMetaData::Value_DUMMY) const;
	void         RemoveSelectedFunctionMetaData(const FName & Key) const;
	void         AddSelectedFunctionFlags(EFunctionFlags Flags) const;
	void         RemoveSelectedFunctionFlags(EFunctionFlags Flags) const;

#pragma endregion ~ Utils

	// UI
	static EVisibility OnGetCategoryButtonTextVisibility(TWeakPtr<SWidget> RowWidget);

	// FReply OnAddLocalEnumClicked( );

	// Properties
private:
	DeclareProperty_Bool(CallableWithoutWorldContext);
	DeclareProperty_Bool(DevelopmentOnly);
	DeclareProperty_Bool(EditorOnly);
	DeclareProperty_Bool(CommutativeAssociativeBinaryOperator);

	friend class FParameterEnchantments;

	// Fields
private:
	friend class FLocalEnumLayout;

	FOnGetDetailCustomizationInstance OriginalCustomizerProvider;
	TSharedPtr<IDetailCustomization>  OriginalCustomizer;
	TWeakObjectPtr<UEdGraph>          SelectedGraph;

	// Identifiers
private:
	static const FName Category_Graph;
	static const FName Category_Inputs;
	static const FName Category_Outputs;
};

//  //
/*************************************************************************************************/

class FParameterGroupEnchantments
	: public IDetailCustomNodeBuilder, public TSharedFromThis<FParameterGroupEnchantments>
{
public:
	FParameterGroupEnchantments(
		const TWeakPtr<FKismetInspectorEnchantments> &         InOwner,
		const TSharedRef<FBlueprintGraphArgumentGroupLayout> & InOriginalLayoutBuilder
	)
		: Owner(InOwner)
	  , OriginalLayoutBuilder(InOriginalLayoutBuilder) { }

private:
	virtual void  SetOnRebuildChildren(FSimpleDelegate InOnRegenerateChildren) override;
	virtual void  GenerateHeaderRowContent(FDetailWidgetRow & NodeRow) override;
	virtual void  GenerateChildContent(IDetailChildrenBuilder & ChildrenBuilder) override;
	virtual void  Tick(float DeltaTime) override;
	virtual bool  RequiresTick( ) const override;
	virtual FName GetName( ) const override;
	virtual bool  InitiallyCollapsed( ) const override;

private:
	friend class FParameterEnchantments;
	TWeakPtr<FKismetInspectorEnchantments>         Owner;
	TSharedRef<FBlueprintGraphArgumentGroupLayout> OriginalLayoutBuilder;
};

//  //
/*************************************************************************************************/

class FParameterEnchantments : public IDetailCustomNodeBuilder, public TSharedFromThis<FParameterEnchantments>
{
public:
	FParameterEnchantments(
		const TSharedRef<FBlueprintGraphArgumentLayout> & InOriginalLayoutBuilder,
		const TWeakPtr<FParameterGroupEnchantments> &     InOwner
	);
	virtual ~FParameterEnchantments( ) override;

private:
	/** IDetailCustomNodeBuilder Interface*/
	virtual void  SetOnRebuildChildren(FSimpleDelegate InOnRegenerateChildren) override;
	virtual void  GenerateHeaderRowContent(FDetailWidgetRow & NodeRow) override;
	virtual void  GenerateChildContent(IDetailChildrenBuilder & ChildrenBuilder) override;
	virtual void  Tick(float DeltaTime) override;
	virtual bool  RequiresTick( ) const override;
	virtual FName GetName( ) const override;
	virtual bool  InitiallyCollapsed( ) const override;

private:
	FName          GetPinName( ) const;

	UE_DEPRECATED(, "Not used anymore, for removal")
	static FString MakePinNameString(FName PinFName);

private:
	DeclareProperty_Bool(AdvancedDisplay);
	DeclareProperty_Bool(AutoCreateRefTerm);

	bool        bIsAutoCreateRefTermVisible = false;
	EVisibility OnGetAutoCreateRefTermVisibility( ) const;

private:
	TWeakPtr<FParameterGroupEnchantments>     Owner;
	TWeakObjectPtr<UEdGraph>                  SelectedGraph;
	TSharedRef<FBlueprintGraphArgumentLayout> OriginalLayoutBuilder;

	FName CachedPinName;
};

#undef DeclareProperty_Bool
