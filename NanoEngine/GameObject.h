#pragma once
#include "Module.h"
#include <vector>
#include <string>


enum class ComponentType;
enum class GeometryType;
class Component;
class ComponentTransform;
class ComponentMesh;
class ComponentMaterial;
class ComponentCamera;
class ComponentPhysics;
class GameObject
{
public:

	GameObject();
	GameObject(GameObject* parent);
	~GameObject();

	void AddComponent(Component* component);
	void Update();
	GameObject* GetParent();
	bool SetNewParent(GameObject* newParent);
	bool GameObject::SetAsChildOf(GameObject* gameObject);
	void PrintInspector();
	bool CheckChildOf(GameObject* parent);
	void DeleteChild(GameObject* child);


	ComponentMesh* GetMeshComponent();
	ComponentTransform* GetTransformComponent();
	ComponentMaterial* GetComponentTexture();
	ComponentCamera* GetComponentCamera();
	ComponentPhysics* GetComponentPhysics();

	bool isTimetoDelete;
	std::string name;

	GameObject* mParent;
	std::vector<GameObject*> mChildren;
	std::vector<Component*> mComponents;

	ComponentTransform* transform;
	ComponentMesh* mesh;
	ComponentMaterial* material;
	ComponentCamera* camera;
	ComponentPhysics* physics;

	GeometryType Stype;

};