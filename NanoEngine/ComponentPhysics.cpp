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
	if (ImGui::CollapsingHeader("Physics"))
	{
		ImGui::Checkbox("Static", &isStatic);

		if (ImGui::Combo("Collider", reinterpret_cast<int*>(&shapeSelected), "Box\0Sphere\0Cylinder\0None"))

		//	CheckShapes();

		if (shapeSelected != ColliderShape::NONE) {
			switch (shapeSelected)
			{
			case ComponentPhysics::ColliderShape::BOXSHAPE:
			{
				PrimCube cube;
				collider = phys->AddBody(cube, 3.f);
			}
			break;
			case ComponentPhysics::ColliderShape::SPHERESHAPE:
			{
				PrimSphere sphere;
				collider = phys->AddBody(sphere, 3.f);
			}
			break;
			case ComponentPhysics::ColliderShape::CYLINDERSHAPE:
			{
				PrimCylinder cylinder;
				collider = phys->AddBody(cylinder, 3.f);
			}
			break;
			default:
				break;
			}
		}

	//	if (ImGui::Button("Remove Collider"))
	//	{
	//		phys->RemoveBody(collider);
	//		collider->~PhysBody3D();
	//		collider = nullptr;
	//	}
	
		if (ImGui::Button("Remove Component", ImVec2(140, 20)))
		{

		}
	}
}