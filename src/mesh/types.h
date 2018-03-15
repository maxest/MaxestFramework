#pragma once


#include "../essentials/stl.h"
#include "../math/types.h"
#include "../math/vector.h"


namespace NMaxestFramework { namespace NMesh
{
	struct SVertex
	{
		NMath::SVector3 position;
		NMath::SVector3 normal;
		NMath::SVector3 tangent;
		NMath::SVector3 bitangent;
		NMath::SVector2 texCoord0;
		NMath::SVector2 texCoord1;
		NMath::SVector3 color;

		void Reset()
		{
			position = NMath::VectorCustom(0.0f, 0.0f, 0.0f);
			normal = NMath::VectorCustom(0.0f, 0.0f, 0.0f);
			tangent = NMath::VectorCustom(0.0f, 0.0f, 0.0f);
			bitangent = NMath::VectorCustom(0.0f, 0.0f, 0.0f);
			texCoord0 = NMath::VectorCustom(0.0f, 0.0f);
			texCoord1 = NMath::VectorCustom(0.0f, 0.0f);
			color = NMath::VectorCustom(0.0f, 0.0f, 0.0f);
		}
	};

	struct SMaterial
	{
		string name;
		string textureFileName;
	};

	struct SChunk
	{
		int materialIndex;
		int lightmapIndex;

		bool hasNormals;
		bool hasTangents;
		bool hasTexCoords0;
		bool hasTexCoords1;
		bool hasColors;

		vector<SVertex> vertices;
		vector<int> indices;

		bool IsIndexed() { return indices.size() > 0; }
	};

	struct SMesh
	{
		int lightmapsCount;

		vector<SMaterial> materials;
		vector<SChunk> chunks;
	};

	enum class EMirrorType { NoMirror, AxisX, AxisY, AxisZ };
} }
