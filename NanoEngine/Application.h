#pragma once

#include "Globals.h"
#include "Timer.h"
#include "Module.h"
#include "ModuleWindow.h"
#include "ModuleInput.h"
#include "ModuleRenderer3D.h"
#include "ModuleCamera3D.h"
#include "ModuleEditor.h"
#include "ModuleAssimpMeshes.h"
#include "ModuleTexture.h"
#include "ModuleScene.h"
#include"ModuleHierarchy.h"

#include<vector>

enum class GameState
{
	PLAYING,
	PAUSED,
	STOPPED
};

class Application
{
public:
	ModuleWindow* window;
	ModuleInput* input;
	ModuleRenderer3D* renderer3D;
	ModuleCamera3D* camera;
	ModuleEditor* editor;
	ModuleAssimpMeshes* assimpMeshes;
	ModuleTexture* textures;
	ModuleScene* scene;
	ModuleHierarchy* hierarchy;

private:

	Timer	ms_timer;
	std::vector<Module*> list_modules;

public:
	float	dt;
	uint	maxFrameRate;
	float	msLastFrame;

	float	gameDt;
	GameState gameState;

public:

	Application();
	~Application();

	bool Init();
	update_status Update();
	bool CleanUp();

	void LOGToEditor(const char* text);

	float GetDt();
	void SetDt(float dt);
	float GetFrameRate();
	float GetGameDt();
	bool GetIsRunning();
	bool GetIsPaused();
	
	GameState GetState();
	void SetState(GameState gameState);

private:

	void AddModule(Module* mod);
	void PrepareUpdate();
	void FinishUpdate();
};

extern Application* App;