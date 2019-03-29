#ifdef MAXEST_FRAMEWORK_WINDOWS


#include "d3d11.h"
#include "../essentials/file.h"
#include "../math/common.h"
#include "../image/main.h"


using namespace NMaxestFramework;
using namespace NEssentials;
using namespace NMath;
using namespace NImage;


extern "C"
{
	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001; // in case of laptops that have both Intel GPU and NVIDIA GPU this forces use of NVIDIA GPU
}


vector<NGPU::NShader::ShaderMacro> NGPU::NShader::ShaderMacrosFromString(const string& shaderMacros)
{
	vector<string> shaderMacrosArray_string = Split(shaderMacros, '|');

	vector<ShaderMacro> shaderMacrosArray;
	for (uint i = 0; i < shaderMacrosArray_string.size(); i++)
	{
		if (shaderMacrosArray_string[i] == "")
			continue;

		vector<string> macro_string = Split(shaderMacrosArray_string[i], '=');

		ShaderMacro macro;
		macro.name = macro_string[0];
		macro.definition = (macro_string.size() == 2 ? macro_string[1] : "");
		shaderMacrosArray.push_back(macro);
	}

	return shaderMacrosArray;
}


string NGPU::NShader::ShaderMacrosName(const string& baseName, const vector<ShaderMacro>& shaderMacros)
{
	string shaderMacrosName = baseName;

	for (uint i = 0; i < shaderMacros.size(); i++)
	{
		shaderMacrosName += "--" + shaderMacros[i].name;

		if (shaderMacros[i].definition.length() > 0)
			shaderMacrosName += "=" + shaderMacros[i].definition;
	}

	return shaderMacrosName;
}


D3D_SHADER_MACRO* NGPU::NShader::ShaderMacros_D3D11(const vector<ShaderMacro>& shaderMacros)
{
	D3D_SHADER_MACRO* shaderMacros_D3D11 = new D3D_SHADER_MACRO[shaderMacros.size() + 1];
	for (uint i = 0; i < shaderMacros.size(); i++)
	{
		shaderMacros_D3D11[i].Name = shaderMacros[i].name.c_str();
		shaderMacros_D3D11[i].Definition = shaderMacros[i].definition.c_str();
	}
	shaderMacros_D3D11[shaderMacros.size()].Name = nullptr;
	shaderMacros_D3D11[shaderMacros.size()].Definition = nullptr;

	return shaderMacros_D3D11;
}


D3D_FEATURE_LEVEL NGPU::featureLevel;
ID3D11Device* NGPU::device = nullptr;
ID3D11Device1* NGPU::device1 = nullptr;
ID3D11DeviceContext* NGPU::deviceContext = nullptr;
ID3D11DeviceContext1* NGPU::deviceContext1 = nullptr;
IDXGISwapChain* NGPU::swapChain = nullptr;
IDXGISwapChain1* NGPU::swapChain1 = nullptr;
ID3D11RenderTargetView* NGPU::backBufferRTV = nullptr;

NGPU::EShaderFileType NGPU::gGPUCreateShaderFileType = NGPU::EShaderFileType::Source;


void NGPU::CreateD3D11(int width, int height, bool fullScreen, float fpsGuess, const string& adapterNameGuess)
{
	// finding matching adapter

	bool useDefaultAdapter = (adapterNameGuess == "");

	vector<IDXGIAdapter*> dxgiAdapters;
	{
		IDXGIAdapter* dxgiAdapter = nullptr;
		IDXGIFactory* dxgiFactory = nullptr;
		MF_ASSERT(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&dxgiFactory) == S_OK);

		for (uint i = 0; dxgiFactory->EnumAdapters(i, &dxgiAdapter) != DXGI_ERROR_NOT_FOUND; i++)
			dxgiAdapters.push_back(dxgiAdapter);

		SAFE_RELEASE(dxgiFactory);
	}

	IDXGIAdapter* dxgiAdapter = dxgiAdapters[0];

	if (!useDefaultAdapter)
	{
		for (uint i = 0; i < dxgiAdapters.size(); i++)
		{
			DXGI_ADAPTER_DESC adapterDesc;
			dxgiAdapters[i]->GetDesc(&adapterDesc);

			if (Find(WStringToString(wstring(adapterDesc.Description)), adapterNameGuess))
			{
				if (i == 0)
					useDefaultAdapter = true;
				dxgiAdapter = dxgiAdapters[i];
				break;
			}
		}
	}

	// finding matching display mode
	DXGI_MODE_DESC displayMode;
	{
		IDXGIOutput* dxgiOutput;
		dxgiAdapter->EnumOutputs(0, &dxgiOutput);

		uint displayModesCount;
		dxgiOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, 0, &displayModesCount, 0);
		DXGI_MODE_DESC* displayModes = new DXGI_MODE_DESC[displayModesCount];
		dxgiOutput->GetDisplayModeList( DXGI_FORMAT_R8G8B8A8_UNORM, 0, &displayModesCount, displayModes);

		int displayModeIndex = -1;
		float displayModeClosestMatchingFPS = 0.0f;
		for (uint i = 0; i < displayModesCount; i++)
		{
			if (displayModes[i].Format == DXGI_FORMAT_R8G8B8A8_UNORM &&
				displayModes[i].Width == width &&
				displayModes[i].Height == height)
			{
				float fps = (float)displayModes[i].RefreshRate.Numerator / (float)displayModes[i].RefreshRate.Denominator;

				if (Abs(fps - fpsGuess) < Abs(displayModeClosestMatchingFPS - fpsGuess))
				{
					displayModeIndex = i;
					displayModeClosestMatchingFPS = fps;
				}
			}
		}

		if (displayModeIndex >= 0)
		{
			displayMode = displayModes[displayModeIndex];
		}
		else
		{
			displayMode.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			displayMode.Width = width;
			displayMode.Height = height;
			displayMode.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE;
			displayMode.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		}

		delete[] displayModes;
	}

	uint createDeviceFlags = 0;
