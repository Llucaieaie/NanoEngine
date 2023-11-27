#pragma once
#include "Globals.h"
#include "Component.h"
#include "ImGui/imgui.h"
#include "MathGeoLib/include/MathGeoLib.h"

class GameObject;
class Component;

class ComponentCamera : public Component
{
public:

	ComponentCamera();
	ComponentCamera(GameObject* owner);
	~ComponentCamera();

	void SetCam();
	void GenBuffer();

	float* GetViewMatrix();
	float* GetProjectionMatrix();

	void LookAt(const float3& target);

	void TransformCam();

	void PrintInspector();

	Frustum FrustumCam;
	float4x4 viewMatrix;
	float4x4 projectionMatrix;

	unsigned int cameraBuffer;
	unsigned int frameBuffer;

private:

	unsigned int renderObjBuffer;
};