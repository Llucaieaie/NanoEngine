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

	boxSize = float3(3, 3, 3);
	sphereRadius = 2.0f;
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
		
		switch (shapeSelected)
		{
		case ComponentPhysics::ColliderShape::BOXSHAPE:
			ImGui::Text("X\t\t Y\t\t Z");
			ImGui::PushItemWidth(200.0f);
			if (ImGui::DragFloat3("Box Size ", boxSize.ptr()), 1.0f, 0.0f, 1000.0f)
			{
				cube.size.x = boxSize.x;
				cube.size.y = boxSize.y;
				cube.size.z = boxSize.z;
				if (ImGui::Button("Update Collider")) {
					App->physics->RemoveBody(collider);
					collider = App->physics->AddBody(cube, mass);
				}
			}
			break;
		case ComponentPhysics::ColliderShape::SPHERESHAPE:
			if (ImGui::DragFloat("Sphere Radius ", &sphereRadius), 1.0f, 0.0f, 1000.0f)
			{
				sphere.radius = sphereRadius;
				if (ImGui::Button("Update Collider")) {
					App->physics->RemoveBody(collider);
					collider = App->physics->AddBody(sphere, mass);
				}
			}
			break;
		}

		if (ImGui::Button("Remove Collider"))
		{
			App->physics->RemoveBody(collider);
			collider = nullptr;
		}

	}
	Update();
}

void ComponentPhysics::DefaultBoxCollider()
{
	cube.size = (3, 3, 3);
	transformComponent = App->hierarchy->objSelected->GetTransformComponent();
	if (transformComponent) {
		float3 pos = transformComponent->getPosition();
		cube.SetPos(pos.x / 2, pos.y / 2 / 4, pos.z / 2);
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
		sphere.SetPos(pos.x / 2, pos.y / 2 / 2, pos.z / 2);
	}
	collider = App->physics->AddBody(sphere, 1);
	mass = 1;
}

