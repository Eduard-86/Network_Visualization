#include "KismetInspectorEnchantments.h"

// Beware: include order matters more then ever

#include "Enchantments.h"
// #include "GeneratedFunction.h"

#include "K2Node_CallFunction.h"
#include "K2Node_EditablePinBase.h"
#include "BlueprintGraph/Classes/EdGraphSchema_K2.h"
#include "EdGraph/EdGraph.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "PropertyEditor/Public/DetailWidgetRow.h"

#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Text/STextBlock.h"

// Stonks
#define private public
#define protected public
#include "Kismet/Private/BlueprintDetailsCustomization.h"
#include "PropertyEditor/Private/DetailCategoryBuilderImpl.h"
#undef private
#undef protected

#include "K2Node_FunctionTerminator.h"
#include "K2Node_MacroInstance.h"
#include "K2Node_Tunnel.h"
#include "ScopedTransaction.h"
#include "PropertyEditor/Private/CustomChildBuilder.h"

const FName FKismetInspectorEnchantments::Category_Graph("Graph");
const FName FKismetInspectorEnchantments::Category_Inputs("Inputs");
const FName FKismetInspectorEnchantments::Category_Outputs("Outputs");

#define LOCTEXT_NAMESPACE "BlueprintEnchantments"

// Macro //
/**************************************************************************************************/

#pragma region Macro

