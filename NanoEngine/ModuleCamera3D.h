#pragma once
#include "Module.h"
#include "Globals.h"
#include "MathGeoLib/include/Math/float3.h"
#include "MathGeoLib/include/Math/float4x4.h"

class ComponentCamera;

enum class CamStates {

	NORMAL,
	FLYING,
	FOCUSED,
};

class ModuleCamera3D : public Module
{
public:
	ModuleCamera3D(Application* app, bool start_enabled = true);
	~ModuleCamera3D();

	bool Start();
	update_status Update(float dt);
	bool CleanUp();

	void Look(const float3& Position, const float3& Reference, bool RotateAroundReference = false);
	void LookAt(const float3& Spot);
	void Move(const float3& Movement);
	float* GetViewMatrix();
	void FocusCameraToSelectedObject();
	void OrbitSelectedObject(float dt);
	void RotationAroundCamera(float dx, float dy, float sensitivity);
	float3 RotateVector(const float3& u, float angle, const float3& v);
	float3 SelectedObjectPos();
private:

	void CalculateViewMatrix();

public:
	
	float3 X, Y, Z, Position, Reference;

	ComponentCamera* camera;
	CamStates camState;

private:

	float4x4 ViewMatrix;
	
};