#ifdef MAXEST_FRAMEWORK_DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};

	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
	swapChainDesc.BufferDesc = displayMode;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1;
	swapChainDesc.OutputWindow = GetActiveWindow();
	swapChainDesc.Windowed = !fullScreen;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	// try D3D11_1 level
	HRESULT hr = D3D11CreateDeviceAndSwapChain(
		useDefaultAdapter ? nullptr : dxgiAdapter,
		useDefaultAdapter ? D3D_DRIVER_TYPE_HARDWARE : D3D_DRIVER_TYPE_UNKNOWN,
		nullptr,
		createDeviceFlags,
		&featureLevels[0],
		ARRAY_SIZE(featureLevels),
		D3D11_SDK_VERSION,
		&swapChainDesc,
		&swapChain,
		&device,
		&featureLevel,
		&deviceContext);
	// try D3D11_0 level
	if (hr != S_OK)
	{
		hr = D3D11CreateDeviceAndSwapChain(
			useDefaultAdapter ? nullptr : dxgiAdapter,
			useDefaultAdapter ? D3D_DRIVER_TYPE_HARDWARE : D3D_DRIVER_TYPE_UNKNOWN,
			nullptr,
			createDeviceFlags,
			&featureLevels[1],
			ARRAY_SIZE(featureLevels) - 1,
			D3D11_SDK_VERSION,
			&swapChainDesc,
			&swapChain,
			&device,
			&featureLevel,
			&deviceContext);
	}
	MF_ASSERT(hr == S_OK);

/*	D3D11_FEATURE_DATA_DOUBLES featureDataDoubles;
	featureDataDoubles.DoublePrecisionFloatShaderOps = 0;
	hr = device->CheckFeatureSupport(D3D11_FEATURE_DOUBLES, &featureDataDoubles, sizeof(featureDataDoubles));*/

	MF_ASSERT(device->QueryInterface(__uuidof(ID3D11Device1), reinterpret_cast<void**>(&device1)) == S_OK);
	MF_ASSERT(deviceContext->QueryInterface(__uuidof(ID3D11DeviceContext1), reinterpret_cast<void**>(&deviceContext1)) == S_OK);
	MF_ASSERT(swapChain->QueryInterface(__uuidof(IDXGISwapChain1), reinterpret_cast<void**>(&swapChain1)) == S_OK);

	ID3D11Texture2D* backBufferTexture = nullptr;
	MF_ASSERT(swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBufferTexture)) == S_OK);
	MF_ASSERT(device->CreateRenderTargetView(backBufferTexture, nullptr, &backBufferRTV) == S_OK);
	SAFE_RELEASE(backBufferTexture);
}


void NGPU::DestroyD3D11()
{
	swapChain->SetFullscreenState(false, nullptr);

	SAFE_RELEASE(device);
	SAFE_RELEASE(device1);
	SAFE_RELEASE(deviceContext);
	SAFE_RELEASE(deviceContext1);
	SAFE_RELEASE(swapChain);
	SAFE_RELEASE(swapChain1);
	SAFE_RELEASE(backBufferRTV);
}


bool NGPU::CompileShaderFromSourceFile(const string& path, const string& entryPointName, const string& shaderModelName, const string& shaderMacros, ID3DBlob*& blob)
{
	DWORD shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;// | D3DCOMPILE_WARNINGS_ARE_ERRORS;
#ifdef MAXEST_FRAMEWORK_DEBUG
	shaderFlags |= D3DCOMPILE_DEBUG;
	shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	vector<NShader::ShaderMacro> shaderMacrosArray = NShader::ShaderMacrosFromString(shaderMacros);

	ID3DBlob* unstrippedBlob = nullptr;
	ID3DBlob* errorsBlob = nullptr;

	D3D_SHADER_MACRO* shaderMacros_D3D11 = NShader::ShaderMacros_D3D11(shaderMacrosArray);
	if (FAILED(D3DCompileFromFile(StringToWString(path).c_str(), shaderMacros_D3D11, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPointName.c_str(), shaderModelName.c_str(), shaderFlags, 0, &unstrippedBlob, &errorsBlob)))
	{
		// dump errors to Output window
		if (errorsBlob)
		{
			OutputDebugStringA((char*)errorsBlob->GetBufferPointer());
			errorsBlob->Release();
		}

		return false;
	}
	delete[] shaderMacros_D3D11;

/*	ID3D11ShaderReflection* shaderReflection = nullptr;
	D3DReflect(unstrippedBlob->GetBufferPointer(), unstrippedBlob->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&shaderReflection);
	D3D11_SHADER_DESC desc;
	shaderReflection->GetDesc(&desc);*/

	D3DStripShader(unstrippedBlob->GetBufferPointer(), unstrippedBlob->GetBufferSize(), D3DCOMPILER_STRIP_REFLECTION_DATA | D3DCOMPILER_STRIP_DEBUG_INFO, &blob);

	string shaderMacrosName = NShader::ShaderMacrosName(path + "--" + entryPointName, shaderMacrosArray);

	// dump disassembly
	{
		ID3DBlob* disassemblyBlob = nullptr;
		D3DDisassemble(unstrippedBlob->GetBufferPointer(), unstrippedBlob->GetBufferSize(), 0, nullptr, &disassemblyBlob);
		FileOpenAndWrite(shaderMacrosName + "--asm.txt", (uint8*)disassemblyBlob->GetBufferPointer(), (int)disassemblyBlob->GetBufferSize() - 1);
		SAFE_RELEASE(disassemblyBlob);
	}

	// dump blob
	D3DWriteBlobToFile(unstrippedBlob, StringToWString(shaderMacrosName + "--blob.dat").c_str(), true);

	SAFE_RELEASE(unstrippedBlob);
	SAFE_RELEASE(errorsBlob);

	return true;
}


void NGPU::ClearRenderTargets()
{
	deviceContext->OMSetRenderTargets(0, nullptr, nullptr);
}


void NGPU::ClearPSShaderResources(int offset, int count)
{
	ID3D11ShaderResourceView** nullSRVs = new ID3D11ShaderResourceView*[count];
	for (int i = 0; i < count; i++)
		nullSRVs[i] = nullptr;

	deviceContext->PSSetShaderResources(offset, count, nullSRVs);

	delete[] nullSRVs;
}


void NGPU::ClearCSShaderResources(int offset, int count)
{
	ID3D11ShaderResourceView** nullSRVs = new ID3D11ShaderResourceView*[count];
	for (int i = 0; i < count; i++)
		nullSRVs[i] = nullptr;

	deviceContext->CSSetShaderResources(offset, count, nullSRVs);

	delete[] nullSRVs;
}


