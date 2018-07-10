#include "../../../src/main.h"
#include "../../../src/namespaces.h"


struct SMeshConstantBuffer
{
	SMatrix worldTransform;
	SMatrix viewProjTransform;
} meshConstantBuffer;


bool fullScreen = false;
int screenWidth;
int screenHeight;

int viewMode = 0; // 0 - diffuse with SSAO, 1 - diffuse, 2 - SSAO, 3 - raw SSAO
int ssaoVariant = 0;
float ssaoRadius_world = 1.0f;
float ssaoMaxRadius_screen = 0.1f;
float ssaoContrast = 4.0f;


CApplication application;

TRenderTarget gbufferDiffuseRT;
TRenderTarget gbufferNormalRT;
TRenderTarget depth16RT_x4;
TRenderTarget ssaoRT_x4;
TRenderTarget ssaoBlurXRT_x4;
TRenderTarget ssaoBlurRT_x4;
TRenderTarget ssaoUpsampleRT;
TRenderTarget compositeRT;
TDepthStencilTarget depthStencilTarget;

ID3D11VertexShader* meshVS = nullptr;

ID3D11PixelShader* meshGBufferPS = nullptr;
ID3D11PixelShader* downsamplePS = nullptr;
ID3D11PixelShader* ssaoPS[4] = { nullptr, nullptr, nullptr, nullptr };
ID3D11PixelShader* ssaoBlurPS = nullptr;
ID3D11PixelShader* ssaoUpsamplePS = nullptr;
ID3D11PixelShader* compositePS = nullptr;

NUtils::CMesh sponzaMesh;
map<string, NUtils::CTexture> textures;

CCamera camera;


void CreateShaders()
{
	MF_ASSERT(CreateVertexShader("../../data/mesh_vs.hlsl", meshVS));

	MF_ASSERT(CreatePixelShader("../../data/mesh_gbuffer_ps.hlsl", meshGBufferPS));
	MF_ASSERT(CreatePixelShader("../../data/downsample_ps.hlsl", downsamplePS));
	MF_ASSERT(CreatePixelShader("../../data/ssao_ps.hlsl", "VARIANT=1", ssaoPS[0]));
	MF_ASSERT(CreatePixelShader("../../data/ssao_ps.hlsl", "VARIANT=2", ssaoPS[1]));
	MF_ASSERT(CreatePixelShader("../../data/ssao_ps.hlsl", "VARIANT=3", ssaoPS[2]));
	MF_ASSERT(CreatePixelShader("../../data/ssao_ps.hlsl", "VARIANT=4", ssaoPS[3]));
	MF_ASSERT(CreatePixelShader("../../data/ssao_blur_ps.hlsl", ssaoBlurPS));
	MF_ASSERT(CreatePixelShader("../../data/ssao_upsample_ps.hlsl", ssaoUpsamplePS));
	MF_ASSERT(CreatePixelShader("../../data/composite_ps.hlsl", compositePS));
}


void DestroyShaders()
{
	DestroyPixelShader(meshGBufferPS);
	DestroyPixelShader(downsamplePS);
	for (int i = 0; i < ARRAY_SIZE(ssaoPS); i++)
		DestroyPixelShader(ssaoPS[i]);
	DestroyPixelShader(ssaoBlurPS);
	DestroyPixelShader(ssaoUpsamplePS);
	DestroyPixelShader(compositePS);

	DestroyVertexShader(meshVS);
}


bool Create()
{
	CreateD3D11(screenWidth, screenHeight, fullScreen);

	gGPUProfiler.Create();
	gGPUUtilsResources.Create("../../../../");

	DXGI_FORMAT floatFormat = DXGI_FORMAT_R11G11B10_FLOAT;
	CreateRenderTarget(screenWidth, screenHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 1, gbufferDiffuseRT);
	CreateRenderTarget(screenWidth, screenHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 1, gbufferNormalRT);
	CreateRenderTarget(screenWidth/2, screenHeight/2, DXGI_FORMAT_R16_FLOAT, 1, depth16RT_x4);
	CreateRenderTarget(screenWidth/2, screenHeight/2, DXGI_FORMAT_R8G8B8A8_UNORM, 1, ssaoRT_x4);
	CreateRenderTarget(screenWidth/2, screenHeight/2, DXGI_FORMAT_R8G8B8A8_UNORM, 1, ssaoBlurXRT_x4);
	CreateRenderTarget(screenWidth/2, screenHeight/2, DXGI_FORMAT_R8G8B8A8_UNORM, 1, ssaoBlurRT_x4);
	CreateRenderTarget(screenWidth, screenHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 1, ssaoUpsampleRT);
	CreateRenderTarget(screenWidth, screenHeight, floatFormat, 1, compositeRT);
	CreateDepthStencilTarget(screenWidth, screenHeight, 1, depthStencilTarget);

	CreateShaders();

	sponzaMesh.CreateFromFile("../../../common/data/sponza/sponza.obj", NUtils::CMesh::EFileType::OBJ, &textures);

	camera.UpdateFixed(VectorCustom(0.0f, 50.0f, 50.0f), VectorCustom(0.0f, 0.0f, 0.0f));

	return true;
}


