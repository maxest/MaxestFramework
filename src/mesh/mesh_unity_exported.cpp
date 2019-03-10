#include "mesh.h"
#include "../essentials/string.h"
#include "../essentials/file.h"
#include "../math/main.h"


using namespace NMaxestFramework;
using namespace NEssentials;
using namespace NMath;


bool NMesh::ImportUnityExported(const string& path, SMesh& mesh)
{
	struct FileVertex
	{
		SVector3 position;
		SVector3 normal;
		SVector2 texCoord0;
		SVector2 texCoord1;
	};

	vector<FileVertex> fileVertices;
	vector<uint16> fileIndices;

	CFile file;
	if (file.Open(path, CFile::EOpenMode::ReadBinary))
	{
		file.ReadBin((char*)&mesh.lightmapsCount, 4);

		int chunksCount;
		file.ReadBin((char*)&chunksCount, 4);

		mesh.materials.resize(chunksCount);
		mesh.chunks.resize(chunksCount);

		for (int i = 0; i < chunksCount; i++)
		{
			mesh.materials[i].name = "";
			file.ReadBin(mesh.materials[i].textureFileName);

			mesh.chunks[i].materialIndex = i;

			file.ReadBin((char*)&mesh.chunks[i].lightmapIndex, 4);
			SVector4 lightmapScaleAndOffset;
			file.ReadBin((char*)&lightmapScaleAndOffset.x, 4);
			file.ReadBin((char*)&lightmapScaleAndOffset.y, 4);
			file.ReadBin((char*)&lightmapScaleAndOffset.z, 4);
			file.ReadBin((char*)&lightmapScaleAndOffset.w, 4);

			mesh.chunks[i].hasNormals = true;
			mesh.chunks[i].hasTangents = false;
			mesh.chunks[i].hasTexCoords0 = true;
			mesh.chunks[i].hasTexCoords1 = true;
			mesh.chunks[i].hasColors = false;

			int verticesCount, indicesCount;

			file.ReadBin((char*)&verticesCount, 4);
			file.ReadBin((char*)&indicesCount, 4);

			fileVertices.resize(verticesCount);
			fileIndices.resize(indicesCount);
			mesh.chunks[i].vertices.resize(verticesCount);
			mesh.chunks[i].indices.resize(indicesCount);

			file.ReadBin((char*)&fileVertices[0], verticesCount * sizeof(FileVertex));
			file.ReadBin((char*)&fileIndices[0], indicesCount * sizeof(uint16));

			for (int j = 0; j < verticesCount; j++)
			{
				mesh.chunks[i].vertices[j].Reset();

				mesh.chunks[i].vertices[j].position = fileVertices[j].position;
				mesh.chunks[i].vertices[j].normal = fileVertices[j].normal;
				mesh.chunks[i].vertices[j].texCoord0 = fileVertices[j].texCoord0;
				mesh.chunks[i].vertices[j].texCoord1 = fileVertices[j].texCoord1;

				mesh.chunks[i].vertices[j].texCoord1.x =
					lightmapScaleAndOffset.x * mesh.chunks[i].vertices[j].texCoord1.x +
					lightmapScaleAndOffset.z;
				mesh.chunks[i].vertices[j].texCoord1.y =
					lightmapScaleAndOffset.y * mesh.chunks[i].vertices[j].texCoord1.y +
					lightmapScaleAndOffset.w;
			}

			for (int j = 0; j < indicesCount; j++)
			{
				mesh.chunks[i].indices[j] = fileIndices[j];
			}
		}

		file.Close();

		return true;
	}
	else
	{
		return false;
	}
}
