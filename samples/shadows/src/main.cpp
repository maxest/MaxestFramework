#include "../../../src/main.h"
#include "../../../src/namespaces.h"


enum class ShadowMode { NonContactHardening, ContactHardeningAvgBlockers, ContactHardeningMinFilter };
string ToString(ShadowMode shadowMode)
{
	if (shadowMode == ShadowMode::NonContactHardening) return "NonContactHardening";
	else if (shadowMode == ShadowMode::ContactHardeningAvgBlockers) return "ContactHardeningAvgBlockers";
	else if (shadowMode == ShadowMode::ContactHardeningMinFilter) return "ContactHardeningMinFilter";
	else return "";
}


struct SMeshInstance
{
	NUtils::CMesh* mesh;
	SMatrix worldTransform;
};


struct SShadowMapConstantBuffer
{
	SMatrix worldViewTransform;
	SMatrix worldViewProjTransform;
} shadowMapConstantBuffer;


struct SMeshConstantBuffer
{
	SMatrix worldTransform;
	SMatrix viewProjTransform;
} meshConstantBuffer;


bool fullScreen = false;
int screenWidth;
int screenHeight;
int shadowMapWidth = 2 * 1024;
int shadowMapHeight = 2 * 1024;

ShadowMode shadowMode = ShadowMode::ContactHardeningAvgBlockers;
bool penumbraMask = true;
bool penumbraDownsampledShadowMap = false;
bool penumbraLowSamplesCount = false;
bool checker = false;
bool checkerUpsampleDepthBuffer16 = false;
bool shadowOnly = true;
float shadowMapSize = 500.0f;
float shadowFilterMaxSize = 0.002f;
float penumbraFilterScale = 1.2f;


CApplication application;

TRenderTarget depthBuffer16RT;
TRenderTarget shadowMapRT, shadowMapRT_x4, shadowMapMinBlurredRT;
TRenderTarget penumbraRT;
TRenderTarget shadowMaskCheckerRT;
TRenderTarget shadowMaskRT;
TRenderTarget lightingRT;
TDepthStencilTarget backBufferDST;
TDepthStencilTarget shadowMapDST;
STexture ignTexture; // interleaved gradient noise

ID3D11VertexShader* shadowMapVS = nullptr;
ID3D11VertexShader* meshVS = nullptr;

ID3D11PixelShader* shadowMapPS = nullptr;
ID3D11PixelShader* penumbraPS[4];
ID3D11PixelShader* shadowMaskPS[14];
ID3D11PixelShader* shadowMaskCheckerUpsamplePS[2];
ID3D11PixelShader* meshPS[2];

NUtils::CMesh sponzaMesh, planeMesh, boxMesh;
map<string, NUtils::CTexture> textures;
vector<SMeshInstance> meshesInstances;
SMatrix worldTransform = MatrixScale(10.0f, 10.0f, 10.0f);

CCamera camera;


ID3D11PixelShader* PenumbraPS()
{
	int shaderIndex = 0;
	if (penumbraDownsampledShadowMap)
		shaderIndex += 1;
	if (penumbraLowSamplesCount)
		shaderIndex += 2;
	return penumbraPS[shaderIndex];
}
ID3D11PixelShader* ShadowMaskPS()
{
	int checkerShaderOffset = 0;
	if (checker)
		checkerShaderOffset += 7;

	if (shadowMode == ShadowMode::NonContactHardening)
	{
		return shadowMaskPS[checkerShaderOffset + 0];
	}
	else if (shadowMode == ShadowMode::ContactHardeningAvgBlockers)
	{
		if (penumbraMask)
		{
			return shadowMaskPS[checkerShaderOffset + 1];
		}
		else
		{
			int shaderIndex = 2;
			if (penumbraDownsampledShadowMap)
				shaderIndex += 1;
			if (penumbraLowSamplesCount)
				shaderIndex += 2;
			return shadowMaskPS[checkerShaderOffset + shaderIndex];
		}
	}
	else if (shadowMode == ShadowMode::ContactHardeningMinFilter)
	{
		return shadowMaskPS[checkerShaderOffset + 6];
	}
	else
	{
		MF_ASSERT(false);
		return nullptr;
	}
}
ID3D11PixelShader* ShadowMaskCheckerUpsamplePS()
{
	if (checkerUpsampleDepthBuffer16)
		return shadowMaskCheckerUpsamplePS[1];
	else
		return shadowMaskCheckerUpsamplePS[0];
}
ID3D11PixelShader* MeshPS()
{
	if (shadowOnly)
		return meshPS[1];
	else
		return meshPS[0];
}


void Log(const string& msg)
{
	cout << msg << endl;
}


