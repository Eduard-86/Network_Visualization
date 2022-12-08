#pragma once

#include "EnchantedNode.generated.h"

UINTERFACE( )
class UEnchantedNode : public UInterface
{
	GENERATED_BODY( )
};


class IEnchantedNode
{
	GENERATED_BODY( )
public:

	/** */
	virtual bool CanEnchant(const UK2Node& Node) const = 0;

	/**
	 * Splits first valid pin if any.
	 * Supposed to be called on class default objects.
	 *
	 * @return The new node on success
	 */
	virtual UK2Node * Enchant(const UK2Node & Node) const = 0;

	/**
	 * Splits the provided pin if possible.
	 * Supposed to be called on class default objects.
	 *
	 * @return Explicit value on success, nullptr on existing node modification, non-nullptr on new node construction
	 */
	virtual TOptional<UK2Node *> Enchant(UK2Node & Node, const UEdGraphPin & Pin) const = 0;

	/**
	 * Reconstructs the original node from which this enchantment was created
	 */
	virtual UK2Node & Disenchant( ) const = 0;

	/*
	 * Splits next valid pin if any, or performs Disenchant and returns its result
	 */
	virtual UK2Node * SplitNextPin( ) = 0;
};
