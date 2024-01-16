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

void ComponentTransform::SetTransformFromMatrix(const float* matrix)
{
	position = float3(matrix[12], matrix[13], matrix[14]);

	float3 extractedScale;
	extractedScale.x = float3(matrix[0], matrix[1], matrix[2]).Length();
	extractedScale.y = float3(matrix[4], matrix[5], matrix[6]).Length();
	extractedScale.z = float3(matrix[8], matrix[9], matrix[10]).Length();
	scale = extractedScale;

	rotation.x = atan2(matrix[6], matrix[10]) * RADTODEG;
	rotation.y = atan2(-matrix[2], sqrt(matrix[1] * matrix[1] + matrix[0] * matrix[0])) * RADTODEG;
	rotation.z = atan2(matrix[1], matrix[0]) * RADTODEG;

	// Recalcular la matriz local
	calculateMatrix();
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