void CreateShaders()
{
	MF_ASSERT(CreateVertexShader("../../data/mesh_vs.hlsl", meshVS));
	MF_ASSERT(CreateVertexShader("../../data/shadow_map_vs.hlsl", shadowMapVS));

	MF_ASSERT(CreatePixelShader("../../data/shadow_map_ps.hlsl", shadowMapPS));
	// penumbra shaders
	{
		MF_ASSERT(CreatePixelShader("../../data/penumbra_ps.hlsl", "SAMPLES_COUNT=32|USE_ORIGINAL_SHADOW_MAP", penumbraPS[0]));
		MF_ASSERT(CreatePixelShader("../../data/penumbra_ps.hlsl", "SAMPLES_COUNT=32|USE_DOWNSAMPLED_SHADOW_MAP", penumbraPS[1]));
		MF_ASSERT(CreatePixelShader("../../data/penumbra_ps.hlsl", "SAMPLES_COUNT=16|USE_ORIGINAL_SHADOW_MAP", penumbraPS[2]));
		MF_ASSERT(CreatePixelShader("../../data/penumbra_ps.hlsl", "SAMPLES_COUNT=16|USE_DOWNSAMPLED_SHADOW_MAP", penumbraPS[3]));
	}
	// shadow mask shaders
	{
		MF_ASSERT(CreatePixelShader("../../data/shadow_mask_ps.hlsl", "", shadowMaskPS[0]));
		MF_ASSERT(CreatePixelShader("../../data/shadow_mask_ps.hlsl", "USE_PENUMBRA|USE_PENUMBRA_MASK", shadowMaskPS[1]));
		MF_ASSERT(CreatePixelShader("../../data/shadow_mask_ps.hlsl", "USE_PENUMBRA|USE_PENUMBRA_REGULAR|PENUMBRA_SAMPLES_COUNT=32|USE_PENUMBRA_ORIGINAL_SHADOW_MAP", shadowMaskPS[2]));
		MF_ASSERT(CreatePixelShader("../../data/shadow_mask_ps.hlsl", "USE_PENUMBRA|USE_PENUMBRA_REGULAR|PENUMBRA_SAMPLES_COUNT=32|USE_PENUMBRA_DOWNSAMPLED_SHADOW_MAP", shadowMaskPS[3]));
		MF_ASSERT(CreatePixelShader("../../data/shadow_mask_ps.hlsl", "USE_PENUMBRA|USE_PENUMBRA_REGULAR|PENUMBRA_SAMPLES_COUNT=16|USE_PENUMBRA_ORIGINAL_SHADOW_MAP", shadowMaskPS[4]));
		MF_ASSERT(CreatePixelShader("../../data/shadow_mask_ps.hlsl", "USE_PENUMBRA|USE_PENUMBRA_REGULAR|PENUMBRA_SAMPLES_COUNT=16|USE_PENUMBRA_DOWNSAMPLED_SHADOW_MAP", shadowMaskPS[5]));
		MF_ASSERT(CreatePixelShader("../../data/shadow_mask_ps.hlsl", "USE_PENUMBRA|USE_MIN_FILTER", shadowMaskPS[6]));
		MF_ASSERT(CreatePixelShader("../../data/shadow_mask_ps.hlsl", "USE_CHECKER", shadowMaskPS[7]));
		MF_ASSERT(CreatePixelShader("../../data/shadow_mask_ps.hlsl", "USE_CHECKER|USE_PENUMBRA|USE_PENUMBRA_MASK", shadowMaskPS[8]));
		MF_ASSERT(CreatePixelShader("../../data/shadow_mask_ps.hlsl", "USE_CHECKER|USE_PENUMBRA|USE_PENUMBRA_REGULAR|PENUMBRA_SAMPLES_COUNT=32|USE_PENUMBRA_ORIGINAL_SHADOW_MAP", shadowMaskPS[9]));
		MF_ASSERT(CreatePixelShader("../../data/shadow_mask_ps.hlsl", "USE_CHECKER|USE_PENUMBRA|USE_PENUMBRA_REGULAR|PENUMBRA_SAMPLES_COUNT=32|USE_PENUMBRA_DOWNSAMPLED_SHADOW_MAP", shadowMaskPS[10]));
		MF_ASSERT(CreatePixelShader("../../data/shadow_mask_ps.hlsl", "USE_CHECKER|USE_PENUMBRA|USE_PENUMBRA_REGULAR|PENUMBRA_SAMPLES_COUNT=16|USE_PENUMBRA_ORIGINAL_SHADOW_MAP", shadowMaskPS[11]));
		MF_ASSERT(CreatePixelShader("../../data/shadow_mask_ps.hlsl", "USE_CHECKER|USE_PENUMBRA|USE_PENUMBRA_REGULAR|PENUMBRA_SAMPLES_COUNT=16|USE_PENUMBRA_DOWNSAMPLED_SHADOW_MAP", shadowMaskPS[12]));
		MF_ASSERT(CreatePixelShader("../../data/shadow_mask_ps.hlsl", "USE_CHECKER|USE_PENUMBRA|USE_MIN_FILTER", shadowMaskPS[13]));
	}
	MF_ASSERT(CreatePixelShader("../../data/shadow_mask_checker_upsample_ps.hlsl", shadowMaskCheckerUpsamplePS[0]));
	MF_ASSERT(CreatePixelShader("../../data/shadow_mask_checker_upsample_ps.hlsl", "DEPTH_BUFFER_16", shadowMaskCheckerUpsamplePS[1]));
	MF_ASSERT(CreatePixelShader("../../data/mesh_ps.hlsl", meshPS[0]));
	MF_ASSERT(CreatePixelShader("../../data/mesh_ps.hlsl", "SHADOW_ONLY", meshPS[1]));
}


