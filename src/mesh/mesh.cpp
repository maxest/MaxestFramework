#include "mesh.h"
#include "../math/main.h"


using namespace NMaxestFramework;
using namespace NMath;


void NMesh::ToIndexed(const vector<SVertex>& vertices, vector<SVertex>& indexedVertices, vector<int>& indices)
{
	MF_ASSERT(indexedVertices.size() == 0);
	MF_ASSERT(indices.size() == 0);

	vector<bool> checkedVertices;
	for (uint i = 0; i < vertices.size(); i++)
		checkedVertices.push_back(false);

	indices.resize(vertices.size());

	for (uint i = 0; i < vertices.size(); i++)
	{
		if (!checkedVertices[i])
		{
			checkedVertices[i] = true;
			indexedVertices.push_back(vertices[i]);
			indices[i] = (int)indexedVertices.size() - 1;

			for (uint j = i + 1; j < vertices.size(); j++)
			{
				if (!checkedVertices[j])
				{
					if (Equal(vertices[i], vertices[j]))
					{
						checkedVertices[j] = true;
						indices[j] = indices[i];
					}
				}
			}
		}
	}
}


void NMesh::ToIndexed(SChunk& chunk)
{
	vector<SVertex> indexedVertices;
	ToIndexed(chunk.vertices, indexedVertices, chunk.indices);
	chunk.vertices.resize(indexedVertices.size());
	copy(indexedVertices.begin(), indexedVertices.end(), chunk.vertices.begin());
}


void NMesh::ToIndexed(SMesh& mesh)
{
	for (uint i = 0; i < mesh.chunks.size(); i++)
		ToIndexed(mesh.chunks[i]);
}


void NMesh::FlipVectors(vector<SVertex>& vertices, bool normals, bool tangents, bool bitangents)
{
	for (uint i = 0; i < vertices.size(); i++)
	{
		if (normals)
			vertices[i].normal *= -1.0f;
		if (tangents)
			vertices[i].tangent *= -1.0f;
		if (bitangents)
			vertices[i].bitangent *= -1.0f;
	}
}


void NMesh::GenerateTangents(vector<SVertex>& vertices)
{
	for (uint i = 0; i < vertices.size() / 3; i++)
	{
		SVector3 tangent, bitangent, normal;

		TriangleTangentBasis(
			vertices[3*i + 0].position, vertices[3*i + 0].texCoord0,
			vertices[3*i + 1].position, vertices[3*i + 1].texCoord0,
			vertices[3*i + 2].position, vertices[3*i + 2].texCoord0,
			tangent, bitangent, normal);

		NormalizeIn(tangent);
		NormalizeIn(bitangent);

		for (int j = 0; j < 3; j++)
		{
			vertices[3*i + j].tangent = tangent;
			vertices[3*i + j].bitangent = -bitangent;
		}
	}
}


void NMesh::AverageTangents(vector<SVertex>& vertices, float minTangentsPairDotProduct, EMirrorType mirrorType)
{
	SVector3 *tempTangents = new SVector3[vertices.size()];
	SVector3 *tempBitangents = new SVector3[vertices.size()];
	for (uint i = 0; i < vertices.size(); i++)
	{
		tempTangents[i] = vertices[i].tangent;
		tempBitangents[i] = vertices[i].bitangent;
	}

	SVector3 *trianglesCenterPoints = nullptr;
	if (mirrorType != EMirrorType::NoMirror)
	{
		trianglesCenterPoints = new SVector3[vertices.size() / 3];

		for (uint i = 0; i < vertices.size() / 3; i++)
			trianglesCenterPoints[i] = (1.0f / 3.0f) * (vertices[3*i + 0].position + vertices[3*i + 1].position + vertices[3*i + 2].position);
	}

	for (uint i = 0; i < vertices.size(); i++)
	{
		for (uint j = 0; j < vertices.size(); j++)
		{
			if (i != j)
			{
				if ( (vertices[i].position == vertices[j].position) && (vertices[i].normal == vertices[j].normal) )
				{
					if (mirrorType != EMirrorType::NoMirror)
					{
						SVector3& i_triangleCenterPoint = trianglesCenterPoints[i / 3];
						SVector3& j_triangleCenterPoint = trianglesCenterPoints[j / 3];

						if (mirrorType == EMirrorType::AxisX)
						{
							if (i_triangleCenterPoint.x * j_triangleCenterPoint.x < 0.0f)
								continue;
						}
						else if (mirrorType == EMirrorType::AxisY)
						{
							if (i_triangleCenterPoint.y * j_triangleCenterPoint.y < 0.0f)
								continue;
						}
						else if (mirrorType == EMirrorType::AxisZ)
						{
							if (i_triangleCenterPoint.z * j_triangleCenterPoint.z < 0.0f)
								continue;
						}
					}

					if (Dot(tempTangents[i], tempTangents[j]) >= minTangentsPairDotProduct)
						vertices[i].tangent += tempTangents[j];

					if (Dot(tempBitangents[i], tempBitangents[j]) >= minTangentsPairDotProduct)
						vertices[i].bitangent += tempBitangents[j];
				}
			}
		}

		NormalizeIn(vertices[i].tangent);
		NormalizeIn(vertices[i].bitangent);
	}

	if (trianglesCenterPoints)
		delete[] trianglesCenterPoints;

	delete[] tempTangents;
	delete[] tempBitangents;
}


void NMesh::OrthogonalizeTangents(vector<SVertex>& vertices)
{
	for (uint i = 0; i < vertices.size(); i++)
	{
		SVector3& v1 = vertices[i].normal;
		SVector3& v2 = vertices[i].tangent;
		SVector3& v3 = vertices[i].bitangent;

		SVector3 u1 = v1;
		SVector3 u2 = v2 - (Dot(v2, u1)/Dot(u1, u1))*u1;
		SVector3 u3 = v3 - (Dot(v3, u1)/Dot(u1, u1))*u1 - (Dot(v3, u2)/Dot(u2, u2))*u2;

		vertices[i].tangent = u2;
		vertices[i].bitangent = u3;
	}
}


bool NMesh::Equal(const SVertex& v1, const SVertex& v2)
{
	if (v1.position == v2.position &&
		v1.normal == v2.normal &&
		v1.tangent == v2.tangent &&
		v1.bitangent == v2.bitangent &&
		v1.texCoord0 == v2.texCoord0 &&
		v1.texCoord1 == v2.texCoord1 &&
		v1.color == v2.color)
	{
		return true;
	}
	else
	{
		return false;
	}
}