void Destroy()
{
	for (auto it = textures.begin(); it != textures.end(); it++)
		it->second.Destroy();
	sponzaMesh.Destroy();

	DestroyShaders();

	DestroyRenderTarget(gbufferDiffuseRT);
	DestroyRenderTarget(gbufferNormalRT);
	DestroyRenderTarget(depth16RT_x4);
	DestroyRenderTarget(ssaoRT_x4);
	DestroyRenderTarget(ssaoBlurXRT_x4);
	DestroyRenderTarget(ssaoBlurRT_x4);
	DestroyRenderTarget(ssaoUpsampleRT);
	DestroyRenderTarget(compositeRT);
	DestroyDepthStencilTarget(depthStencilTarget);

	gGPUProfiler.Destroy();
	gGPUUtilsResources.Destroy();

	DestroyD3D11();
}


void DumpVogelDiskSamples()
{
	vector<SVector2> samples = VogelDiskSamples(16);

	for (int i = 0; i < 16; i++)
	{
		string s = "float2(" + ToString(samples[i].x) + "f, " + ToString(samples[i].y) + "f),\n";
		OutputDebugStringA(s.c_str());
	}
}


void DumpAlchemySpiralDiskSamples()
{
	vector<SVector2> samples = AlchemySpiralDiskSamples(16, 7);

	for (int i = 0; i < 16; i++)
	{
		string s = "float2(" + ToString(samples[i].x) + "f, " + ToString(samples[i].y) + "f),\n";
		OutputDebugStringA(s.c_str());
	}
}


