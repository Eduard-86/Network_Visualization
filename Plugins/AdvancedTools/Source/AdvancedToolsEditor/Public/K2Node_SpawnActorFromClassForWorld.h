#pragma once

#include "K2Node_SpawnActorFromClass.h"

#include "K2Node_SpawnActorFromClassForWorld.generated.h"

/**
 * 
 */
UCLASS( )
class ADVANCEDTOOLSEDITOR_API UK2Node_SpawnActorFromClassForWorld final : public UK2Node_SpawnActorFromClass
{
	GENERATED_BODY( )

public:
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual bool  IsCompatibleWithGraph(const UEdGraph * TargetGraph) const override;

	virtual bool UseWorldContext( ) const override { return true; }
};
