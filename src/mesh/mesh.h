#pragma once


#include "types.h"
#include "../essentials/stl.h"


namespace NMaxestFramework { namespace NMesh
{
	bool ImportASE(const string& path, SMesh& mesh);
	bool ImportOBJ(const string& path, SMesh& mesh);
	bool ImportUnityExported(const string& path, SMesh& mesh);
	void ToIndexed(const vector<SVertex>& vertices, vector<SVertex>& indexedVertices, vector<int>& indices);
	void ToIndexed(SChunk& chunk);
	void ToIndexed(SMesh& mesh);
	void FlipVectors(vector<SVertex>& vertices, bool normals, bool tangents, bool bitangents);
	void GenerateTangents(vector<SVertex>& vertices);
	void AverageTangents(vector<SVertex>& vertices, float minTangentsPairDotProduct = 0.5f, EMirrorType mirrorType = EMirrorType::NoMirror); // averages if positions and normals of vertices are the same and the dot product of tangents is greater or equal to minTangentsPairDotProduct
	void OrthogonalizeTangents(vector<SVertex>& vertices);
	bool Equal(const SVertex& v1, const SVertex& v2);
} }