void NGPU::CreateRenderTarget(int width, int height, DXGI_FORMAT format, int samplesCount, TRenderTarget& renderTarget)
{
	renderTarget.type = STexture::EType::RenderTarget;
	renderTarget.width = width;
	renderTarget.height = height;
	renderTarget.mipmapsCount = 1;
	renderTarget.arraysCount = 1;
	renderTarget.samplesCount = samplesCount;
	renderTarget.format = format;

	//

	D3D11_TEXTURE2D_DESC td;
	ZeroMemory(&td, sizeof(td));
	td.Width = width;
	td.Height = height;
	td.MipLevels = renderTarget.mipmapsCount;
	td.ArraySize = renderTarget.arraysCount;
	td.Format = format;
	td.SampleDesc.Count = Max(samplesCount, 1);
	td.SampleDesc.Quality = 0;
	td.Usage = D3D11_USAGE_DEFAULT;
	td.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	td.CPUAccessFlags = 0;

	HRESULT hr = device->CreateTexture2D(&td, NULL, (ID3D11Texture2D**)&renderTarget.texture);
	MF_ASSERT(hr == S_OK);

	//

	D3D11_RENDER_TARGET_VIEW_DESC rtvd;
	ZeroMemory(&rtvd, sizeof(rtvd));
	rtvd.Format = format;
	rtvd.ViewDimension = (samplesCount < 2 ? D3D11_RTV_DIMENSION_TEXTURE2D : D3D11_RTV_DIMENSION_TEXTURE2DMS);
	rtvd.Texture2D.MipSlice = 0;

	hr = device->CreateRenderTargetView(renderTarget.texture, &rtvd, &renderTarget.rtv);
	MF_ASSERT(hr == S_OK);

	//

	D3D11_SHADER_RESOURCE_VIEW_DESC srvd;
	ZeroMemory(&srvd, sizeof(srvd));
	srvd.Format = format;
	srvd.ViewDimension = (samplesCount < 2 ? D3D11_SRV_DIMENSION_TEXTURE2D : D3D11_SRV_DIMENSION_TEXTURE2DMS);
	srvd.Texture2D.MipLevels = renderTarget.mipmapsCount;
	srvd.Texture2D.MostDetailedMip = 0;

	hr = device->CreateShaderResourceView(renderTarget.texture, &srvd, &renderTarget.srv);
	MF_ASSERT(hr == S_OK);
}


void NGPU::CreateDepthStencilTarget(int width, int height, int samplesCount, TDepthStencilTarget& depthStencilTarget)
{
	depthStencilTarget.type = STexture::EType::DepthStencilTarget;
	depthStencilTarget.width = width;
	depthStencilTarget.height = height;
	depthStencilTarget.mipmapsCount = 1;
	depthStencilTarget.arraysCount = 1;
	depthStencilTarget.samplesCount = samplesCount;
	depthStencilTarget.format = DXGI_FORMAT_R24G8_TYPELESS;

	//

	D3D11_TEXTURE2D_DESC td;
	ZeroMemory(&td, sizeof(td));
	td.Width = width;
	td.Height = height;
	td.MipLevels = depthStencilTarget.mipmapsCount;
	td.ArraySize = depthStencilTarget.arraysCount;
	td.Format = DXGI_FORMAT_R24G8_TYPELESS;
	td.SampleDesc.Count = Max(samplesCount, 1);
	td.SampleDesc.Quality = 0;
	td.Usage = D3D11_USAGE_DEFAULT;
	td.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	td.CPUAccessFlags = 0;
	td.MiscFlags = 0;

	HRESULT hr = device->CreateTexture2D(&td, nullptr, (ID3D11Texture2D**)&depthStencilTarget.texture);
	MF_ASSERT(hr == S_OK);

	//

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvd;
	ZeroMemory(&dsvd, sizeof(dsvd));
	dsvd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvd.ViewDimension = (samplesCount < 2 ? D3D11_DSV_DIMENSION_TEXTURE2D : D3D11_DSV_DIMENSION_TEXTURE2DMS);
	dsvd.Texture2D.MipSlice = 0;

	hr = device->CreateDepthStencilView(depthStencilTarget.texture, &dsvd, &depthStencilTarget.dsv);
	MF_ASSERT(hr == S_OK);

	//

	D3D11_SHADER_RESOURCE_VIEW_DESC srvd;
	ZeroMemory(&srvd, sizeof(srvd));
	srvd.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	srvd.ViewDimension = (samplesCount < 2 ? D3D11_SRV_DIMENSION_TEXTURE2D : D3D11_SRV_DIMENSION_TEXTURE2DMS);
	srvd.Texture2D.MipLevels = depthStencilTarget.mipmapsCount;
	srvd.Texture2D.MostDetailedMip = 0;

	hr = device->CreateShaderResourceView(depthStencilTarget.texture, &srvd, &depthStencilTarget.srv);
	MF_ASSERT(hr == S_OK);
}


void NGPU::CreateDepthStencilTarget64(int width, int height, int samplesCount, TDepthStencilTarget& depthStencilTarget)
{
	depthStencilTarget.type = STexture::EType::DepthStencilTarget;
	depthStencilTarget.width = width;
	depthStencilTarget.height = height;
	depthStencilTarget.mipmapsCount = 1;
	depthStencilTarget.arraysCount = 1;
	depthStencilTarget.samplesCount = samplesCount;
	depthStencilTarget.format = DXGI_FORMAT_R32G8X24_TYPELESS;

	//

	D3D11_TEXTURE2D_DESC td;
	ZeroMemory(&td, sizeof(td));
	td.Width = width;
	td.Height = height;
	td.MipLevels = depthStencilTarget.mipmapsCount;
	td.ArraySize = depthStencilTarget.arraysCount;
	td.Format = DXGI_FORMAT_R32G8X24_TYPELESS;
	td.SampleDesc.Count = Max(samplesCount, 1);
	td.SampleDesc.Quality = 0;
	td.Usage = D3D11_USAGE_DEFAULT;
	td.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	td.CPUAccessFlags = 0;
	td.MiscFlags = 0;

	HRESULT hr = device->CreateTexture2D(&td, nullptr, (ID3D11Texture2D**)&depthStencilTarget.texture);
	MF_ASSERT(hr == S_OK);

	//

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvd;
	ZeroMemory(&dsvd, sizeof(dsvd));
	dsvd.Format = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
	dsvd.ViewDimension = (samplesCount < 2 ? D3D11_DSV_DIMENSION_TEXTURE2D : D3D11_DSV_DIMENSION_TEXTURE2DMS);
	dsvd.Texture2D.MipSlice = 0;

	hr = device->CreateDepthStencilView(depthStencilTarget.texture, &dsvd, &depthStencilTarget.dsv);
	MF_ASSERT(hr == S_OK);

	//

	D3D11_SHADER_RESOURCE_VIEW_DESC srvd;
	ZeroMemory(&srvd, sizeof(srvd));
	srvd.Format = DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
	srvd.ViewDimension = (samplesCount < 2 ? D3D11_SRV_DIMENSION_TEXTURE2D : D3D11_SRV_DIMENSION_TEXTURE2DMS);
	srvd.Texture2D.MipLevels = depthStencilTarget.mipmapsCount;
	srvd.Texture2D.MostDetailedMip = 0;

	hr = device->CreateShaderResourceView(depthStencilTarget.texture, &srvd, &depthStencilTarget.srv);
	MF_ASSERT(hr == S_OK);
}


