#pragma once
#include "Component.h"
#include "MathGeoLib/include/MathGeoLib.h"

struct Mesh;

class ComponentCamera : public Component
{
public:
	ComponentCamera();
	ComponentCamera(GameObject owner);
	~ComponentCamera();

	void PrintInspector();
	void Update() override;
	void OnEditor();

	void Look(const float3& Position, const float3& Reference);
	void LookAt(const float3& Spot);
	void Move(const float3& Movement);
	void RotationAroundCamera(float dx, float dy, float sensitivity);
	void CreateFrameBuffer();
	void SetActiveCam(ComponentCamera* cam);

	float* GetViewMatrix();
	float* GetProjetionMatrix();

	void RenderCameraFrustum();
	void DrawFrustumEdges(float3* corners, float3 color);
	bool ObjectInsideFrustrum(Mesh* mesh);

	Frustum frustum;
	float3 reference;

	float fov;
	uint frameBuffer, cameraBuffer, renderBuffer;

	bool isActive;
	bool isCulling;

private:
	float4x4 viewMatrix;
	float4x4 projectionMatrix;
};