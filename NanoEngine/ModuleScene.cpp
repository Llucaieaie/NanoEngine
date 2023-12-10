#include "ModuleScene.h"
#include "GameObject.h"
#include "Application.h"
#include "OurPrimitive.h"
#include "ModuleAssimpMeshes.h"
#include "ComponentCamera.h"
#include "ComponentTransform.h"
#include "ComponentMaterial.h"
#include "ComponentMesh.h"

ModuleScene::ModuleScene(Application* app, bool start_enabled) : Module(app, start_enabled) 
{
    name = "Scene";
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
    
    if (App->hierarchy->objSelected != nullptr)
        bakerHouse->transform->DrawGuizmo(App->hierarchy->objSelected, cam->GetViewMatrix(), cam->GetProjetionMatrix());

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

bool ModuleScene::SaveConfig(JsonParser& json)
{
    LOG("Scene children: %i",root->mChildren.size());
    for (int i = 0; i < root->mChildren.size(); i++)
    {
        objNode = json.SetChild(json.GetRootValue(), root->mChildren[i]->name.c_str());

        if (root->mChildren[i]->mComponents.size() != 0)
        {
            for (int j = 0; j < root->mChildren[i]->mComponents.size(); j++)
            {
                JsonParser compNode = objNode.SetChild(objNode.GetRootValue(), root->mChildren[i]->mComponents[j]->name);

                if (root->mChildren[i]->mComponents[j]->type == ComponentType::TRANSFORM)
                {                    
                    compNode.SetNewJsonNumber(compNode.ValueToObject(compNode.GetRootValue()), "pos.x", root->mChildren[i]->GetTransformComponent()->getPosition().x);
                    compNode.SetNewJsonNumber(compNode.ValueToObject(compNode.GetRootValue()), "pos.y", root->mChildren[i]->GetTransformComponent()->getPosition().y);
                    compNode.SetNewJsonNumber(compNode.ValueToObject(compNode.GetRootValue()), "pos.z", root->mChildren[i]->GetTransformComponent()->getPosition().z);

                    compNode.SetNewJsonNumber(compNode.ValueToObject(compNode.GetRootValue()), "rot.x", root->mChildren[i]->GetTransformComponent()->getRotation().x);
                    compNode.SetNewJsonNumber(compNode.ValueToObject(compNode.GetRootValue()), "rot.y", root->mChildren[i]->GetTransformComponent()->getRotation().y);
                    compNode.SetNewJsonNumber(compNode.ValueToObject(compNode.GetRootValue()), "rot.z", root->mChildren[i]->GetTransformComponent()->getRotation().z);

                    compNode.SetNewJsonNumber(compNode.ValueToObject(compNode.GetRootValue()), "scale.x", root->mChildren[i]->GetTransformComponent()->getScale().x);
                    compNode.SetNewJsonNumber(compNode.ValueToObject(compNode.GetRootValue()), "scale.y", root->mChildren[i]->GetTransformComponent()->getScale().y);
                    compNode.SetNewJsonNumber(compNode.ValueToObject(compNode.GetRootValue()), "scale.z", root->mChildren[i]->GetTransformComponent()->getScale().z);
                }

                if (root->mChildren[i]->mComponents[j]->type == ComponentType::CAMERA)
                {
                    compNode.SetNewJsonBool(compNode.ValueToObject(compNode.GetRootValue()), "Active", root->mChildren[i]->GetComponentCamera()->isActive);
                    compNode.SetNewJsonNumber(compNode.ValueToObject(compNode.GetRootValue()), "FOV", root->mChildren[i]->GetComponentCamera()->fov);
                    compNode.SetNewJsonNumber(compNode.ValueToObject(compNode.GetRootValue()), "Far Distance", root->mChildren[i]->GetComponentCamera()->frustum.farPlaneDistance);
                    compNode.SetNewJsonNumber(compNode.ValueToObject(compNode.GetRootValue()), "Near Distance", root->mChildren[i]->GetComponentCamera()->frustum.nearPlaneDistance);
                }

                if (root->mChildren[i]->mComponents[j]->type == ComponentType::MATERIAL)
                {
                    compNode.SetNewJsonBool(compNode.ValueToObject(compNode.GetRootValue()), "Active", root->mChildren[i]->GetComponentTexture()->isActive);
                    compNode.SetNewJsonNumber(compNode.ValueToObject(compNode.GetRootValue()), "ID", root->mChildren[i]->GetComponentTexture()->textureID);
                    compNode.SetNewJsonString(compNode.ValueToObject(compNode.GetRootValue()), "Texture Path", root->mChildren[i]->GetComponentTexture()->pathTexture.c_str());
                }

                if (root->mChildren[i]->mComponents[j]->type == ComponentType::MESH)
                {
                    compNode.SetNewJsonBool(compNode.ValueToObject(compNode.GetRootValue()), "Active", root->mChildren[i]->GetMeshComponent()->isActive);
                    compNode.SetNewJsonBool(compNode.ValueToObject(compNode.GetRootValue()), "Show normals", root->mChildren[i]->GetMeshComponent()->faceNormals);
                }
            }
        }
    }

    return true;
}

bool ModuleScene::LoadConfig(JsonParser& json)
{
    //for (int i = 0; i < root->mChildren.size(); i++)
    //{
    //    for (int j = 0; j < root->mChildren[i]->mComponents.size(); j++)
    //    {
    //        if (root->mChildren[i]->mComponents[j]->type == ComponentType::TRANSFORM)
    //        {
    //            float3 pos = { (float)json.JsonValToNumber("pos.x"),
    //                (float)json.JsonValToNumber("pos.y"),
    //                (float)json.JsonValToNumber("pos.z")
    //            };
    //            LOG("position: %d, %d, %d", compNode.JsonValToNumber("pos.x"), compNode.JsonValToNumber("pos.y"), compNode.JsonValToNumber("pos.z"));
    //            JSON_Value* test = json.GetChild(json.GetRootValue(), root->mChildren[i]->name.c_str()).GetRootValue();
    //            LOG("positiony: %d", json.GetChild(json.GetRootValue(), root->mChildren[i]->name.c_str()).GetChild(test, root->mChildren[i]->mComponents[j]->name));

    //            root->mChildren[i]->transform->setPosition(pos);

    //        }
    //    }
    //}
    return true;
}