void SSAO(const SMatrix& viewTransform, const SMatrix& projTransform, int variant, float radius_world, float maxRadius_screen, float contrast)
{
	CProfilerScopedQuery psq("All");

	SetViewport(screenWidth/2, screenHeight/2);

	// downsample
	{
		CProfilerScopedQuery psq("Downsample");

		deviceContext->OMSetRenderTargets(1, &depth16RT_x4.rtv, nullptr);

		deviceContext->VSSetShader(gGPUUtilsResources.screenQuadVS, nullptr, 0);
		deviceContext->PSSetShader(downsamplePS, nullptr, 0);
		deviceContext->PSSetShaderResources(0, 1, &depthStencilTarget.srv);

		deviceContext->IASetIndexBuffer(gGPUUtilsResources.screenQuadIB.buffer, DXGI_FORMAT_R16_UINT, 0);

		struct SParams
		{
			SVector2 pixelSize;
			SVector2 projParams;
		} params;
		params.pixelSize = VectorCustom(2.0f/(float)screenWidth, 2.0f/(float)screenHeight);
		params.projParams = VectorCustom(projTransform.m[2][2], projTransform.m[3][2]);
		deviceContext->UpdateSubresource(gGPUUtilsResources.ConstantBuffer(1).buffer, 0, nullptr, &params, 0, 0);
		deviceContext->PSSetConstantBuffers(0, 1, &gGPUUtilsResources.ConstantBuffer(1).buffer);

		deviceContext->DrawIndexed(6, 0, 0);
	}

	// computation
	{
		CProfilerScopedQuery psq("Computation");

		deviceContext->OMSetRenderTargets(1, &ssaoRT_x4.rtv, nullptr);

		deviceContext->VSSetShader(gGPUUtilsResources.screenQuadVS, nullptr, 0);
		deviceContext->PSSetShader(ssaoPS[variant], nullptr, 0);
		deviceContext->PSSetShaderResources(0, 1, &depth16RT_x4.srv);
		deviceContext->PSSetShaderResources(1, 1, &gbufferNormalRT.srv);

		deviceContext->IASetIndexBuffer(gGPUUtilsResources.screenQuadIB.buffer, DXGI_FORMAT_R16_UINT, 0);

		struct SParams
		{
			SVector2 pixelSize;
			SVector2 nearPlaneSize_normalized;
			SMatrix viewTransform;
			float aspect;
			float radius_world;
			float maxRadius_screen;
			float contrast;
		} params;
		params.pixelSize = VectorCustom(2.0f/(float)screenWidth, 2.0f/(float)screenHeight);
		params.nearPlaneSize_normalized = PlaneSize(1.0f, cPi/3.0f, (float)screenWidth/(float)screenHeight);
		params.viewTransform = viewTransform;
		params.aspect = (float)screenWidth / (float)screenHeight;
		params.radius_world = radius_world;
		params.maxRadius_screen = maxRadius_screen;
		params.contrast = contrast;
		deviceContext->UpdateSubresource(gGPUUtilsResources.ConstantBuffer(6).buffer, 0, nullptr, &params, 0, 0);
		deviceContext->PSSetConstantBuffers(0, 1, &gGPUUtilsResources.ConstantBuffer(6).buffer);

		deviceContext->DrawIndexed(6, 0, 0);
	}

	// blur X
	{
		CProfilerScopedQuery psq("Blur X");

		deviceContext->OMSetRenderTargets(1, &ssaoBlurXRT_x4.rtv, nullptr);

		deviceContext->VSSetShader(gGPUUtilsResources.screenQuadVS, nullptr, 0);
		deviceContext->PSSetShader(ssaoBlurPS, nullptr, 0);
		deviceContext->PSSetShaderResources(0, 1, &depth16RT_x4.srv);
		deviceContext->PSSetShaderResources(1, 1, &ssaoRT_x4.srv);

		deviceContext->IASetIndexBuffer(gGPUUtilsResources.screenQuadIB.buffer, DXGI_FORMAT_R16_UINT, 0);

		struct SParams
		{
			SVector2 pixelOffset;
			SVector2 padding;
		} params;
		params.pixelOffset = VectorCustom(2.0f/(float)screenWidth, 0.0f);
		deviceContext->UpdateSubresource(gGPUUtilsResources.ConstantBuffer(1).buffer, 0, nullptr, &params, 0, 0);
		deviceContext->PSSetConstantBuffers(0, 1, &gGPUUtilsResources.ConstantBuffer(1).buffer);

		deviceContext->DrawIndexed(6, 0, 0);
	}

	// blur Y
	{
		CProfilerScopedQuery psq("Blur Y");

		deviceContext->OMSetRenderTargets(1, &ssaoBlurRT_x4.rtv, nullptr);

		deviceContext->VSSetShader(gGPUUtilsResources.screenQuadVS, nullptr, 0);
		deviceContext->PSSetShader(ssaoBlurPS, nullptr, 0);
		deviceContext->PSSetShaderResources(0, 1, &depth16RT_x4.srv);
		deviceContext->PSSetShaderResources(1, 1, &ssaoBlurXRT_x4.srv);

		deviceContext->IASetIndexBuffer(gGPUUtilsResources.screenQuadIB.buffer, DXGI_FORMAT_R16_UINT, 0);

		struct SParams
		{
			SVector2 pixelOffset;
			SVector2 padding;
		} params;
		params.pixelOffset = VectorCustom(0.0f, 2.0f/(float)screenHeight);
		deviceContext->UpdateSubresource(gGPUUtilsResources.ConstantBuffer(1).buffer, 0, nullptr, &params, 0, 0);
		deviceContext->PSSetConstantBuffers(0, 1, &gGPUUtilsResources.ConstantBuffer(1).buffer);

		deviceContext->DrawIndexed(6, 0, 0);
	}

	SetViewport(screenWidth, screenHeight);

	// upsample
	{
		CProfilerScopedQuery psq("Upsample");

		deviceContext->OMSetRenderTargets(1, &ssaoUpsampleRT.rtv, nullptr);

		deviceContext->VSSetShader(gGPUUtilsResources.screenQuadVS, nullptr, 0);
		deviceContext->PSSetShader(ssaoUpsamplePS, nullptr, 0);
		deviceContext->PSSetShaderResources(0, 1, &depthStencilTarget.srv);
		deviceContext->PSSetShaderResources(1, 1, &depth16RT_x4.srv);
		deviceContext->PSSetShaderResources(2, 1, &ssaoBlurRT_x4.srv);

		deviceContext->IASetIndexBuffer(gGPUUtilsResources.screenQuadIB.buffer, DXGI_FORMAT_R16_UINT, 0);

		struct SParams
		{
			SVector2 pixelSize;
			SVector2 projParams;
		} params;
		params.pixelSize = VectorCustom(1.0f/(float)screenWidth, 1.0f/(float)screenHeight);
		params.projParams = VectorCustom(projTransform.m[2][2], projTransform.m[3][2]);
		deviceContext->UpdateSubresource(gGPUUtilsResources.ConstantBuffer(1).buffer, 0, nullptr, &params, 0, 0);
		deviceContext->PSSetConstantBuffers(0, 1, &gGPUUtilsResources.ConstantBuffer(1).buffer);

		deviceContext->DrawIndexed(6, 0, 0);
	}
}