void DestroyShaders()
{
	DestroyPixelShader(shadowMapPS);
	for (int i = 0; i < ARRAY_SIZE(penumbraPS); i++)
		DestroyPixelShader(penumbraPS[i]);
	for (int i = 0; i < ARRAY_SIZE(shadowMaskPS); i++)
		DestroyPixelShader(shadowMaskPS[i]);
	for (int i = 0; i < ARRAY_SIZE(shadowMaskCheckerUpsamplePS); i++)
		DestroyPixelShader(shadowMaskCheckerUpsamplePS[i]);
	for (int i = 0; i < ARRAY_SIZE(meshPS); i++)
		DestroyPixelShader(meshPS[i]);

	DestroyVertexShader(meshVS);
	DestroyVertexShader(shadowMapVS);
}


bool Create()
{
	CreateD3D11(screenWidth, screenHeight, false);

	gGPUProfiler.Create();
	gGPUUtilsResources.Create("../../../../");
	gGPUUtilsFontRenderer.Create(screenWidth, screenHeight);
	gGPUUtilsDebugRenderer.Create("../../../../");

	DXGI_FORMAT floatFormat = DXGI_FORMAT_R11G11B10_FLOAT;
	CreateRenderTarget(screenWidth, screenHeight, DXGI_FORMAT_R16_FLOAT, 1, depthBuffer16RT);
	CreateRenderTarget(shadowMapWidth, shadowMapHeight, DXGI_FORMAT_R32_FLOAT, 1, shadowMapRT);
	CreateRenderTarget(shadowMapWidth/2, shadowMapHeight/2, DXGI_FORMAT_R32_FLOAT, 1, shadowMapRT_x4);
	CreateRenderTarget(shadowMapWidth/4, shadowMapHeight/4, DXGI_FORMAT_R32_FLOAT, 1, shadowMapMinBlurredRT);
	CreateRenderTarget(screenWidth/4, screenHeight/4, DXGI_FORMAT_R8_UNORM, 1, penumbraRT);
	CreateRenderTarget(screenWidth/2, screenHeight, DXGI_FORMAT_R8_UNORM, 1, shadowMaskCheckerRT);
	CreateRenderTarget(screenWidth, screenHeight, DXGI_FORMAT_R8_UNORM, 1, shadowMaskRT);
	CreateRenderTarget(screenWidth, screenHeight, floatFormat, 1, lightingRT);
	CreateDepthStencilTarget(screenWidth, screenHeight, 1, backBufferDST);
	CreateDepthStencilTarget(shadowMapWidth, shadowMapHeight, 1, shadowMapDST);
	ignTexture = gGPUUtilsResources.CreateTexture2D_InterleavedGradientNoise(2048, 2048, 0);

	CreateShaders();

	camera.UpdateFixed(VectorCustom(0.0f, 50.0f, 50.0f), VectorCustom(0.0f, 0.0f, 0.0f));

	sponzaMesh.CreateFromFile("../../../common/data/sponza_unity/mesh.dat", NUtils::CMesh::EFileType::UnityExported, &textures);
	planeMesh.CreatePlane(100.0f, 100.0f, VectorCustom(10.0f, 10.0f), MatrixRotateX(-cPi / 2.0f));
	boxMesh.CreateFromFile("../../../../data/meshes/unit_box.ase", NUtils::CMesh::EFileType::ASE);

	SMeshInstance mi;
	mi.mesh = &sponzaMesh;
	mi.worldTransform = worldTransform;
	meshesInstances.push_back(mi);
/*	mi.mesh = &planeMesh;
	mi.worldTransform = worldTransform;
	meshesInstances.push_back(mi);
	for (int i = 0; i < 10; i++)
	{
		float x = RandomFloat(-50.0f, 50.0f);
		float z = RandomFloat(-50.0f, 50.0f);
		float height = RandomFloat(1.0f, 5.0f);

		mi.mesh = &boxMesh;
		mi.worldTransform = worldTransform * MatrixTranslate(x, 5.0f, z) * MatrixScale(1.0f, height, 1.0f);
		meshesInstances.push_back(mi);
	}*/

	return true;
}


