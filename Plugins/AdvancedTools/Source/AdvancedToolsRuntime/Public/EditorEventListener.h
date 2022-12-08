#pragma once

#include "UObject/Object.h"

#include "EditorEventListener.generated.h"

/**
 * Create a function with corresponding name to handle the event
 * Supported events:
 * - PostInitProperties
 * - BeginCacheForCookedPlatformData
 * - IsCachedCookedPlatformDataLoaded WILL BE IN THE FUTURE
 * - WillNeverCacheCookedPlatformDataAgain
 * - ClearCachedCookedPlatformData
 * - ClearAllCachedCookedPlatformData
 */
UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class UEditorEventListener final : public UObject
{
	GENERATED_BODY( )

#if WITH_EDITOR
public:
	virtual void PostInitProperties( ) override;

	virtual void BeginCacheForCookedPlatformData(const ITargetPlatform * TargetPlatform) override;
	// virtual bool IsCachedCookedPlatformDataLoaded(const ITargetPlatform * TargetPlatform) override;
	virtual void WillNeverCacheCookedPlatformDataAgain( ) override;
	virtual void ClearCachedCookedPlatformData(const ITargetPlatform * TargetPlatform) override;
	virtual void ClearAllCachedCookedPlatformData( ) override;

private:
	void ForwardEvent(FName MethodName) const;
#endif
};

/**
 * Create a function with corresponding name to handle the event
 * Supported events:
 * - PostInitProperties
 * - BeginCacheForCookedPlatformData
 * - IsCachedCookedPlatformDataLoaded WILL BE IN THE FUTURE
 * - WillNeverCacheCookedPlatformDataAgain
 * - ClearCachedCookedPlatformData
 * - ClearAllCachedCookedPlatformData
 */
USTRUCT(BlueprintType)
struct FEditorEventListenerContainer
{
	GENERATED_BODY( )

#if WITH_EDITORONLY_DATA
	UPROPERTY(Instanced, EditAnywhere)
	UEditorEventListener * Listener;
#endif
};
