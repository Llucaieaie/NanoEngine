#include "Globals.h"
#include "Application.h"
#include "ModuleCamera3D.h"
#include "ComponentTransform.h"
#include "ComponentCamera.h"

#include "MathGeoLib/include/Math/Quat.h"
#include "MathGeoLib/include/Math/float3.h"
#include "MathGeoLib/include/Math/float3x3.h"

ModuleCamera3D::ModuleCamera3D(Application* app, bool start_enabled) : Module(app, start_enabled)
{
	camState = CamStates::NORMAL;

	X = float3(1.0f, 0.0f, 0.0f);
	Y = float3(0.0f, 1.0f, 0.0f);
	Z = float3(0.0f, 0.0f, 1.0f);

	Position = float3(0.0f, 10.0f, 5.0f);
	Reference = float3(0.0f, 0.0f, 0.0f);
}

ModuleCamera3D::~ModuleCamera3D()
{}

// -----------------------------------------------------------------
bool ModuleCamera3D::Start()
{
	LOG("Setting up the camera");
	bool ret = true;

	camera = new ComponentCamera();
	camera->frustum.pos = float3(5, 5, 5);
	camera->LookAt(float3(0, 0, 0));

	return ret;
}

// -----------------------------------------------------------------
bool ModuleCamera3D::CleanUp()
{
	LOG("Cleaning camera");
	delete camera;

	return true;
}

// -----------------------------------------------------------------
update_status ModuleCamera3D::Update(float dt)
{
	float3 newPos(0, 0, 0);
	Quat direction = Quat::identity;

	float sensitivity = 25.f * dt;

	float speed = 6.0f * dt;
	if (App->input->GetKey(SDL_SCANCODE_LSHIFT) == KEY_REPEAT)
		speed = 15.0f * dt;

	int dx = -App->input->GetMouseXMotion();
	int dy = -App->input->GetMouseYMotion();

	int wheel = -App->input->GetMouseZ();

	if (wheel != 0)
		camera->frustum.pos += camera->frustum.front * 100 * -wheel * dt;


	if (App->input->GetKey(SDL_SCANCODE_LALT) == KEY_REPEAT && App->input->GetMouseButton(SDL_BUTTON_LEFT) == KEY_REPEAT) {
		camState = CamStates::FOCUSED;
	}
	else if (App->input->GetMouseButton(SDL_BUTTON_RIGHT) == KEY_REPEAT) {
		camState = CamStates::FLYING;
	}
	else {
		camState = CamStates::NORMAL;
	}

	//if (App->input->GetKey(SDL_SCANCODE_W) == KEY_REPEAT) camera->FrustumCam.pos += camera->FrustumCam.front * speed;
	//if (App->input->GetKey(SDL_SCANCODE_S) == KEY_REPEAT) camera->FrustumCam.pos -= camera->FrustumCam.front * speed;

	//if (App->input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT) camera->FrustumCam.pos -= camera->FrustumCam.WorldRight() * speed;
	//if (App->input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT) camera->FrustumCam.pos += camera->FrustumCam.WorldRight() * speed;

	//if (App->input->GetKey(SDL_SCANCODE_SPACE) == KEY_REPEAT) camera->FrustumCam.pos.y += speed;
	//if (App->input->GetKey(SDL_SCANCODE_LCTRL) == KEY_REPEAT) camera->FrustumCam.pos.y -= speed;

	//if (App->input->GetKey(SDL_SCANCODE_F) == KEY_REPEAT) FocusCameraToSelectedObject();

	if (App->input->GetKey(SDL_SCANCODE_F) == KEY_DOWN)
	{
		if (App->hierarchy->objSelected != nullptr) {
			float3 target = App->hierarchy->objSelected->transform->getPosition();

			camera->LookAt(target);
		}
	}

	switch (camState) {
		case CamStates::FLYING:
		{
			if (App->input->GetKey(SDL_SCANCODE_W) == KEY_REPEAT) camera->frustum.pos += camera->frustum.front * speed;
			if (App->input->GetKey(SDL_SCANCODE_S) == KEY_REPEAT) camera->frustum.pos -= camera->frustum.front * speed;
			if (App->input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT) camera->frustum.pos -= camera->frustum.WorldRight() * speed;
			if (App->input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT) camera->frustum.pos += camera->frustum.WorldRight() * speed;
			if (App->input->GetKey(SDL_SCANCODE_SPACE) == KEY_REPEAT) camera->frustum.pos.y += speed;
			if (App->input->GetKey(SDL_SCANCODE_LCTRL) == KEY_REPEAT) camera->frustum.pos.y -= speed;

			RotationAroundCamera(dx, dy, sensitivity);
			break;
		}
		case CamStates::FOCUSED:
		{
			camera->LookAt(SelectedObjectPos());
			float len = float3(camera->reference - camera->frustum.pos).Length();

			RotationAroundCamera(dx, dy, sensitivity);

			camera->frustum.pos = camera->reference + (camera->frustum.front * -len);
			break;
		}
		case CamStates::NORMAL:
		{
			if (App->input->GetMouseButton(SDL_BUTTON_MIDDLE) == KEY_REPEAT) {
				camera->frustum.pos += camera->frustum.WorldRight() * (speed * dx / 2);
				camera->frustum.pos -= camera->frustum.up * (speed * dy / 2);
			}
			break;
		}
	}
	//OrbitSelectedObject(dt);

	//LookAt(Reference);

	// Recalculate matrix -------------
	//CalculateViewMatrix();

	return UPDATE_CONTINUE;
}