void Destroy()
{
	sponzaMesh.Destroy();
	for (auto it = textures.begin(); it != textures.end(); it++)
		it->second.Destroy();
	planeMesh.Destroy();
	boxMesh.Destroy();

	DestroyShaders();

	DestroyRenderTarget(depthBuffer16RT);
	DestroyRenderTarget(shadowMapRT);
	DestroyRenderTarget(shadowMapRT_x4);
	DestroyRenderTarget(shadowMapMinBlurredRT);
	DestroyRenderTarget(penumbraRT);
	DestroyRenderTarget(shadowMaskCheckerRT);
	DestroyRenderTarget(shadowMaskRT);
	DestroyRenderTarget(lightingRT);
	DestroyDepthStencilTarget(backBufferDST);
	DestroyDepthStencilTarget(shadowMapDST);
	DestroyTexture(ignTexture);

	gGPUProfiler.Destroy();
	gGPUUtilsResources.Destroy();
	gGPUUtilsFontRenderer.Destroy();
	gGPUUtilsDebugRenderer.Destroy();

	DestroyD3D11();
}


void RenderDepthBuffer(const SMatrix& cameraViewProjTransform)
{
	SetViewport(screenWidth, screenHeight);
	deviceContext->OMSetRenderTargets(0, nullptr, backBufferDST.dsv);
	deviceContext->ClearDepthStencilView(backBufferDST.dsv, D3D11_CLEAR_DEPTH, 1.0f, 0);

	deviceContext->VSSetShader(meshVS, nullptr, 0);
	deviceContext->PSSetShader(nullptr, nullptr, 0);

	deviceContext->IASetInputLayout(gGPUUtilsResources.posNorUV0UV1InputLayout);

	for (uint i = 0; i < meshesInstances.size(); i++)
	{
		struct SMeshConstantBuffer
		{
			SMatrix worldTransform;
			SMatrix viewProjTransform;
		} data;
		data.worldTransform = meshesInstances[i].worldTransform;
		data.viewProjTransform = cameraViewProjTransform;
		deviceContext->UpdateSubresource(gGPUUtilsResources.ConstantBuffer(8).buffer, 0, nullptr, &data, 0, 0);
		deviceContext->VSSetConstantBuffers(0, 1, &gGPUUtilsResources.ConstantBuffer(8).buffer);

		meshesInstances[i].mesh->Render();
	}
}


void RenderDepthBuffer16(const SMatrix& cameraProjTransform)
{
	NGPU::CProfilerScopedQuery psq("DepthBuffer16");
	SetViewport(depthBuffer16RT);
	gGPUUtilsPostprocessor.Deproject(depthBuffer16RT, backBufferDST, cameraProjTransform);
}


void RenderShadowMap(const SMatrix& lightViewTransform, const SMatrix& lightViewProjTransform)
{
	SetViewport(shadowMapRT);
	deviceContext->OMSetRenderTargets(1, &shadowMapRT.rtv, shadowMapDST.dsv);
	deviceContext->ClearRenderTargetView(shadowMapRT.rtv, &cVector4One.x);
	deviceContext->ClearDepthStencilView(shadowMapDST.dsv, D3D11_CLEAR_DEPTH, 1.0f, 0);

	deviceContext->VSSetShader(shadowMapVS, nullptr, 0);
	deviceContext->PSSetShader(shadowMapPS, nullptr, 0);

	deviceContext->IASetInputLayout(gGPUUtilsResources.posNorUV0UV1InputLayout);

	for (uint i = 0; i < meshesInstances.size(); i++)
	{
		shadowMapConstantBuffer.worldViewTransform = meshesInstances[i].worldTransform * lightViewTransform;
		shadowMapConstantBuffer.worldViewProjTransform = meshesInstances[i].worldTransform * lightViewProjTransform;
		deviceContext->UpdateSubresource(gGPUUtilsResources.ConstantBuffer(8).buffer, 0, nullptr, &shadowMapConstantBuffer, 0, 0);
		deviceContext->VSSetConstantBuffers(0, 1, &gGPUUtilsResources.ConstantBuffer(8).buffer);

		meshesInstances[i].mesh->Render();
	}
}


