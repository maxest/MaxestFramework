#include "mesh.h"
#include "../essentials/file.h"
#include "../math/main.h"


using namespace NMaxestFramework;
using namespace NEssentials;
using namespace NMath;


struct SGeomobject
{
	struct SFace
	{
		int vertexIndex[3];
		int texCoordIndex[3];
	};

	string name;

	SMatrix normalTransform; // every loaded normal must be multiplied by this matrix

	vector<SVector3> vertices;
	vector<SVector2> texCoords;
	vector<SFace> faces;
	vector<SVector3> normals;
	vector<SVector3> colors;
	vector<SVector3> facesNormals;
};


bool NMesh::ImportASE(const string& path, SMesh& mesh)
{
	vector<SGeomobject> geomobjects;
	int currentGeomobjectIndex;
	string s1, s2;
	float f1, f2, f3;
	int i1, i2, i3, i4;
	SVector3 nodeNormalTransform[3] = { cVector3Zero, cVector3Zero, cVector3Zero };

	mesh.lightmapsCount = 0;

	//

	CFile file;
	if (!file.Open(path, CFile::EOpenMode::ReadText))
		return false;

	// get general information

	currentGeomobjectIndex = -1;

	while (!file.EndOfFile())
	{
		file.ReadText(s1);

		if (s1 == "*GEOMOBJECT")
		{
			currentGeomobjectIndex++;
			geomobjects.push_back(SGeomobject());
		}
		else if (s1 == "*NODE_NAME")
		{
			file.ReadText(s2);
			geomobjects[currentGeomobjectIndex].name = s2.substr(1, s2.length() - 2);
		}
		else if (s1 == "*TM_ROW0")
		{
			file.ReadText(f1);
			file.ReadText(f2);
			file.ReadText(f3);

			nodeNormalTransform[0] = Normalize(VectorCustom(f1, f2, f3));
		}
		else if (s1 == "*TM_ROW1")
		{
			file.ReadText(f1);
			file.ReadText(f2);
			file.ReadText(f3);

			nodeNormalTransform[1] = Normalize(VectorCustom(f1, f2, f3));
		}
		else if (s1 == "*TM_ROW2")
		{
			file.ReadText(f1);
			file.ReadText(f2);
			file.ReadText(f3);

			nodeNormalTransform[2] = Normalize(VectorCustom(f1, f2, f3));

			//

			geomobjects[currentGeomobjectIndex].normalTransform = MatrixCustom(
				nodeNormalTransform[0].x, nodeNormalTransform[0].y, nodeNormalTransform[0].z, 0.0f,
				nodeNormalTransform[1].x, nodeNormalTransform[1].y, nodeNormalTransform[1].z, 0.0f,
				nodeNormalTransform[2].x, nodeNormalTransform[2].y, nodeNormalTransform[2].z, 0.0f,
				0.0f					, 0.0f					  , 0.0f					, 1.0f);
		}
		else if (s1 == "*MESH_NUMFACES")
		{
			file.ReadText(i1);
			geomobjects[currentGeomobjectIndex].faces.resize(i1);
		}
	}

	file.Seek(0);

	// get data

	currentGeomobjectIndex = -1;

	while (!file.EndOfFile())
	{
		file.ReadText(s1);

		if (s1 == "*GEOMOBJECT")
		{
			currentGeomobjectIndex++;
		}
		else if (s1 == "*MESH_VERTEX")
		{
			file.ReadText(i1);
			file.ReadText(f1);
			file.ReadText(f2);
			file.ReadText(f3);

			geomobjects[currentGeomobjectIndex].vertices.push_back(VectorCustom(f1, f2, f3));
		}
		else if (s1 == "*MESH_FACE")
		{
			file.ReadText(i1);
			file.ReadText(s1);
			file.ReadText(s1);
			file.ReadText(i2);
			file.ReadText(s1);
			file.ReadText(i3);
			file.ReadText(s1);
			file.ReadText(i4);

			geomobjects[currentGeomobjectIndex].faces[i1].vertexIndex[0] = i2;
			geomobjects[currentGeomobjectIndex].faces[i1].vertexIndex[1] = i3;
			geomobjects[currentGeomobjectIndex].faces[i1].vertexIndex[2] = i4;
		}
		else if (s1 == "*MESH_TVERT")
		{
			file.ReadText(i1);
			file.ReadText(f1);
			file.ReadText(f2);

			geomobjects[currentGeomobjectIndex].texCoords.push_back(VectorCustom(f1, f2));
		}
		else if (s1 == "*MESH_TFACE")
		{
			file.ReadText(i1);
			file.ReadText(i2);
			file.ReadText(i3);
			file.ReadText(i4);

			geomobjects[currentGeomobjectIndex].faces[i1].texCoordIndex[0] = i2;
			geomobjects[currentGeomobjectIndex].faces[i1].texCoordIndex[1] = i3;
			geomobjects[currentGeomobjectIndex].faces[i1].texCoordIndex[2] = i4;
		}
		else if (s1 == "*MESH_VERTCOL")
		{
			file.ReadText(i1);
			file.ReadText(f1);
			file.ReadText(f2);
			file.ReadText(f3);

			geomobjects[currentGeomobjectIndex].colors.push_back(VectorCustom(f1, f2, f3));
		}
		else if (s1 == "*MESH_VERTEXNORMAL")
		{
			file.ReadText(i1);
			file.ReadText(f1);
			file.ReadText(f2);
			file.ReadText(f3);

			geomobjects[currentGeomobjectIndex].normals.push_back(VectorCustom(f1, f2, f3));
		}
		else if (s1 == "*MESH_FACENORMAL")
		{
			file.ReadText(i1);
			file.ReadText(f1);
			file.ReadText(f2);
			file.ReadText(f3);

			geomobjects[currentGeomobjectIndex].facesNormals.push_back(VectorCustom(f1, f2, f3));
		}
	}

	file.Close();

	// create meshes

	for (int i = 0; i < currentGeomobjectIndex + 1; i++)
	{
		SChunk chunk;

		chunk.materialIndex = -1;
		chunk.lightmapIndex = -1;
		chunk.hasNormals = geomobjects[i].normals.size() > 0;
		chunk.hasTangents = false;
		chunk.hasTexCoords0 = geomobjects[i].texCoords.size() > 0;
		chunk.hasTexCoords1 = false;
		chunk.hasColors = geomobjects[i].colors.size() > 0;

		for (uint j = 0; j < geomobjects[i].faces.size(); j++)
		{
			SVertex vertex1, vertex2, vertex3;
			vertex1.Reset();
			vertex2.Reset();
			vertex3.Reset();

			vertex1.position = geomobjects[i].vertices[geomobjects[i].faces[j].vertexIndex[0]];
			vertex2.position = geomobjects[i].vertices[geomobjects[i].faces[j].vertexIndex[1]];
			vertex3.position = geomobjects[i].vertices[geomobjects[i].faces[j].vertexIndex[2]];

			if (chunk.hasNormals)
			{
				vertex1.normal = geomobjects[i].normals[3*j + 0] * geomobjects[i].normalTransform;
				vertex2.normal = geomobjects[i].normals[3*j + 1] * geomobjects[i].normalTransform;
				vertex3.normal = geomobjects[i].normals[3*j + 2] * geomobjects[i].normalTransform;
			}

			if (chunk.hasTexCoords0)
			{
				vertex1.texCoord0 = geomobjects[i].texCoords[geomobjects[i].faces[j].texCoordIndex[0]];
				vertex2.texCoord0 = geomobjects[i].texCoords[geomobjects[i].faces[j].texCoordIndex[1]];
				vertex3.texCoord0 = geomobjects[i].texCoords[geomobjects[i].faces[j].texCoordIndex[2]];
			}

			if (chunk.hasColors)
			{
				vertex1.color = geomobjects[i].colors[3*j + 0];
				vertex2.color = geomobjects[i].colors[3*j + 1];
				vertex3.color = geomobjects[i].colors[3*j + 2];
			}

			chunk.vertices.push_back(vertex1);
			chunk.vertices.push_back(vertex2);
			chunk.vertices.push_back(vertex3);
		}

		mesh.chunks.push_back(chunk);
	}

	//

	return true;
}
