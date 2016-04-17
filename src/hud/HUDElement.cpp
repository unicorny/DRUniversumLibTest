#include "HUD/HUDElement.h"

namespace HUD {
	Element::Element(const char* name, ContainerNode* parent)
		: ContainerNode(name, parent)
	{

	}

	Element::~Element()
	{

	}
}