void RenderShadowMapDownsampleMin2x2()
{
	NGPU::CProfilerScopedQuery psq("ShadowMapDownsampleMin2x2");
	SetViewport(shadowMapRT_x4);
	gGPUUtilsPostprocessor.DownsampleMin2x2(shadowMapRT_x4, shadowMapRT);
}


void RenderShadowMapMinBlurred()
{
	NGPU::CProfilerScopedQuery psq("ShadowMapMinBlurred");
	SetViewport(shadowMapMinBlurredRT);
	gGPUUtilsPostprocessor.DownsampleMin4x4(shadowMapMinBlurredRT, shadowMapRT);
	gGPUUtilsPostprocessor.Min(shadowMapMinBlurredRT, shadowMapMinBlurredRT, 1, shadowMapMinBlurredRT.PixelSize(), -2, 2);
	gGPUUtilsPostprocessor.Blur(shadowMapMinBlurredRT, shadowMapMinBlurredRT, 1, shadowMapMinBlurredRT.PixelSize(), -1, 1);
}


void RenderPenumbraMask(const SMatrix& cameraViewProjTransform, const SMatrix& lightViewProjTransform, const SMatrix& lightProjTransform)
{
	NGPU::CProfilerScopedQuery psq("Penumbra");

	SetViewport(penumbraRT);
	deviceContext->OMSetRenderTargets(1, &penumbraRT.rtv, nullptr);
	deviceContext->ClearRenderTargetView(penumbraRT.rtv, &cVector4One.x);

	deviceContext->VSSetShader(gGPUUtilsResources.screenQuadVS, nullptr, 0);
	deviceContext->PSSetShader(PenumbraPS(), nullptr, 0);

	deviceContext->PSSetShaderResources(0, 1, &backBufferDST.srv);
	deviceContext->PSSetShaderResources(1, 1, &shadowMapRT.srv);
	deviceContext->PSSetShaderResources(2, 1, &shadowMapRT_x4.srv);
	deviceContext->PSSetShaderResources(3, 1, &ignTexture.srv);

	SMatrix data[2];
	data[0] = Invert(cameraViewProjTransform) * lightViewProjTransform;
	data[1] = Invert(lightProjTransform);
	deviceContext->UpdateSubresource(gGPUUtilsResources.ConstantBuffer(8).buffer, 0, nullptr, &data[0], 0, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &gGPUUtilsResources.ConstantBuffer(8).buffer);

	deviceContext->IASetIndexBuffer(gGPUUtilsResources.screenQuadIB.buffer, DXGI_FORMAT_R16_UINT, 0);
	deviceContext->DrawIndexed(6, 0, 0);

	ClearRenderTargets();
	ClearPSShaderResources(0, 3);

	// max
//	gGPUUtilsPostprocessor.Max(penumbraRT, penumbraRT, 4, penumbraRT.PixelSize(), -3, 3);

	// blur
	if (!application.IsKeyPressed(EKey::E))
		gGPUUtilsPostprocessor.BlurGauss7_Sigma3(penumbraRT, penumbraRT, 4, penumbraRT.PixelSize());
}


