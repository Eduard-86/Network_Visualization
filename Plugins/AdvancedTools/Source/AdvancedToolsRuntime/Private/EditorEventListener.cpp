#if WITH_EDITOR

#include "EditorEventListener.h"

void UEditorEventListener::PostInitProperties( )
{
	UObject::PostInitProperties( );
	ForwardEvent(TEXT("PostInitProperties"));
}

void UEditorEventListener::BeginCacheForCookedPlatformData(const ITargetPlatform * TargetPlatform)
{
	Super::BeginCacheForCookedPlatformData(TargetPlatform);
	ForwardEvent(TEXT("BeginCacheForCookedPlatformData"));
}

void UEditorEventListener::WillNeverCacheCookedPlatformDataAgain( )
{
	UObject::WillNeverCacheCookedPlatformDataAgain( );
	ForwardEvent(TEXT("WillNeverCacheCookedPlatformDataAgain"));
}

void UEditorEventListener::ClearCachedCookedPlatformData(const ITargetPlatform * TargetPlatform)
{
	UObject::ClearCachedCookedPlatformData(TargetPlatform);
	ForwardEvent(TEXT("ClearCachedCookedPlatformData"));
}

void UEditorEventListener::ClearAllCachedCookedPlatformData( )
{
	UObject::ClearAllCachedCookedPlatformData( );
	ForwardEvent(TEXT("ClearAllCachedCookedPlatformData"));
}

void UEditorEventListener::ForwardEvent(const FName MethodName) const
{
	UObject * Outer = GetOuter( );
	if (!Outer)
		return;

	UFunction * Function = Outer->GetClass( )->FindFunctionByName(MethodName);
	if (!Function) return;

	Outer->ProcessEvent(Function, nullptr);
}
#endif
