#ifdef MAXEST_FRAMEWORK_WINDOWS


#include "utils.h"
#include "d3d11.h"
#include "../system/file.h"
#include "../math/main.h"
#include "../mesh/main.h"
#include "../image/main.h"


using namespace NMaxestFramework;
using namespace NEssentials;
using namespace NMath;
using namespace NSystem;
using namespace NImage;


//


NGPU::NUtils::CResources gGPUUtilsResources;
NGPU::NUtils::CPostprocessor gGPUUtilsPostprocessor;
NGPU::NUtils::CFontRenderer gGPUUtilsFontRenderer;
NGPU::NUtils::CDebugRenderer gGPUUtilsDebugRenderer;


// Texture


bool NGPU::NUtils::CTexture::CreateFromFile(const string& path, bool swapChannels, bool invertY, bool generateMipmaps)
{
	string cachedFilePath = path + ".cache";

	if (FileExists(cachedFilePath) && FileMoreRecentlyModified(cachedFilePath, path))
	{
		if (!CreateCPUDataFromCacheFile(cachedFilePath))
			return false;
	}
	else
	{
		if (!CreateCPUDataFromSourceFile(path, swapChannels, invertY, generateMipmaps))
			return false;
		if (!SaveCPUDataToCacheFile(cachedFilePath))
			return false;
	}

	CreateGPUData();

	return true;
}


void NGPU::NUtils::CTexture::Destroy()
{
	DestroyCPUData();
	DestroyGPUData();
}


bool NGPU::NUtils::CTexture::CreateCPUDataFromSourceFile(const string& path, bool swapChannels, bool invertY, bool generateMipmaps)
{
	SImage image;

	if (!Load(path, image, swapChannels, invertY, EFormat::RGBA8))
		return false;

	if (generateMipmaps)
	{
		mipmaps = GenerateMipmaps(image, NImage::EFilter::Box);
		delete[] image.data;
	}
	else
	{
		mipmaps.push_back(image);
	}

	return true;
}


bool NGPU::NUtils::CTexture::CreateCPUDataFromCacheFile(const string& path)
{
	CFile file;
	if (file.Open(path, CFile::EOpenMode::ReadBinary))
	{
		uint32 mipmapsCount;

		file.ReadBin((char*)&mipmapsCount, sizeof(uint32));
		mipmaps.resize(mipmapsCount);

		for (uint i = 0; i < mipmapsCount; i++)
		{
			file.ReadBin((char*)&mipmaps[i].width, sizeof(uint16));
			file.ReadBin((char*)&mipmaps[i].height, sizeof(uint16));

			mipmaps[i].data = new uint8[4 * mipmaps[i].width * mipmaps[i].height];

			file.ReadBin((char*)mipmaps[i].data, 4 * mipmaps[i].width * mipmaps[i].height);
		}

		file.Close();

		return true;
	}
	else
	{
		return false;
	}
}


void NGPU::NUtils::CTexture::CreateGPUData()
{
	CreateTexture(mipmaps[0].width, mipmaps[0].height, texture);

	for (uint i = 0; i < mipmaps.size(); i++)
		UpdateTexture(texture, i, mipmaps[i].data, 4 * mipmaps[i].width);
}


void NGPU::NUtils::CTexture::DestroyCPUData()
{
	for (uint i = 0; i < mipmaps.size(); i++)
		delete[] mipmaps[i].data;

	mipmaps.clear();
}


void NGPU::NUtils::CTexture::DestroyGPUData()
{
	DestroyTexture(texture);
}


bool NGPU::NUtils::CTexture::SaveCPUDataToCacheFile(const string& path)
{
	CFile file;
	if (file.Open(path, CFile::EOpenMode::WriteBinary))
	{
		uint32 mipmapsCount = (uint32)mipmaps.size();

		file.WriteBin((char*)&mipmapsCount, sizeof(uint32));
		for (uint i = 0; i < mipmapsCount; i++)
		{
			file.WriteBin((char*)&mipmaps[i].width, sizeof(uint16));
			file.WriteBin((char*)&mipmaps[i].height, sizeof(uint16));
			file.WriteBin((char*)mipmaps[i].data, 4 * mipmaps[i].width * mipmaps[i].height);
		}

		file.Close();

		return true;
	}
	else
	{
		return false;
	}
}


// Mesh


NGPU::NUtils::CMesh::SChunk::SChunk()
{
	verticesCount = 0;
	indicesCount = 0;
	vertices = nullptr;
	indices = nullptr;
}


void NGPU::NUtils::CMesh::SChunk::Destroy()
{
	SAFE_DELETE_ARRAY(vertices);
	SAFE_DELETE_ARRAY(indices);
	DestroyBuffer(vb);
	DestroyBuffer(ib);
}


bool NGPU::NUtils::CMesh::CreateFromFile(const string& path, EFileType fileType, map<string, CTexture>* textures)
{
	string cachedFilePath = path + ".cache";

	if (FileExists(cachedFilePath) && FileMoreRecentlyModified(cachedFilePath, path))
	{
		if (!CreateCPUBuffersFromCacheFile(cachedFilePath))
			return false;
	}
	else
	{
		if (!CreateCPUBuffersFromSourceFile(path, fileType))
			return false;
		if (!SaveCPUBuffersToCacheFile(cachedFilePath))
			return false;
	}

	CreateGPUBuffers();
	CalculateBoundingBox();

	if (textures)
		*textures = CreateGPUTextures(ExtractDir(path));

	return true;
}


bool NGPU::NUtils::CMesh::CreatePlane(float width, float height, const NMath::SVector2& uvScale, const NMath::SMatrix& transform)
{
	if (!CreateCPUBuffersForPlane(width, height, uvScale, transform))
		return false;

	CreateGPUBuffers();
	CalculateBoundingBox();

	return true;
}


void NGPU::NUtils::CMesh::Destroy()
{
	for (uint i = 0; i < chunks.size(); i++)
		chunks[i].Destroy();
	chunks.clear();
}


bool NGPU::NUtils::CMesh::CreateCPUBuffersFromSourceFile(const string& path, EFileType fileType)
{
	NMesh::SMesh mesh;

	bool importResult = false;
	if (fileType == EFileType::ASE)
		importResult = ImportASE(path, mesh);
	else if (fileType == EFileType::OBJ)
		importResult = ImportOBJ(path, mesh);
	else if (fileType == EFileType::UnityExported)
		importResult = NMesh::ImportUnityExported(path, mesh);
	if (!importResult)
		return false;

	this->lightmapsCount = mesh.lightmapsCount;

	for (uint i = 0; i < mesh.chunks.size(); i++)
	{
		if (!mesh.chunks[i].IsIndexed())
			ToIndexed(mesh.chunks[i]);

		SChunk chunk;

		chunk.verticesCount = (uint32)mesh.chunks[i].vertices.size();
		chunk.indicesCount = (uint32)mesh.chunks[i].indices.size();
		chunk.vertices = new SVertex_PosNorUV0UV1[chunk.verticesCount];
		chunk.indices = new uint16[chunk.indicesCount];

		for (uint j = 0; j < chunk.verticesCount; j++)
		{
			chunk.vertices[j].position = mesh.chunks[i].vertices[j].position;
			chunk.vertices[j].normal = mesh.chunks[i].vertices[j].normal;
			chunk.vertices[j].texCoord0 = mesh.chunks[i].vertices[j].texCoord0;
			chunk.vertices[j].texCoord1 = mesh.chunks[i].vertices[j].texCoord1;
		}

		for (uint j = 0; j < chunk.indicesCount; j++)
		{
			chunk.indices[j] = mesh.chunks[i].indices[j];
		}

		chunk.textureFileName = "";
		if (mesh.chunks[i].materialIndex >= 0)
			chunk.textureFileName = mesh.materials[mesh.chunks[i].materialIndex].textureFileName;

		chunk.lightmapIndex = mesh.chunks[i].lightmapIndex;

		this->chunks.push_back(chunk);
	}

	return true;
}


