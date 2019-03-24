#pragma once


#include "material.h"

#include "../../../src/math/main.h"
#include "../../../src/mesh/main.h"


using namespace NMaxestFramework;
using namespace NMath;


namespace NRayTracer
{
	class CScene
	{
	public:
		void AddMesh(const NMesh::SMesh& mesh, const SMatrix& transform, int materialIndex);

		bool IntersectionPrimary(const SVector3& rayStart, const SVector3& rayDir, float maxDistance, SSceneIntersectionResult& sceneIntersectionResult) const;
		bool IntersectionShadow(const SVector3& rayStart, const SVector3& rayDir, float maxDistance, int triangleIndex) const;

	public:
		vector<STrianglePrimitive> triangles;
		vector<SSpherePrimitive> spheres;
		vector<CMaterial> materials;
		vector<SDirLight> dirLights;
		vector<SPointLight> pointLights;

		float ambientConst;
		float ambientOcclusionFactor;
	};
}
