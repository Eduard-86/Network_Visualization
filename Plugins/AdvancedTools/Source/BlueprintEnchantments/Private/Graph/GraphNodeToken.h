#pragma once

#include "Logging/TokenizedMessage.h"


class FGraphNodeToken final : public IMessageToken
{
public:
	static TSharedRef<FGraphNodeToken> Create(UEdGraphNode & Node);

	// IMessageToken
	virtual EMessageToken::Type GetType( ) const override { return EMessageToken::EdGraph; }
	// ~IMessageToken

private:
#if ENGINE_MAJOR_VERSION >= 5
	friend class SharedPointerInternals::TIntrusiveReferenceController<FGraphNodeToken, ESPMode::ThreadSafe>;
#else
	friend class SharedPointerInternals::TIntrusiveReferenceController<FGraphNodeToken>;
#endif

	FGraphNodeToken(UEdGraphNode & Node);
	void OnActivated(const TSharedRef<IMessageToken> & Token) const;

private:
	TWeakObjectPtr<UEdGraphNode> Node;
};
