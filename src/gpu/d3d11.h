#pragma once
#ifdef MAXEST_FRAMEWORK_WINDOWS


#include "types.h"
#include "../essentials/main.h"

#include <d3d11_1.h>
#include <d3dcompiler.h>


namespace NMaxestFramework { namespace NGPU
{
	namespace NShader
	{
		struct ShaderMacro
		{
			string name;
			string definition;
		};

		vector<ShaderMacro> ShaderMacrosFromString(const string& shaderMacros);
		string ShaderMacrosName(const string& baseName, const vector<ShaderMacro>& shaderMacros);
		D3D_SHADER_MACRO* ShaderMacros_D3D11(const vector<ShaderMacro>& shaderMacros); // you must free memory yourself
	}

	//

	enum class EShaderType { Vertex, Pixel, Compute };
	enum class EShaderFileType { Source, Blob };

	//

	extern D3D_FEATURE_LEVEL featureLevel;
	extern ID3D11Device* device;
	extern ID3D11Device1* device1;
	extern ID3D11DeviceContext* deviceContext;
	extern ID3D11DeviceContext1* deviceContext1;
	extern IDXGISwapChain* swapChain;
	extern IDXGISwapChain1* swapChain1;
	extern ID3D11RenderTargetView* backBufferRTV;

	extern EShaderFileType gGPUCreateShaderFileType;

	//

	void CreateD3D11(int width, int height, bool fullScreen, float fpsGuess = 60.0f, const string& adapterNameGuess = "");
	void DestroyD3D11();

	// will dump disassembly and blob files; also errors file if there are any
	bool CompileShaderFromSourceFile(const string& path, const string& entryPointName, const string& shaderModelName, const string& shaderMacros, ID3DBlob*& blob);

	void ClearRenderTargets();
	void ClearPSShaderResources(int offset, int count);
	void ClearCSShaderResources(int offset, int count);

	void CreateRenderTarget(int width, int height, DXGI_FORMAT format, int samplesCount, TRenderTarget& renderTarget);
	void CreateDepthStencilTarget(int width, int height, int samplesCount, TDepthStencilTarget& depthStencilTarget);
	void CreateDepthStencilTarget64(int width, int height, int samplesCount, TDepthStencilTarget& depthStencilTarget);
	void CreateTexture(int width, int height, int mipmapsCount, DXGI_FORMAT format, STexture& texture);
	void CreateTexture(int width, int height, STexture& texture);
	void CreateRWTexture(int width, int height, int mipmapsCount, DXGI_FORMAT format, STexture& texture);
	void CreateRWTexture3D(int width, int height, int depth, int mipmapsCount, DXGI_FORMAT format, STexture& texture);
	void CreateRWTextureArray(int width, int height, int arraysCount, DXGI_FORMAT format, STexture& texture);
	void CreateStagingTexture(const STexture& srcTexture, STexture& texture);
	void CreateStructuredBuffer(int dataSize, int dataStride, SBuffer& buffer);
	void CreateRWStructuredBuffer(int dataSize, int dataStride, SBuffer& buffer);
	void CreateStagingBuffer(const SBuffer& srcBuffer, SBuffer& buffer);
	bool CreateShaderBlob(EShaderFileType shaderFileType, EShaderType shaderType, const string& path, const string& entryPointName, const string& shaderMacros, ID3DBlob*& blob);
	bool CreateVertexShader(EShaderFileType shaderFileType, const string& path, const string& entryPointName, const string& shaderMacros, ID3D11VertexShader*& shader);
	bool CreatePixelShader(EShaderFileType shaderFileType, const string& path, const string& entryPointName, const string& shaderMacros, ID3D11PixelShader*& shader);
	bool CreateComputeShader(EShaderFileType shaderFileType, const string& path, const string& entryPointName, const string& shaderMacros, ID3D11ComputeShader*& shader);
	bool CreateInputLayout(const string& dummyVertexShaderPath, const string& dummyVertexShaderMacros, D3D11_INPUT_ELEMENT_DESC inputLayoutElements[], int inputLayoutElementsCount, ID3D11InputLayout*& inputLayout);
	void CreateVertexBuffer(bool dynamic, uint8* data, int dataSize, SBuffer& vertexBuffer);
	void CreateIndexBuffer(bool dynamic, uint8* data, int dataSize, SBuffer& indexBuffer);
	void CreateConstantBuffer(int dataSize, SBuffer& constantBuffer);
	void CreateRasterizerState(ID3D11RasterizerState*& rasterizerState, bool visibleIsCCW);
	void CreateDepthStencilState(ID3D11DepthStencilState*& depthStencilState, bool depthTest, bool stencilTest);
	void CreateBlendState(ID3D11BlendState*& blendState, bool transparency);
	void CreateSamplerState(ID3D11SamplerState*& samplerState, ESamplerFilter filter, ESamplerAddressing addressing, ESamplerComparisonFunction comparisonFunction);

