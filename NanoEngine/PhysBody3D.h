#ifndef __PhysBody3D_H__
#define __PhysBody3D_H__

#include <vector>

class btRigidBody;
class Module;

// =================================================
struct PhysBody3D
{
	friend class ModulePhysics;
public:
	PhysBody3D(btRigidBody* body);
	~PhysBody3D();

	void Push(float x, float y, float z);
	void GetTransform(float* matrix) const;
	void SetTransform(const float* matrix) const;
	void SetPos(float x, float y, float z);

	btRigidBody* body = nullptr;

public:
	std::vector <Module*> collision_listeners;
};

#endif // __PhysBody3D_H__