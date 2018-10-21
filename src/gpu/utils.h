#pragma once
#ifdef MAXEST_FRAMEWORK_WINDOWS


#include "types.h"
#include "../essentials/main.h"
#include "../math/main.h"
#include "../image/types.h"

#include <d3d11_1.h>


namespace NMaxestFramework { namespace NGPU { namespace NUtils
{
	struct SVertex_Pos
	{
		NMath::SVector3 position;
	};

	struct SVertex_PosUV
	{
		NMath::SVector3 position;
		NMath::SVector2 texCoord;
	};

	struct SVertex_PosNorUV
	{
		NMath::SVector3 position;
		NMath::SVector3 normal;
		NMath::SVector2 texCoord;
	};

	struct SVertex_PosNorUV0UV1
	{
		NMath::SVector3 position;
		NMath::SVector3 normal;
		NMath::SVector2 texCoord0;
		NMath::SVector2 texCoord1;
	};

	//

	class CTexture
	{
	public:
		bool CreateFromFile(const string& path, bool swapChannels, bool invertY, bool generateMipmaps);
		void Destroy();

		bool CreateCPUDataFromSourceFile(const string& path, bool swapChannels, bool invertY, bool generateMipmaps);
		bool CreateCPUDataFromCacheFile(const string& path);
		void CreateGPUData();
		void DestroyCPUData();
		void DestroyGPUData();

		bool SaveCPUDataToCacheFile(const string& path);

	public: // readonly
		vector<NImage::SImage> mipmaps;
		NGPU::STexture texture;
	};

	//

	class CMesh
	{
	public:
		enum class EFileType { OBJ, ASE, UnityExported };

		struct SChunk
		{
			uint32 verticesCount;
			uint32 indicesCount;
			SVertex_PosNorUV0UV1* vertices;
			uint16* indices;
			SBuffer vb;
			SBuffer ib;

			NMath::SAABB bbox;
			string textureFileName;
			uint8 lightmapIndex;

			SChunk();
			void Destroy();
		};

	public:
		bool CreateFromFile(const string& path, EFileType fileType, map<string, CTexture>* textures = nullptr);
		bool CreatePlane(float width, float height, const NMath::SVector2& uvScale, const NMath::SMatrix& transform = NMath::cMatrixIdentity);
		void Destroy();

		bool CreateCPUBuffersFromSourceFile(const string& path, EFileType fileType);
		bool CreateCPUBuffersFromCacheFile(const string& path);
		bool CreateCPUBuffersForPlane(float width, float height, const NMath::SVector2& uvScale, const NMath::SMatrix& transform = NMath::cMatrixIdentity);
		void CreateGPUBuffers();
		void CalculateBoundingBox();

		bool SaveCPUBuffersToCacheFile(const string& path);

		map<string, CTexture> CreateGPUTextures(const string& dir);

		void Render(int chunkIndex);
		void Render();

	public: // readonly
		uint32 lightmapsCount;
		vector<SChunk> chunks;
		NMath::SAABB bbox; // from all chunks
	};

	//

	class CResources
	{
	public:
		static const int cPrefixSumBatchSize;

	public:
		void Create(const string& frameworkPath);
		void Destroy();

		void CreateShaders(const string& frameworkPath);
		void DestroyShaders();

		SBuffer& ConstantBuffer(int registersCount, uint32 id = 0);
		STexture& Texture(int width, int height, DXGI_FORMAT format, uint32 id = 0);
		TRenderTarget& RenderTarget(int width, int height, DXGI_FORMAT format, uint32 id = 0);
		inline SBuffer& ConstantBuffer(int registersCount, const string& name)								{ return ConstantBuffer(registersCount, NEssentials::Murmur32(name.c_str(), (int)name.length(), 1)); }
		inline STexture& Texture(int width, int height, DXGI_FORMAT format, const string& name)				{ return Texture(width, height, format, NEssentials::Murmur32(name.c_str(), (int)name.length(), 1)); }
		inline TRenderTarget& RenderTarget(int width, int height, DXGI_FORMAT format, const string& name)	{ return RenderTarget(width, height, format, NEssentials::Murmur32(name.c_str(), (int)name.length(), 1)); }

		STexture CreateTexture2D_InterleavedGradientNoise(int width, int height, int mipmapsCount);

	public: // readonly
		// input layouts

		ID3D11InputLayout* posInputLayout;
		ID3D11InputLayout* posUVInputLayout;
		ID3D11InputLayout* posNorUVInputLayout;
		ID3D11InputLayout* posNorUV0UV1InputLayout;

		// buffers

		SBuffer screenQuadIB;

		SBuffer prefixSumBuffer_blocksSums;
		SBuffer prefixSumBuffer_cumulativeBlocksSums;

		// rasterizer states

		ID3D11RasterizerState* ccwRasterizerState;
		ID3D11RasterizerState* cwRasterizerState;

		// blend states

		ID3D11BlendState* defaultBlendState;
		ID3D11BlendState* transparencyBlendState;

		// samplers

