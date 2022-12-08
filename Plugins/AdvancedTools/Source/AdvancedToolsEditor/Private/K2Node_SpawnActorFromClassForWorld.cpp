// Fill out your copyright notice in the Description page of Project Settings.


#include "K2Node_SpawnActorFromClassForWorld.h"

#include "Kismet2/BlueprintEditorUtils.h"

FText UK2Node_SpawnActorFromClassForWorld::GetNodeTitle(const ENodeTitleType::Type TitleType) const
{
	return TitleType == ENodeTitleType::MenuTitle
	       ? NSLOCTEXT("K2Node", "SpawnActorForWorld_BaseTitle", "Spawn Actor from Class for World")
	       : FText::FromString(
		       Super::GetNodeTitle(TitleType)
		       .ToString( )
		       .Replace(TEXT("SpawnActor"), TEXT("SpawnActorForWorld"))
	       );
}

bool UK2Node_SpawnActorFromClassForWorld::IsCompatibleWithGraph(const UEdGraph * TargetGraph) const
{
	const UBlueprint * Blueprint = FBlueprintEditorUtils::FindBlueprintForGraph(TargetGraph);
	return true
			&& Super::Super::IsCompatibleWithGraph(TargetGraph)
			&& (
				false
				|| !Blueprint
				|| (
					true
					&& FBlueprintEditorUtils::FindUserConstructionScript(Blueprint) != TargetGraph
					// NOPE. NOT. YOUR. ... BUSINESS
					// && Blueprint->GeneratedClass->GetDefaultObject( )->ImplementsGetWorld( )
				)
			);
}
