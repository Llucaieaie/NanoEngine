#pragma once
#include "Module.h"
#include "Globals.h"

class GameObject;
class ComponentCamera;

class ModuleScene : public Module
{
public:
	ModuleScene(Application* app, bool start_enabled = true);
	~ModuleScene();

	bool Init();
	bool Start();
	update_status PreUpdate(float dt);
	update_status Update(float dt);
	update_status PostUpdate(float dt);
	bool CleanUp();
	void DefaultCamera();
	void SetGameObjectSelected(GameObject* gameObject);

	GameObject* CreateGameObject(GameObject* parent);
	GameObject* root = nullptr;
	GameObject* prova1;
	GameObject* prova2;
	GameObject* prova3;
	GameObject* prova4;
	GameObject* prova5;
	GameObject* prova6;

	GameObject* bakerHouse;
	GameObject* cameraObj;
	ComponentCamera* cam;
};