#pragma once

#include "BPAssetUserData.generated.h"

UCLASS(Blueprintable, DefaultToInstanced, abstract, editinlinenew)
class UBPAssetUserData : public UAssetUserData
{
	GENERATED_BODY( )

public:
	virtual void PostEditChangeOwner() override { PostEditChangeOwner_BP(); }

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "PostEditChangeOwner"))
	void PostEditChangeOwner_BP();
};
