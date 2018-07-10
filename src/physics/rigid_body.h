#pragma once


#include "../math/main.h"


namespace NMaxestFramework { namespace NPhysics
{
	NMath::SMatrix BoxInertiaTensor(float mass, float width, float height, float depth)
	{
		NMath::SMatrix m;

		m.m[0][0] = 1.0f / 12.0f * mass * (height*height + depth*depth);
		m.m[0][1] = 0.0f;
		m.m[0][2] = 0.0f;
		m.m[0][3] = 0.0f;

		m.m[1][0] = 0.0f;
		m.m[1][1] = 1.0f / 12.0f * mass * (width*width + depth*depth);
		m.m[1][2] = 0.0f;
		m.m[1][3] = 0.0f;

		m.m[2][0] = 0.0f;
		m.m[2][1] = 0.0f;
		m.m[2][2] = 1.0f / 12.0f * mass * (width*width + height*height);
		m.m[2][3] = 0.0f;

		m.m[3][0] = 0.0f;
		m.m[3][1] = 0.0f;
		m.m[3][2] = 0.0f;
		m.m[3][3] = 1.0f;

		return m;
	}

	//

	class CRigidBody
	{
	public:
		CRigidBody()
		{
			mass = 1.0f;
			inertiaTensor_local = BoxInertiaTensor(mass, 1.0f, 1.0f, 1.0f);

			velocityDamping = 0.999f;

			position = NMath::cVector3Zero;
			linearVelocity = NMath::cVector3Zero;

			orientation = NMath::cQuaternionIdentity;
			angularVelocity = NMath::cVector3Zero;

			totalForce = NMath::cVector3Zero;
			totalTorque = NMath::cVector3Zero;
		}

		NMath::SMatrix LocalToWorld()
		{
			return NMath::QuaternionToMatrix(orientation) * NMath::MatrixTranslate(position);
		}

		void AddForce_World(const NMath::SVector3& force)
		{
			totalForce += force;
		}

		void AddForce_World(const NMath::SVector3& force, const NMath::SVector3& point_world)
		{
			totalForce += force;

			NMath::SVector3 r = point_world - position;
			NMath::SVector3 torque = Cross(r, force);
			totalTorque += torque;
		}

		void AddForce_Local(const NMath::SVector3& force, const NMath::SVector3& point_local)
		{
			NMath::SVector3 point_world = TransformPoint(point_local, LocalToWorld());
			AddForce_World(force, point_world);
		}

		void Integrate(float deltaTime)
		{
			NMath::SMatrix localToWorld = LocalToWorld();

			NMath::SVector3 linearAcceleration = totalForce / mass;
			linearVelocity = linearVelocity * NMath::Pow(velocityDamping, deltaTime) + linearAcceleration * deltaTime;
			position = position + linearVelocity * deltaTime;

			NMath::SVector3 angularAcceleration = totalTorque * Invert(localToWorld) * Invert(inertiaTensor_local) * localToWorld;
			angularVelocity = angularVelocity * NMath::Pow(velocityDamping, deltaTime) + angularAcceleration * deltaTime;
			orientation = Normalize(Add(orientation, angularVelocity));

			totalForce = NMath::cVector3Zero;
			totalTorque = NMath::cVector3Zero;
		}

	public:
		float mass;
		NMath::SMatrix inertiaTensor_local;

		float velocityDamping;

		NMath::SVector3 position;
		NMath::SVector3 linearVelocity;

		NMath::SQuaternion orientation;
		NMath::SVector3 angularVelocity;

		NMath::SVector3 totalForce;
		NMath::SVector3 totalTorque;
	};
} }