void RenderShadowMask(const SMatrix& cameraViewProjTransform, const SMatrix& cameraProjTransform, const SMatrix& lightViewProjTransform, const SMatrix& lightProjTransform)
{
	NGPU::CProfilerScopedQuery psq("ShadowMask");

	if (!checker)
	{
		SetViewport(shadowMaskRT);
		deviceContext->OMSetRenderTargets(1, &shadowMaskRT.rtv, nullptr);
		deviceContext->ClearRenderTargetView(shadowMaskRT.rtv, &cVector4One.x);
	}
	else
	{
		SetViewport(shadowMaskCheckerRT);
		deviceContext->OMSetRenderTargets(1, &shadowMaskCheckerRT.rtv, nullptr);
		deviceContext->ClearRenderTargetView(shadowMaskCheckerRT.rtv, &cVector4One.x);
	}

	deviceContext->VSSetShader(gGPUUtilsResources.screenQuadVS, nullptr, 0);
	deviceContext->PSSetShader(ShadowMaskPS(), nullptr, 0);

	deviceContext->PSSetShaderResources(0, 1, &backBufferDST.srv);
	deviceContext->PSSetShaderResources(1, 1, &shadowMapRT.srv);
	deviceContext->PSSetShaderResources(2, 1, &shadowMapRT_x4.srv);
	deviceContext->PSSetShaderResources(3, 1, &shadowMapMinBlurredRT.srv);
	deviceContext->PSSetShaderResources(4, 1, &penumbraRT.srv);

	SMatrix data[2];
	data[0] = Invert(cameraViewProjTransform) * lightViewProjTransform;
	data[1] = Invert(lightProjTransform);
	deviceContext->UpdateSubresource(gGPUUtilsResources.ConstantBuffer(8).buffer, 0, nullptr, &data[0], 0, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &gGPUUtilsResources.ConstantBuffer(8).buffer);

	deviceContext->IASetIndexBuffer(gGPUUtilsResources.screenQuadIB.buffer, DXGI_FORMAT_R16_UINT, 0);
	deviceContext->DrawIndexed(6, 0, 0);

	ClearRenderTargets();
	ClearPSShaderResources(0, 5);

	// upsample if checker
	if (checker)
	{
		SetViewport(shadowMaskRT);
		deviceContext->OMSetRenderTargets(1, &shadowMaskRT.rtv, nullptr);
		deviceContext->ClearRenderTargetView(shadowMaskRT.rtv, &cVector4One.x);

		deviceContext->VSSetShader(gGPUUtilsResources.screenQuadVS, nullptr, 0);
		deviceContext->PSSetShader(ShadowMaskCheckerUpsamplePS(), nullptr, 0);

		deviceContext->PSSetShaderResources(0, 1, checkerUpsampleDepthBuffer16 ? &depthBuffer16RT.srv : &backBufferDST.srv);
		deviceContext->PSSetShaderResources(1, 1, &shadowMaskCheckerRT.srv);

		SVector2 projParams = VectorCustom(cameraProjTransform.m[2][2], cameraProjTransform.m[3][2]);
		deviceContext->UpdateSubresource(gGPUUtilsResources.ConstantBuffer(1).buffer, 0, nullptr, &projParams, 0, 0);
		deviceContext->PSSetConstantBuffers(0, 1, &gGPUUtilsResources.ConstantBuffer(1).buffer);

		deviceContext->IASetIndexBuffer(gGPUUtilsResources.screenQuadIB.buffer, DXGI_FORMAT_R16_UINT, 0);
		deviceContext->DrawIndexed(6, 0, 0);

		ClearRenderTargets();
		ClearPSShaderResources(0, 2);
	}
}


void RenderScene(const SMatrix& cameraViewProjTransform, const SVector3& lightDirection)
{
	float backgroundColor[] = { 0.2f, 0.2f, 0.2f, 1.0f };

	SetViewport(lightingRT);
	deviceContext->OMSetRenderTargets(1, &lightingRT.rtv, backBufferDST.dsv);
	deviceContext->ClearRenderTargetView(lightingRT.rtv, backgroundColor);
	deviceContext->ClearDepthStencilView(backBufferDST.dsv, D3D11_CLEAR_DEPTH, 1.0f, 0);

	deviceContext->VSSetShader(meshVS, nullptr, 0);
	deviceContext->PSSetShader(MeshPS(), nullptr, 0);

	deviceContext->IASetInputLayout(gGPUUtilsResources.posNorUV0UV1InputLayout);

	struct Data
	{
		SVector3 lightDirection;
		float padding1;
		SVector2 screenSize;
		SVector2 padding2;
	} data;
	data.lightDirection = lightDirection;
	data.screenSize = VectorCustom((float)screenWidth, (float)screenHeight);
	deviceContext->UpdateSubresource(gGPUUtilsResources.ConstantBuffer(2).buffer, 0, nullptr, &data, 0, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &gGPUUtilsResources.ConstantBuffer(2).buffer);

	deviceContext->PSSetShaderResources(1, 1, &shadowMaskRT.srv);
	deviceContext->PSSetShaderResources(0, 1, &textures.find("Assets/sponza/textures/sponza_column_a_diff.tga")->second.texture.srv);

	for (uint i = 0; i < meshesInstances.size(); i++)
	{
		meshConstantBuffer.worldTransform = meshesInstances[i].worldTransform;
		meshConstantBuffer.viewProjTransform = cameraViewProjTransform;
		deviceContext->UpdateSubresource(gGPUUtilsResources.ConstantBuffer(8).buffer, 0, nullptr, &meshConstantBuffer, 0, 0);
		deviceContext->VSSetConstantBuffers(0, 1, &gGPUUtilsResources.ConstantBuffer(8).buffer);

		for (uint j = 0; j < meshesInstances[i].mesh->chunks.size(); j++)
		{
			auto texture = textures.find(meshesInstances[i].mesh->chunks[j].textureFileName);
			if (texture != textures.end())
				deviceContext->PSSetShaderResources(0, 1, &texture->second.texture.srv);
			
			meshesInstances[i].mesh->Render(j);
		}
	}
}


