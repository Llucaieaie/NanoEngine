#include "ComponentPhysics.h"
#include "ImGui/imgui.h"
#include "Primitive.h"
#include "ModulePhysics.h"

#include "Module.h"

ComponentPhysics::ComponentPhysics(GameObject* owner) : Component(owner)
{

	phys = nullptr;

	shapeSelected = ColliderShape::NONE;

	isStatic = false;
	collider = nullptr;
}

ComponentPhysics::~ComponentPhysics()
{

}

void ComponentPhysics::Update()
{

}

void ComponentPhysics::PrintInspector()
{

}