	void DestroyRenderTarget(TRenderTarget& renderTarget);
	void DestroyDepthStencilTarget(TDepthStencilTarget& depthStencilTarget);
	void DestroyTexture(STexture& texture);
	void DestroyVertexShader(ID3D11VertexShader*& vertexShader);
	void DestroyPixelShader(ID3D11PixelShader*& pixelShader);
	void DestroyComputeShader(ID3D11ComputeShader*& computeShader);
	void DestroyInputLayout(ID3D11InputLayout*& inputLayout);
	void DestroyBuffer(SBuffer& buffer);
	void DestroyRasterizerState(ID3D11RasterizerState*& rasterizerState);
	void DestroyDepthStencilState(ID3D11DepthStencilState*& depthStencilState);
	void DestroyBlendState(ID3D11BlendState*& blendState);
	void DestroySamplerState(ID3D11SamplerState*& samplerState);

	void UpdateTexture(STexture& texture, int mipmapIndex, uint8* data, int rowPitch);
	void UpdateBuffer(SBuffer& buffer, uint8* data, int dataSize);
	void CopyTextureFromGPUToCPU(const STexture& srcTexture, STexture* stagingTexture, uint8*& dstData);
	void CopyBufferFromGPUToCPU(const SBuffer& srcBuffer, SBuffer* stagingBuffer, uint8*& dstData);
	void ResolveMSAA(const NGPU::STexture& input, const NGPU::STexture& output);

	void SetViewport(int x, int y, int width, int height);
	void SetViewport(int width, int height);
	void SetViewport(const STexture& texture);

	// a bunch of helper shader creation functions
	bool CreateVertexShader(const string& path, const string& entryPointName, const string& shaderMacros, ID3D11VertexShader*& shader);
	bool CreateVertexShader(const string& path, const string& shaderMacros, ID3D11VertexShader*& shader);
	bool CreateVertexShader(const string& path, ID3D11VertexShader*& shader);
	bool CreatePixelShader(const string& path, const string& entryPointName, const string& shaderMacros, ID3D11PixelShader*& shader);
	bool CreatePixelShader(const string& path, const string& shaderMacros, ID3D11PixelShader*& shader);
	bool CreatePixelShader(const string& path, ID3D11PixelShader*& shader);
	bool CreateComputeShader(const string& path, const string& entryPointName, const string& shaderMacros, ID3D11ComputeShader*& shader);
	bool CreateComputeShader(const string& path, const string& shaderMacros, ID3D11ComputeShader*& shader);
	bool CreateComputeShader(const string& path, ID3D11ComputeShader*& shader);
	bool CreateVertexShaderFromSourceFile(const string& path, const string& entryPointName, const string& shaderMacros, ID3D11VertexShader*& shader);
	bool CreateVertexShaderFromSourceFile(const string& path, const string& shaderMacros, ID3D11VertexShader*& shader);
	bool CreateVertexShaderFromSourceFile(const string& path, ID3D11VertexShader*& shader);
	bool CreateVertexShaderFromBlobFile(const string& path, const string& entryPointName, const string& shaderMacros, ID3D11VertexShader*& shader);
	bool CreateVertexShaderFromBlobFile(const string& path, const string& shaderMacros, ID3D11VertexShader*& shader);
	bool CreateVertexShaderFromBlobFile(const string& path, ID3D11VertexShader*& shader);
	bool CreatePixelShaderFromSourceFile(const string& path, const string& entryPointName, const string& shaderMacros, ID3D11PixelShader*& shader);
	bool CreatePixelShaderFromSourceFile(const string& path, const string& shaderMacros, ID3D11PixelShader*& shader);
	bool CreatePixelShaderFromSourceFile(const string& path, ID3D11PixelShader*& shader);
	bool CreatePixelShaderFromBlobFile(const string& path, const string& entryPointName, const string& shaderMacros, ID3D11PixelShader*& shader);
	bool CreatePixelShaderFromBlobFile(const string& path, const string& shaderMacros, ID3D11PixelShader*& shader);
	bool CreatePixelShaderFromBlobFile(const string& path, ID3D11PixelShader*& shader);
	bool CreateComputeShaderFromSourceFile(const string& path, const string& entryPointName, const string& shaderMacros, ID3D11ComputeShader*& shader);
	bool CreateComputeShaderFromSourceFile(const string& path, const string& shaderMacros, ID3D11ComputeShader*& shader);
	bool CreateComputeShaderFromSourceFile(const string& path, ID3D11ComputeShader*& shader);
	bool CreateComputeShaderFromBlobFile(const string& path, const string& entryPointName, const string& shaderMacros, ID3D11ComputeShader*& shader);
	bool CreateComputeShaderFromBlobFile(const string& path, const string& shaderMacros, ID3D11ComputeShader*& shader);
	bool CreateComputeShaderFromBlobFile(const string& path, ID3D11ComputeShader*& shader);
} }


#endif
