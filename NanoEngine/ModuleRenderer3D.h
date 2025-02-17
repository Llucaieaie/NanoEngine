#pragma once
#include "Module.h"
#include "Globals.h"
#include "Light.h"
#include "GameObject.h"
#include "ComponentCamera.h"

#include "Primitive.h"
#include "MathGeoLib/include/MathGeoLib.h"
#include "Glew/include/glew.h"
#include "ImGui/imgui.h"

#define MAX_LIGHTS 8

class ComponentCamera;

class ModuleRenderer3D : public Module
{
public:
	ModuleRenderer3D(Application* app, bool start_enabled = true);
	~ModuleRenderer3D();

	bool Init();
	bool Start() override;
	update_status PreUpdate(float dt);
	update_status PostUpdate(float dt);
	void DirectModeTriangleDrawing();
	bool CleanUp();

	void OnResize(int width, int height);

	// Setter functions for renderer attributes
	void SetDepthTestAttribute(bool enable);
	void SetCullFaceAttribute(bool enable);
	void SetLightingAttribute(bool enable);
	void SetColorMaterialAttribute(bool enable);
	void SetTexture2DAttribute(bool enable);
	void SetBlendAttribute(bool enable);
	void SetAlphaTestAttribute(bool enable);
	void SetLineSmoothAttribute(bool enable);
	void SetPointSmoothAttribute(bool enable);
	void SetPolygonSmoothAttribute(bool enable);
	void SetVsync(bool vsync);

	//Getter functions for renderer attributes
	bool GetDepthTestAttribute() const { return depthTestAttribute; }
	bool GetCullFaceAttribute() const { return cullFaceAttribute; }
	bool GetLightingAttribute() const { return lightingAttribute; }
	bool GetColorMaterialAttribute() const { return colorMaterialAttribute; }
	bool GetTexture2DAttribute() const { return texture2DAttribute; }
	bool GetBlendAttribute() const { return blendAttribute; }
	bool GetAlphaTestAttribute() const { return alphaTestAttribute; }
	bool GetLineSmoothAttribute() const { return lineSmoothAttribute; }
	bool GetPointSmoothAttribute() const { return pointSmoothAttribute; }
	bool GetPolygonSmoothAttribute() const { return polygonSmoothAttribute; }
	bool GetVsync() const { return vsync; }

	void SetWireframeMode(bool enable);
	void DrawWithWireframe();

	void DrawLine(float3 a, float3 b);

public:

	Light lights[MAX_LIGHTS];
	SDL_GLContext context;
	PrimPlane Grid;
	
	float3x3 NormalMatrix;
	float4x4 ModelMatrix;
	float4x4 ViewMatrix;
	float4x4 ProjectionMatrix;

	unsigned int cameraBuffer;
	unsigned int inGameBuffer;
	unsigned int frameBuffer;
	unsigned int renderObjBuffer;

	GLuint test;
	GLuint VBO;
	GLuint EBO;
	GLuint VAO;

	//test rectangulo
	GLuint VAORect;
	GLuint VBORect;
	GLuint EBORect;
	
	ComponentCamera* activeCam;

	LineSegment ray;

private:
	//renderer attributes
	bool vsync;
	bool depthTestAttribute;
	bool cullFaceAttribute;
	bool lightingAttribute;
	bool colorMaterialAttribute;
	bool texture2DAttribute;
	bool blendAttribute;
	bool alphaTestAttribute;
	bool lineSmoothAttribute;
	bool pointSmoothAttribute;
	bool polygonSmoothAttribute;

	//renderer mode
	bool wireframeMode;
};