// -----------------------------------------------------------------
void ModuleCamera3D::Look(const float3& Position, const float3& Reference, bool RotateAroundReference)
{
	this->Position = Position;
	this->Reference = Reference;

	Z = (Position - Reference).Normalized();
	X = (float3(0.0f, 1.0f, 0.0f).Cross(Z)).Normalized();
	Y = Z.Cross(X);

	if (!RotateAroundReference)
	{
		this->Reference = this->Position;
		this->Position += Z * 0.05f;
	}

	CalculateViewMatrix();
}

// -----------------------------------------------------------------
void ModuleCamera3D::LookAt(const float3& Spot)
{
	Reference = Spot;

	Z = (Position - Reference).Normalized();
	X = (float3(0.0f, 1.0f, 0.0f).Cross(Z)).Normalized();
	Y = Z.Cross(X);

	CalculateViewMatrix();
}


// -----------------------------------------------------------------
void ModuleCamera3D::Move(const float3& Movement)
{
	Position += Movement;
	Reference += Movement;

	CalculateViewMatrix();
}

// -----------------------------------------------------------------
float* ModuleCamera3D::GetViewMatrix()
{
	return ViewMatrix.ptr();
}

void ModuleCamera3D::FocusCameraToSelectedObject()
{

	float3 focusObjectPosition;
	//Get the GameObject selected in hierarchy
	if (App->hierarchy->objSelected != nullptr)
	{
		focusObjectPosition = App->hierarchy->objSelected->transform->getPosition();
		LookAt(focusObjectPosition);
	}

}

void ModuleCamera3D::OrbitSelectedObject(float dt)
{
	float3 pivot = float3(0, 0, 0);
	GameObject* gameObject = App->hierarchy->objSelected;

	float3 posGO = { 0, 0, 0 };

	if (gameObject != nullptr)posGO = gameObject->transform->getPosition();

	if (App->input->GetMouseButton(SDL_BUTTON_LEFT) == KEY_REPEAT)
	{
		int dx = -App->input->GetMouseXMotion();
		int dy = -App->input->GetMouseYMotion();
		float Sensitivity = 0.5f * dt;

		if (App->input->GetKey(SDL_SCANCODE_LALT) == KEY_REPEAT)
		{

			if (gameObject != nullptr)
			{
				pivot = float3(posGO.x, posGO.y, posGO.z);
			}
			else
			{
				return;
			}
		}
		else
		{
			return;
		}

		Position -= pivot;

		if (dx != 0)
		{
			float DeltaX = (float)dx * Sensitivity;

			X = RotateVector(X, DeltaX, float3(0.0f, 1.0f, 0.0f));
			Y = RotateVector(Y, DeltaX, float3(0.0f, 1.0f, 0.0f));
			Z = RotateVector(Z, DeltaX, float3(0.0f, 1.0f, 0.0f));
		}

		if (dy != 0)
		{
			float DeltaY = (float)dy * Sensitivity;

			Y = RotateVector(Y, DeltaY, X);
			Z = RotateVector(Z, DeltaY, X);

			if (Y.y < 0.0f)
			{
				Z = float3(0.0f, Z.y > 0.0f ? 1.0f : -1.0f, 0.0f);
				Y = Cross(Z, X);

			}
		}
		Position = pivot + Z * Length(Position);
		Reference = pivot;

	}

}

void ModuleCamera3D::RotationAroundCamera(float dx, float dy, float sensitivity)
{
	//Rotation
	Quat dir;
	camera->frustum.WorldMatrix().Decompose(float3(), dir, float3());

	//Mouse look direction
	if (dx != 0)
	{
		float DeltaX = (float)dx * sensitivity;
		Quat X = Quat::identity;
		X.SetFromAxisAngle(float3(0.0f, 1.0f, 0.0f), DeltaX * DEGTORAD);
		dir = X * dir;
	}

	if (dy != 0)
	{
		float DeltaY = (float)dy * sensitivity;
		Quat Y = Quat::identity;
		Y.SetFromAxisAngle(float3(1.0f, 0.0f, 0.0f), DeltaY * DEGTORAD);
		dir = dir * Y;
	}

	//Set direction
	float4x4 rm = camera->frustum.WorldMatrix();
	rm.SetRotatePart(dir.Normalized());
	camera->frustum.SetWorldMatrix(rm.Float3x4Part());
}

float3 ModuleCamera3D::SelectedObjectPos()
{
	float3 SelectedObject = { 0,0,0 };
	if (App->hierarchy->objSelected != nullptr) {
		SelectedObject = App->hierarchy->objSelected->transform->getPosition(true);
	}
	return SelectedObject;
}

float3 ModuleCamera3D::RotateVector(const float3& u, float angle, const float3& v)
{
	// Crear un cuaterni�n de rotaci�n a partir del eje y el �ngulo
	Quat rotationQuat = Quat::RotateAxisAngle(v, angle);

	// Aplicar la rotaci�n al vector
	return rotationQuat * u;
}

// -----------------------------------------------------------------
void ModuleCamera3D::CalculateViewMatrix()
{
	//todo: USE MATHGEOLIB here BEFORE 1st delivery! (TIP: Use MathGeoLib/Geometry/Frustum.h, view and projection matrices are managed internally.)
	ViewMatrix = float4x4(X.x, Y.x, Z.x, 0.0f, X.y, Y.y, Z.y, 0.0f, X.z, Y.z, Z.z, 0.0f, -(X.Dot(Position)), -(Y.Dot(Position)), -(Z.Dot(Position)), 1.0f);
}