void NGPU::CreateTexture(int width, int height, int mipmapsCount, DXGI_FORMAT format, STexture& texture)
{
	texture.type = STexture::EType::Read2D;
	texture.width = width;
	texture.height = height;
	texture.depth = 1;
	texture.mipmapsCount = (mipmapsCount == 0 ? MipmapsCount(width, height) : mipmapsCount);
	texture.arraysCount = 1;
	texture.samplesCount = 1;
	texture.format = format;

	//

	D3D11_TEXTURE2D_DESC td;
	ZeroMemory(&td, sizeof(td));
	td.Width = width;
	td.Height = height;
	td.MipLevels = texture.mipmapsCount;
	td.ArraySize = texture.arraysCount;
	td.Format = texture.format;
	td.SampleDesc.Count = texture.samplesCount;
	td.SampleDesc.Quality = 0;
	td.Usage = D3D11_USAGE_DEFAULT;
	td.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	td.CPUAccessFlags = 0;

	HRESULT hr = device->CreateTexture2D(&td, nullptr, (ID3D11Texture2D**)&texture.texture);
	MF_ASSERT(hr == S_OK);

	//

	D3D11_SHADER_RESOURCE_VIEW_DESC srvd;
	ZeroMemory(&srvd, sizeof(srvd));
	srvd.Format = texture.format;
	srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvd.Texture2D.MipLevels = texture.mipmapsCount;
	srvd.Texture2D.MostDetailedMip = 0;

	hr = device->CreateShaderResourceView(texture.texture, &srvd, &texture.srv);
	MF_ASSERT(hr == S_OK);
}


void NGPU::CreateTexture(int width, int height, STexture& texture)
{
	CreateTexture(width, height, 0, DXGI_FORMAT_B8G8R8A8_UNORM, texture);
}


void NGPU::CreateRWTexture(int width, int height, int mipmapsCount, DXGI_FORMAT format, STexture& texture)
{
	texture.type = STexture::EType::ReadWrite2D;
	texture.width = width;
	texture.height = height;
	texture.depth = 1;
	texture.mipmapsCount = (mipmapsCount == 0 ? MipmapsCount(width, height) : mipmapsCount);
	texture.arraysCount = 1;
	texture.samplesCount = 1;
	texture.format = format;

	//

	D3D11_TEXTURE2D_DESC td;
	ZeroMemory(&td, sizeof(td));
	td.Width = width;
	td.Height = height;
	td.MipLevels = texture.mipmapsCount;
	td.ArraySize = texture.arraysCount;
	td.Format = texture.format;
	td.SampleDesc.Count = texture.samplesCount;
	td.SampleDesc.Quality = 0;
	td.Usage = D3D11_USAGE_DEFAULT;
	td.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	td.CPUAccessFlags = 0;

	HRESULT hr = device->CreateTexture2D(&td, NULL, (ID3D11Texture2D**)&texture.texture);
	MF_ASSERT(hr == S_OK);

	//

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavd;
	ZeroMemory(&uavd, sizeof(uavd));
	uavd.Format = format;
	uavd.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	uavd.Texture2D.MipSlice = 0;

	hr = device->CreateUnorderedAccessView(texture.texture, &uavd, &texture.uav);
	MF_ASSERT(hr == S_OK);

	//

	D3D11_SHADER_RESOURCE_VIEW_DESC srvd;
	ZeroMemory(&srvd, sizeof(srvd));
	srvd.Format = format;
	srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvd.Texture2D.MipLevels = texture.mipmapsCount;
	srvd.Texture2D.MostDetailedMip = 0;

	hr = device->CreateShaderResourceView(texture.texture, &srvd, &texture.srv);
	MF_ASSERT(hr == S_OK);
}


void NGPU::CreateRWTexture3D(int width, int height, int depth, int mipmapsCount, DXGI_FORMAT format, STexture& texture)
{
	texture.type = STexture::EType::ReadWrite2D;
	texture.width = width;
	texture.height = height;
	texture.depth = depth;
	texture.mipmapsCount = (mipmapsCount == 0 ? MipmapsCount(width, height) : mipmapsCount);
	texture.arraysCount = 1;
	texture.samplesCount = 1;
	texture.format = format;

	//

	D3D11_TEXTURE3D_DESC td;
	ZeroMemory(&td, sizeof(td));
	td.Width = width;
	td.Height = height;
	td.Depth = depth;
	td.MipLevels = texture.mipmapsCount;
	td.Format = texture.format;
	td.Usage = D3D11_USAGE_DEFAULT;
	td.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	td.CPUAccessFlags = 0;

	HRESULT hr = device->CreateTexture3D(&td, NULL, (ID3D11Texture3D**)&texture.texture);
	MF_ASSERT(hr == S_OK);

	//

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavd;
	ZeroMemory(&uavd, sizeof(uavd));
	uavd.Format = format;
	uavd.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE3D;
	uavd.Texture3D.MipSlice = 0;
	uavd.Texture3D.FirstWSlice = 0;
	uavd.Texture3D.WSize = depth;

	hr = device->CreateUnorderedAccessView(texture.texture, &uavd, &texture.uav);
	MF_ASSERT(hr == S_OK);

	//

	D3D11_SHADER_RESOURCE_VIEW_DESC srvd;
	ZeroMemory(&srvd, sizeof(srvd));
	srvd.Format = format;
	srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
	srvd.Texture3D.MipLevels = texture.mipmapsCount;
	srvd.Texture3D.MostDetailedMip = 0;

	hr = device->CreateShaderResourceView(texture.texture, &srvd, &texture.srv);
	MF_ASSERT(hr == S_OK);
}


