#pragma once

#include "KismetNodes/SGraphNodeK2Base.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

class UK2Node;

class SGraphNodeK2Enchanted final : public SGraphNodeK2Base
{
public:
	SLATE_BEGIN_ARGS(SGraphNodeK2Enchanted) { }

	SLATE_END_ARGS()

	void                   Construct(const FArguments & InArgs, UK2Node * InNode);
	EActiveTimerReturnType DeferredConstruct(double InCurrentTime, float InDeltaTime);
	virtual FReply         OnMouseButtonDown(const FGeometry & MyGeometry, const FPointerEvent & MouseEvent) override;

	virtual void UpdateGraphNode( ) override;

private:
	// ReSharper disable once CppHidingFunction
	void UpdateStandardNode( );
	// ReSharper disable once CppHidingFunction
	void         CreateAdvancedViewArrow(TSharedPtr<SVerticalBox> MainBox);
	virtual void AddPin(const TSharedRef<SGraphPin> & PinToAdd) override;

	void OnDoubleClicked(UEdGraphNode * Node) const;

	bool HaveAnyAdvancedPins( ) const;
	bool HaveAnyRegularPins( ) const;
	bool HaveBothRegularAndAdvancedPins( ) const;

	static FName GetPinName(const UEdGraphPin * Pin);
	static FName GetPinName(const SGraphPin & Pin);
	static FName GetPinName(const TSharedRef<SGraphPin> & Pin);
	static bool  IsPinAdvanced(const UEdGraphPin & Pin);
	static bool  IsPinAdvanced(const TSharedRef<SGraphPin> & Pin);

	bool PinsColumnWillCompletelyCollapse(const UEdGraphPin & Pin) const;
	bool CanHidePin(const UEdGraphPin & Pin) const;
	bool CanHidePin(const SGraphPin & Pin) const;
	bool CanHidePin(const TSharedRef<SGraphPin> & Pin) const;
	bool ShouldHidePin(const UEdGraphPin & Pin) const;
	bool ShouldHidePin(const SGraphPin & Pin) const;

	void SetIsShowingAdvancedPins(bool Value);
	bool GetIsShowingAdvancedPins( ) const;
	void SwitchIsShowingAdvancedPins( );

private:
	static constexpr TCHAR HideMe_Prefix = '_';

	bool             bIsDefaultDoubleClickBehaviourOverriden = false;
	FSingleNodeEvent OriginalDoubleClickHandler;
};