bool Run()
{
	float deltaTime = application.LastFrameTime();

	//

	if (application.IsKeyDown(EKey::_1))
		shadowMode = (ShadowMode)FlipFlop((int)shadowMode, 0, 1);
	if (application.IsKeyDown(EKey::_2))
		penumbraMask = !penumbraMask;
	if (application.IsKeyDown(EKey::_3))
		penumbraDownsampledShadowMap = !penumbraDownsampledShadowMap;
	if (application.IsKeyDown(EKey::_4))
		penumbraLowSamplesCount = !penumbraLowSamplesCount;
	if (application.IsKeyDown(EKey::_5))
		checker = !checker;
	if (application.IsKeyDown(EKey::_6))
		checkerUpsampleDepthBuffer16 = !checkerUpsampleDepthBuffer16;
	if (application.IsKeyDown(EKey::_7))
		shadowOnly = !shadowOnly;
	if (application.IsKeyDown(EKey::_8))
		shadowMapSize = FlipFlop(shadowMapSize, 100.0f, 500.0f);

	if (application.IsKeyPressed(EKey::PageUp))
		shadowFilterMaxSize += 0.00001f * deltaTime;
	if (application.IsKeyPressed(EKey::PageDown))
		shadowFilterMaxSize -= 0.00001f * deltaTime;

	if (application.IsKeyPressed(EKey::Home))
		penumbraFilterScale += 0.001f * deltaTime;
	if (application.IsKeyPressed(EKey::End))
		penumbraFilterScale -= 0.001f * deltaTime;

	//

	SVector3 eye;

	float speed = 0.025f * deltaTime;
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

	deviceContext->RSSetState(gGPUUtilsResources.ccwRasterizerState);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	static SVector3 lightDirection = Normalize(VectorCustom(-2.0f, -8.0f, -2.0f));
	if (application.IsKeyPressed(EKey::Space))
		lightDirection = camera.forwardVector;

	SMatrix cameraViewTransform = MatrixLookAtRH(camera.eye, camera.at, camera.up);
	SMatrix cameraProjTransform = MatrixPerspectiveFovRH(EZRange::ZeroToOne, cPi / 3.0f, (float)screenWidth / (float)screenHeight, 1.0f, 1000.0f);
	SMatrix cameraViewProjTransform = cameraViewTransform * cameraProjTransform;
	SMatrix lightViewTransform = MatrixLookAtRH(500.0f * -lightDirection, cVector3Zero, VectorCustom(0.0f, 1.0f, 0.0f));
	SMatrix lightProjTransform = MatrixOrthoRH(EZRange::ZeroToOne, shadowMapSize, shadowMapSize, 1.0f, 1000.0f);
	SMatrix lightViewProjTransform = lightViewTransform * lightProjTransform;

	SetSamplers();

	// common PS constant buffer
	{
		float data[8];
		data[0] = (float)screenWidth;
		data[1] = (float)screenHeight;
		data[2] = 1.0f / (float)screenWidth;
		data[3] = 1.0f / (float)screenHeight;
		data[4] = 1.0f / (float)shadowMaskCheckerRT.width;
		data[5] = 1.0f / (float)shadowMaskCheckerRT.height;
		data[6] = shadowFilterMaxSize;
		data[7] = penumbraFilterScale * shadowFilterMaxSize;
		deviceContext->UpdateSubresource(gGPUUtilsResources.ConstantBuffer(2, "common").buffer, 0, nullptr, &data[0], 0, 0);
		deviceContext->PSSetConstantBuffers(1, 1, &gGPUUtilsResources.ConstantBuffer(2, "common").buffer);
	}

	RenderDepthBuffer(cameraViewProjTransform);
	RenderDepthBuffer16(cameraProjTransform);
	RenderShadowMap(lightViewTransform, lightViewProjTransform);
	RenderShadowMapDownsampleMin2x2();
	RenderShadowMapMinBlurred();
	RenderPenumbraMask(cameraViewProjTransform, lightViewProjTransform, lightProjTransform);
	RenderShadowMask(cameraViewProjTransform, cameraProjTransform, lightViewProjTransform, lightProjTransform);
	RenderScene(cameraViewProjTransform, lightDirection);

	// render full-screen quad
	{
		SetViewport(screenWidth, screenHeight);
		deviceContext->OMSetRenderTargets(1, &backBufferRTV, nullptr);

		deviceContext->VSSetShader(gGPUUtilsResources.screenQuadVS, nullptr, 0);
		deviceContext->PSSetShader(gGPUUtilsResources.postprocess_copyPointTexturePS, nullptr, 0);
		deviceContext->PSSetShaderResources(0, 1, &lightingRT.srv);
		if (application.IsKeyPressed(EKey::Q))
			deviceContext->PSSetShaderResources(0, 1, &penumbraRT.srv);

		deviceContext->IASetIndexBuffer(gGPUUtilsResources.screenQuadIB.buffer, DXGI_FORMAT_R16_UINT, 0);

		deviceContext->DrawIndexed(6, 0, 0);
	}

	// HUD
	{
		float x = 40.0f;
		float y = 40.0f;
		float w = 12.0f;
		float h = 18.0f;
		float yHeight1 = 18.0f;
		float yHeight2 = 18.0f;
		SVector4 color = VectorCustom(0.7f, 0.7f, 0.7f, 1.0f);

		gGPUUtilsFontRenderer.Print("contact hardening (1): " + ToString(shadowMode), x, y, w, h, color); y += yHeight1;
		if (shadowMode == ShadowMode::ContactHardeningAvgBlockers)
		{
			gGPUUtilsFontRenderer.Print("  penumbra mask (2): " + ToString(penumbraMask), x, y, w, h, color); y += yHeight1;
			gGPUUtilsFontRenderer.Print("  penumbra downsampled shadow map (3): " + ToString(penumbraDownsampledShadowMap), x, y, w, h, color); y += yHeight1;
			gGPUUtilsFontRenderer.Print("  penumbra samples count (4): " + ToString(penumbraLowSamplesCount ? 16 : 32), x, y, w, h, color); y += yHeight1;
		}
		gGPUUtilsFontRenderer.Print("checkerboard (5): " + ToString(checker), x, y, w, h, color); y += yHeight1;
		if (checker)
		{
			gGPUUtilsFontRenderer.Print("  checkerboard upsample depth buffer 16 (6): " + ToString(checkerUpsampleDepthBuffer16), x, y, w, h, color); y += yHeight1;
		}
		y += yHeight2;

		gGPUUtilsFontRenderer.Print("shadow only (7): " + ToString(shadowOnly), x, y, w, h, color); y += yHeight1;
		gGPUUtilsFontRenderer.Print("shadow map size (8): " + ToString(shadowMapSize), x, y, w, h, color); y += yHeight1;
		gGPUUtilsFontRenderer.Print("shadow filter max size (PgUp/PgDn): " + ToString(shadowFilterMaxSize), x, y, w, h, color); y += yHeight1;
		gGPUUtilsFontRenderer.Print("penumbra filter scale (Home/End): " + ToString(penumbraFilterScale), x, y, w, h, color); y += yHeight1;
		y += yHeight2;

		float totalTime = 0.0f;
		if (shadowMode == ShadowMode::ContactHardeningAvgBlockers && penumbraMask)
		{
			float time = gGPUProfiler.Query("Penumbra").avgTime;
			totalTime += time;
			gGPUUtilsFontRenderer.Print("Penumbra: " + ToString(time) + " ms", x, y, w, h, color); y += yHeight1;
		}
		if (shadowMode == ShadowMode::ContactHardeningAvgBlockers && penumbraDownsampledShadowMap)
		{
			float time = gGPUProfiler.Query("ShadowMapDownsampleMin2x2").avgTime;
			totalTime += time;
			gGPUUtilsFontRenderer.Print("ShadowMapDownsampleMin2x2: " + ToString(time) + " ms", x, y, w, h, color); y += yHeight1;
		}
		if (shadowMode == ShadowMode::ContactHardeningMinFilter)
		{
			float time = gGPUProfiler.Query("ShadowMapMinBlurred").avgTime;
			totalTime += time;
			gGPUUtilsFontRenderer.Print("ShadowMapMinBlurred: " + ToString(time) + " ms", x, y, w, h, color); y += yHeight1;
		}
		if (checker && checkerUpsampleDepthBuffer16)
		{
			float time = gGPUProfiler.Query("DepthBuffer16").avgTime;
			totalTime += time;
			gGPUUtilsFontRenderer.Print("DepthBuffer16: " + ToString(time) + " ms", x, y, w, h, color); y += yHeight1;
		}
		{
			float time = gGPUProfiler.Query("ShadowMask").avgTime;
			totalTime += time;
			gGPUUtilsFontRenderer.Print("ShadowMask: " + ToString(time) + " ms", x, y, w, h, color); y += yHeight1;
		}
		gGPUUtilsFontRenderer.Print("total time: " + ToString(totalTime) + " ms", x, y, w, h, color);
	}

	ID3D11ShaderResourceView* nullSRVS[] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
	deviceContext->PSSetShaderResources(0, 6, nullSRVS);

	swapChain->Present(0, 0);

	gGPUProfiler.EndFrame();
	gGPUProfiler.StopProfiling();
	if (!gGPUProfiler.isProfiling)
		gGPUProfiler.StartProfiling();

	//

	if (application.IsKeyPressed(EKey::F4))
	{
		DestroyShaders();
		CreateShaders();

		gGPUUtilsResources.DestroyShaders();
		gGPUUtilsResources.CreateShaders("../../../../");
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