bool Run()
{
	float lastFrameTime = application.LastFrameTime();

	//

	SVector3 eye;

	float speed = 0.025f * lastFrameTime;
	if (application.IsKeyPressed(EKey::LShift))
		speed *= 8.0f;

	eye = camera.eye;
	if (application.IsKeyPressed(EKey::W))
		eye = eye + speed*camera.forwardVector;
	if (application.IsKeyPressed(EKey::S))
		eye = eye - speed*camera.forwardVector;
	if (application.IsKeyPressed(EKey::A))
		eye = eye - speed*camera.rightVector;
	if (application.IsKeyPressed(EKey::D))
		eye = eye + speed*camera.rightVector;

	camera.horizontalAngle -= application.MouseRelX() / 1000.0f;
	camera.verticalAngle -= application.MouseRelY() / 1000.0f;

	camera.UpdateFree(eye);

	//

	if (application.IsKeyPressed(EKey::F1))
		viewMode = 0;
	if (application.IsKeyPressed(EKey::F2))
		viewMode = 1;
	if (application.IsKeyPressed(EKey::F3))
		viewMode = 2;
	if (application.IsKeyPressed(EKey::F4))
		viewMode = 3;

	if (application.IsKeyPressed(EKey::F5))
		ssaoVariant = 0;
	if (application.IsKeyPressed(EKey::F6))
		ssaoVariant = 1;
	if (application.IsKeyPressed(EKey::F7))
		ssaoVariant = 2;
	if (application.IsKeyPressed(EKey::F8))
		ssaoVariant = 3;

	if (application.IsKeyPressed(EKey::Insert))
		ssaoRadius_world += 0.001f * lastFrameTime;
	if (application.IsKeyPressed(EKey::Delete))
		ssaoRadius_world -= 0.001f * lastFrameTime;

	if (application.IsKeyPressed(EKey::Home))
		ssaoMaxRadius_screen += 0.001f * lastFrameTime;
	if (application.IsKeyPressed(EKey::End))
		ssaoMaxRadius_screen -= 0.001f * lastFrameTime;

	if (application.IsKeyPressed(EKey::PageUp))
		ssaoContrast += 0.005f * lastFrameTime;
	if (application.IsKeyPressed(EKey::PageDown))
		ssaoContrast -= 0.005f * lastFrameTime;

	//

	gGPUProfiler.StartFrame();

	float backgroundColor[] = { 0.2f, 0.2f, 0.2f, 1.0f };

	deviceContext->RSSetState(gGPUUtilsResources.ccwRasterizerState);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	SMatrix viewTransform = MatrixLookAtRH(camera.eye, camera.at, camera.up);
	SMatrix projTransform = MatrixPerspectiveFovRH(EZRange::ZeroToOne, cPi/3.0f, (float)screenWidth/(float)screenHeight, 1.0f, 1000.0f);

	SetViewport(screenWidth, screenHeight);

	SetSamplers();

	// gbuffer
	{
		ID3D11RenderTargetView* rtvs[] = { gbufferDiffuseRT.rtv, gbufferNormalRT.rtv };
		deviceContext->OMSetRenderTargets(2, rtvs, depthStencilTarget.dsv);
		deviceContext->ClearDepthStencilView(depthStencilTarget.dsv, D3D11_CLEAR_DEPTH, 1.0f, 0);

		deviceContext->VSSetShader(meshVS, nullptr, 0);
		deviceContext->PSSetShader(meshGBufferPS, nullptr, 0);

		deviceContext->IASetInputLayout(gGPUUtilsResources.posNorUV0UV1InputLayout);

		meshConstantBuffer.worldTransform = MatrixScale(0.1f, 0.1f, 0.1f);
		meshConstantBuffer.viewProjTransform = viewTransform * projTransform;
		deviceContext->UpdateSubresource(gGPUUtilsResources.ConstantBuffer(8).buffer, 0, nullptr, &meshConstantBuffer, 0, 0);
		deviceContext->VSSetConstantBuffers(0, 1, &gGPUUtilsResources.ConstantBuffer(8).buffer);

		for (uint i = 0; i < sponzaMesh.chunks.size(); i++)
		{
			UINT stride = sizeof(SVertex_PosNorUV0UV1);
			UINT offset = 0;
			deviceContext->IASetVertexBuffers(0, 1, &sponzaMesh.chunks[i].vb.buffer, &stride, &offset);
			deviceContext->IASetIndexBuffer(sponzaMesh.chunks[i].ib.buffer, DXGI_FORMAT_R16_UINT, 0);

			auto texture = textures.find(sponzaMesh.chunks[i].textureFileName);
			if (texture != textures.end())
				deviceContext->PSSetShaderResources(0, 1, &texture->second.texture.srv);

			deviceContext->DrawIndexed(sponzaMesh.chunks[i].indicesCount, 0, 0);
		}
	}

	// SSAO
	{
		SSAO(viewTransform, projTransform, ssaoVariant, ssaoRadius_world, ssaoMaxRadius_screen, ssaoContrast);
	}

	SetViewport(screenWidth, screenHeight);

	// composite diffuse with SSAO
	{
		deviceContext->OMSetRenderTargets(1, &compositeRT.rtv, nullptr);

		deviceContext->VSSetShader(gGPUUtilsResources.screenQuadVS, nullptr, 0);
		deviceContext->PSSetShader(compositePS, nullptr, 0);
		deviceContext->PSSetShaderResources(0, 1, &gbufferDiffuseRT.srv);
		deviceContext->PSSetShaderResources(1, 1, &ssaoUpsampleRT.srv);

		deviceContext->IASetIndexBuffer(gGPUUtilsResources.screenQuadIB.buffer, DXGI_FORMAT_R16_UINT, 0);

		deviceContext->DrawIndexed(6, 0, 0);
	}

	// render full-screen quad
	{
		deviceContext->OMSetRenderTargets(1, &backBufferRTV, nullptr);

		deviceContext->VSSetShader(gGPUUtilsResources.screenQuadVS, nullptr, 0);
		deviceContext->PSSetShader(gGPUUtilsResources.postprocess_copyPointTexturePS, nullptr, 0);
		if (viewMode == 0)
			deviceContext->PSSetShaderResources(0, 1, &compositeRT.srv);
		else if (viewMode == 1)
			deviceContext->PSSetShaderResources(0, 1, &gbufferDiffuseRT.srv);
		else if (viewMode == 2)
			deviceContext->PSSetShaderResources(0, 1, &ssaoUpsampleRT.srv);
		else if (viewMode == 3)
			deviceContext->PSSetShaderResources(0, 1, &ssaoRT_x4.srv);

		deviceContext->IASetIndexBuffer(gGPUUtilsResources.screenQuadIB.buffer, DXGI_FORMAT_R16_UINT, 0);

		deviceContext->DrawIndexed(6, 0, 0);
	}

	ID3D11ShaderResourceView* nullSRVS[] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
	deviceContext->PSSetShaderResources(0, 6, nullSRVS);

	swapChain->Present(0, 0);

	gGPUProfiler.EndFrame();
	gGPUProfiler.StopProfiling();
	if (!gGPUProfiler.isProfiling)
		gGPUProfiler.StartProfiling();

	//

	if (application.IsKeyPressed(EKey::F11))
	{
		DestroyShaders();
		CreateShaders();
	}

	if (application.IsKeyPressed(EKey::Escape))
		return false;

	//

	return true;
}


void LoadConfigFile()
{
	CConfigFile configFile;
	configFile.Open("config.txt");
	configFile.Process("fullScreen", fullScreen);
	configFile.Process("screenWidth", screenWidth);
	configFile.Process("screenHeight", screenHeight);
	configFile.Close();
}


int main()
{
	NSystem::Initialize();
	NImage::Initialize();

	NSystem::ScreenSize(screenWidth, screenHeight);
	LoadConfigFile();

	if (!application.Create(screenWidth, screenHeight, fullScreen))
		return 1;
	application.SetMouseWrapping(true);
	application.ShowCursor(false);

	Create();
	application.Run(Run);
	Destroy();

	application.Destroy();

	NSystem::Deinitialize();
	NImage::Deinitialize();

	return 0;
}