void NGPU::CreateRWTextureArray(int width, int height, int arraysCount, DXGI_FORMAT format, STexture& texture)
{
	texture.type = STexture::EType::ReadWrite2D;
	texture.width = width;
	texture.height = height;
	texture.depth = 1;
	texture.mipmapsCount = 1;
	texture.arraysCount = arraysCount;
	texture.samplesCount = 1;
	texture.format = format;

	//

	D3D11_TEXTURE2D_DESC td;
	ZeroMemory(&td, sizeof(td));
	td.Width = width;
	td.Height = height;
	td.MipLevels = texture.mipmapsCount;
	td.ArraySize = texture.arraysCount;
	td.Format = texture.format;
	td.SampleDesc.Count = texture.samplesCount;
	td.SampleDesc.Quality = 0;
	td.Usage = D3D11_USAGE_DEFAULT;
	td.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	td.CPUAccessFlags = 0;

	HRESULT hr = device->CreateTexture2D(&td, NULL, (ID3D11Texture2D**)&texture.texture);
	MF_ASSERT(hr == S_OK);

	//

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavd;
	ZeroMemory(&uavd, sizeof(uavd));
	uavd.Format = format;
	uavd.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
	uavd.Texture2DArray.MipSlice = 0;
	uavd.Texture2DArray.FirstArraySlice = 0;
	uavd.Texture2DArray.ArraySize = texture.arraysCount;

	hr = device->CreateUnorderedAccessView(texture.texture, &uavd, &texture.uav);
	MF_ASSERT(hr == S_OK);

	//

	D3D11_SHADER_RESOURCE_VIEW_DESC srvd;
	ZeroMemory(&srvd, sizeof(srvd));
	srvd.Format = format;
	srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	srvd.Texture2DArray.MostDetailedMip = 0;
	srvd.Texture2DArray.MipLevels = texture.mipmapsCount;
	srvd.Texture2DArray.FirstArraySlice = 0;
	srvd.Texture2DArray.ArraySize = texture.arraysCount;

	hr = device->CreateShaderResourceView(texture.texture, &srvd, &texture.srv);
	MF_ASSERT(hr == S_OK);
}


void NGPU::CreateStagingTexture(const STexture& srcTexture, STexture& texture)
{
	texture.type = STexture::EType::Unknown;
	texture.width = srcTexture.width;
	texture.height = srcTexture.height;
	texture.depth = srcTexture.depth;
	texture.mipmapsCount = srcTexture.mipmapsCount;
	texture.arraysCount = srcTexture.arraysCount;
	texture.samplesCount = srcTexture.samplesCount;
	texture.format = srcTexture.format;

	//

	D3D11_TEXTURE2D_DESC td;
	ZeroMemory(&td, sizeof(td));
	td.Width = srcTexture.width;
	td.Height = srcTexture.height;
	td.MipLevels = srcTexture.mipmapsCount;
	td.ArraySize = srcTexture.arraysCount;
	td.Format = srcTexture.format;
	td.SampleDesc.Count = srcTexture.samplesCount;
	td.SampleDesc.Quality = 0;
	td.Usage = D3D11_USAGE_STAGING;
	td.BindFlags = 0;
	td.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

	HRESULT hr = device->CreateTexture2D(&td, NULL, (ID3D11Texture2D**)&texture.texture);
	MF_ASSERT(hr == S_OK);
}


void NGPU::CreateStructuredBuffer(int dataSize, int dataStride, SBuffer& buffer)
{
	MF_ASSERT(dataSize % dataStride == 0);

	int elementsCount = dataSize / dataStride;

	buffer.type = SBuffer::EType::Read;
	buffer.size = dataSize;

	//

	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
	bufferDesc.ByteWidth = dataSize;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	bufferDesc.StructureByteStride = dataStride;

	device->CreateBuffer(&bufferDesc, nullptr, &buffer.buffer);

	//

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	ZeroMemory(&shaderResourceViewDesc, sizeof(shaderResourceViewDesc));
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	shaderResourceViewDesc.Buffer.FirstElement = 0;
	shaderResourceViewDesc.Buffer.NumElements = elementsCount;
	shaderResourceViewDesc.Format = DXGI_FORMAT_UNKNOWN;

	device->CreateShaderResourceView(buffer.buffer, &shaderResourceViewDesc, &buffer.srv);
}


void NGPU::CreateRWStructuredBuffer(int dataSize, int dataStride, SBuffer& buffer)
{
	MF_ASSERT(dataSize % dataStride == 0);

	int elementsCount = dataSize / dataStride;

	buffer.type = SBuffer::EType::ReadWrite;
	buffer.size = dataSize;

	//

	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
	bufferDesc.ByteWidth = dataSize;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	bufferDesc.StructureByteStride = dataStride;

	HRESULT hr = device->CreateBuffer(&bufferDesc, nullptr, &buffer.buffer);
	MF_ASSERT(hr == S_OK);

	//

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavd;
	ZeroMemory(&uavd, sizeof(uavd));
	uavd.Format = DXGI_FORMAT_UNKNOWN;
	uavd.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	uavd.Buffer.FirstElement = 0;
	uavd.Buffer.NumElements = elementsCount;
	uavd.Buffer.Flags = 0;

	hr = device->CreateUnorderedAccessView(buffer.buffer, &uavd, &buffer.uav);
	MF_ASSERT(hr == S_OK);

	//

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	ZeroMemory(&shaderResourceViewDesc, sizeof(shaderResourceViewDesc));
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	shaderResourceViewDesc.Buffer.FirstElement = 0;
	shaderResourceViewDesc.Buffer.NumElements = elementsCount;
	shaderResourceViewDesc.Format = DXGI_FORMAT_UNKNOWN;

	hr = device->CreateShaderResourceView(buffer.buffer, &shaderResourceViewDesc, &buffer.srv);
	MF_ASSERT(hr == S_OK);
}


void NGPU::CreateStagingBuffer(const SBuffer& srcBuffer, SBuffer& buffer)
{
	buffer.type = srcBuffer.type;
	buffer.size = srcBuffer.size;

	//

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.ByteWidth = srcBuffer.size;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	bd.Usage = D3D11_USAGE_STAGING;

	HRESULT hr = device->CreateBuffer(&bd, NULL, &buffer.buffer);
	MF_ASSERT(hr == S_OK);
}


bool NGPU::CreateShaderBlob(EShaderFileType shaderFileType, EShaderType shaderType, const string& path, const string& entryPointName, const string& shaderMacros, ID3DBlob*& blob)
{
	if (shaderFileType == EShaderFileType::Blob)
	{
		vector<NShader::ShaderMacro> shaderMacrosArray = NShader::ShaderMacrosFromString(shaderMacros);
		string shaderMacrosName = NShader::ShaderMacrosName(path + "--" + entryPointName, shaderMacrosArray);

		if (FAILED(D3DReadFileToBlob(StringToWString(shaderMacrosName + "--blob.dat").c_str(), &blob)))
			return false;
	}
	else if (shaderFileType == EShaderFileType::Source)
	{
		string shaderModelName = "vs_5_0";
		if (shaderType == EShaderType::Pixel)
			shaderModelName = "ps_5_0";
		else if (shaderType == EShaderType::Compute)
			shaderModelName = "cs_5_0";

		if (!CompileShaderFromSourceFile(path, entryPointName, shaderModelName, shaderMacros, blob))
			return false;
	}

	if (!blob)
		return false;

	return true;
}


