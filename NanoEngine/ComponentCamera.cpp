#include "Application.h"
#include "ComponentCamera.h"
#include "GameObject.h"
#include "ComponentTransform.h"
#include "ModuleRenderer3D.h"
#include "ModuleAssimpMeshes.h"

ComponentCamera::ComponentCamera() :Component(nullptr)
{
	name = "Camera component";
	type = ComponentType::CAMERA;
	mOwner = nullptr;

	fov = 60.0f;

	frustum.type = PerspectiveFrustum;
	frustum.nearPlaneDistance = 0.1f;
	frustum.farPlaneDistance = 100.f;
	frustum.front = float3::unitZ;
	frustum.up = float3::unitY;
	frustum.verticalFov = fov * DEGTORAD;
	frustum.horizontalFov = 2.0f * atanf(tanf(frustum.verticalFov / 2.0f) * 16 / 9);
	frustum.pos = float3(0, 0, 0);

	isActive = true;
	SetActiveCam(this);

	CreateFrameBuffer();
}

ComponentCamera::~ComponentCamera()
{
}

void ComponentCamera::PrintInspector()
{
	if (ImGui::CollapsingHeader("Camera"))
	{
		if (ImGui::Checkbox("Active", &isActive)) {
			SetActiveCam(this);
		}
		if (ImGui::SliderFloat("FOV", &fov, 10, 180)) {
			frustum.verticalFov = fov * DEGTORAD;
			frustum.horizontalFov = 2.0f * atanf(tanf(frustum.verticalFov / 2.0f) * 16 / 9);
		}
		ImGui::SliderFloat("Far Distance", &frustum.farPlaneDistance, 10, 1000);
		ImGui::SliderFloat("Near Distance", &frustum.nearPlaneDistance, 0.1f, 10);
		if (ImGui::Button("Set default")) {
			fov = 60.0f;
			frustum.verticalFov = fov * DEGTORAD;
			frustum.horizontalFov = 2.0f * atanf(tanf(frustum.verticalFov / 2.0f) * 16 / 9);
			frustum.farPlaneDistance = 100.f;
			frustum.nearPlaneDistance = 0.1f;
		}
	}
}

void ComponentCamera::Update()
{
	if (mOwner == nullptr) return;
	
	frustum.pos = mOwner->transform->getPosition();
	float4x4 m = mOwner->transform->getGlobalMatrix();
	frustum.up = m.RotatePart().Col(1).Normalized();
	frustum.front = m.RotatePart().Col(2).Normalized();

	RenderCameraFrustum();
}

void ComponentCamera::Look(const float3& Position, const float3& Reference)
{
	frustum.pos = Position;
	reference = Reference;

	LookAt(reference);
}

void ComponentCamera::LookAt(const float3& Spot)
{
	reference = Spot;
	frustum.front = (Spot - frustum.pos).Normalized();
	float3 X = float3(0, 1, 0).Cross(frustum.front).Normalized();
	frustum.up = frustum.front.Cross(X);
}

void ComponentCamera::Move(const float3& Movement)
{
	frustum.pos += Movement;
}

void ComponentCamera::CreateFrameBuffer()
{
	glGenFramebuffers(1, &frameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

	glGenTextures(1, &cameraBuffer);
	glBindTexture(GL_TEXTURE_2D, cameraBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

	float color[4] = { 0.0f,0.0f,0.0f,0.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, cameraBuffer, 0);

	glGenRenderbuffers(1, &renderBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCREEN_WIDTH, SCREEN_HEIGHT);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderBuffer);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		LOG("Framebuffer is not complete!");
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	SetActiveCam(this);
}

void ComponentCamera::SetActiveCam(ComponentCamera* cam)
{
	if (isActive) App->renderer3D->activeCam = cam;
	else App->renderer3D->activeCam = nullptr;
}

float* ComponentCamera::GetViewMatrix()
{
	viewMatrix = frustum.ViewMatrix();
	viewMatrix.Transpose();
	return viewMatrix.ptr();
}

float* ComponentCamera::GetProjetionMatrix()
{
	projectionMatrix = frustum.ProjectionMatrix();
	projectionMatrix.Transpose();
	return projectionMatrix.ptr();
}

void ComponentCamera::RenderCameraFrustum() {
	float3 corners[8];
	frustum.GetCornerPoints(corners);

	// Assuming you have a function similar to DrawBB to render lines
	DrawFrustumEdges(corners, float3(1.0f, 1.0f, 1.0f));
}

void ComponentCamera::DrawFrustumEdges(float3* corners, float3 color) {
	// Define indices to draw lines between corners
	int indices[24] = { 0,1, 1,3, 3,2, 2,0, 4,5, 5,7, 7,6, 6,4, 0,4, 1,5, 2,6, 3,7 };

	glLineWidth(2.0f);
	glBegin(GL_LINES);
	glColor3fv(color.ptr());

	for (size_t i = 0; i < 24; i++) {
		glVertex3fv(corners[indices[i]].ptr());
	}
	glEnd();
}

bool ComponentCamera::ObjectInsideFrustrum(Mesh* mesh)
{
	float3 boxPoints[8];
	Plane frustumPlanes[6];

	mesh->Global_AABB.GetCornerPoints(boxPoints);
	frustum.GetPlanes(frustumPlanes);

	for (size_t i = 0; i < 6; i++)
	{
		int p = 0;
		for (size_t j = 0; j < 8; j++)
		{
			if (frustumPlanes[i].IsOnPositiveSide(boxPoints[j]))
				p++;
		}
		if (p == 8) {
			return false;
		}
	}
	return true;
}