		ID3D11SamplerState* pointClampSamplerState;
		ID3D11SamplerState* linearClampSamplerState;
		ID3D11SamplerState* anisotropicWrapSamplerState;
		ID3D11SamplerState* pointClampComparisonSamplerState;
		ID3D11SamplerState* linearClampComparisonSamplerState;
		ID3D11SamplerState* anisotropicClampComparisonSamplerState;
		ID3D11SamplerState* pointBorderGreySamplerState;
		ID3D11SamplerState* linearBorderGreySamplerState;
		ID3D11SamplerState* pointWrapSamplerState;
		ID3D11SamplerState* linearWrapSamplerState;

		// shaders

		ID3D11VertexShader* fontVS;
		ID3D11PixelShader* fontPS;

		ID3D11VertexShader* debugMeshVS;
		ID3D11PixelShader* debugMeshPS;

		ID3D11VertexShader* screenQuadVS;

		ID3D11PixelShader* postprocess_copyPointTexturePS;
		ID3D11PixelShader* postprocess_copyLinearTexturePS;
		ID3D11PixelShader* postprocess_copyPointTextureWithScaleAndOffsetPS;
		ID3D11PixelShader* postprocess_copyLinearTextureWithScaleAndOffsetPS;
		ID3D11PixelShader* postprocess_lumaPS;
		ID3D11PixelShader* postprocess_addPS;
		ID3D11PixelShader* postprocess_mulPS;
		ID3D11PixelShader* postprocess_powPS;
		ID3D11PixelShader* postprocess_minXPS[4];
		ID3D11PixelShader* postprocess_minYPS[4];
		ID3D11PixelShader* postprocess_minX13PS[4];
		ID3D11PixelShader* postprocess_minY13PS[4];
		ID3D11PixelShader* postprocess_maxXPS[4];
		ID3D11PixelShader* postprocess_maxYPS[4];
		ID3D11PixelShader* postprocess_maxX13PS[4];
		ID3D11PixelShader* postprocess_maxY13PS[4];
		ID3D11PixelShader* postprocess_blurXPS[4];
		ID3D11PixelShader* postprocess_blurYPS[4];
		ID3D11PixelShader* postprocess_blurX13PS[4];
		ID3D11PixelShader* postprocess_blurY13PS[4];
		ID3D11PixelShader* postprocess_blurGaussXPS[4];
		ID3D11PixelShader* postprocess_blurGaussYPS[4];

		ID3D11PixelShader* downsampleMin2x2PS;
		ID3D11PixelShader* downsampleMin4x4PS;
		ID3D11PixelShader* downsampleMax2x2PS;
		ID3D11PixelShader* downsampleMax4x4PS;

		ID3D11PixelShader* deprojectPS;
		ID3D11PixelShader* reprojectPS;

		ID3D11PixelShader* resolvePS;
		ID3D11PixelShader* resolvePS_msaa;

		ID3D11ComputeShader* pixelCopyCS;

		ID3D11ComputeShader* prefixSumCS_prefixSum;
		ID3D11ComputeShader* prefixSumCS_prefixSum_writeBlocksSums;
		ID3D11ComputeShader* prefixSumCS_sum;

		// helper resources

		vector<SBuffer> constantBuffers;
		vector<STexture> textures;
		vector<TRenderTarget> renderTargets;

		// textures

		CTexture fontTexture;
	};

	//