bool NGPU::CreateVertexShader(EShaderFileType shaderFileType, const string& path, const string& entryPointName, const string& shaderMacros, ID3D11VertexShader*& shader)
{
	ID3DBlob* blob;

	if (!CreateShaderBlob(shaderFileType, EShaderType::Vertex, path, entryPointName, shaderMacros, blob))
		return false;
	if (FAILED(device->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &shader)))
		return false;

	SAFE_RELEASE(blob);

	return true;
}


bool NGPU::CreatePixelShader(EShaderFileType shaderFileType, const string& path, const string& entryPointName, const string& shaderMacros, ID3D11PixelShader*& shader)
{
	ID3DBlob* blob;

	if (!CreateShaderBlob(shaderFileType, EShaderType::Pixel, path, entryPointName, shaderMacros, blob))
		return false;
	if (FAILED(device->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &shader)))
		return false;

	SAFE_RELEASE(blob);

	return true;
}


bool NGPU::CreateComputeShader(EShaderFileType shaderFileType, const string& path, const string& entryPointName, const string& shaderMacros, ID3D11ComputeShader*& shader)
{
	ID3DBlob* blob;

	if (!CreateShaderBlob(shaderFileType, EShaderType::Compute, path, entryPointName, shaderMacros, blob))
		return false;
	if (FAILED(device->CreateComputeShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &shader)))
		return false;

	SAFE_RELEASE(blob);

	return true;
}


bool NGPU::CreateInputLayout(const string& dummyVertexShaderPath, const string& dummyVertexShaderMacros, D3D11_INPUT_ELEMENT_DESC inputLayoutElements[], int inputLayoutElementsCount, ID3D11InputLayout*& inputLayout)
{
	ID3D11VertexShader* vs = nullptr;
	ID3DBlob* vsBlob = nullptr;

	if (!CreateShaderBlob(gGPUCreateShaderFileType, EShaderType::Vertex, dummyVertexShaderPath, "main", dummyVertexShaderMacros, vsBlob))
		return false;

	if (!vsBlob)
		return false;

	if (FAILED(device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &vs)))
	{
		vsBlob->Release();
		return false;
	}

	if (FAILED(device->CreateInputLayout(inputLayoutElements, inputLayoutElementsCount, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &inputLayout)))
	{
		vs->Release();
		vsBlob->Release();
		return false;
	}

	vs->Release();
	vsBlob->Release();

	return true;
}


void NGPU::CreateVertexBuffer(bool dynamic, uint8* data, int dataSize, SBuffer& vertexBuffer)
{
	vertexBuffer.type = SBuffer::EType::Vertex;
	vertexBuffer.size = dataSize;

	//

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = dataSize;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = dynamic ? D3D11_CPU_ACCESS_WRITE : 0;

	D3D11_SUBRESOURCE_DATA subresourceData;
	ZeroMemory(&subresourceData, sizeof(subresourceData));
	subresourceData.pSysMem = data;

	HRESULT hr = device->CreateBuffer(&bd, data ? &subresourceData : nullptr, &vertexBuffer.buffer);
	MF_ASSERT(hr == S_OK);
}


void NGPU::CreateIndexBuffer(bool dynamic, uint8* data, int dataSize, SBuffer& indexBuffer)
{
	indexBuffer.type = SBuffer::EType::Index;
	indexBuffer.size = dataSize;

	//

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = dataSize;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = dynamic ? D3D11_CPU_ACCESS_WRITE : 0;

	D3D11_SUBRESOURCE_DATA subresourceData;
	ZeroMemory(&subresourceData, sizeof(subresourceData));
	subresourceData.pSysMem = data;

	HRESULT hr = device->CreateBuffer(&bd, data ? &subresourceData : nullptr, &indexBuffer.buffer);
	MF_ASSERT(hr == S_OK);
}


void NGPU::CreateConstantBuffer(int dataSize, SBuffer& constantBuffer)
{
	constantBuffer.type = SBuffer::EType::Constant;
	constantBuffer.size = dataSize;

	//

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = dataSize;
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;

	HRESULT hr = device->CreateBuffer(&bd, nullptr, &constantBuffer.buffer);
	MF_ASSERT(hr == S_OK);
}


void NGPU::CreateRasterizerState(ID3D11RasterizerState*& rasterizerState, bool visibleIsCCW)
{
	D3D11_RASTERIZER_DESC rd;
	ZeroMemory(&rd, sizeof(rd));
	rd.FillMode = D3D11_FILL_SOLID;
	rd.CullMode = D3D11_CULL_BACK;
	rd.FrontCounterClockwise = visibleIsCCW;
	rd.DepthBias = 0;
	rd.DepthBiasClamp = 0.0f;
	rd.SlopeScaledDepthBias = 0.0f;
	rd.DepthClipEnable = true;
	rd.ScissorEnable = false;
	rd.MultisampleEnable = false;
	rd.AntialiasedLineEnable = false;

	HRESULT hr = device->CreateRasterizerState(&rd, &rasterizerState);
	MF_ASSERT(hr == S_OK);
}


void NGPU::CreateDepthStencilState(ID3D11DepthStencilState*& depthStencilState, bool depthTest, bool stencilTest)
{
	D3D11_DEPTH_STENCIL_DESC dsd;
	ZeroMemory(&dsd, sizeof(dsd));

	dsd.DepthEnable = depthTest;
	dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsd.DepthFunc = D3D11_COMPARISON_LESS;

	dsd.StencilEnable = stencilTest;
	dsd.StencilReadMask = 0xFF;
	dsd.StencilWriteMask = 0xFF;

	dsd.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsd.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	dsd.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsd.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	dsd.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsd.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	dsd.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsd.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	HRESULT hr = device->CreateDepthStencilState(&dsd, &depthStencilState);
	MF_ASSERT(hr == S_OK);
}


void NGPU::CreateBlendState(ID3D11BlendState*& blendState, bool transparency)
{
	D3D11_BLEND_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.AlphaToCoverageEnable = false;
	bd.IndependentBlendEnable = false;
	if (transparency)
	{
		bd.RenderTarget[0].BlendEnable = true;
		bd.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		bd.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		bd.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		bd.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		bd.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		bd.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	}
	else
	{
		bd.RenderTarget[0].BlendEnable = true;
		bd.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
		bd.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
		bd.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		bd.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		bd.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		bd.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	}

	HRESULT hr = device->CreateBlendState(&bd, &blendState);
	MF_ASSERT(hr == S_OK);
}


