// shadow map
// render to half-res, blur smart and upsample


#include "../../../src/main.h"
#include "../../../src/namespaces.h"


bool fullScreen = false;
int screenWidth;
int screenHeight;

int lightVolumeTextureWidth = 160;
int lightVolumeTextureHeight = 88;
int lightVolumeTextureDepth = 64;
float fovY = cPi / 3.0f;
float nearPlaneDistance = 3.0f;
float farPlaneDistance = 300.0f;
SMatrix viewTransforms[2];


CApplication application;

TRenderTarget gbufferDiffuseRT;
TRenderTarget gbufferNormalRT;
TRenderTarget compositeRT;
TDepthStencilTarget depthStencilTarget;

STexture lightVolumeTexture3D[2];
STexture lightVolumeIntegratedTexture3D;

ID3D11VertexShader* meshVS = nullptr;

ID3D11PixelShader* meshGBufferPS = nullptr;
ID3D11PixelShader* compositePS = nullptr;
ID3D11ComputeShader* lightCalculateCS = nullptr;
ID3D11ComputeShader* lightIntegrateCS = nullptr;
ID3D11ComputeShader* blur3DXCS = nullptr;
ID3D11ComputeShader* blur3DYCS = nullptr;
ID3D11ComputeShader* blur3DZCS = nullptr;

NUtils::CMesh sponzaMesh;
map<string, NUtils::CTexture> textures;

CCamera camera;


void CreateShaders()
{
	MF_ASSERT(CreateVertexShader("../../data/mesh_vs.hlsl", meshVS));

	MF_ASSERT(CreatePixelShader("../../data/mesh_gbuffer_ps.hlsl", meshGBufferPS));
	MF_ASSERT(CreatePixelShader("../../data/composite_ps.hlsl", compositePS));

	MF_ASSERT(CreateComputeShader("../../data/light_calculate_cs.hlsl",
		"LIGHT_VOLUME_TEXTURE_WIDTH=" + ToString(lightVolumeTextureWidth) + "|" +
		"LIGHT_VOLUME_TEXTURE_HEIGHT=" + ToString(lightVolumeTextureHeight) + "|" +
		"LIGHT_VOLUME_TEXTURE_DEPTH=" + ToString(lightVolumeTextureDepth),
		lightCalculateCS));
	MF_ASSERT(CreateComputeShader("../../data/light_integrate_cs.hlsl",
		"LIGHT_VOLUME_TEXTURE_DEPTH=" + ToString(lightVolumeTextureDepth),
		lightIntegrateCS));
	MF_ASSERT(CreateComputeShader("../../data/blur_3d.hlsl", "X", blur3DXCS));
	MF_ASSERT(CreateComputeShader("../../data/blur_3d.hlsl", "Y", blur3DYCS));
	MF_ASSERT(CreateComputeShader("../../data/blur_3d.hlsl", "Z", blur3DZCS));
}


void DestroyShaders()
{
	DestroyComputeShader(lightCalculateCS);
	DestroyComputeShader(lightIntegrateCS);
	DestroyComputeShader(blur3DXCS);
	DestroyComputeShader(blur3DYCS);
	DestroyComputeShader(blur3DZCS);

	DestroyPixelShader(meshGBufferPS);
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
	CreateRenderTarget(screenWidth, screenHeight, floatFormat, 1, compositeRT);
	CreateDepthStencilTarget(screenWidth, screenHeight, 1, depthStencilTarget);

	CreateRWTexture3D(lightVolumeTextureWidth, lightVolumeTextureHeight, lightVolumeTextureDepth, 1, DXGI_FORMAT_R16G16B16A16_FLOAT, lightVolumeTexture3D[0]);
	CreateRWTexture3D(lightVolumeTextureWidth, lightVolumeTextureHeight, lightVolumeTextureDepth, 1, DXGI_FORMAT_R16G16B16A16_FLOAT, lightVolumeTexture3D[1]);
	CreateRWTexture3D(lightVolumeTextureWidth, lightVolumeTextureHeight, lightVolumeTextureDepth, 1, DXGI_FORMAT_R16G16B16A16_FLOAT, lightVolumeIntegratedTexture3D);

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

	DestroyTexture(lightVolumeTexture3D[0]);
	DestroyTexture(lightVolumeTexture3D[1]);
	DestroyTexture(lightVolumeIntegratedTexture3D);

	DestroyRenderTarget(gbufferDiffuseRT);
	DestroyRenderTarget(gbufferNormalRT);
	DestroyRenderTarget(compositeRT);
	DestroyDepthStencilTarget(depthStencilTarget);

	gGPUProfiler.Destroy();
	gGPUUtilsResources.Destroy();

	DestroyD3D11();
}


