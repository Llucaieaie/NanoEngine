#include "Application.h"

Application::Application()
{
	window = new ModuleWindow(this);
	input = new ModuleInput(this);
	renderer3D = new ModuleRenderer3D(this);
	camera = new ModuleCamera3D(this);
	editor = new ModuleEditor(this);
	assimpMeshes = new ModuleAssimpMeshes(this);
	textures = new ModuleTexture(this);
	scene = new ModuleScene(this);
	hierarchy = new ModuleHierarchy(this);
	// The order of calls is very important!
	// Modules will Init() Start() and Update in this order
	// They will CleanUp() in reverse order

	// Main Modules
	AddModule(window);
	AddModule(camera);
	AddModule(input);
	AddModule(hierarchy);
	AddModule(assimpMeshes);
	AddModule(textures);
	AddModule(scene);

	// Renderer last!
	AddModule(renderer3D);
	AddModule(editor);

	isSavingConfig = false;
	isLoadingConfig = false;
}

Application::~Application()
{
	for (std::vector<Module*>::iterator it = list_modules.begin(); it != list_modules.end(); ++it)
	{
		delete (*it);
		(*it) = nullptr;
	}
}

bool Application::Init()
{
	gameState = GameState::STOPPED;
	
	bool ret = true;

	configFileName = "EngineConfiguration.json";

	JSON_Value* root = configFile.FileToValue(configFileName);

	if (configFile.GetRootValue() == NULL)
	{
		LOG("Error loading configuration file");
		ret = false;
	}

	JsonParser application = configFile.GetChild(root, "App");

	maxFrameRate = application.JsonValToNumber("FPS");

	// Call Init() in all modules
	for (std::vector<Module*>::const_iterator it = list_modules.cbegin(); it != list_modules.cend() && ret; ++it)
	{
		(*it)->Init();
	}

	// After all Init calls we call Start() in all modules
	LOG("-------------- Application Start --------------");
	for (std::vector<Module*>::const_iterator it = list_modules.cbegin(); it != list_modules.cend() && ret; ++it)
	{
		(*it)->Start();
	}
	
	ms_timer.Start();
	return ret;
}

// ---------------------------------------------
void Application::PrepareUpdate()
{
	dt = (float)ms_timer.Read() / 1000.0f;
	ms_timer.Start();
}

// ---------------------------------------------
void Application::FinishUpdate()
{
	if (renderer3D->GetVsync())
	{
		msLastFrame = ms_timer.Read();

		float timeToWait = 1000.0f / (float)maxFrameRate;

		if (msLastFrame < timeToWait)
			SDL_Delay(static_cast<Uint32>(fabs(timeToWait - msLastFrame)));

		msLastFrame = ms_timer.Read();
	}

	if (isLoadingConfig)
		LoadConfig();
	
	if (isSavingConfig)
		SaveConfig();
}



// Call PreUpdate, Update and PostUpdate on all modules
update_status Application::Update()
{
	update_status ret = UPDATE_CONTINUE;

	if (gameState == GameState::PLAYING) gameDt = dt;
	else gameDt = 0;

	PrepareUpdate();
	
	for (std::vector<Module*>::const_iterator it = list_modules.cbegin(); it != list_modules.cend() && ret == UPDATE_CONTINUE; ++it)
	{
		ret = (*it)->PreUpdate(dt);
	}

	for (std::vector<Module*>::const_iterator it = list_modules.cbegin(); it != list_modules.cend() && ret == UPDATE_CONTINUE; ++it)
	{
		ret = (*it)->Update(dt);
	}

	for (std::vector<Module*>::const_iterator it = list_modules.cbegin(); it != list_modules.cend() && ret == UPDATE_CONTINUE; ++it)
	{
		ret = (*it)->PostUpdate(dt);
	}

	FinishUpdate();
	return ret;
}

bool Application::CleanUp()
{
	bool ret = true;

	for (std::vector<Module*>::reverse_iterator it = list_modules.rbegin(); it != list_modules.rend() && ret; ++it)
	{
		ret = (*it)->CleanUp();
	}


	return ret;
}

void Application::LOGToEditor(const char* text)
{
	
	editor->LOGToConsole(text);
	
}

float Application::GetDt() 
{ 
	return dt; 
}

float Application::GetFrameRate() 
{ 
	return 1.f / dt; 
}

void Application::SetDt(float dt)
{
	this->dt = dt;
}

float Application::GetGameDt()
{
	return gameDt;
}

bool Application::GetIsRunning()
{
	return gameState == GameState::PLAYING;
}

bool Application::GetIsPaused()
{
	return gameState == GameState::PAUSED;
}

GameState Application::GetState()
{
	return gameState;
}

void Application::SetState(GameState gameState)
{
	this->gameState = gameState;
}

void Application::SaveConfigRequest() 
{ 
	isSavingConfig = true; 
}

void Application::LoadConfigRequest() 
{ 
	isLoadingConfig = true; 
}

void Application::AddModule(Module* mod)
{
	list_modules.push_back(mod);
}

void Application::SaveConfig()
{
	JSON_Value* root = configFile.GetRootValue();

	JsonParser application = configFile.SetChild(root, "App");

	application.SetNewJsonNumber(application.ValueToObject(application.GetRootValue()), "FPS", maxFrameRate);


	// Call SaveConfig() in all modules
	std::vector<Module*>::iterator item;

	for (item = list_modules.begin(); item != list_modules.end(); ++item)
	{
		(*item)->SaveConfig(configFile.SetChild(root, (*item)->name));
	}

	configFile.SerializeFile(root, configFileName);
	isSavingConfig = false;

	LOG("Configuration saved");
}


void Application::LoadConfig()
{
	JSON_Value* root = configFile.GetRootValue();

	JsonParser application = configFile.GetChild(root, "App");

	maxFrameRate = application.JsonValToNumber("FPS");


	std::vector<Module*>::iterator item;

	for (item = list_modules.begin(); item != list_modules.end(); ++item)
	{
		(*item)->LoadConfig(configFile.GetChild(root, (*item)->name));
	}

	isLoadingConfig = false;

	LOG("Configuration loaded");
}