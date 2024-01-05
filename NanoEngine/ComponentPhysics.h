#pragma once
#include "Component.h"
#include "PhysBody3D.h"
#include "Application.h"
#include "Globals.h"

class ComponentPhysics : public Component
{
public:
	enum class ColliderShape
	{
		BOXSHAPE,
		SPHERESHAPE,
		CYLINDERSHAPE,
		NONE,
	};

	ComponentPhysics(GameObject* owner);
	virtual	~ComponentPhysics();

	void Update() override;

	void Enable() { isActive = true; }
	void Disable() { isActive = false; }
	bool IsEnable() { return isActive; }

	void PrintInspector();

public:

	ModulePhysics* phys;

	ColliderShape shapeSelected;

	bool isStatic;
	PhysBody3D* collider;

};