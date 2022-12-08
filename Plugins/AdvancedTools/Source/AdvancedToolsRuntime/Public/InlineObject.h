#pragma once

#include "UObject/Object.h"

#include "InlineObject.generated.h"

UCLASS(BlueprintType, Blueprintable, Abstract, DefaultToInstanced, EditInlineNew)
class ADVANCEDTOOLSRUNTIME_API UInlineObject : public UObject
{
	GENERATED_BODY( )
};
