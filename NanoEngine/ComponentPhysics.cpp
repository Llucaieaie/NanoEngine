#include "ComponentPhysics.h"
#include "ImGui/imgui.h"
#include "Primitive.h"
#include "ModulePhysics.h"
#include "ComponentTransform.h"

#include "Module.h"

ComponentPhysics::ComponentPhysics(GameObject* owner) : Component(owner)
{

	phys = nullptr;

	shapeSelected = ColliderShape::NONE;

	isStatic = false;
	collider = nullptr;

	scale = float3(1, 1, 1);
}

ComponentPhysics::~ComponentPhysics()
{

}

void ComponentPhysics::Update()
{
	if (isStatic)
		App->physics->SetBodyMass(collider, 0);
	else
		App->physics->SetBodyMass(collider, mass);

	if (collider != nullptr) {
		float matrix[16];
		collider->GetTransform(matrix);
		mOwner->GetTransformComponent()->SetTransformFromMatrix(matrix);
	}
}

void ComponentPhysics::PrintInspector()
{
	Update();

	if (ImGui::CollapsingHeader("Physics"))
	{
		ImGui::Checkbox("Static\t", &isStatic);

		ImGui::SameLine();
		ImGui::PushItemWidth(120.0f);
		if (ImGui::InputFloat("Mass", &mass, 1.0f, 33))
			App->physics->SetBodyMass(collider, mass);

		ImGui::Text("\n");

		if (ImGui::Combo("Collider", reinterpret_cast<int*>(&shapeSelected), "Box\0Sphere\0Cylinder\0None"))

			//	CheckShapes();
		
		switch (shapeSelected)
		{
		case ComponentPhysics::ColliderShape::BOXSHAPE:
		{
			if (collider == nullptr) {
				DefaultBoxCollider();
			}
			else {
				App->physics->RemoveBody(collider);
				DefaultBoxCollider();
			}
			break;
		}
		case ComponentPhysics::ColliderShape::SPHERESHAPE:
		{
			if (collider == nullptr) {
				DefaultSphereCollider();
			}
			else {
				App->physics->RemoveBody(collider);
				DefaultSphereCollider();
			}
		}
		break;
		case ComponentPhysics::ColliderShape::CYLINDERSHAPE:
		{
			if (collider == nullptr) {
				collider = App->physics->AddBody(cylinder, 1);
			}
		}
		break;
		default:
			break;
		}

		//if (shapeSelected == ColliderShape::BOXSHAPE) 
		//{
		//	
		//	ImGui::Text("X\t\t Y\t\t Z");
		//	if (ImGui::DragFloat3("Scale: ", scale.ptr()));
		//	{
		//		cube.size.x *= scale.x;
		//		cube.size.y *= scale.y;
		//		cube.size.z *= scale.z;
		//	}
		//	cube.Scale(scale.x, scale.y, scale.z); 
		//}

		if (ImGui::Button("Remove Collider"))
		{
			App->physics->RemoveBody(collider);
			collider = nullptr;
		}

	}
}

void ComponentPhysics::DefaultBoxCollider()
{
	cube.size = (3, 3, 3);
	transformComponent = App->hierarchy->objSelected->GetTransformComponent();
	if (transformComponent) {
		float3 pos = transformComponent->getPosition();
		cube.SetPos(pos.x / 2, pos.y / 2 + cube.size.y / 4, pos.z / 2);
	}
	collider = App->physics->AddBody(cube, 1);
	mass = 1;
}

void ComponentPhysics::DefaultSphereCollider()
{
	sphere.radius = 2;
	transformComponent = App->hierarchy->objSelected->GetTransformComponent();
	if (transformComponent) {
		float3 pos = transformComponent->getPosition();
		sphere.SetPos(pos.x / 2, pos.y / 2 + sphere.radius / 2, pos.z / 2);
	}
	collider = App->physics->AddBody(sphere, 1);
	mass = 1;
}