void NGPU::CreateSamplerState(ID3D11SamplerState*& samplerState, ESamplerFilter filter, ESamplerAddressing addressing, ESamplerComparisonFunction comparisonFunction)
{
	D3D11_SAMPLER_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	if (filter == ESamplerFilter::Point)
	{
		if (comparisonFunction == ESamplerComparisonFunction::None)
			sd.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		else
			sd.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
	}
	else if (filter == ESamplerFilter::Linear)
	{
		if (comparisonFunction == ESamplerComparisonFunction::None)
			sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		else
			sd.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
	}
	else if (filter == ESamplerFilter::Anisotropic)
	{
		if (comparisonFunction == ESamplerComparisonFunction::None)
			sd.Filter = D3D11_FILTER_ANISOTROPIC;
		else
			sd.Filter = D3D11_FILTER_COMPARISON_ANISOTROPIC;
	}
	sd.MaxAnisotropy = 16;
	if (addressing == ESamplerAddressing::Clamp)
	{
		sd.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		sd.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		sd.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	}
	else if (addressing == ESamplerAddressing::Wrap)
	{
		sd.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sd.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		sd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	}
	else if (addressing == ESamplerAddressing::BorderGrey)
	{
		sd.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
		sd.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
		sd.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
		sd.BorderColor[0] = 0.5f;
		sd.BorderColor[1] = 0.5f;
		sd.BorderColor[2] = 0.5f;
		sd.BorderColor[3] = 0.5f;
	}
	sd.ComparisonFunc = (D3D11_COMPARISON_FUNC)comparisonFunction;
	sd.MinLOD = 0;
	sd.MaxLOD = D3D11_FLOAT32_MAX;

	HRESULT hr = device->CreateSamplerState(&sd, &samplerState);
	MF_ASSERT(hr == S_OK);
}


void NGPU::DestroyRenderTarget(TRenderTarget& renderTarget)
{
	MF_ASSERT(renderTarget.type == STexture::EType::RenderTarget);

	SAFE_RELEASE(renderTarget.texture);
	SAFE_RELEASE(renderTarget.rtv);
	SAFE_RELEASE(renderTarget.srv);
}


void NGPU::DestroyDepthStencilTarget(TDepthStencilTarget& depthStencilTarget)
{
	MF_ASSERT(depthStencilTarget.type == STexture::EType::DepthStencilTarget);

	SAFE_RELEASE(depthStencilTarget.texture);
	SAFE_RELEASE(depthStencilTarget.dsv);
	SAFE_RELEASE(depthStencilTarget.srv);
}


void NGPU::DestroyTexture(STexture& texture)
{
	MF_ASSERT(texture.type == STexture::EType::Read2D || texture.type == STexture::EType::ReadWrite2D);

	SAFE_RELEASE(texture.texture);
	SAFE_RELEASE(texture.srv);
	if (texture.type == STexture::EType::ReadWrite2D)
		SAFE_RELEASE(texture.uav);
}


void NGPU::DestroyVertexShader(ID3D11VertexShader*& vertexShader)
{
	SAFE_RELEASE(vertexShader);
}


void NGPU::DestroyPixelShader(ID3D11PixelShader*& pixelShader)
{
	SAFE_RELEASE(pixelShader);
}


void NGPU::DestroyComputeShader(ID3D11ComputeShader*& computeShader)
{
	SAFE_RELEASE(computeShader);
}


void NGPU::DestroyInputLayout(ID3D11InputLayout*& inputLayout)
{
	SAFE_RELEASE(inputLayout);
}


void NGPU::DestroyBuffer(SBuffer& buffer)
{
	SAFE_RELEASE(buffer.buffer);
	if (buffer.type == SBuffer::EType::Read || buffer.type == SBuffer::EType::ReadWrite)
	{
		SAFE_RELEASE(buffer.srv);
		if (buffer.type == SBuffer::EType::ReadWrite)
			SAFE_RELEASE(buffer.uav);
	}
}


void NGPU::DestroyRasterizerState(ID3D11RasterizerState*& rasterizerState)
{
	SAFE_RELEASE(rasterizerState);
}


void NGPU::DestroyDepthStencilState(ID3D11DepthStencilState*& depthStencilState)
{
	SAFE_RELEASE(depthStencilState);
}


void NGPU::DestroyBlendState(ID3D11BlendState*& blendState)
{
	SAFE_RELEASE(blendState);
}


void NGPU::DestroySamplerState(ID3D11SamplerState*& samplerState)
{
	SAFE_RELEASE(samplerState);
}


void NGPU::UpdateTexture(STexture& texture, int mipmapIndex, uint8* data, int rowPitch)
{
	deviceContext->UpdateSubresource(
		texture.texture,
		D3D11CalcSubresource(mipmapIndex, 0, texture.mipmapsCount),
		NULL,
		data,
		rowPitch,
		0);
}


void NGPU::UpdateBuffer(SBuffer& buffer, uint8* data, int dataSize)
{
	D3D11_MAPPED_SUBRESOURCE ms;
	deviceContext->Map(buffer.buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);
	memcpy(ms.pData, data, dataSize);
	deviceContext->Unmap(buffer.buffer, 0);
}


void NGPU::CopyTextureFromGPUToCPU(const STexture& srcTexture, STexture* stagingTexture, uint8*& dstData)
{
	STexture* tempTexture = stagingTexture;

	if (stagingTexture == nullptr)
	{
		tempTexture = new STexture();
		CreateStagingTexture(srcTexture, *tempTexture);
	}

	deviceContext->CopyResource(tempTexture->texture, srcTexture.texture);

	D3D11_MAPPED_SUBRESOURCE ms;
	deviceContext->Map(tempTexture->texture, 0, D3D11_MAP_READ, 0, &ms);
	{
		dstData = new uint8[ms.DepthPitch];
		memcpy(dstData, (uint8*)ms.pData, ms.DepthPitch);
	}
	deviceContext->Unmap(tempTexture->texture, 0);

	if (stagingTexture == nullptr)
	{
		SAFE_RELEASE(tempTexture->texture);
		delete tempTexture;
	}
}


void NGPU::CopyBufferFromGPUToCPU(const SBuffer& srcBuffer, SBuffer* stagingBuffer, uint8*& dstData)
{
	SBuffer* tempBuffer = stagingBuffer;

	if (stagingBuffer == nullptr)
	{
		tempBuffer = new SBuffer();
		CreateStagingBuffer(srcBuffer, *tempBuffer);
	}

	deviceContext->CopyResource(tempBuffer->buffer, srcBuffer.buffer);

	D3D11_MAPPED_SUBRESOURCE ms;
	deviceContext->Map(tempBuffer->buffer, 0, D3D11_MAP_READ, 0, &ms);
	{
		dstData = new uint8[ms.DepthPitch];
		memcpy(dstData, (uint8*)ms.pData, ms.DepthPitch);
	}
	deviceContext->Unmap(tempBuffer->buffer, 0);

	if (stagingBuffer == nullptr)
	{
		SAFE_RELEASE(tempBuffer->buffer);
		delete tempBuffer;
	}
}