bool NGPU::NUtils::CMesh::CreateCPUBuffersFromCacheFile(const string& path)
{
	CFile file;
	if (file.Open(path, CFile::EOpenMode::ReadBinary))
	{
		file.ReadBin((char*)&lightmapsCount, sizeof(uint32));

		uint32 chunksCount;
		file.ReadBin((char*)&chunksCount, sizeof(uint32));

		chunks.resize(chunksCount);

		for (uint i = 0; i < chunksCount; i++)
		{
			file.ReadBin((char*)&chunks[i].verticesCount, sizeof(uint32));
			file.ReadBin((char*)&chunks[i].indicesCount, sizeof(uint32));

			chunks[i].vertices = new SVertex_PosNorUV0UV1[chunks[i].verticesCount];
			chunks[i].indices = new uint16[chunks[i].indicesCount];

			file.ReadBin((char*)chunks[i].vertices, chunks[i].verticesCount * sizeof(SVertex_PosNorUV0UV1));
			file.ReadBin((char*)chunks[i].indices, chunks[i].indicesCount * sizeof(uint16));

			file.ReadBin(chunks[i].textureFileName);
			file.ReadBin((char*)&chunks[i].lightmapIndex, 1);
		}

		file.Close();

		return true;
	}
	else
	{
		return false;
	}
}


bool NGPU::NUtils::CMesh::CreateCPUBuffersForPlane(float width, float height, const NMath::SVector2& uvScale, const NMath::SMatrix& transform)
{
	SChunk chunk;

	chunk.verticesCount = 4;
	chunk.indicesCount = 6;
	chunk.vertices = new SVertex_PosNorUV0UV1[chunk.verticesCount];
	chunk.indices = new uint16[chunk.indicesCount];

	SVertex_PosNorUV0UV1 vertices[] =
	{
		VectorCustom(-0.5f*width, 0.5f*height, 0.0f), VectorCustom(0.0f, 0.0f, 1.0f), VectorCustom(0.0f, 0.0f), VectorCustom(0.0f, 0.0f),
		VectorCustom(-0.5f*width, -0.5f*height, 0.0f), VectorCustom(0.0f, 0.0f, 1.0f), VectorCustom(0.0f, 1.0f), VectorCustom(0.0f, 0.0f),
		VectorCustom(0.5f*width, -0.5f*height, 0.0f), VectorCustom(0.0f, 0.0f, 1.0f), VectorCustom(1.0f, 1.0f), VectorCustom(0.0f, 0.0f),
		VectorCustom(0.5f*width, 0.5f*height, 0.0f), VectorCustom(0.0f, 0.0f, 1.0f), VectorCustom(1.0f, 0.0f), VectorCustom(0.0f, 0.0f),
	};
	for (int i = 0; i < ARRAY_SIZE(vertices); i++)
	{
		vertices[i].position = TransformPoint(vertices[i].position, transform);
		vertices[i].normal = TransformNormal(vertices[i].normal, transform);
		vertices[i].texCoord0 *= uvScale;
	}

	uint16 indices[] = { 0, 1, 2, 0, 2, 3 };

	memcpy(chunk.vertices, vertices, 4 * sizeof(SVertex_PosNorUV0UV1));
	memcpy(chunk.indices, indices, 6 * sizeof(uint16));

	chunks.resize(1);
	chunks[0] = chunk;

	return true;
}


void NGPU::NUtils::CMesh::CreateGPUBuffers()
{
	for (uint i = 0; i < chunks.size(); i++)
	{
		CreateVertexBuffer(false, (uint8*)chunks[i].vertices, chunks[i].verticesCount * sizeof(SVertex_PosNorUV0UV1), chunks[i].vb);
		CreateIndexBuffer(false, (uint8*)chunks[i].indices, chunks[i].indicesCount * sizeof(uint16), chunks[i].ib);
	}
}


void NGPU::NUtils::CMesh::CalculateBoundingBox()
{
	bbox = AABB();

	for (uint i = 0; i < chunks.size(); i++)
	{
		chunks[i].bbox = AABB();
		for (uint j = 0; j < chunks[i].verticesCount; j++)
			AABBAddVertex(chunks[i].bbox, chunks[i].vertices[j].position);

		bbox = AABBMerge(bbox, chunks[i].bbox);
	}
}


bool NGPU::NUtils::CMesh::SaveCPUBuffersToCacheFile(const string& path)
{
	CFile file;
	if (file.Open(path, CFile::EOpenMode::WriteBinary))
	{
		file.WriteBin((char*)&lightmapsCount, sizeof(uint32));

		uint32 chunksCount = (uint32)chunks.size();
		file.WriteBin((char*)&chunksCount, sizeof(uint32));

		for (uint i = 0; i < chunksCount; i++)
		{
			file.WriteBin((char*)&chunks[i].verticesCount, sizeof(uint32));
			file.WriteBin((char*)&chunks[i].indicesCount, sizeof(uint32));

			file.WriteBin((char*)chunks[i].vertices, chunks[i].verticesCount * sizeof(SVertex_PosNorUV0UV1));
			file.WriteBin((char*)chunks[i].indices, chunks[i].indicesCount * sizeof(uint16));

			file.WriteBin(chunks[i].textureFileName);
			file.WriteBin((char*)&chunks[i].lightmapIndex, 1);
		}

		file.Close();

		return true;
	}
	else
	{
		return false;
	}
}


map<string, NGPU::NUtils::CTexture> NGPU::NUtils::CMesh::CreateGPUTextures(const string& dir)
{
	map<string, CTexture> textures;

	for (uint i = 0; i < chunks.size(); i++)
	{
		string& key = chunks[i].textureFileName;

		if (textures.find(key) == textures.end())
		{
			CTexture texture;

			if (texture.CreateFromFile(dir + chunks[i].textureFileName, false, false, true))
				textures[key] = texture;
		}
	}

	return textures;
}


void NGPU::NUtils::CMesh::Render(int chunkIndex)
{
	UINT stride = sizeof(SVertex_PosNorUV0UV1);
	UINT offset = 0;

	deviceContext->IASetVertexBuffers(0, 1, &chunks[chunkIndex].vb.buffer, &stride, &offset);
	deviceContext->IASetIndexBuffer(chunks[chunkIndex].ib.buffer, DXGI_FORMAT_R16_UINT, 0);

	deviceContext->DrawIndexed((uint)chunks[chunkIndex].indicesCount, 0, 0);
}


void NGPU::NUtils::CMesh::Render()
{
	for (uint i = 0; i < chunks.size(); i++)
		Render(i);
}


// Resources


const int NGPU::NUtils::CResources::cPrefixSumBatchSize = 512;