void VolumetricFog(int frameIndex, const SVector3& eyePosition, const SMatrix& viewToWorldTransform)
{
	MF_ASSERT(lightVolumeTextureWidth % 4 == 0);
	MF_ASSERT(lightVolumeTextureHeight % 4 == 0);

	// light calculation
	{
		CProfilerScopedQuery psq("LightCalculate");

		deviceContext->CSSetUnorderedAccessViews(0, 1, &lightVolumeTexture3D[frameIndex % 2].uav, nullptr);
		deviceContext->CSSetShader(lightCalculateCS, nullptr, 0);

		static float time = 0.0f;
		time += 0.001f * application.LastFrameTime();

		struct SParams
		{
			SMatrix viewToWorldTransform;
			SMatrix viewReprojectTransform;
			SVector2 nearPlaneSize;
			float nearPlaneDistance;
			float viewDistance;
			float dither[3];
			float padding1;
			SVector3 eyePosition;
			float time;
		} params;
		float ditherZ[8] =
		{
			0.3125f,
			0.9375f,
			0.4375f,
			0.8125f,
			0.6875f,
			0.0625f,
			0.1875f,
			0.5625f,
		};
		SVector2 ditherXY[] =
		{
			VectorCustom(0.0997132f, -0.992501f),
			VectorCustom(-0.0314335f, 0.167088f),
			VectorCustom(0.757265f, -0.231721f),
			VectorCustom(-0.47771f, -0.518986f),
			VectorCustom(-0.190608f, -0.963504f),
			VectorCustom(0.448828f, 0.406f),
			VectorCustom(-0.712654f, 0.0617393f),
			VectorCustom(0.609766f, -0.770284f),
		};
		params.viewToWorldTransform = viewToWorldTransform;
		params.viewReprojectTransform = Invert(viewTransforms[frameIndex % 2]) * viewTransforms[(frameIndex - 1) % 2];
		params.nearPlaneSize = PlaneSize(fovY, (float)screenWidth / (float)screenHeight, nearPlaneDistance);
		params.nearPlaneDistance = nearPlaneDistance;
		params.viewDistance = farPlaneDistance - nearPlaneDistance;
		params.dither[0] = ditherXY[frameIndex % 8].x;
		params.dither[1] = ditherXY[frameIndex % 8].y;
		params.dither[2] = ditherZ[frameIndex % 8];
		params.eyePosition = eyePosition;
		params.time = time;
		deviceContext->UpdateSubresource(gGPUUtilsResources.ConstantBuffer(11).buffer, 0, nullptr, &params, 0, 0);
		deviceContext->CSSetConstantBuffers(0, 1, &gGPUUtilsResources.ConstantBuffer(11).buffer);

		deviceContext->CSSetShaderResources(0, 1, &lightVolumeTexture3D[(frameIndex - 1) % 2].srv);
		deviceContext->Dispatch(lightVolumeTextureWidth / 4, lightVolumeTextureHeight / 4, lightVolumeTextureDepth / 4);

		ClearUAVs(0, 1);
		ClearCSShaderResources(0, 1);
	}

	// light integration
	{
		CProfilerScopedQuery psq("LightIntegrate");

		deviceContext->CSSetUnorderedAccessViews(0, 1, &lightVolumeIntegratedTexture3D.uav, nullptr);
		deviceContext->CSSetShader(lightIntegrateCS, nullptr, 0);
		deviceContext->CSSetShaderResources(0, 1, &lightVolumeTexture3D[frameIndex % 2].srv);
		deviceContext->Dispatch(lightVolumeTextureWidth / 8, lightVolumeTextureHeight / 8, 1);

		ClearUAVs(0, 1);
		ClearCSShaderResources(0, 1);
	}
}


