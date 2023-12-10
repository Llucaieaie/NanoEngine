#include "ModuleScene.h"
#include "GameObject.h"
#include "Application.h"
#include "OurPrimitive.h"
#include "ModuleAssimpMeshes.h"
#include "ComponentCamera.h"
#include "ComponentTransform.h"

ModuleScene::ModuleScene(Application* app, bool start_enabled) : Module(app, start_enabled) 
{
}

ModuleScene::~ModuleScene() 
{
}

bool ModuleScene::Init()
{
    return false;
}

bool ModuleScene::Start() {
    
    root = new GameObject(nullptr);
    root->name = ("Scene");

    DefaultCamera();

    //Load Baker House
    bakerHouse = App->assimpMeshes->LoadMeshFromFile("Assets/Models/BakerHouse.fbx");
    return true;
}

update_status ModuleScene::PreUpdate(float dt) {
    
    return UPDATE_CONTINUE;
}

update_status ModuleScene::Update(float dt) {

    //cameraObj->transform->setPosition(cameraObj->transform->getPosition() + float3(.01f * App->gameDt, .01f * App->gameDt, .01f * App->gameDt));
    
    return UPDATE_CONTINUE;
}

update_status ModuleScene::PostUpdate(float dt) {
    
    return UPDATE_CONTINUE;
}

bool ModuleScene::CleanUp() {
    
    LOG("Scene Cleanup")
    delete root;
    return true;
}

GameObject* ModuleScene::CreateGameObject(GameObject* parent)
{
    GameObject* newGameObject = new GameObject(parent);

    return newGameObject;
}

void ModuleScene::DefaultCamera()
{
    cameraObj = new GameObject(root);//PrimitivesGeomtriesLibrary::InstanciatePrimitiveGeometry(GeometryType::EMPTY);
    cam = new ComponentCamera();
    cameraObj->AddComponent(cam);
    cameraObj->name = "Main Camera";
    cameraObj->transform->setPosition(float3(0, 2, -5));
    //cameraObj->transform->calculateMatrix();
}

void ModuleScene::SetGameObjectSelected(GameObject* gameObject)
{
    App->hierarchy->objSelected = gameObject;
}