void NGPU::NUtils::CResources::Create(const string& frameworkPath)
{
	D3D11_INPUT_ELEMENT_DESC posInputLayoutElements[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	CreateInputLayout(frameworkPath + "/data/gpu/vertex_factory_vs.hlsl", "VERTEX_STRUCTURE=Vertex_Pos", posInputLayoutElements, ARRAY_SIZE(posInputLayoutElements), posInputLayout);

	D3D11_INPUT_ELEMENT_DESC posUVInputLayoutElements[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	CreateInputLayout(frameworkPath + "/data/gpu/vertex_factory_vs.hlsl", "VERTEX_STRUCTURE=Vertex_PosUV", posUVInputLayoutElements, ARRAY_SIZE(posUVInputLayoutElements), posUVInputLayout);

	D3D11_INPUT_ELEMENT_DESC posNorUVInputLayoutElements[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	CreateInputLayout(frameworkPath + "/data/gpu/vertex_factory_vs.hlsl", "VERTEX_STRUCTURE=Vertex_PosNorUV", posNorUVInputLayoutElements, ARRAY_SIZE(posNorUVInputLayoutElements), posNorUVInputLayout);

	D3D11_INPUT_ELEMENT_DESC posNorUV0UV1InputLayoutElements[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	CreateInputLayout(frameworkPath + "/data/gpu/vertex_factory_vs.hlsl", "VERTEX_STRUCTURE=Vertex_PosNorUV0UV1", posNorUV0UV1InputLayoutElements, ARRAY_SIZE(posNorUV0UV1InputLayoutElements), posNorUV0UV1InputLayout);

	//

	uint16 indices[] = { 0, 1, 2, 0, 2, 3 };
	CreateIndexBuffer(false, (uint8*)indices, 6 * sizeof(uint16), screenQuadIB);

	CreateRWStructuredBuffer(cPrefixSumBatchSize * sizeof(uint32), sizeof(uint32), prefixSumBuffer_blocksSums);
	CreateRWStructuredBuffer(cPrefixSumBatchSize * sizeof(uint32), sizeof(uint32), prefixSumBuffer_cumulativeBlocksSums);

	//

	CreateRasterizerState(ccwRasterizerState, true);
	CreateRasterizerState(cwRasterizerState, false);

	//

	CreateBlendState(defaultBlendState, false);
	CreateBlendState(transparencyBlendState, true);

	//

	CreateSamplerState(pointClampSamplerState, ESamplerFilter::Point, ESamplerAddressing::Clamp, ESamplerComparisonFunction::None);
	CreateSamplerState(linearClampSamplerState, ESamplerFilter::Linear, ESamplerAddressing::Clamp, ESamplerComparisonFunction::None);
	CreateSamplerState(anisotropicWrapSamplerState, ESamplerFilter::Anisotropic, ESamplerAddressing::Wrap, ESamplerComparisonFunction::None);
	CreateSamplerState(pointClampComparisonSamplerState, ESamplerFilter::Point, ESamplerAddressing::Clamp, ESamplerComparisonFunction::Less);
	CreateSamplerState(linearClampComparisonSamplerState, ESamplerFilter::Linear, ESamplerAddressing::Clamp, ESamplerComparisonFunction::Less);
	CreateSamplerState(anisotropicClampComparisonSamplerState, ESamplerFilter::Anisotropic, ESamplerAddressing::Clamp, ESamplerComparisonFunction::Less);
	CreateSamplerState(pointBorderGreySamplerState, ESamplerFilter::Point, ESamplerAddressing::BorderGrey, ESamplerComparisonFunction::None);
	CreateSamplerState(linearBorderGreySamplerState, ESamplerFilter::Linear, ESamplerAddressing::BorderGrey, ESamplerComparisonFunction::None);
	CreateSamplerState(pointWrapSamplerState, ESamplerFilter::Point, ESamplerAddressing::Wrap, ESamplerComparisonFunction::None);
	CreateSamplerState(linearWrapSamplerState, ESamplerFilter::Linear, ESamplerAddressing::Wrap, ESamplerComparisonFunction::None);

	//

	CreateShaders(frameworkPath);

	//

	MF_ASSERT(fontTexture.CreateFromFile(frameworkPath + "/data/textures/font.png", false, false, true));
}


void NGPU::NUtils::CResources::Destroy()
{
	fontTexture.Destroy();

	//

	for (uint i = 0; i < constantBuffers.size(); i++)
		DestroyBuffer(constantBuffers[i]);
	constantBuffers.clear();
	for (uint i = 0; i < textures.size(); i++)
		DestroyTexture(textures[i]);
	textures.clear();
	for (uint i = 0; i < renderTargets.size(); i++)
		DestroyRenderTarget(renderTargets[i]);
	renderTargets.clear();

	//

	DestroyShaders();

	//

	DestroySamplerState(pointClampSamplerState);
	DestroySamplerState(linearClampSamplerState);
	DestroySamplerState(anisotropicWrapSamplerState);
	DestroySamplerState(pointClampComparisonSamplerState);
	DestroySamplerState(linearClampComparisonSamplerState);
	DestroySamplerState(anisotropicClampComparisonSamplerState);
	DestroySamplerState(pointBorderGreySamplerState);
	DestroySamplerState(linearBorderGreySamplerState);
	DestroySamplerState(pointWrapSamplerState);
	DestroySamplerState(linearWrapSamplerState);

	//

	DestroyBlendState(defaultBlendState);
	DestroyBlendState(transparencyBlendState);

	//

	DestroyRasterizerState(ccwRasterizerState);
	DestroyRasterizerState(cwRasterizerState);

	//

	DestroyBuffer(prefixSumBuffer_blocksSums);
	DestroyBuffer(prefixSumBuffer_cumulativeBlocksSums);

	DestroyBuffer(screenQuadIB);

	//

	DestroyInputLayout(posInputLayout);
	DestroyInputLayout(posUVInputLayout);
	DestroyInputLayout(posNorUVInputLayout);
	DestroyInputLayout(posNorUV0UV1InputLayout);
}


void NGPU::NUtils::CResources::CreateShaders(const string& frameworkPath)
{
	MF_ASSERT(CreateVertexShader(frameworkPath + "/data/gpu/font_vs_ps.hlsl", "main_vs", "", fontVS));
	MF_ASSERT(CreatePixelShader(frameworkPath + "/data/gpu/font_vs_ps.hlsl", "main_ps", "", fontPS));

	MF_ASSERT(CreateVertexShader(frameworkPath + "/data/gpu/debug_mesh_vs_ps.hlsl", "main_vs", "", debugMeshVS));
	MF_ASSERT(CreatePixelShader(frameworkPath + "/data/gpu/debug_mesh_vs_ps.hlsl", "main_ps", "", debugMeshPS));

	MF_ASSERT(CreateVertexShader(frameworkPath + "/data/gpu/screen_quad_vs.hlsl", screenQuadVS));

	MF_ASSERT(CreatePixelShader(frameworkPath + "/data/gpu/postprocess_ps.hlsl", "COPY_POINT", postprocess_copyPointTexturePS));
	MF_ASSERT(CreatePixelShader(frameworkPath + "/data/gpu/postprocess_ps.hlsl", "COPY_LINEAR", postprocess_copyLinearTexturePS));
	MF_ASSERT(CreatePixelShader(frameworkPath + "/data/gpu/postprocess_ps.hlsl", "COPY_POINT|SCALE_AND_OFFSET", postprocess_copyPointTextureWithScaleAndOffsetPS));
	MF_ASSERT(CreatePixelShader(frameworkPath + "/data/gpu/postprocess_ps.hlsl", "COPY_LINEAR|SCALE_AND_OFFSET", postprocess_copyLinearTextureWithScaleAndOffsetPS));
	MF_ASSERT(CreatePixelShader(frameworkPath + "/data/gpu/postprocess_ps.hlsl", "LUMA", postprocess_lumaPS));
	MF_ASSERT(CreatePixelShader(frameworkPath + "/data/gpu/postprocess_ps.hlsl", "ADD", postprocess_addPS));
	MF_ASSERT(CreatePixelShader(frameworkPath + "/data/gpu/postprocess_ps.hlsl", "MUL", postprocess_mulPS));
	MF_ASSERT(CreatePixelShader(frameworkPath + "/data/gpu/postprocess_ps.hlsl", "POW", postprocess_powPS));
	for (int i = 0; i < 4; i++)
	{
		string channelsCount = ToString(i + 1);

		MF_ASSERT(CreatePixelShader(frameworkPath + "/data/gpu/postprocess_ps.hlsl", "MIN|HORIZONTAL|CHANNELS_COUNT=" + channelsCount, postprocess_minXPS[i]));
		MF_ASSERT(CreatePixelShader(frameworkPath + "/data/gpu/postprocess_ps.hlsl", "MIN|VERTICAL|CHANNELS_COUNT=" + channelsCount, postprocess_minYPS[i]));
		MF_ASSERT(CreatePixelShader(frameworkPath + "/data/gpu/postprocess_ps.hlsl", "MIN13|HORIZONTAL|CHANNELS_COUNT=" + channelsCount, postprocess_minX13PS[i]));
		MF_ASSERT(CreatePixelShader(frameworkPath + "/data/gpu/postprocess_ps.hlsl", "MIN13|VERTICAL|CHANNELS_COUNT=" + channelsCount, postprocess_minY13PS[i]));
		MF_ASSERT(CreatePixelShader(frameworkPath + "/data/gpu/postprocess_ps.hlsl", "MAX|HORIZONTAL|CHANNELS_COUNT=" + channelsCount, postprocess_maxXPS[i]));
		MF_ASSERT(CreatePixelShader(frameworkPath + "/data/gpu/postprocess_ps.hlsl", "MAX|VERTICAL|CHANNELS_COUNT=" + channelsCount, postprocess_maxYPS[i]));
		MF_ASSERT(CreatePixelShader(frameworkPath + "/data/gpu/postprocess_ps.hlsl", "MAX13|HORIZONTAL|CHANNELS_COUNT=" + channelsCount, postprocess_maxX13PS[i]));
		MF_ASSERT(CreatePixelShader(frameworkPath + "/data/gpu/postprocess_ps.hlsl", "MAX13|VERTICAL|CHANNELS_COUNT=" + channelsCount, postprocess_maxY13PS[i]));
		MF_ASSERT(CreatePixelShader(frameworkPath + "/data/gpu/postprocess_ps.hlsl", "BLUR|HORIZONTAL|CHANNELS_COUNT=" + channelsCount, postprocess_blurXPS[i]));
		MF_ASSERT(CreatePixelShader(frameworkPath + "/data/gpu/postprocess_ps.hlsl", "BLUR|VERTICAL|CHANNELS_COUNT=" + channelsCount, postprocess_blurYPS[i]));
		MF_ASSERT(CreatePixelShader(frameworkPath + "/data/gpu/postprocess_ps.hlsl", "BLUR13|HORIZONTAL|CHANNELS_COUNT=" + channelsCount, postprocess_blurX13PS[i]));
		MF_ASSERT(CreatePixelShader(frameworkPath + "/data/gpu/postprocess_ps.hlsl", "BLUR13|VERTICAL|CHANNELS_COUNT=" + channelsCount, postprocess_blurY13PS[i]));
		MF_ASSERT(CreatePixelShader(frameworkPath + "/data/gpu/postprocess_ps.hlsl", "BLUR_GAUSS|HORIZONTAL|CHANNELS_COUNT=" + channelsCount, postprocess_blurGaussXPS[i]));
		MF_ASSERT(CreatePixelShader(frameworkPath + "/data/gpu/postprocess_ps.hlsl", "BLUR_GAUSS|VERTICAL|CHANNELS_COUNT=" + channelsCount, postprocess_blurGaussYPS[i]));
	}

	MF_ASSERT(CreatePixelShader(frameworkPath + "/data/gpu/downsample_minmax_ps.hlsl", "FUNCTION=min|_2X2", downsampleMin2x2PS));
	MF_ASSERT(CreatePixelShader(frameworkPath + "/data/gpu/downsample_minmax_ps.hlsl", "FUNCTION=min|_4X4", downsampleMin4x4PS));
	MF_ASSERT(CreatePixelShader(frameworkPath + "/data/gpu/downsample_minmax_ps.hlsl", "FUNCTION=max|_2X2", downsampleMax2x2PS));
	MF_ASSERT(CreatePixelShader(frameworkPath + "/data/gpu/downsample_minmax_ps.hlsl", "FUNCTION=max|_4X4", downsampleMax4x4PS));

	MF_ASSERT(CreatePixelShader(frameworkPath + "/data/gpu/deproject_ps.hlsl", deprojectPS));
	MF_ASSERT(CreatePixelShader(frameworkPath + "/data/gpu/reproject_ps.hlsl", reprojectPS));

	MF_ASSERT(CreatePixelShader(frameworkPath + "/data/gpu/resolve_ps.hlsl", resolvePS));
	MF_ASSERT(CreatePixelShader(frameworkPath + "/data/gpu/resolve_ps.hlsl", "MSAA", resolvePS_msaa));

	MF_ASSERT(CreateComputeShader(frameworkPath + "/data/gpu/pixel_copy_cs.hlsl", pixelCopyCS));

	MF_ASSERT(CreateComputeShader(frameworkPath + "/data/gpu/prefix_sum_cs.hlsl", "PREFIX_SUM|ELEMENTS_COUNT=" + ToString(cPrefixSumBatchSize) + "|VARIANT=1", prefixSumCS_prefixSum));
	MF_ASSERT(CreateComputeShader(frameworkPath + "/data/gpu/prefix_sum_cs.hlsl", "PREFIX_SUM|ELEMENTS_COUNT=" + ToString(cPrefixSumBatchSize) + "|VARIANT=1|WRITE_BLOCKS_SUMS", prefixSumCS_prefixSum_writeBlocksSums));
	MF_ASSERT(CreateComputeShader(frameworkPath + "/data/gpu/prefix_sum_cs.hlsl", "SUM|ELEMENTS_COUNT=" + ToString(cPrefixSumBatchSize), prefixSumCS_sum));
}


void NGPU::NUtils::CResources::DestroyShaders()
{
	DestroyComputeShader(prefixSumCS_prefixSum);
	DestroyComputeShader(prefixSumCS_prefixSum_writeBlocksSums);
	DestroyComputeShader(prefixSumCS_sum);

	DestroyComputeShader(pixelCopyCS);

	DestroyPixelShader(resolvePS);
	DestroyPixelShader(resolvePS_msaa);

	DestroyPixelShader(deprojectPS);
	DestroyPixelShader(reprojectPS);

	DestroyPixelShader(downsampleMin2x2PS);
	DestroyPixelShader(downsampleMin4x4PS);
	DestroyPixelShader(downsampleMax2x2PS);
	DestroyPixelShader(downsampleMax4x4PS);

	DestroyPixelShader(postprocess_copyPointTexturePS);
	DestroyPixelShader(postprocess_copyLinearTexturePS);
	DestroyPixelShader(postprocess_copyPointTextureWithScaleAndOffsetPS);
	DestroyPixelShader(postprocess_copyLinearTextureWithScaleAndOffsetPS);
	DestroyPixelShader(postprocess_lumaPS);
	DestroyPixelShader(postprocess_addPS);
	DestroyPixelShader(postprocess_mulPS);
	DestroyPixelShader(postprocess_powPS);
	for (int i = 0; i < 4; i++)
	{
		DestroyPixelShader(postprocess_minXPS[i]);
		DestroyPixelShader(postprocess_minYPS[i]);
		DestroyPixelShader(postprocess_minX13PS[i]);
		DestroyPixelShader(postprocess_minY13PS[i]);
		DestroyPixelShader(postprocess_maxXPS[i]);
		DestroyPixelShader(postprocess_maxYPS[i]);
		DestroyPixelShader(postprocess_maxX13PS[i]);
		DestroyPixelShader(postprocess_maxY13PS[i]);
		DestroyPixelShader(postprocess_blurXPS[i]);
		DestroyPixelShader(postprocess_blurYPS[i]);
		DestroyPixelShader(postprocess_blurX13PS[i]);
		DestroyPixelShader(postprocess_blurY13PS[i]);
		DestroyPixelShader(postprocess_blurGaussXPS[i]);
		DestroyPixelShader(postprocess_blurGaussYPS[i]);
	}

	DestroyVertexShader(screenQuadVS);

	DestroyVertexShader(debugMeshVS);
	DestroyPixelShader(debugMeshPS);

	DestroyVertexShader(fontVS);
	DestroyPixelShader(fontPS);
}


NGPU::SBuffer& NGPU::NUtils::CResources::ConstantBuffer(int registersCount, uint32 id)
{
	int index = -1;
	for (uint i = 0; i < constantBuffers.size(); i++)
	{
		if (constantBuffers[i].size == 16*registersCount && constantBuffers[i].tag == id)
		{
			index = i;
			break;
		}
	}

	if (index >= 0)
	{
		return constantBuffers[index];
	}
	else
	{
		SBuffer buffer;
		CreateConstantBuffer(16 * registersCount, buffer);
		buffer.tag = id;
		constantBuffers.push_back(buffer);
		return constantBuffers.back();
	}
}


NGPU::STexture& NGPU::NUtils::CResources::Texture(int width, int height, DXGI_FORMAT format, uint32 id)
{
	int index = -1;
	for (uint i = 0; i < textures.size(); i++)
	{
		if (textures[i].width == width && textures[i].height == height && textures[i].format == format && textures[i].tag == id)
		{
			index = i;
			break;
		}
	}

	if (index >= 0)
	{
		return textures[index];
	}
	else
	{
		STexture texture;
		CreateTexture(width, height, 1, format, texture);
		texture.tag = id;
		textures.push_back(texture);
		return textures.back();
	}
}


NGPU::TRenderTarget& NGPU::NUtils::CResources::RenderTarget(int width, int height, DXGI_FORMAT format, uint32 id)
{
	int index = -1;
	for (uint i = 0; i < renderTargets.size(); i++)
	{
		if (renderTargets[i].width == width && renderTargets[i].height == height && renderTargets[i].format == format && renderTargets[i].tag == id)
		{
			index = i;
			break;
		}
	}

	if (index >= 0)
	{
		return renderTargets[index];
	}
	else
	{
		TRenderTarget renderTarget;
		CreateRenderTarget(width, height, format, 1, renderTarget);
		renderTarget.tag = id;
		renderTargets.push_back(renderTarget);
		return renderTargets.back();
	}
}


NGPU::STexture NGPU::NUtils::CResources::CreateTexture2D_InterleavedGradientNoise(int width, int height, int mipmapsCount)
{
	STexture texture;
	NGPU::CreateTexture(width, height, 0, DXGI_FORMAT_R32_FLOAT, texture);

	MF_ASSERT(mipmapsCount <= texture.mipmapsCount);

	int mipmapWidth = texture.width;
	int mipmapHeight = texture.height;
	for (int i = 0; i < texture.mipmapsCount; i++)
	{
		float* data = new float[mipmapWidth * mipmapHeight];

		for (int y = 0; y < mipmapHeight; y++)
		{
			for (int x = 0; x < mipmapWidth; x++)
			{
				int idx = Idx(x, y, mipmapWidth);
				data[idx] = InterleavedGradientNoise(VectorCustom(x + 0.5f, y + 0.5f));
			}
		}

		NGPU::UpdateTexture(texture, i, (uint8*)data, mipmapWidth * sizeof(float));

		delete[] data;

		mipmapWidth >>= 1;
		mipmapHeight >>= 1;
	}

	return texture;
}


// Postprocessor


void NGPU::NUtils::CPostprocessor::Quad(const NGPU::STexture& output, const NGPU::STexture& input, ID3D11PixelShader* pixelShader)
{
	deviceContext->OMSetRenderTargets(1, &output.rtv, nullptr);

	deviceContext->VSSetShader(gGPUUtilsResources.screenQuadVS, nullptr, 0);
	deviceContext->PSSetShader(pixelShader, nullptr, 0);
	SetPSSamplers();
	deviceContext->PSSetShaderResources(0, 1, &input.srv);

	deviceContext->IASetIndexBuffer(gGPUUtilsResources.screenQuadIB.buffer, DXGI_FORMAT_R16_UINT, 0);

	deviceContext->DrawIndexed(6, 0, 0);

	ClearRenderTargets();
	ClearPSShaderResources(0, 1);

}


void NGPU::NUtils::CPostprocessor::CopyPointTexture(const NGPU::STexture& output, const NGPU::STexture& input)
{
	Quad(output, input, gGPUUtilsResources.postprocess_copyPointTexturePS);
}


void NGPU::NUtils::CPostprocessor::CopyLinearTexture(const NGPU::STexture& output, const NGPU::STexture& input)
{
	Quad(output, input, gGPUUtilsResources.postprocess_copyLinearTexturePS);
}


void NGPU::NUtils::CPostprocessor::CopyPointTexture(const NGPU::STexture& output, const NGPU::STexture& input, const SVector2& scale, const SVector2& offset)
{
	float data[4] = { scale.x, scale.y, offset.x, offset.y };
	deviceContext->UpdateSubresource(gGPUUtilsResources.ConstantBuffer(1, "gpu_utils_pp").buffer, 0, nullptr, data, 0, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &gGPUUtilsResources.ConstantBuffer(1, "gpu_utils_pp").buffer);

	Quad(output, input, gGPUUtilsResources.postprocess_copyPointTextureWithScaleAndOffsetPS);
}


void NGPU::NUtils::CPostprocessor::CopyLinearTexture(const NGPU::STexture& output, const NGPU::STexture& input, const SVector2& scale, const SVector2& offset)
{
	float data[4] = { scale.x, scale.y, offset.x, offset.y };
	deviceContext->UpdateSubresource(gGPUUtilsResources.ConstantBuffer(1, "gpu_utils_pp").buffer, 0, nullptr, data, 0, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &gGPUUtilsResources.ConstantBuffer(1, "gpu_utils_pp").buffer);

	Quad(output, input, gGPUUtilsResources.postprocess_copyLinearTextureWithScaleAndOffsetPS);
}


void NGPU::NUtils::CPostprocessor::Luma(const NGPU::STexture& output, const NGPU::STexture& input)
{
	Quad(output, input, gGPUUtilsResources.postprocess_lumaPS);
}


void NGPU::NUtils::CPostprocessor::Add(const NGPU::STexture& output, const NGPU::STexture& input, const NMath::SVector4& value)
{
	deviceContext->UpdateSubresource(gGPUUtilsResources.ConstantBuffer(1, "gpu_utils_pp").buffer, 0, nullptr, &value, 0, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &gGPUUtilsResources.ConstantBuffer(1, "gpu_utils_pp").buffer);

	Quad(output, input, gGPUUtilsResources.postprocess_addPS);
}


void NGPU::NUtils::CPostprocessor::Add(const NGPU::STexture& output, const NGPU::STexture& input, float value)
{
	Add(output, input, VectorCustom(value, value, value, value));
}


void NGPU::NUtils::CPostprocessor::Mul(const NGPU::STexture& output, const NGPU::STexture& input, const NMath::SVector4& value)
{
	deviceContext->UpdateSubresource(gGPUUtilsResources.ConstantBuffer(1, "gpu_utils_pp").buffer, 0, nullptr, &value, 0, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &gGPUUtilsResources.ConstantBuffer(1, "gpu_utils_pp").buffer);

	Quad(output, input, gGPUUtilsResources.postprocess_mulPS);
}


void NGPU::NUtils::CPostprocessor::Mul(const NGPU::STexture& output, const NGPU::STexture& input, float value)
{
	Mul(output, input, VectorCustom(value, value, value, value));
}


void NGPU::NUtils::CPostprocessor::Pow(const NGPU::STexture& output, const NGPU::STexture& input, const NMath::SVector4& value)
{
	deviceContext->UpdateSubresource(gGPUUtilsResources.ConstantBuffer(1, "gpu_utils_pp").buffer, 0, nullptr, &value, 0, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &gGPUUtilsResources.ConstantBuffer(1, "gpu_utils_pp").buffer);

	Quad(output, input, gGPUUtilsResources.postprocess_powPS);
}


void NGPU::NUtils::CPostprocessor::Pow(const NGPU::STexture& output, const NGPU::STexture& input, float value)
{
	Pow(output, input, VectorCustom(value, value, value, value));
}


void NGPU::NUtils::CPostprocessor::MinX(const NGPU::STexture& output, const NGPU::STexture& input, byte channelsCount, const SVector2& pixelSize, int32 from, int32 to)
{
	uint8 data[16];
	memcpy(&data[0], &pixelSize, 8);
	memcpy(&data[8], &from, 4);
	memcpy(&data[12], &to, 4);
	deviceContext->UpdateSubresource(gGPUUtilsResources.ConstantBuffer(1, "gpu_utils_pp").buffer, 0, nullptr, data, 0, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &gGPUUtilsResources.ConstantBuffer(1, "gpu_utils_pp").buffer);

	if (from == -6 && to == 6)
		Quad(output, input, gGPUUtilsResources.postprocess_minX13PS[channelsCount - 1]);
	else
		Quad(output, input, gGPUUtilsResources.postprocess_minXPS[channelsCount - 1]);
}


void NGPU::NUtils::CPostprocessor::MinY(const NGPU::STexture& output, const NGPU::STexture& input, byte channelsCount, const SVector2& pixelSize, int32 from, int32 to)
{
	uint8 data[16];
	memcpy(&data[0], &pixelSize, 8);
	memcpy(&data[8], &from, 4);
	memcpy(&data[12], &to, 4);
	deviceContext->UpdateSubresource(gGPUUtilsResources.ConstantBuffer(1, "gpu_utils_pp").buffer, 0, nullptr, data, 0, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &gGPUUtilsResources.ConstantBuffer(1, "gpu_utils_pp").buffer);

	if (from == -6 && to == 6)
		Quad(output, input, gGPUUtilsResources.postprocess_minY13PS[channelsCount - 1]);
	else
		Quad(output, input, gGPUUtilsResources.postprocess_minYPS[channelsCount - 1]);
}


void NGPU::NUtils::CPostprocessor::Min(const NGPU::STexture& output, const NGPU::STexture& input, byte channelsCount, const NMath::SVector2& pixelSize, int32 from, int32 to)
{
	TRenderTarget tempRT = gGPUUtilsResources.RenderTarget(output.width, output.height, output.format);

	MinX(tempRT, input, channelsCount, pixelSize, from, to);
	MinY(output, tempRT, channelsCount, pixelSize, from, to);
}


void NGPU::NUtils::CPostprocessor::MaxX(const NGPU::STexture& output, const NGPU::STexture& input, byte channelsCount, const SVector2& pixelSize, int32 from, int32 to)
{
	uint8 data[16];
	memcpy(&data[0], &pixelSize, 8);
	memcpy(&data[8], &from, 4);
	memcpy(&data[12], &to, 4);
	deviceContext->UpdateSubresource(gGPUUtilsResources.ConstantBuffer(1, "gpu_utils_pp").buffer, 0, nullptr, data, 0, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &gGPUUtilsResources.ConstantBuffer(1, "gpu_utils_pp").buffer);

	if (from == -6 && to == 6)
		Quad(output, input, gGPUUtilsResources.postprocess_maxX13PS[channelsCount - 1]);
	else
		Quad(output, input, gGPUUtilsResources.postprocess_maxXPS[channelsCount - 1]);
}


void NGPU::NUtils::CPostprocessor::MaxY(const NGPU::STexture& output, const NGPU::STexture& input, byte channelsCount, const SVector2& pixelSize, int32 from, int32 to)
{
	uint8 data[16];
	memcpy(&data[0], &pixelSize, 8);
	memcpy(&data[8], &from, 4);
	memcpy(&data[12], &to, 4);
	deviceContext->UpdateSubresource(gGPUUtilsResources.ConstantBuffer(1, "gpu_utils_pp").buffer, 0, nullptr, data, 0, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &gGPUUtilsResources.ConstantBuffer(1, "gpu_utils_pp").buffer);

	if (from == -6 && to == 6)
		Quad(output, input, gGPUUtilsResources.postprocess_maxY13PS[channelsCount - 1]);
	else
		Quad(output, input, gGPUUtilsResources.postprocess_maxYPS[channelsCount - 1]);
}


void NGPU::NUtils::CPostprocessor::Max(const NGPU::STexture& output, const NGPU::STexture& input, byte channelsCount, const NMath::SVector2& pixelSize, int32 from, int32 to)
{
	TRenderTarget tempRT = gGPUUtilsResources.RenderTarget(output.width, output.height, output.format);

	MaxX(tempRT, input, channelsCount, pixelSize, from, to);
	MaxY(output, tempRT, channelsCount, pixelSize, from, to);
}


void NGPU::NUtils::CPostprocessor::BlurX(const NGPU::STexture& output, const NGPU::STexture& input, byte channelsCount, const SVector2& pixelSize, int32 from, int32 to)
{
	uint8 data[16];
	memcpy(&data[0], &pixelSize, 8);
	memcpy(&data[8], &from, 4);
	memcpy(&data[12], &to, 4);
	deviceContext->UpdateSubresource(gGPUUtilsResources.ConstantBuffer(1, "gpu_utils_pp").buffer, 0, nullptr, data, 0, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &gGPUUtilsResources.ConstantBuffer(1, "gpu_utils_pp").buffer);

	if (from == -6 && to == 6)
		Quad(output, input, gGPUUtilsResources.postprocess_blurX13PS[channelsCount - 1]);
	else
		Quad(output, input, gGPUUtilsResources.postprocess_blurXPS[channelsCount - 1]);
}


void NGPU::NUtils::CPostprocessor::BlurY(const NGPU::STexture& output, const NGPU::STexture& input, byte channelsCount, const SVector2& pixelSize, int32 from, int32 to)
{
	uint8 data[16];
	memcpy(&data[0], &pixelSize, 8);
	memcpy(&data[8], &from, 4);
	memcpy(&data[12], &to, 4);
	deviceContext->UpdateSubresource(gGPUUtilsResources.ConstantBuffer(1, "gpu_utils_pp").buffer, 0, nullptr, data, 0, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &gGPUUtilsResources.ConstantBuffer(1, "gpu_utils_pp").buffer);

	if (from == -6 && to == 6)
		Quad(output, input, gGPUUtilsResources.postprocess_blurY13PS[channelsCount - 1]);
	else
		Quad(output, input, gGPUUtilsResources.postprocess_blurYPS[channelsCount - 1]);
}


void NGPU::NUtils::CPostprocessor::Blur(const NGPU::STexture& output, const NGPU::STexture& input, byte channelsCount, const NMath::SVector2& pixelSize, int32 from, int32 to)
{
	TRenderTarget tempRT = gGPUUtilsResources.RenderTarget(output.width, output.height, output.format);

	BlurX(tempRT, input, channelsCount, pixelSize, from, to);
	BlurY(output, tempRT, channelsCount, pixelSize, from, to);
}


void NGPU::NUtils::CPostprocessor::BlurGaussX7_Sigma3(const NGPU::STexture& output, const NGPU::STexture& input, byte channelsCount, const NMath::SVector2& pixelSize)
{
	uint8 data[16];
	memcpy(&data[0], &pixelSize, 8);
	deviceContext->UpdateSubresource(gGPUUtilsResources.ConstantBuffer(1, "gpu_utils_pp").buffer, 0, nullptr, data, 0, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &gGPUUtilsResources.ConstantBuffer(1, "gpu_utils_pp").buffer);

	Quad(output, input, gGPUUtilsResources.postprocess_blurGaussXPS[channelsCount - 1]);
}


void NGPU::NUtils::CPostprocessor::BlurGaussY7_Sigma3(const NGPU::STexture& output, const NGPU::STexture& input, byte channelsCount, const NMath::SVector2& pixelSize)
{
	uint8 data[16];
	memcpy(&data[0], &pixelSize, 8);
	deviceContext->UpdateSubresource(gGPUUtilsResources.ConstantBuffer(1, "gpu_utils_pp").buffer, 0, nullptr, data, 0, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &gGPUUtilsResources.ConstantBuffer(1, "gpu_utils_pp").buffer);

	Quad(output, input, gGPUUtilsResources.postprocess_blurGaussYPS[channelsCount - 1]);
}


void NGPU::NUtils::CPostprocessor::BlurGauss7_Sigma3(const NGPU::STexture& output, const NGPU::STexture& input, byte channelsCount, const NMath::SVector2& pixelSize)
{
	TRenderTarget tempRT = gGPUUtilsResources.RenderTarget(output.width, output.height, output.format);

	BlurGaussX7_Sigma3(tempRT, input, channelsCount, pixelSize);
	BlurGaussY7_Sigma3(output, tempRT, channelsCount, pixelSize);
}


void NGPU::NUtils::CPostprocessor::DownsampleMin2x2(const NGPU::STexture& output, const NGPU::STexture& input)
{
	SVector2 pixelSize = output.PixelSize();
	uint8 data[16];
	memcpy(&data[0], &pixelSize, 8);
	deviceContext->UpdateSubresource(gGPUUtilsResources.ConstantBuffer(1, "gpu_utils_pp").buffer, 0, nullptr, data, 0, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &gGPUUtilsResources.ConstantBuffer(1, "gpu_utils_pp").buffer);

	Quad(output, input, gGPUUtilsResources.downsampleMin2x2PS);
}


void NGPU::NUtils::CPostprocessor::DownsampleMin4x4(const NGPU::STexture& output, const NGPU::STexture& input)
{
	SVector2 pixelSize = output.PixelSize();
	uint8 data[16];
	memcpy(&data[0], &pixelSize, 8);
	deviceContext->UpdateSubresource(gGPUUtilsResources.ConstantBuffer(1, "gpu_utils_pp").buffer, 0, nullptr, data, 0, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &gGPUUtilsResources.ConstantBuffer(1, "gpu_utils_pp").buffer);

	Quad(output, input, gGPUUtilsResources.downsampleMin4x4PS);
}


void NGPU::NUtils::CPostprocessor::DownsampleMax2x2(const NGPU::STexture& output, const NGPU::STexture& input)
{
	SVector2 pixelSize = output.PixelSize();
	uint8 data[16];
	memcpy(&data[0], &pixelSize, 8);
	deviceContext->UpdateSubresource(gGPUUtilsResources.ConstantBuffer(1, "gpu_utils_pp").buffer, 0, nullptr, data, 0, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &gGPUUtilsResources.ConstantBuffer(1, "gpu_utils_pp").buffer);

	Quad(output, input, gGPUUtilsResources.downsampleMax2x2PS);
}


void NGPU::NUtils::CPostprocessor::DownsampleMax4x4(const NGPU::STexture& output, const NGPU::STexture& input)
{
	SVector2 pixelSize = output.PixelSize();
	uint8 data[16];
	memcpy(&data[0], &pixelSize, 8);
	deviceContext->UpdateSubresource(gGPUUtilsResources.ConstantBuffer(1, "gpu_utils_pp").buffer, 0, nullptr, data, 0, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &gGPUUtilsResources.ConstantBuffer(1, "gpu_utils_pp").buffer);

	Quad(output, input, gGPUUtilsResources.downsampleMax4x4PS);
}


void NGPU::NUtils::CPostprocessor::Deproject(const NGPU::STexture& output, const NGPU::STexture& input, const NMath::SMatrix& projTrasnform)
{
	SVector2 projParams = VectorCustom(projTrasnform.m[2][2], projTrasnform.m[3][2]);
	deviceContext->UpdateSubresource(gGPUUtilsResources.ConstantBuffer(1, "gpu_utils_pp").buffer, 0, nullptr, &projParams, 0, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &gGPUUtilsResources.ConstantBuffer(1, "gpu_utils_pp").buffer);

	Quad(output, input, gGPUUtilsResources.deprojectPS);
}


void NGPU::NUtils::CPostprocessor::Reproject(const NGPU::STexture& input, const NMath::SMatrix& reprojectTransform, const NGPU::STexture& output)
{
	float data[16 + 4];
	memcpy(&data, &reprojectTransform, sizeof(reprojectTransform));
	data[16] = (float)input.width;
	data[17] = (float)input.height;
	deviceContext->UpdateSubresource(gGPUUtilsResources.ConstantBuffer(5, "gpu_utils_pp").buffer, 0, nullptr, data, 0, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &gGPUUtilsResources.ConstantBuffer(5, "gpu_utils_pp").buffer);

	Quad(output, input, gGPUUtilsResources.reprojectPS);
}


void NGPU::NUtils::CPostprocessor::Resolve(const NGPU::STexture& output, const NGPU::STexture& input, int ssScale, int msaaCount)
{
	int32 data[4] = { ssScale, msaaCount, 0, 0 };
	deviceContext->UpdateSubresource(gGPUUtilsResources.ConstantBuffer(1, "gpu_utils_pp").buffer, 0, nullptr, data, 0, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &gGPUUtilsResources.ConstantBuffer(1, "gpu_utils_pp").buffer);

	if (msaaCount == 1)
		Quad(output, input, gGPUUtilsResources.resolvePS);
	else
		Quad(output, input, gGPUUtilsResources.resolvePS_msaa);
}


void NGPU::NUtils::CPostprocessor::Downsample(const NGPU::STexture& input, int scale, const NGPU::STexture& output)
{
	Resolve(output, input, scale, 1);
}


void NGPU::NUtils::CPostprocessor::PixelCopy(const NGPU::STexture& dstTexture, const NGPU::STexture& srcTexture, int width, int height)
{
	uint data[] = { width, height, 0, 0 };
	deviceContext->UpdateSubresource(gGPUUtilsResources.ConstantBuffer(1, "gpu_utils_pp").buffer, 0, nullptr, data, 0, 0);
	deviceContext->CSSetConstantBuffers(0, 1, &gGPUUtilsResources.ConstantBuffer(1, "gpu_utils_pp").buffer);

	deviceContext->CSSetUnorderedAccessViews(0, 1, &dstTexture.uav, nullptr);
	deviceContext->CSSetShaderResources(0, 1, &srcTexture.srv);
	deviceContext->CSSetShader(gGPUUtilsResources.pixelCopyCS, NULL, 0);
	deviceContext->Dispatch(Pad(width, 8) / 8, Pad(height, 8) / 8, 1);
}


// FontRenderer


const int NGPU::NUtils::CFontRenderer::cMaxCharsCountPerPrint = 1024;


void NGPU::NUtils::CFontRenderer::Create(int screenWidth, int screenHeight)
{
	this->screenWidth = screenWidth;
	this->screenHeight = screenHeight;

	_buffer.resize(cMaxCharsCountPerPrint * 4);
	CreateVertexBuffer(true, nullptr, cMaxCharsCountPerPrint * 4 * sizeof(SVertex_PosUV), _vb);
	// ib
	{
		uint16* indices = new uint16[cMaxCharsCountPerPrint * 6];
		for (int i = 0; i < cMaxCharsCountPerPrint; i++)
		{
			indices[6*i + 0] = 4*i + 0;
			indices[6*i + 1] = 4*i + 1;
			indices[6*i + 2] = 4*i + 2;
			indices[6*i + 3] = 4*i + 0;
			indices[6*i + 4] = 4*i + 2;
			indices[6*i + 5] = 4*i + 3;
		}

		CreateIndexBuffer(false, (uint8*)indices, cMaxCharsCountPerPrint * 6 * sizeof(uint16), _ib);

		delete[] indices;
	}
}


void NGPU::NUtils::CFontRenderer::Destroy()
{
	DestroyBuffer(_vb);
	DestroyBuffer(_ib);
}


void NGPU::NUtils::CFontRenderer::Print(const string& text, float posX, float posY, float charWidth, float charHeight, const NMath::SVector4& color)
{
	struct SUtils
	{
		SVector4 ASCIIToTexCoord(uint8 theChar)
		{
			int x = theChar % 16;
			int y = theChar / 16;

			SVector2 min, max;
			min.x = (float)x / 16.0f;
			min.y = (float)y / 16.0f;
			max.x = min.x + 1.0f/16.0f;
			max.y = min.y + 1.0f/16.0f;

			SVector4 texCoords;
			texCoords.x = min.x;
			texCoords.y = 1.0f - min.y;
			texCoords.z = max.x;
			texCoords.w = 1.0f - max.y;

			return texCoords;
		}
	} utils;

	int textLength = (int)text.size();
	MF_ASSERT(textLength <= cMaxCharsCountPerPrint);

	for (int i = 0; i < textLength; i++)
	{
		float offsetX = posX + i*charWidth;
		float offsetY = posY;

		_buffer[4*i + 0].position = VectorCustom(offsetX, offsetY, 0.5f);
		_buffer[4*i + 1].position = VectorCustom(offsetX, offsetY + charHeight, 0.5f);
		_buffer[4*i + 2].position = VectorCustom(offsetX + charWidth, offsetY + charHeight, 0.5f);
		_buffer[4*i + 3].position = VectorCustom(offsetX + charWidth, offsetY, 0.5f);

		SVector4 texCoords = utils.ASCIIToTexCoord(text[i]);
		_buffer[4*i + 0].texCoord = VectorCustom(texCoords.x, texCoords.y);
		_buffer[4*i + 1].texCoord = VectorCustom(texCoords.x, texCoords.w);
		_buffer[4*i + 2].texCoord = VectorCustom(texCoords.z, texCoords.w);
		_buffer[4*i + 3].texCoord = VectorCustom(texCoords.z, texCoords.y);
	}
	UpdateBuffer(_vb, (uint8*)&_buffer[0], textLength * 4 * sizeof(SVertex_PosUV));

	deviceContext->VSSetShader(gGPUUtilsResources.fontVS, nullptr, 0);
	deviceContext->PSSetShader(gGPUUtilsResources.fontPS, nullptr, 0);
	SetPSSamplers();
	deviceContext->PSSetShaderResources(0, 1, &gGPUUtilsResources.fontTexture.texture.srv);

	UINT stride = sizeof(SVertex_PosUV);
	UINT offset = 0;
	deviceContext->IASetInputLayout(gGPUUtilsResources.posUVInputLayout);
	deviceContext->IASetVertexBuffers(0, 1, &_vb.buffer, &stride, &offset);
	deviceContext->IASetIndexBuffer(_ib.buffer, DXGI_FORMAT_R16_UINT, 0);

	float data[4] = { (float)screenWidth, (float)screenHeight, 0.0f, 0.0f };
	uint32 color2 = Vector4ToUInt32(color);
	memcpy(data + 2, &color2, 4);
	deviceContext->UpdateSubresource(gGPUUtilsResources.ConstantBuffer(1, "gpu_utils_font").buffer, 0, nullptr, data, 0, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &gGPUUtilsResources.ConstantBuffer(1, "gpu_utils_font").buffer);
	deviceContext->PSSetConstantBuffers(0, 1, &gGPUUtilsResources.ConstantBuffer(1, "gpu_utils_font").buffer);

	if (color.w < 1.0f)
		deviceContext->OMSetBlendState(gGPUUtilsResources.transparencyBlendState, 0, 0xFFFFFFFF);
	deviceContext->DrawIndexed(textLength * 6, 0, 0);
	deviceContext->OMSetBlendState(gGPUUtilsResources.defaultBlendState, 0, 0xFFFFFFFF);
}


//


void NGPU::NUtils::CDebugRenderer::Create(const string& frameworkPath)
{
	MF_ASSERT(boxMesh.CreateFromFile(frameworkPath + "/data/meshes/unit_box.ase", CMesh::EFileType::ASE));
}


void NGPU::NUtils::CDebugRenderer::Destroy()
{
	boxMesh.Destroy();
}


void NGPU::NUtils::CDebugRenderer::RenderBox(const NMath::SMatrix& worldTransform, const NMath::SMatrix& viewProjTransform, const NMath::SVector4& color)
{
	MF_ASSERT(boxMesh.chunks.size() > 0);

	deviceContext->VSSetShader(gGPUUtilsResources.debugMeshVS, nullptr, 0);
	deviceContext->PSSetShader(gGPUUtilsResources.debugMeshPS, nullptr, 0);

	deviceContext->IASetInputLayout(gGPUUtilsResources.posNorUV0UV1InputLayout);

	struct Data
	{
		SMatrix worldViewProjTransform;
		uint32 color;
	} data;
	data.worldViewProjTransform = worldTransform * viewProjTransform;
	data.color = Vector4ToUInt32(color);
	deviceContext->UpdateSubresource(gGPUUtilsResources.ConstantBuffer(5, "gpu_utils_debug").buffer, 0, nullptr, &data, 0, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &gGPUUtilsResources.ConstantBuffer(5, "gpu_utils_debug").buffer);
	deviceContext->PSSetConstantBuffers(0, 1, &gGPUUtilsResources.ConstantBuffer(5, "gpu_utils_debug").buffer);

	if (color.w < 1.0f)
		deviceContext->OMSetBlendState(gGPUUtilsResources.transparencyBlendState, 0, 0xFFFFFFFF);
	boxMesh.Render();
	deviceContext->OMSetBlendState(gGPUUtilsResources.defaultBlendState, 0, 0xFFFFFFFF);
}


//


void NGPU::NUtils::SetPSSamplers()
{
	deviceContext->PSSetSamplers(0, 1, &gGPUUtilsResources.pointClampSamplerState);
	deviceContext->PSSetSamplers(1, 1, &gGPUUtilsResources.linearClampSamplerState);
	deviceContext->PSSetSamplers(2, 1, &gGPUUtilsResources.anisotropicWrapSamplerState);
	deviceContext->PSSetSamplers(3, 1, &gGPUUtilsResources.pointClampComparisonSamplerState);
	deviceContext->PSSetSamplers(4, 1, &gGPUUtilsResources.linearClampComparisonSamplerState);
	deviceContext->PSSetSamplers(5, 1, &gGPUUtilsResources.anisotropicClampComparisonSamplerState);
	deviceContext->PSSetSamplers(6, 1, &gGPUUtilsResources.pointBorderGreySamplerState);
	deviceContext->PSSetSamplers(7, 1, &gGPUUtilsResources.linearBorderGreySamplerState);
	deviceContext->PSSetSamplers(8, 1, &gGPUUtilsResources.pointWrapSamplerState);
	deviceContext->PSSetSamplers(9, 1, &gGPUUtilsResources.linearWrapSamplerState);
}


void NGPU::NUtils::SetCSSamplers()
{
	deviceContext->CSSetSamplers(0, 1, &gGPUUtilsResources.pointClampSamplerState);
	deviceContext->CSSetSamplers(1, 1, &gGPUUtilsResources.linearClampSamplerState);
	deviceContext->CSSetSamplers(2, 1, &gGPUUtilsResources.anisotropicWrapSamplerState);
	deviceContext->CSSetSamplers(3, 1, &gGPUUtilsResources.pointClampComparisonSamplerState);
	deviceContext->CSSetSamplers(4, 1, &gGPUUtilsResources.linearClampComparisonSamplerState);
	deviceContext->CSSetSamplers(5, 1, &gGPUUtilsResources.anisotropicClampComparisonSamplerState);
	deviceContext->CSSetSamplers(6, 1, &gGPUUtilsResources.pointBorderGreySamplerState);
	deviceContext->CSSetSamplers(7, 1, &gGPUUtilsResources.linearBorderGreySamplerState);
	deviceContext->CSSetSamplers(8, 1, &gGPUUtilsResources.pointWrapSamplerState);
	deviceContext->CSSetSamplers(9, 1, &gGPUUtilsResources.linearWrapSamplerState);
}


void NGPU::NUtils::SetSamplers()
{
	SetPSSamplers();
	SetCSSamplers();
}


void NGPU::NUtils::PrefixSum(int elementsCount, const NGPU::SBuffer& inputBuffer, const NGPU::SBuffer& outputBuffer)
{
	MF_ASSERT(elementsCount % gGPUUtilsResources.cPrefixSumBatchSize == 0);

	//

	ID3D11UnorderedAccessView* nullUAVs2[] = { nullptr, nullptr };
	ID3D11ShaderResourceView* nullSRVs2[] = { nullptr, nullptr };

	uint32 clearValues[] = { 0, 0, 0, 0 };
	deviceContext->ClearUnorderedAccessViewUint(gGPUUtilsResources.prefixSumBuffer_blocksSums.uav, clearValues);

	int batchesCount = elementsCount / gGPUUtilsResources.cPrefixSumBatchSize;
	MF_ASSERT(batchesCount <= gGPUUtilsResources.cPrefixSumBatchSize);

	// prefix sum of each block

	deviceContext->CSSetUnorderedAccessViews(0, 1, &outputBuffer.uav, nullptr);
	deviceContext->CSSetUnorderedAccessViews(1, 1, &gGPUUtilsResources.prefixSumBuffer_blocksSums.uav, nullptr);
	deviceContext->CSSetShaderResources(0, 1, &inputBuffer.srv);
	deviceContext->CSSetShader(gGPUUtilsResources.prefixSumCS_prefixSum_writeBlocksSums, NULL, 0);
	deviceContext->Dispatch(batchesCount, 1, 1);

	deviceContext->CSSetUnorderedAccessViews(0, 2, nullUAVs2, nullptr);
	deviceContext->CSSetShaderResources(0, 2, nullSRVs2);

	// prefix sum of blocks prefix sums

	deviceContext->CSSetUnorderedAccessViews(0, 1, &gGPUUtilsResources.prefixSumBuffer_cumulativeBlocksSums.uav, nullptr);
	deviceContext->CSSetShaderResources(0, 1, &gGPUUtilsResources.prefixSumBuffer_blocksSums.srv);
	deviceContext->CSSetShader(gGPUUtilsResources.prefixSumCS_prefixSum, NULL, 0);
	deviceContext->Dispatch(1, 1, 1);

	deviceContext->CSSetUnorderedAccessViews(0, 2, nullUAVs2, nullptr);
	deviceContext->CSSetShaderResources(0, 2, nullSRVs2);

	// add "prefix sum of blocks prefix sums" to "prefix sum of each block" to get prefix sum for the entire input buffer

	deviceContext->CSSetUnorderedAccessViews(0, 1, &outputBuffer.uav, nullptr);
	deviceContext->CSSetShaderResources(0, 1, &gGPUUtilsResources.prefixSumBuffer_cumulativeBlocksSums.srv);
	deviceContext->CSSetShader(gGPUUtilsResources.prefixSumCS_sum, NULL, 0);
	deviceContext->Dispatch(batchesCount - 1, 1, 1);

	deviceContext->CSSetUnorderedAccessViews(0, 2, nullUAVs2, nullptr);
	deviceContext->CSSetShaderResources(0, 2, nullSRVs2);
}


void NGPU::NUtils::DumpGPUTextureToFile(const NGPU::STexture& texture, const string& path)
{
	uint8* data;
	CopyTextureFromGPUToCPU(texture, nullptr, data);

	SImage image = NImage::Create(texture.width, texture.height, NImage::EFormat::RGBA8);
	memcpy(image.data, data, 4 * image.width * image.height);
	NImage::Save(path, image, false, true);
	NImage::Destroy(image);

	delete[] data;
}


// UnityLightmap & UnityLightmapGroup


bool NGPU::NUtils::CUnityLightmap::CreateFromFile(const string& dir, int index)
{
	string path = dir + "/Lightmap-" + ToString(index) + "_comp_";

	if (!colorTexture.CreateFromFile(path + "light.exr.png", false, false, true))
		return false;
	if (!dirTexture.CreateFromFile(path + "dir.png", false, false, true))
		return false;

	return true;
}


void NGPU::NUtils::CUnityLightmap::Destroy()
{
	colorTexture.Destroy();
	dirTexture.Destroy();
}


bool NGPU::NUtils::CUnityLightmapGroup::CreateFromFile(const string& dir, int count)
{
	lightmaps.resize(count);

	for (int i = 0; i < count; i++)
	{
		if (!lightmaps[i].CreateFromFile(dir, i))
			return false;
	}

	return true;
}


void NGPU::NUtils::CUnityLightmapGroup::Destroy()
{
	for (uint i = 0; i < lightmaps.size(); i++)
		lightmaps[i].Destroy();

	lightmaps.clear();
}


#endif