	class CPostprocessor
	{
	public:
		void Quad(const NGPU::STexture& output, const NGPU::STexture& input, ID3D11PixelShader* pixelShader);
		void CopyPointTexture(const NGPU::STexture& output, const NGPU::STexture& input);
		void CopyLinearTexture(const NGPU::STexture& output, const NGPU::STexture& input);
		void CopyPointTexture(const NGPU::STexture& output, const NGPU::STexture& input, const NMath::SVector2& scale, const NMath::SVector2& offset);
		void CopyLinearTexture(const NGPU::STexture& output, const NGPU::STexture& input, const NMath::SVector2& scale, const NMath::SVector2& offset);
		void Luma(const NGPU::STexture& output, const NGPU::STexture& input);
		void Add(const NGPU::STexture& output, const NGPU::STexture& input, const NMath::SVector4& value);
		void Add(const NGPU::STexture& output, const NGPU::STexture& input, float value);
		void Mul(const NGPU::STexture& output, const NGPU::STexture& input, const NMath::SVector4& value);
		void Mul(const NGPU::STexture& output, const NGPU::STexture& input, float value);
		void Pow(const NGPU::STexture& output, const NGPU::STexture& input, const NMath::SVector4& value);
		void Pow(const NGPU::STexture& output, const NGPU::STexture& input, float value);
		void MinX(const NGPU::STexture& output, const NGPU::STexture& input, byte channelsCount, const NMath::SVector2& pixelSize, int32 from, int32 to);
		void MinY(const NGPU::STexture& output, const NGPU::STexture& input, byte channelsCount, const NMath::SVector2& pixelSize, int32 from, int32 to);
		void Min(const NGPU::STexture& output, const NGPU::STexture& input, byte channelsCount, const NMath::SVector2& pixelSize, int32 from, int32 to);
		void MaxX(const NGPU::STexture& output, const NGPU::STexture& input, byte channelsCount, const NMath::SVector2& pixelSize, int32 from, int32 to);
		void MaxY(const NGPU::STexture& output, const NGPU::STexture& input, byte channelsCount, const NMath::SVector2& pixelSize, int32 from, int32 to);
		void Max(const NGPU::STexture& output, const NGPU::STexture& input, byte channelsCount, const NMath::SVector2& pixelSize, int32 from, int32 to);
		void BlurX(const NGPU::STexture& output, const NGPU::STexture& input, byte channelsCount, const NMath::SVector2& pixelSize, int32 from, int32 to);
		void BlurY(const NGPU::STexture& output, const NGPU::STexture& input, byte channelsCount, const NMath::SVector2& pixelSize, int32 from, int32 to);
		void Blur(const NGPU::STexture& output, const NGPU::STexture& input, byte channelsCount, const NMath::SVector2& pixelSize, int32 from, int32 to);
		void BlurGaussX7_Sigma3(const NGPU::STexture& output, const NGPU::STexture& input, byte channelsCount, const NMath::SVector2& pixelSize);
		void BlurGaussY7_Sigma3(const NGPU::STexture& output, const NGPU::STexture& input, byte channelsCount, const NMath::SVector2& pixelSize);
		void BlurGauss7_Sigma3(const NGPU::STexture& output, const NGPU::STexture& input, byte channelsCount, const NMath::SVector2& pixelSize);
		void DownsampleMin2x2(const NGPU::STexture& output, const NGPU::STexture& input);
		void DownsampleMin4x4(const NGPU::STexture& output, const NGPU::STexture& input);
		void DownsampleMax2x2(const NGPU::STexture& output, const NGPU::STexture& input);
		void DownsampleMax4x4(const NGPU::STexture& output, const NGPU::STexture& input);
		void Deproject(const NGPU::STexture& output, const NGPU::STexture& input, const NMath::SMatrix& projTrasnform);
		void Reproject(const NGPU::STexture& input, const NMath::SMatrix& reprojectTransform, const NGPU::STexture& output);
		void Resolve(const NGPU::STexture& output, const NGPU::STexture& input, int ssScale, int msaaCount);
		void Downsample(const NGPU::STexture& input, int scale, const NGPU::STexture& output);
		void PixelCopy(const NGPU::STexture& dstTexture, const NGPU::STexture& srcTexture, int width, int height);
	};

	//

	class CFontRenderer
	{
	public:
		static const int cMaxCharsCountPerPrint;

	public:
		void Create(int screenWidth, int screenHeight);
		void Destroy();

		void Print(const string& text, float posX, float posY, float charWidth, float charHeight, const NMath::SVector4& color = NMath::cVector4One);
		template<typename TYPE> void Print(TYPE text, float posX, float posY, float charWidth, float charHeight, const NMath::SVector4& color = NMath::cVector4One) { Print(ToString(text), posX, posY, charWidth, charHeight, color); }

	public:
		int screenWidth;
		int screenHeight;

	public: // readonly
		vector<SVertex_PosUV> _buffer;
		SBuffer _vb;
		SBuffer _ib;
	};

	//

	class CDebugRenderer
	{
	public:
		void Create(const string& frameworkPath);
		void Destroy();

		void RenderBox(const NMath::SMatrix& worldTransform, const NMath::SMatrix& viewProjTransform, const NMath::SVector4& color = NMath::cVector4One);
		inline void RenderBox(const NMath::SVector3& position, const NMath::SMatrix& viewProjTransform, const NMath::SVector4& color = NMath::cVector4One) { RenderBox(MatrixTranslate(position), viewProjTransform, color); }

	public: // readonly
		CMesh boxMesh;
	};

	//

	void SetPSSamplers();
	void SetCSSamplers();
	void SetSamplers();

	void PrefixSum(int elementsCount, const NGPU::SBuffer& inputBuffer, const NGPU::SBuffer& outputBuffer);

	void DumpGPUTextureToFile(const NGPU::STexture& texture, const string& path);

	//

	class CUnityLightmap
	{
	public:
		bool CreateFromFile(const string& dir, int index);
		void Destroy();

	public: // readonly
		CTexture colorTexture;
		CTexture dirTexture;
	};

	class CUnityLightmapGroup
	{
	public:
		bool CreateFromFile(const string& dir, int count);
		void Destroy();

	public: // readonly
		vector<CUnityLightmap> lightmaps;
	};
} } }


extern NMaxestFramework::NGPU::NUtils::CResources gGPUUtilsResources;
extern NMaxestFramework::NGPU::NUtils::CPostprocessor gGPUUtilsPostprocessor;
extern NMaxestFramework::NGPU::NUtils::CFontRenderer gGPUUtilsFontRenderer;
extern NMaxestFramework::NGPU::NUtils::CDebugRenderer gGPUUtilsDebugRenderer;


#endif