bool Run()
{
	float lastFrameTime = application.LastFrameTime();

	static int frameIndex = 10;

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

	gGPUProfiler.StartFrame();

	float backgroundColor[] = { 0.2f, 0.2f, 0.2f, 1.0f };

	deviceContext->RSSetState(gGPUUtilsResources.ccwRasterizerState);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	SMatrix viewTransform = MatrixLookAtRH(camera.eye, camera.at, camera.up);
	SMatrix projTransform = MatrixPerspectiveFovRH(EZRange::ZeroToOne, fovY, (float)screenWidth/(float)screenHeight, nearPlaneDistance, farPlaneDistance);
	SMatrix viewProjTransform = viewTransform * projTransform;
	viewTransforms[frameIndex % 2] = viewTransform;

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

		struct SMeshConstantBuffer
		{
			SMatrix worldTransform;
			SMatrix viewProjTransform;
		} meshConstantBuffer;
		meshConstantBuffer.worldTransform = MatrixScale(0.1f, 0.1f, 0.1f);
		meshConstantBuffer.viewProjTransform = viewProjTransform;
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

	VolumetricFog(frameIndex, camera.eye, Invert(viewTransform));

	// composite
	{
		CProfilerScopedQuery psq("Composite");
	
		SetViewport(screenWidth, screenHeight);
		deviceContext->OMSetRenderTargets(1, &compositeRT.rtv, nullptr);
	
		deviceContext->VSSetShader(gGPUUtilsResources.screenQuadVS, nullptr, 0);
		deviceContext->PSSetShader(compositePS, nullptr, 0);
		deviceContext->PSSetShaderResources(0, 1, &depthStencilTarget.srv);
		deviceContext->PSSetShaderResources(1, 1, &lightVolumeIntegratedTexture3D.srv);
		deviceContext->PSSetShaderResources(2, 1, &gbufferDiffuseRT.srv);
	
		deviceContext->IASetIndexBuffer(gGPUUtilsResources.screenQuadIB.buffer, DXGI_FORMAT_R16_UINT, 0);
	
		struct SParams
		{
			SVector2 projParams;
			float nearPlaneDistance;
			float viewDistance;
		} params;
		params.projParams = VectorCustom(projTransform.m[2][2], projTransform.m[3][2]);
		params.nearPlaneDistance = nearPlaneDistance;
		params.viewDistance = farPlaneDistance - nearPlaneDistance;
		deviceContext->UpdateSubresource(gGPUUtilsResources.ConstantBuffer(1).buffer, 0, nullptr, &params, 0, 0);
		deviceContext->PSSetConstantBuffers(0, 1, &gGPUUtilsResources.ConstantBuffer(1).buffer);
	
		deviceContext->DrawIndexed(6, 0, 0);

		ClearRenderTargets();
		ClearPSShaderResources(0, 3);
	}

	// render full-screen quad
	{
		SetViewport(screenWidth, screenHeight);
		deviceContext->OMSetRenderTargets(1, &backBufferRTV, nullptr);

		deviceContext->VSSetShader(gGPUUtilsResources.screenQuadVS, nullptr, 0);
		deviceContext->PSSetShader(gGPUUtilsResources.postprocess_copyPointTexturePS, nullptr, 0);
		deviceContext->PSSetShaderResources(0, 1, &compositeRT.srv);

		deviceContext->IASetIndexBuffer(gGPUUtilsResources.screenQuadIB.buffer, DXGI_FORMAT_R16_UINT, 0);

		deviceContext->DrawIndexed(6, 0, 0);

		ClearRenderTargets();
		ClearPSShaderResources(0, 1);
	}

	swapChain->Present(2, 0);

	gGPUProfiler.EndFrame();
	gGPUProfiler.StopProfiling();
	if (!gGPUProfiler.isProfiling)
		gGPUProfiler.StartProfiling();

	frameIndex++;

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
	application.MouseShowCursor(false);
	application.MouseSetRelativeMode(true);

	Create();
	application.Run(Run);
	Destroy();

	application.Destroy();

	NSystem::Deinitialize();
	NImage::Deinitialize();

	return 0;
}