void NGPU::ResolveMSAA(const NGPU::STexture& input, const NGPU::STexture& output)
{
	MF_ASSERT(input.format == output.format);
	deviceContext->ResolveSubresource(output.texture, 0, input.texture, 0, input.format);
}


void NGPU::SetViewport(int x, int y, int width, int height)
{
	D3D11_VIEWPORT viewport;

	viewport.Width = (float)width;
	viewport.Height = (float)height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = (float)x;
	viewport.TopLeftY = (float)y;

	deviceContext->RSSetViewports(1, &viewport);
}


void NGPU::SetViewport(int width, int height)
{
	SetViewport(0, 0, width, height);
}


void NGPU::SetViewport(const STexture& texture)
{
	SetViewport(texture.width, texture.height);
}


bool NGPU::CreateVertexShader(const string& path, const string& entryPointName, const string& shaderMacros, ID3D11VertexShader*& shader)
{
	return CreateVertexShader(gGPUCreateShaderFileType, path, entryPointName, shaderMacros, shader);
}


bool NGPU::CreateVertexShader(const string& path, const string& shaderMacros, ID3D11VertexShader*& shader)
{
	return CreateVertexShader(path, "main", shaderMacros, shader);
}


bool NGPU::CreateVertexShader(const string& path, ID3D11VertexShader*& shader)
{
	return CreateVertexShader(path, "", shader);
}


bool NGPU::CreatePixelShader(const string& path, const string& entryPointName, const string& shaderMacros, ID3D11PixelShader*& shader)
{
	return CreatePixelShader(gGPUCreateShaderFileType, path, entryPointName, shaderMacros, shader);
}


bool NGPU::CreatePixelShader(const string& path, const string& shaderMacros, ID3D11PixelShader*& shader)
{
	return CreatePixelShader(path, "main", shaderMacros, shader);
}


bool NGPU::CreatePixelShader(const string& path, ID3D11PixelShader*& shader)
{
	return CreatePixelShader(path, "", shader);
}


bool NGPU::CreateComputeShader(const string& path, const string& entryPointName, const string& shaderMacros, ID3D11ComputeShader*& shader)
{
	return CreateComputeShader(gGPUCreateShaderFileType, path, entryPointName, shaderMacros, shader);
}


bool NGPU::CreateComputeShader(const string& path, const string& shaderMacros, ID3D11ComputeShader*& shader)
{
	return CreateComputeShader(path, "main", shaderMacros, shader);
}


bool NGPU::CreateComputeShader(const string& path, ID3D11ComputeShader*& shader)
{
	return CreateComputeShader(path, "", shader);
}


bool NGPU::CreateVertexShaderFromSourceFile(const string& path, const string& entryPointName, const string& shaderMacros, ID3D11VertexShader*& shader)
{
	return CreateVertexShader(EShaderFileType::Source, path, entryPointName, shaderMacros, shader);
}


bool NGPU::CreateVertexShaderFromSourceFile(const string& path, const string& shaderMacros, ID3D11VertexShader*& shader)
{
	return CreateVertexShader(path, "main", shaderMacros, shader);
}


bool NGPU::CreateVertexShaderFromSourceFile(const string& path, ID3D11VertexShader*& shader)
{
	return CreateVertexShader(path, "", shader);
}


bool NGPU::CreateVertexShaderFromBlobFile(const string& path, const string& entryPointName, const string& shaderMacros, ID3D11VertexShader*& shader)
{
	return CreateVertexShader(EShaderFileType::Blob, path, entryPointName, shaderMacros, shader);
}


bool NGPU::CreateVertexShaderFromBlobFile(const string& path, const string& shaderMacros, ID3D11VertexShader*& shader)
{
	return CreateVertexShaderFromBlobFile(path, "main", shaderMacros, shader);
}


bool NGPU::CreateVertexShaderFromBlobFile(const string& path, ID3D11VertexShader*& shader)
{
	return CreateVertexShaderFromBlobFile(path, "", shader);
}


bool NGPU::CreatePixelShaderFromSourceFile(const string& path, const string& entryPointName, const string& shaderMacros, ID3D11PixelShader*& shader)
{
	return CreatePixelShader(EShaderFileType::Source, path, entryPointName, shaderMacros, shader);
}


bool NGPU::CreatePixelShaderFromSourceFile(const string& path, const string& shaderMacros, ID3D11PixelShader*& shader)
{
	return CreatePixelShader(path, "main", shaderMacros, shader);
}


bool NGPU::CreatePixelShaderFromSourceFile(const string& path, ID3D11PixelShader*& shader)
{
	return CreatePixelShader(path, "", shader);
}


bool NGPU::CreatePixelShaderFromBlobFile(const string& path, const string& entryPointName, const string& shaderMacros, ID3D11PixelShader*& shader)
{
	return CreatePixelShader(EShaderFileType::Blob, path, entryPointName, shaderMacros, shader);
}


bool NGPU::CreatePixelShaderFromBlobFile(const string& path, const string& shaderMacros, ID3D11PixelShader*& shader)
{
	return CreatePixelShaderFromBlobFile(path, "main", shaderMacros, shader);
}


bool NGPU::CreatePixelShaderFromBlobFile(const string& path, ID3D11PixelShader*& shader)
{
	return CreatePixelShaderFromBlobFile(path, "", shader);
}


bool NGPU::CreateComputeShaderFromSourceFile(const string& path, const string& entryPointName, const string& shaderMacros, ID3D11ComputeShader*& shader)
{
	return CreateComputeShader(EShaderFileType::Source, path, entryPointName, shaderMacros, shader);
}


bool NGPU::CreateComputeShaderFromSourceFile(const string& path, const string& shaderMacros, ID3D11ComputeShader*& shader)
{
	return CreateComputeShader(path, "main", shaderMacros, shader);
}


bool NGPU::CreateComputeShaderFromSourceFile(const string& path, ID3D11ComputeShader*& shader)
{
	return CreateComputeShader(path, "", shader);
}


bool NGPU::CreateComputeShaderFromBlobFile(const string& path, const string& entryPointName, const string& shaderMacros, ID3D11ComputeShader*& shader)
{
	return CreateComputeShader(EShaderFileType::Blob, path, entryPointName, shaderMacros, shader);
}


bool NGPU::CreateComputeShaderFromBlobFile(const string& path, const string& shaderMacros, ID3D11ComputeShader*& shader)
{
	return CreateComputeShaderFromBlobFile(path, "main", shaderMacros, shader);
}


bool NGPU::CreateComputeShaderFromBlobFile(const string& path, ID3D11ComputeShader*& shader)
{
	return CreateComputeShaderFromBlobFile(path, "", shader);
}


#endif
