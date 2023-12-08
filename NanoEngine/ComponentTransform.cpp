#include "ComponentTransform.h"
#include "Globals.h"
#include "GameObject.h"

ComponentTransform::ComponentTransform(GameObject* owner) : Component(owner) {
	type = ComponentType::TRANSFORM;
	mOwner = owner;
	resetMatrix();
}

ComponentTransform::~ComponentTransform()
{
}



float4x4 ComponentTransform::getGlobalMatrix()
{
	if (mOwner->GetParent() == nullptr) return getLocalMatrix();

	return  matrix * mOwner->GetParent()->transform->getGlobalMatrix();
}

float4x4 ComponentTransform::getLocalMatrix()
{
	return matrix;
}

void ComponentTransform::resetMatrix()
{
	matrix = { 1, 0, 0, 0,
			   0, 1, 0, 0,
			   0, 0, 1, 0,
				0, 0, 0, 1 };

	position = { 0, 0, 0 };
	rotation = { 0, 0, 0 };
	scale = { 1, 1, 1 };
}

float3 ComponentTransform::getPosition(bool globalPosition)
{
	if (!globalPosition)return float3(position);

	float4x4 m = getGlobalMatrix();
	return m.Row3(3);
}

void ComponentTransform::setPosition(float3 pos)
{
	position = pos;
	calculateMatrix();
}

float3 ComponentTransform::getRotation()
{
	return rotation;
}

void ComponentTransform::setRotation(float3 rot)
{
	rotation = rot;
	calculateMatrix();
}

float3 ComponentTransform::getScale()
{
	return scale;
}

void ComponentTransform::setScale(float3 sca)
{
	scale = sca;
	calculateMatrix();
}



void ComponentTransform::calculateMatrix()
{
	float rx = rotation.x * DEGTORAD;
	float ry = rotation.y * DEGTORAD;
	float rz = rotation.z * DEGTORAD;

	Quat q;
	q = Quat::FromEulerXYZ(rx, ry, rz);
	matrix = float4x4::FromTRS(position, q, scale).Transposed();
}

void ComponentTransform:: PrintInspector() {

	if (ImGui::CollapsingHeader("Transform"))
	{
		ImGui::Text("X\t\t Y\t\t Z");
		ImGui::DragFloat3("Position", position.ptr());

		ImGui::Text("X\t\t Y\t\t Z");
		ImGui::DragFloat3("Rotation", rotation.ptr());

		ImGui::Text("X\t\t Y\t\t Z");
		ImGui::DragFloat3("Scale", scale.ptr());
	}
	calculateMatrix();
}

void ComponentTransform::DrawGuizmo(GameObject* obj, float* view, float* projection)
{
	//ImGuizmo::BeginFrame();
	//ImGuizmo::Enable(true);

	//ComponentTransform* transform = obj->GetTransformComponent();
	//ImVec2 cornerPos = ImGui::GetWindowPos();
	//ImVec2 size = ImGui::GetContentRegionMax();

	//int offset = ImGui::GetFrameHeight() / 2;
	//ImGuizmo::SetRect(cornerPos.x, cornerPos.y + offset, size.x, size.y);
	//ImGuizmo::SetDrawlist();
	////if (ImGuizmo::Manipulate(App->camera->viewMatrix.Transposed().ptr(), camera->frustum.ProjectionMatrix().Transposed().ptr(), operation, mode, transform->getGlobalMatrix().ptr()))
	//if (ImGuizmo::Manipulate(view, projection, operation, mode, transform->getGlobalMatrix().ptr()))
	//{

	//}
	//ImGui::EndFrame();
}