/** */
#define BoolPropertyDelegates(PropertyName)\
	 IsChecked(this, & TRemovePointer<decltype(this)> :: Type :: Is##PropertyName)\
	.OnCheckStateChanged(this, & TRemovePointer<decltype(this)> :: Type :: OnIs##PropertyName##Modified)

#define EnchantedPropertyColor_Dummy		FLinearColor::Black
#define EnchantedPropertyColor_Experimental FLinearColor::Red
#define EnchantedPropertyColor_Beta			FLinearColor::Yellow
#define EnchantedPropertyColor_Release		FSlateColor()

/** */
#define MakeProperty_Bool(PropertyBaseName)\
	 AddCustomRow(PropertyName)\
	.NameContent()\
	[\
		SNew(STextBlock)\
		.Text(PropertyName)\
		.ToolTipText(ToolTipText)\
		.Font(IDetailLayoutBuilder::GetDetailFont())\
		.ColorAndOpacity(EnchantedPropertyColor)\
	]\
	.ValueContent()\
	[\
		SNew(SCheckBox).BoolPropertyDelegates(PropertyBaseName)\
	]

/** */
#define MakeAdvancedProperty_Bool(PropertyBaseName)\
	 AddCustomRow(PropertyName, true)\
	.NameContent()\
	[\
		SNew(STextBlock)\
		.Text(PropertyName)\
		.ToolTipText(ToolTipText)\
		.Font(IDetailLayoutBuilder::GetDetailFont())\
		.ColorAndOpacity(EnchantedPropertyColor)\
	]\
	.ValueContent()\
	[\
		SNew(SCheckBox).BoolPropertyDelegates(PropertyBaseName)\
	]

/** */
#define InitEnchantments(GraphProvider)\
	UEdGraph * Graph = GraphProvider->SelectedGraph.Get();\
	assumeChecked(IsValid(Graph));\
	FEnchantments      Enchantments(*Graph);

/** */
#define BeginModification(PropertyName)\
	if(UNLIKELY(bIs##PropertyName == NewCheckedState || bIs##PropertyName == ECheckBoxState::Undetermined))\
		return;\
	\
	bIs##PropertyName = NewCheckedState;\
	/* It hides text from LOCTEXT gatherer, but it's unlikely to be localized ever */\
	FScopedTransaction Transaction(\
		LOCTEXT(#PropertyName " modified transaction description", "Change " #PropertyName)\
	);

/** */
#define EndModification()\
	Enchantments.MarkDirty();

/** */
#define ImplementPropertyGetter_Bool(Class, PropertyName)\
	ECheckBoxState Class::Is##PropertyName( ) const\
	{\
		return bIs##PropertyName;\
	}

/** */
#define ImplementBoolProperty(Class, PropertyName, PropertyBody_Checked, PropertyBody_Unchecked)\
	ImplementPropertyGetter_Bool(Class, PropertyName)\
	\
	void Class::OnIs##PropertyName##Modified(const ECheckBoxState NewCheckedState)\
	{\
		BeginModification(PropertyName);\
		InitEnchantments(this);\
		Enchantments.Modify();\
		\
		if (NewCheckedState == ECheckBoxState::Checked)\
		{\
			PropertyBody_Checked\
		}\
		else\
		{\
			PropertyBody_Unchecked;\
		}\
		EndModification();\
	}

#define ImplementBoolProperty_Meta(Class, PropertyName)\
	ImplementBoolProperty(\
		Class,\
		PropertyName,\
		Enchantments.Add   (FEnchantments::Key_##PropertyName, *Graph, true);,\
		Enchantments.Remove(FEnchantments::Key_##PropertyName, *Graph, true); \
	)

#define ImplementBoolProperty_Flag(Class, PropertyName)\
	ImplementBoolProperty(\
		Class,\
		PropertyName,\
		Enchantments.Add   (FEnchantments::Key_##PropertyName, EFunctionFlags::FUNC_##PropertyName, *Graph, true);,\
		Enchantments.Remove(FEnchantments::Key_##PropertyName, EFunctionFlags::FUNC_##PropertyName, *Graph, true); \
	)

#pragma endregion ~ Macro

// ECheckBoxState operators //
/**************************************************************************************************/

#pragma region ECheckBoxState operators

FORCEINLINE void operator<<(ECheckBoxState & CheckBoxState, const bool b)
{
	CheckBoxState = b ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

FORCEINLINE bool operator*(const ECheckBoxState CheckBoxState)
{
	return CheckBoxState == ECheckBoxState::Checked;
}

FORCEINLINE void operator~(ECheckBoxState & CheckBoxState)
{
	CheckBoxState = ECheckBoxState::Undetermined;
}

FORCEINLINE ECheckBoxState operator!(ECheckBoxState CheckBoxState)
{
	switch (CheckBoxState)
	{
		case ECheckBoxState::Unchecked:
			return ECheckBoxState::Checked;
		case ECheckBoxState::Checked:
			return ECheckBoxState::Unchecked;
		case ECheckBoxState::Undetermined:
		default:
			return ECheckBoxState::Undetermined;
	}
}

FORCEINLINE bool operator==(const ECheckBoxState CheckBoxState, const bool b)
{
	return *CheckBoxState == b;
}

FORCEINLINE bool operator==(const bool b, const ECheckBoxState CheckBoxState)
{
	return *CheckBoxState == b;
}

#pragma endregion ~ ECheckBoxState operators


//  //
/*************************************************************************************************/

FKismetInspectorEnchantments::FKismetInspectorEnchantments(
	const FOnGetDetailCustomizationInstance & OriginalCustomizerProvider
)
	: OriginalCustomizerProvider(OriginalCustomizerProvider) {}

void FKismetInspectorEnchantments::CustomizeDetails(IDetailLayoutBuilder & DetailBuilder)
{
	//
	// Run original code
	//
	OriginalCustomizer = OriginalCustomizerProvider.Execute( );
	OriginalCustomizer->CustomizeDetails(DetailBuilder);

	//
	// Extract target graph and sync with it on success
	//
	const TArray<TWeakObjectPtr<UObject>> & SelectionArray = DetailBuilder.GetSelectedObjects( );
	if (SelectionArray.Num( ) > 1)
		return;
	UObject * Selection = SelectionArray[0].Get( );
	if (!IsValid(Selection))
		return;

	// Reset just in case
	new(&SelectedGraph) TWeakObjectPtr<UEdGraph>(nullptr);

	if (UEdGraph * Graph = Cast<UEdGraph>(Selection))
	{
		bool               bIsFunctionGraph = false;
		const UBlueprint * Blueprint        = Cast<UBlueprint>(Graph->GetOuter( ));
		if (Blueprint)
		{
			bIsFunctionGraph = Blueprint->FunctionGraphs.Contains(Graph);
		}

		if (bIsFunctionGraph)
			new(&SelectedGraph) TWeakObjectPtr<UEdGraph>(Graph);
	}
	else if (const UK2Node_CallFunction * CallFunctionNode = Cast<UK2Node_CallFunction>(Selection))
	{
		new(&SelectedGraph) TWeakObjectPtr<UEdGraph>(
			FindObject<UEdGraph>(
				CallFunctionNode->GetBlueprint( ),
				*(CallFunctionNode->FunctionReference.GetMemberName( ).ToString( ))
			)
		);
	}
	else if (const auto * Terminator = Cast<UK2Node_FunctionTerminator>(Selection))
	{
		new(&SelectedGraph) TWeakObjectPtr<UEdGraph>(Terminator->GetGraph( ));
	}

	if (!SelectedGraph.IsValid( ))
		return;
	if (SelectedGraph->GetOuter( )->GetClass( ) != UBlueprint::StaticClass( ))
		return;
	SyncExtraData( );

	TArray<FName> CategoryNames;
	DetailBuilder.GetCategoryNames(CategoryNames);
	const bool bHasGraphCategory   = CategoryNames.Contains(Category_Graph);
	const bool bHasInputsCategory  = CategoryNames.Contains(Category_Inputs);
	const bool bHasOutputsCategory = CategoryNames.Contains(Category_Outputs);
	const bool bAnyCategoriesAdded = bHasGraphCategory || bHasInputsCategory || bHasOutputsCategory;

	//
	// Patch Graph category
	//
	if (bHasGraphCategory)
	{
		IDetailCategoryBuilder & Graph = DetailBuilder.EditCategory(
			Category_Graph,
			NSLOCTEXT("BlueprintDetailsCustomization", "FunctionDetailsGraph", "Graph")
		);
		// @formatter:off
		{
			const FText PropertyName = LOCTEXT("Callable without wolrd context property", "Callable Without World Context");
			const FText ToolTipText = LOCTEXT("Callable without wolrd context property tooltip", "Defines whether function can be called with WorldContext pin unconnected");
			FSlateColor EnchantedPropertyColor = EnchantedPropertyColor_Beta;
			Graph.MakeProperty_Bool(CallableWithoutWorldContext);
		}
		{
			const FText PropertyName = LOCTEXT("Development only property", "Development Only");
			const FText ToolTipText = LOCTEXT(
				"Development only property tooltip",
				"Calls of functions, marked as DevelopmentOnly, will be stripped out while cooking the project if [ProjectSettings->Engine->Cooker->Cooker->Advanced->Compile Blueprints in Development Mode] is false.\n"
				"This is useful for functionality like debug output, which is expected not to exist in shipped products.\n"
				"Still, functions will be compiled, will exist, and will occupy some memory, so consider begining such functions with \"SwitchBuildType\" node to reduce waste."
			);
			FSlateColor EnchantedPropertyColor = EnchantedPropertyColor_Beta;
			Graph.MakeProperty_Bool(DevelopmentOnly);
		}
		{
			const FText PropertyName = LOCTEXT("Is editor only property", "Is Editor Only");
			const FText ToolTipText = LOCTEXT("Is editor only property tooltip", "Defines whether function can only be called from an editor script");
			FSlateColor EnchantedPropertyColor = EnchantedPropertyColor_Beta;
			Graph.MakeAdvancedProperty_Bool(EditorOnly);
		}
		{
			const FText PropertyName = LOCTEXT("Commutative associative binary operator property", "Commutative Associative Binary Operator");
			const FText ToolTipText = LOCTEXT("Commutative associative binary operator property tooltip", "Indicates that a function should use the Commutative Associative Binary node. This node lacks pin names, but features an Add Pin button that creates additional input pins.");
			FSlateColor EnchantedPropertyColor = EnchantedPropertyColor_Experimental;
			Graph.MakeAdvancedProperty_Bool(CommutativeAssociativeBinaryOperator);
		}
	}
	// @formatter:on

	//
	// Patch Inputs Category
	//
	if (bHasInputsCategory)
	{
		IDetailCategoryBuilder & Inputs = DetailBuilder.EditCategory(
			Category_Inputs,
			NSLOCTEXT("BlueprintDetailsCustomization", "FunctionDetailsInputs", "Inputs")
		);
		Inject(Inputs);
	}

	//
	// Patch Outputs Category
	//
	if (bHasOutputsCategory)
	{
		IDetailCategoryBuilder & Outputs = DetailBuilder.EditCategory(
			Category_Outputs,
			NSLOCTEXT("BlueprintDetailsCustomization", "FunctionDetailsOutputs", "Outputs")
		);
		Inject(Outputs);
	}
}

void FKismetInspectorEnchantments::SyncExtraData( )
{
	UEdGraph * Graph = SelectedGraph.Get( );
	assumeChecked(IsValid(Graph));
	const FEnchantments Data(*Graph);

	// @formatter:off
	bIsCallableWithoutWorldContext			<< Data.Has(FEnchantments::Key_CallableWithoutWorldContext);
	bIsDevelopmentOnly						<< Data.Has(FEnchantments::Key_DevelopmentOnly);
	bIsEditorOnly							<< Data.Has(FEnchantments::Key_EditorOnly);
	bIsCommutativeAssociativeBinaryOperator << Data.Has(FEnchantments::Key_CommutativeAssociativeBinaryOperator);
	// @formatter:on
}

void FKismetInspectorEnchantments::Inject(IDetailCategoryBuilder & Category)
{
	FDetailCategoryImpl & CategoryExposed = (FDetailCategoryImpl &)Category;
	FDetailLayoutMap &    LayoutMap       = CategoryExposed.LayoutMap;
	check(LayoutMap.Num() == 1);
	FDetailLayout & Layout = LayoutMap[0];
#if ENGINE_MAJOR_VERSION >= 5
	const TArray<FDetailLayoutCustomization> & SimpleLayouts = Layout.GetSimpleLayouts( );
	check(SimpleLayouts.Num() == 1);
	FDetailLayoutCustomization & TargetLayout = const_cast<FDetailLayoutCustomization &>(SimpleLayouts[0]);
#else
	FCustomizationList & SimpleLayouts = const_cast<FCustomizationList &>(Layout.GetCustomSimpleLayouts( ));
	check(Layout.GetCustomAdvancedLayouts().Num() == 0);
	check(Layout.GetDefaultSimpleLayouts().Num() == 0);
	check(Layout.GetDefaultAdvancedLayouts().Num() == 0);
	check(SimpleLayouts.Num() == 1);
	FDetailLayoutCustomization &                   TargetLayout    = SimpleLayouts[0];
#endif
	TSharedPtr<FDetailCustomBuilderRow> &          BuilderWrapper  = TargetLayout.CustomBuilderRow;
	TSharedRef<FBlueprintGraphArgumentGroupLayout> OriginalBuilder =
			(TSharedRef<FBlueprintGraphArgumentGroupLayout> &)BuilderWrapper->CustomNodeBuilder;
	BuilderWrapper->CustomNodeBuilder = MakeShareable(
		new FParameterGroupEnchantments(SharedThis(this), OriginalBuilder)
	);
}

#pragma region Utils

UBlueprint & FKismetInspectorEnchantments::GetSelectedBlueprint( ) const
{
	assumeChecked(SelectedGraph.IsValid());
	UObject * Outer = SelectedGraph->GetOuter( );
	assumeChecked(Outer);
	check(Outer->IsA<UBlueprint>());

	return (UBlueprint &)*Outer;
}

UFunction & FKismetInspectorEnchantments::GetSelectedFunction( ) const
{
	const UBlueprint & Blueprint = GetSelectedBlueprint( );
	UFunction *        Function  = Blueprint.GeneratedClass->FindFunctionByName(SelectedGraph->GetFName( ));
	assumeChecked(Function);
	return *Function;
}

UFunction & FKismetInspectorEnchantments::GetSelectedFunctionSkeleton( ) const
{
	const UBlueprint & Blueprint = GetSelectedBlueprint( );
	UFunction *        Function  = Blueprint.SkeletonGeneratedClass->FindFunctionByName(SelectedGraph->GetFName( ));
	assumeChecked(Function);
	return *Function;
}

void FKismetInspectorEnchantments::AddSelectedFunctionMetaData(const FName & Key, const TCHAR * Value) const
{
	GetSelectedFunction( ).SetMetaData(Key, Value);
	GetSelectedFunctionSkeleton( ).SetMetaData(Key, Value);
}

void FKismetInspectorEnchantments::RemoveSelectedFunctionMetaData(const FName & Key) const
{
	GetSelectedFunction( ).RemoveMetaData(Key);
	GetSelectedFunctionSkeleton( ).RemoveMetaData(Key);
}

void FKismetInspectorEnchantments::AddSelectedFunctionFlags(EFunctionFlags Flags) const
{
	GetSelectedFunction( ).FunctionFlags |= Flags;
	GetSelectedFunctionSkeleton( ).FunctionFlags |= Flags;
}

void FKismetInspectorEnchantments::RemoveSelectedFunctionFlags(EFunctionFlags Flags) const
{
	GetSelectedFunction( ).FunctionFlags ^= Flags;
	GetSelectedFunctionSkeleton( ).FunctionFlags ^= Flags;
}

#pragma endregion ~ Utils

EVisibility FKismetInspectorEnchantments::OnGetCategoryButtonTextVisibility(TWeakPtr<SWidget> RowWidget)
{
	bool ShowText = RowWidget.Pin( )->IsHovered( );

	// If the row is currently hovered, or a menu is being displayed for a button, keep the button expanded.
	if (ShowText)
	{
		return EVisibility::SelfHitTestInvisible;
	}
	else
	{
		return EVisibility::Collapsed;
	}
}

ImplementBoolProperty_Meta(FKismetInspectorEnchantments, CallableWithoutWorldContext);
ImplementBoolProperty_Meta(FKismetInspectorEnchantments, DevelopmentOnly);
ImplementBoolProperty_Meta(FKismetInspectorEnchantments, CommutativeAssociativeBinaryOperator);
ImplementBoolProperty_Flag(FKismetInspectorEnchantments, EditorOnly);

// Argument group layout //
/**************************************************************************************************/

#pragma region Argument group layout

void FParameterGroupEnchantments::SetOnRebuildChildren(FSimpleDelegate InOnRegenerateChildren)
{
	OriginalLayoutBuilder->SetOnRebuildChildren(InOnRegenerateChildren);
}

void FParameterGroupEnchantments::GenerateHeaderRowContent(FDetailWidgetRow & NodeRow)
{
	OriginalLayoutBuilder->GenerateHeaderRowContent(NodeRow);
}

void FParameterGroupEnchantments::GenerateChildContent(IDetailChildrenBuilder & ChildrenBuilder)
{
	OriginalLayoutBuilder->GenerateChildContent(ChildrenBuilder);

	FCustomChildrenBuilder &             Builder             = (FCustomChildrenBuilder &)ChildrenBuilder;
	TArray<FDetailLayoutCustomization> & ChildCustomizations =
			const_cast<TArray<FDetailLayoutCustomization> &>(Builder.GetChildCustomizations( ));
	for (FDetailLayoutCustomization & Child : ChildCustomizations)
	{
		if (Child.CustomBuilderRow.IsValid( ))
		{
			TSharedRef<FBlueprintGraphArgumentLayout> OriginalBuilder =
					(TSharedRef<FBlueprintGraphArgumentLayout> &)Child.CustomBuilderRow->CustomNodeBuilder;
			Child.CustomBuilderRow->CustomNodeBuilder = MakeShareable(
				new FParameterEnchantments(OriginalBuilder, SharedThis(this))
			);
		}
	}
}

void FParameterGroupEnchantments::Tick(float DeltaTime)
{
	OriginalLayoutBuilder->Tick(DeltaTime);
}

bool FParameterGroupEnchantments::RequiresTick( ) const
{
	return OriginalLayoutBuilder->RequiresTick( );
}

FName FParameterGroupEnchantments::GetName( ) const
{
	return OriginalLayoutBuilder->GetName( );
}

bool FParameterGroupEnchantments::InitiallyCollapsed( ) const
{
	return OriginalLayoutBuilder->InitiallyCollapsed( );
}

#pragma endregion ~ Argument group layout

// Single argument layout //
/**************************************************************************************************/

#pragma region Single argument layout

#define UndefineAllValues()\
	~bIsAdvancedDisplay;\
	~bIsAutoCreateRefTerm

#define InitMe(...)\
	const FParameterGroupEnchantments * ParamGroupWrapper = Owner.Pin().Get();\
	if (!ParamGroupWrapper)\
	{\
		UndefineAllValues();\
		return __VA_ARGS__;\
	}\
	FKismetInspectorEnchantments* InspectorWrapper = ParamGroupWrapper->Owner.Pin().Get();\
	if (!InspectorWrapper)\
	{\
		UndefineAllValues();\
		return __VA_ARGS__;\
	}\
	const FName PinFName = GetPinName();\
	if (PinFName.IsNone())\
	{\
		UndefineAllValues();\
		return __VA_ARGS__;\
	}

FParameterEnchantments::FParameterEnchantments(
	const TSharedRef<FBlueprintGraphArgumentLayout> & InOriginalLayoutBuilder,
	const TWeakPtr<FParameterGroupEnchantments> &     InOwner
)
	: Owner(InOwner)
  , SelectedGraph(Owner.Pin( )->Owner.Pin( )->SelectedGraph)
  , OriginalLayoutBuilder(InOriginalLayoutBuilder)
  , CachedPinName(GetPinName( ))
{
	InitMe( );
	InitEnchantments(InspectorWrapper);

	bIsAdvancedDisplay << EnumHasAllFlags(Enchantments.LoadPropertyFlags(PinFName), CPF_AdvancedDisplay);
	bIsAutoCreateRefTerm << Enchantments.HasListItem(FEnchantments::Key_AutoCreateRefTerm, PinFName);
	// bIsAutoCreateRefTermVisible is controlled by Tick
}

FParameterEnchantments::~FParameterEnchantments( )
{
	assumeChecked(CachedPinName.IsNone() == false);
	assumeChecked(SelectedGraph.IsValid());

	if (OriginalLayoutBuilder->ParamItemPtr.IsValid( ) == false)
	{
		InitEnchantments(this);

		FScopedTransaction Transaction(LOCTEXT("Parameter removed transaction description", "Remove Parameter"));

		Enchantments.RemoveListItem(FEnchantments::Key_AutoCreateRefTerm, CachedPinName, *Graph, true);
		Enchantments.DropPropertyFlagsOnPropertyDestruction(CachedPinName);
	}
}

void FParameterEnchantments::SetOnRebuildChildren(FSimpleDelegate InOnRegenerateChildren)
{
	OriginalLayoutBuilder->SetOnRebuildChildren(InOnRegenerateChildren);
}

void FParameterEnchantments::GenerateHeaderRowContent(FDetailWidgetRow & NodeRow)
{
	OriginalLayoutBuilder->GenerateHeaderRowContent(NodeRow);
}

void FParameterEnchantments::GenerateChildContent(IDetailChildrenBuilder & ChildrenBuilder)
{
	OriginalLayoutBuilder->GenerateChildContent(ChildrenBuilder);

	{
		const FText       PropertyName = LOCTEXT("Advanced display property", "Advanced");
		const FText       ToolTipText = LOCTEXT("Advanced display property tooltip", "Is pin allowed to be hidden");
		const FSlateColor EnchantedPropertyColor = EnchantedPropertyColor_Beta;
		ChildrenBuilder.MakeProperty_Bool(AdvancedDisplay);
	}
	{
		const FText PropertyName = LOCTEXT("Auto create reference terminal property", "Auto Create Reference Terminal");
		const FText ToolTipText  = LOCTEXT(
			"Auto create reference terminal property tooltip",
			"Default to empty container if pin is unconnected"
		);
		const FSlateColor EnchantedPropertyColor = EnchantedPropertyColor_Beta;
		ChildrenBuilder
				.MakeProperty_Bool(AutoCreateRefTerm)
				.Visibility(TAttribute<EVisibility>(this, &FParameterEnchantments::OnGetAutoCreateRefTermVisibility));
	}
	{
		const FText PropertyName = LOCTEXT("Parameter description property", "Description");
		const FText ToolTipText  = LOCTEXT("Parameter description property tooltip", "Parameter description");
		// @formatter:off
		ChildrenBuilder.AddCustomRow(PropertyName)
		.NameContent()
		[
			SNew(STextBlock)
			.Text(PropertyName)
			.ToolTipText(ToolTipText)
			.Font(IDetailLayoutBuilder::GetDetailFont())
			.ColorAndOpacity(EnchantedPropertyColor_Dummy)
		]
		.ValueContent()
		.MinDesiredWidth(1024)
		[
			SNew(SMultiLineEditableTextBox)
			// .Text(this, &FBlueprintGraphActionDetails::OnGetTooltipText)
			// .OnTextCommitted(this, &FBlueprintGraphActionDetails::OnTooltipTextCommitted)
			// .Font(IDetailLayoutBuilder::GetDetailFont())
			// .ModiferKeyForNewLine(EModifierKey::Shift)
		];
		// @formatter:on
	}
}

void FParameterEnchantments::Tick(float DeltaTime)
{
	// it actually doesn't, buuut it's more universal this way. Badly can't constexpr this
	if (OriginalLayoutBuilder->RequiresTick( ))
		OriginalLayoutBuilder->Tick(DeltaTime);

	// So yeah, it's a bad way to react on events. But a really simple one
	// Much easier then wading through slate tree and widgets bodies
	InitMe( );
	InitEnchantments(InspectorWrapper);

	//
	// Monitor param name
	//
	if (PinFName != CachedPinName)
	{
		Enchantments.UpdatePropertyName(CachedPinName, PinFName);
		CachedPinName = PinFName;
		return;
	}

	//
	// Monitor param type
	//
	const FUserPinInfo * PinInfo   = OriginalLayoutBuilder->ParamItemPtr.Pin( ).Get( );
	const UBlueprint *   Blueprint = Cast<UBlueprint>(Graph->GetOuter( ));
	if (PinInfo && Blueprint && Blueprint->bBeingCompiled == false)
	{
		const bool bIsAutoCreateRefTermVisibleNOW =
				PinInfo->PinType.ContainerType != EPinContainerType::None
				// Direction is reverse for some reason
				&& PinInfo->DesiredPinDirection == EEdGraphPinDirection::EGPD_Output;

		if (bIsAutoCreateRefTermVisible != bIsAutoCreateRefTermVisibleNOW)
		{
			FScopedTransaction Transaction(
				LOCTEXT("Parameter type changed transaction description", "Change Parameter Type")
			);

			bIsAutoCreateRefTermVisible = bIsAutoCreateRefTermVisibleNOW;
			if (!bIsAutoCreateRefTermVisible)
				Enchantments.RemoveListItem(FEnchantments::Key_AutoCreateRefTerm, PinFName, *Graph, true);
			return;
		}
	}
}

bool FParameterEnchantments::RequiresTick( ) const
{
	return true;
}

FName FParameterEnchantments::GetName( ) const
{
	return OriginalLayoutBuilder->GetName( );
}

bool FParameterEnchantments::InitiallyCollapsed( ) const
{
	return OriginalLayoutBuilder->InitiallyCollapsed( );
}

FName FParameterEnchantments::GetPinName( ) const
{
	assumeChecked(OriginalLayoutBuilder->ParamItemPtr.IsValid());
	const FUserPinInfo * PinInfo = OriginalLayoutBuilder->ParamItemPtr.Pin( ).Get( );
	if (!PinInfo)
	{
		return NAME_None;
	}

	return PinInfo->PinName;
}

FString FParameterEnchantments::MakePinNameString(FName PinFName)
{
	assumeChecked(!PinFName.IsNone());
	return PinFName.ToString( ).Append(TEXT(", "));
}

ImplementPropertyGetter_Bool(FParameterEnchantments, AdvancedDisplay)

void FParameterEnchantments::OnIsAdvancedDisplayModified(const ECheckBoxState NewCheckedState)
{
	BeginModification(AdvancedDisplay);

	InitMe( );
	InitEnchantments(InspectorWrapper);
	if (*bIsAdvancedDisplay)
	{
		Enchantments.AddPropertyFlag(PinFName, CPF_AdvancedDisplay, *Graph, true);
	}
	else
	{
		Enchantments.RemovePropertyFlag(PinFName, CPF_AdvancedDisplay, *Graph, true);
	}

	EndModification( );
}

ImplementPropertyGetter_Bool(FParameterEnchantments, AutoCreateRefTerm)

void FParameterEnchantments::OnIsAutoCreateRefTermModified(const ECheckBoxState NewCheckedState)
{
	BeginModification(AutoCreateRefTerm);

	InitMe( );
	InitEnchantments(this);
	Enchantments.Modify( );

	if (*bIsAutoCreateRefTerm)
	{
		Enchantments.AddListItem(FEnchantments::Key_AutoCreateRefTerm, PinFName, *Graph, true);
	}
	else
	{
		Enchantments.RemoveListItem(FEnchantments::Key_AutoCreateRefTerm, PinFName, *Graph, true);
	}

	EndModification( );
}

EVisibility FParameterEnchantments::OnGetAutoCreateRefTermVisibility( ) const
{
	return bIsAutoCreateRefTermVisible ? EVisibility::Visible : EVisibility::Collapsed;
}

#undef UndefineAllValues
#undef InitMe

#pragma endregion ~ Single argument layout

#undef LOCTEXT_NAMESPACE

#undef BoolPropertyDelegates
#undef EnchantedPropertyColor_Dummy
#undef EnchantedPropertyColor_Experimental
#undef EnchantedPropertyColor_Beta
#undef EnchantedPropertyColor_Release
#undef MakeProperty_Bool
#undef MakeAdvancedProperty_Bool
#undef InitEnchantments
#undef BeginModification
#undef EndModification
#undef ImplementPropertyGetter_Bool
#undef ImplementBoolProperty
#undef ImplementBoolProperty_Meta
#undef ImplementBoolProperty_Flag
