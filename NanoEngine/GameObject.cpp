#include "GameObject.h"
#include "Component.h"
#include "ComponentTransform.h"
#include "ComponentMaterial.h"
#include "ComponentMesh.h"
#include "ComponentCamera.h"
#include "ComponentPhysics.h"
#include "OurPrimitive.h"

GameObject::GameObject()
{
	name = "GameObject";
	mParent = nullptr;
	transform = new ComponentTransform(this);
	isTimetoDelete = false;
	Stype = GeometryType::NONE;
	mComponents.push_back(transform);
}

GameObject::~GameObject()
{
	name = "";
	transform = nullptr;

	if (isTimetoDelete && mParent != nullptr) {
		mParent->DeleteChild(this);
	}


	for (size_t i = 0; i < mComponents.size(); ++i)
	{
		delete mComponents[i];
		mComponents[i] = nullptr;
	}

	for (size_t i = 0; i < mChildren.size(); ++i)
	{
		delete mChildren[i];
		mChildren[i] = nullptr;
	}

	mComponents.clear();

}

GameObject::GameObject(GameObject* parent)
{
	name = "GameObject";
	this->mParent = parent;
	isTimetoDelete = false;
	Stype = GeometryType::NONE;

	if (parent != nullptr)
	{
		parent->mChildren.push_back(this);
	}

	transform = new ComponentTransform(this);

	mComponents.push_back(transform);
}

void GameObject::AddComponent(Component* component)
{
	mComponents.push_back(component);
	component->mOwner = this;
}


void GameObject::Update()
{
	// No tots els children o components tenen un update
	for (size_t i = 0; i < mChildren.size(); ++i)
	{
		mChildren[i]->Update();
	}

	for (size_t i = 0; i < mComponents.size(); ++i)
	{
		mComponents[i]->Update();
	}
}

GameObject* GameObject::GetParent()
{
	return mParent;
}

ComponentMesh* GameObject::GetMeshComponent()
{
	for (size_t i = 0; i < mComponents.size(); i++)
	{
		if (mComponents[i]->type == ComponentType::MESH)
		{
			return (ComponentMesh*)mComponents[i];
		}
	}
	return nullptr;
}

ComponentTransform* GameObject::GetTransformComponent()
{
	for (size_t i = 0; i < mComponents.size(); i++)
	{
		if (mComponents[i]->type == ComponentType::TRANSFORM)
		{
			if (auto* transformComponent = dynamic_cast<ComponentTransform*>(mComponents[i])) {
				return transformComponent;
			}
		}
	}
	return nullptr;
}

ComponentMaterial* GameObject::GetComponentTexture()
{
	for (size_t i = 0; i < mComponents.size(); i++)
	{
		if (mComponents[i]->type == ComponentType::MATERIAL) {

			return (ComponentMaterial*)mComponents[i];
		}
	}

	return nullptr;
}

ComponentCamera* GameObject::GetComponentCamera()
{
	for (size_t i = 0; i < mComponents.size(); i++)
	{
		if (mComponents[i]->type == ComponentType::CAMERA) {

			return (ComponentCamera*)mComponents[i];
		}
	}

	return nullptr;
}

ComponentPhysics* GameObject::GetComponentPhysics()
{
	for (size_t i = 0; i < mComponents.size(); i++)
	{
		if (mComponents[i]->type == ComponentType::PHYSICS) {

			return (ComponentPhysics*)mComponents[i];
		}
	}

	return nullptr;
}

bool GameObject::CheckChildOf(GameObject* parent)
{
	if (parent == this)
	{
		return true;
	}
	if (parent->mChildren.empty())
	{
		return false;
	}
	for (size_t i = 0; i < parent->mChildren.size(); i++)
	{
		if (CheckChildOf(parent->mChildren[i])) return true;
	}
	return false;
}

void GameObject::DeleteChild(GameObject* child)
{
	for (int i = 0; i < mChildren.size(); i++) {
		if (mChildren[i] == child) {
			mChildren.erase(mChildren.begin() + i);
			child->mParent = nullptr;
		}
	}
}

bool GameObject::SetNewParent(GameObject* newParent)
{
	if (mParent != nullptr) {
		if (newParent->CheckChildOf(this)) return false;

		mParent->DeleteChild(this);
	}

	mParent = newParent;
	newParent->mChildren.push_back(this);

	return true;
}

bool GameObject::SetAsChildOf(GameObject* gameObject)
{
	if (CheckChildOf(gameObject))
	{
		return false;
	}
	gameObject->mParent = this;
	mChildren.push_back(gameObject);
	return true;
}


void GameObject::PrintInspector()
{

	char* listComponenets[]{ "Add Component", "Mesh Component", "Texture Component" };
	char aux[255] = { ' ' }; 


	if (mParent != nullptr)
	{
		strcpy(aux, this->name.c_str());


		ImGui::BulletText("Name:");
		ImGui::SameLine();

		
		ImGui::InputText("##Name", aux, 255, ImGuiInputTextFlags_EnterReturnsTrue);

		if (ImGui::IsKeyDown(ImGuiKey_Enter))
			name = aux;


		if (ImGui::Button("Delete")) {
			
			isTimetoDelete = true;
			delete App->hierarchy->objSelected;
			App->hierarchy->objSelected = nullptr;
		}

		for (size_t i = 0; i < mComponents.size(); i++)
		{
			ImGui::Separator();

			mComponents[i]->PrintInspector();
		}

		ImGui::Separator();

		if (ImGui::Button("Add Component", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
			ImGui::OpenPopup("AddComponentPopup");
		}

		if (ImGui::BeginPopup("AddComponentPopup")) {
			if (ImGui::Selectable("Transform"))
			{
				if (App->hierarchy->objSelected->GetTransformComponent() == nullptr)
				{
					transform = new ComponentTransform(this);
					App->hierarchy->objSelected->AddComponent(transform);
				}
			}
			//if (ImGui::Selectable("Material"))
			//{
			//	if (App->hierarchy->objSelected->GetComponentTexture() == nullptr)
			//	{
			//		material = new ComponentMaterial(this);
			//		App->hierarchy->objSelected->AddComponent(material);
			//	}
			//}
			if (ImGui::Selectable("Camera"))
			{
				if (App->hierarchy->objSelected->GetComponentCamera() == nullptr)
				{
					camera = new ComponentCamera();
					App->hierarchy->objSelected->AddComponent(camera);
				}
			}
			if (ImGui::Selectable("Physics"))
			{
				if (App->hierarchy->objSelected->GetComponentPhysics() == nullptr)
				{
					physics = new ComponentPhysics(this);
					App->hierarchy->objSelected->AddComponent(physics);
				}
			}
			ImGui::EndPopup();
		}
	}
}