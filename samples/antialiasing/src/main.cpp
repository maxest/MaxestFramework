#include "../../../src/main.h"
#include "../../../src/namespaces.h"


struct SCameraTransforms
{
	SMatrix viewTransform;
	SMatrix projTransform;
	SMatrix projTransform_jittered;
	SMatrix viewProjTransform;
	SMatrix viewProjTransform_jittered;
};


struct SMeshConstantBuffer
{
	SMatrix worldTransform;
	SMatrix viewProjTransform;
} meshConstantBuffer;


bool fullScreen = false;
int screenWidth = 1920;
int screenHeight = 1080;
int renderTargetWidth = 1920;
int renderTargetHeight = 1080;
float aspect = (float)renderTargetWidth / (float)renderTargetHeight;


CApplication application;

TRenderTarget colorRTs[2];
TRenderTarget colorRT_temporalAA;
TRenderTarget colorRT_edgeBlurAA;
TRenderTarget colorRT_fxaa;
TDepthStencilTarget screenDST;

ID3D11VertexShader* meshVS = nullptr;

ID3D11PixelShader* mesh1PS = nullptr;
ID3D11PixelShader* mesh2PS = nullptr;
ID3D11PixelShader* aaTemporalPS = nullptr;
ID3D11PixelShader* aaEdgeBlurPS = nullptr;
ID3D11PixelShader* aaFXAAPS = nullptr;

NUtils::CMesh sponzaMesh;
NUtils::CMesh boxMesh;
NUtils::CUnityLightmapGroup sponzaUnityLightmapGroup;
map<string, NUtils::CTexture> textures;

CCamera camera;
SCameraTransforms cameraTransforms[2];


void Log(const string& msg)
{
	cout << msg << endl;
}


void CreateShaders()
{
	MF_ASSERT(CreateVertexShader("../../data/mesh_vs.hlsl", meshVS));

	MF_ASSERT(CreatePixelShader("../../data/mesh_ps.hlsl", "LIGHTMAPPED", mesh1PS));
	MF_ASSERT(CreatePixelShader("../../data/mesh_ps.hlsl", mesh2PS));
	MF_ASSERT(CreatePixelShader("../../data/aa_temporal_ps.hlsl", "CLAMP", aaTemporalPS));
	MF_ASSERT(CreatePixelShader("../../data/aa_edge_blur_ps.hlsl", aaEdgeBlurPS));
	MF_ASSERT(CreatePixelShader("../../data/aa_fxaa_ps.hlsl", aaFXAAPS));
}


void DestroyShaders()
{
	DestroyPixelShader(mesh1PS);
	DestroyPixelShader(mesh2PS);
	DestroyPixelShader(aaTemporalPS);
	DestroyPixelShader(aaEdgeBlurPS);
	DestroyPixelShader(aaFXAAPS);

	DestroyVertexShader(meshVS);
}


bool Create()
{
	CreateD3D11(screenWidth, screenHeight, false);

	gGPUProfiler.Create();
	gGPUUtilsResources.Create("../../../../");

	DXGI_FORMAT floatFormat = DXGI_FORMAT_R11G11B10_FLOAT;
	CreateRenderTarget(renderTargetWidth, renderTargetHeight, floatFormat, 1, colorRTs[0]);
	CreateRenderTarget(renderTargetWidth, renderTargetHeight, floatFormat, 1, colorRTs[1]);
	CreateRenderTarget(renderTargetWidth, renderTargetHeight, floatFormat, 1, colorRT_temporalAA);
	CreateRenderTarget(renderTargetWidth, renderTargetHeight, floatFormat, 1, colorRT_edgeBlurAA);
	CreateRenderTarget(renderTargetWidth, renderTargetHeight, floatFormat, 1, colorRT_fxaa);
	CreateDepthStencilTarget(renderTargetWidth, renderTargetHeight, 1, screenDST);

	CreateShaders();

	camera.UpdateFixed(VectorCustom(0.0f, 50.0f, 50.0f), VectorCustom(0.0f, 0.0f, 0.0f));

	sponzaMesh.CreateFromFile("../../../common/data/sponza_unity/mesh.dat", NUtils::CMesh::EFileType::UnityExported, &textures);
	boxMesh.CreateFromFile("../../../../data/meshes/unit_box.ase", NUtils::CMesh::EFileType::ASE);
	sponzaUnityLightmapGroup.CreateFromFile("../../../common/data/sponza_unity/", sponzaMesh.lightmapsCount);

	return true;
}


void Destroy()
{
	sponzaMesh.Destroy();
	boxMesh.Destroy();
	sponzaUnityLightmapGroup.Destroy();
	for (auto it = textures.begin(); it != textures.end(); it++)
		it->second.Destroy();

	DestroyShaders();

	DestroyRenderTarget(colorRTs[0]);
	DestroyRenderTarget(colorRTs[1]);
	DestroyRenderTarget(colorRT_temporalAA);
	DestroyRenderTarget(colorRT_edgeBlurAA);
	DestroyRenderTarget(colorRT_fxaa);
	DestroyDepthStencilTarget(screenDST);

	gGPUProfiler.Destroy();
	gGPUUtilsResources.Destroy();

	DestroyD3D11();
}


void RenderScene(float tickCount, const TRenderTarget& renderTarget, const TDepthStencilTarget& depthStencilTarget, const SMatrix& viewProjTransform)
{
	SMatrix worldTransform = MatrixScale(10.0f, 10.0f, 10.0f);
	float backgroundColor[] = { 0.2f, 0.2f, 0.2f, 1.0f };

	SetViewport(renderTarget.width, renderTarget.height);
	deviceContext->OMSetRenderTargets(1, &renderTarget.rtv, depthStencilTarget.dsv);
	deviceContext->ClearRenderTargetView(renderTarget.rtv, backgroundColor);
	deviceContext->ClearDepthStencilView(depthStencilTarget.dsv, D3D11_CLEAR_DEPTH, 1.0f, 0);

	meshConstantBuffer.worldTransform = worldTransform;
	meshConstantBuffer.viewProjTransform = viewProjTransform;
	deviceContext->UpdateSubresource(gGPUUtilsResources.ConstantBuffer(8).buffer, 0, nullptr, &meshConstantBuffer, 0, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &gGPUUtilsResources.ConstantBuffer(8).buffer);

	deviceContext->IASetInputLayout(gGPUUtilsResources.posNorUV0UV1InputLayout);
	UINT stride = sizeof(SVertex_PosNorUV0UV1);
	UINT offset = 0;

	deviceContext->VSSetShader(meshVS, nullptr, 0);
	deviceContext->PSSetShader(mesh1PS, nullptr, 0);

	for (uint i = 0; i < sponzaMesh.chunks.size(); i++)
	{
		deviceContext->IASetVertexBuffers(0, 1, &sponzaMesh.chunks[i].vb.buffer, &stride, &offset);
		deviceContext->IASetIndexBuffer(sponzaMesh.chunks[i].ib.buffer, DXGI_FORMAT_R16_UINT, 0);

		auto texture = textures.find(sponzaMesh.chunks[i].textureFileName);
		if (texture != textures.end())
			deviceContext->PSSetShaderResources(0, 1, &texture->second.texture.srv);
		deviceContext->PSSetShaderResources(1, 1, &sponzaUnityLightmapGroup.lightmaps[sponzaMesh.chunks[i].lightmapIndex].colorTexture.texture.srv);
		deviceContext->PSSetShaderResources(2, 1, &sponzaUnityLightmapGroup.lightmaps[sponzaMesh.chunks[i].lightmapIndex].dirTexture.texture.srv);

		deviceContext->DrawIndexed((uint)sponzaMesh.chunks[i].indicesCount, 0, 0);
	}

	deviceContext->VSSetShader(meshVS, nullptr, 0);
	deviceContext->PSSetShader(mesh2PS, nullptr, 0);

	for (int x = -5; x <= 5; x++)
	{
		for (int z = -1; z <= 1; z++)
		{
			SVector3 axis = Normalize(VectorCustom(1.0f, 0.5f, 0.25f));
			float angle = tickCount * (Sin((float)((x + 100) % (z + 50))) + 0.3f);

			meshConstantBuffer.worldTransform = worldTransform * MatrixRotate(axis, angle) * MatrixTranslate(10.0f * (float)x, 30.0f, 10.0f * (float)z);
			deviceContext->UpdateSubresource(gGPUUtilsResources.ConstantBuffer(8).buffer, 0, nullptr, &meshConstantBuffer, 0, 0);

			for (uint i = 0; i < boxMesh.chunks.size(); i++)
			{
				deviceContext->IASetVertexBuffers(0, 1, &boxMesh.chunks[i].vb.buffer, &stride, &offset);
				deviceContext->IASetIndexBuffer(boxMesh.chunks[i].ib.buffer, DXGI_FORMAT_R16_UINT, 0);

				deviceContext->DrawIndexed(boxMesh.chunks[i].indicesCount, 0, 0);
			}
		}
	}
}


void AATemporal(
	const TRenderTarget& output,
	const TRenderTarget& input_currFrame,
	const TRenderTarget& input_prevFrame,
	const TDepthStencilTarget& depthStencilTarget,
	const SCameraTransforms& cameraTransforms_currFrame,
	const SCameraTransforms& cameraTransforms_prevFrame)
{
	gGPUProfiler.Begin("AATemporal");

	deviceContext->OMSetRenderTargets(1, &output.rtv, nullptr);

	deviceContext->VSSetShader(gGPUUtilsResources.screenQuadVS, nullptr, 0);
	deviceContext->PSSetShader(aaTemporalPS, nullptr, 0);
	deviceContext->PSSetShaderResources(0, 1, &input_currFrame.srv);
	deviceContext->PSSetShaderResources(1, 1, &input_prevFrame.srv);
	deviceContext->PSSetShaderResources(2, 1, &screenDST.srv);

	struct Data
	{
		SMatrix reprojectTransform;
		SVector2 pixelSize;
		float padding[2];
	} data;
	data.reprojectTransform = Invert(cameraTransforms_currFrame.viewProjTransform) * cameraTransforms_prevFrame.viewProjTransform;
	data.pixelSize = VectorCustom(1.0f / (float)output.width, 1.0f / (float)output.height);
	deviceContext->UpdateSubresource(gGPUUtilsResources.ConstantBuffer(5).buffer, 0, nullptr, &data, 0, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &gGPUUtilsResources.ConstantBuffer(5).buffer);

	deviceContext->IASetIndexBuffer(gGPUUtilsResources.screenQuadIB.buffer, DXGI_FORMAT_R16_UINT, 0);

	deviceContext->DrawIndexed(6, 0, 0);

	ID3D11ShaderResourceView* nullSRVs1[] = { nullptr };
	deviceContext->OMSetRenderTargets(0, nullptr, nullptr);
	deviceContext->PSSetShaderResources(0, 1, nullSRVs1);

	gGPUProfiler.End("AATemporal");
}


void AAEdgeBlur(
	const TRenderTarget& output,
	const TRenderTarget& input)
{
	gGPUProfiler.Begin("AAEdgeBlur");

	deviceContext->OMSetRenderTargets(1, &output.rtv, nullptr);

	deviceContext->VSSetShader(gGPUUtilsResources.screenQuadVS, nullptr, 0);
	deviceContext->PSSetShader(aaEdgeBlurPS, nullptr, 0);
	deviceContext->PSSetShaderResources(0, 1, &input.srv);

	float data[4] = { 1.0f / (float)output.width, 1.0f / (float)output.height, 0.0f, 0.0f };
	deviceContext->UpdateSubresource(gGPUUtilsResources.ConstantBuffer(1).buffer, 0, nullptr, data, 0, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &gGPUUtilsResources.ConstantBuffer(1).buffer);

	deviceContext->IASetIndexBuffer(gGPUUtilsResources.screenQuadIB.buffer, DXGI_FORMAT_R16_UINT, 0);

	deviceContext->DrawIndexed(6, 0, 0);

	ID3D11ShaderResourceView* nullSRVs1[] = { nullptr };
	deviceContext->OMSetRenderTargets(0, nullptr, nullptr);
	deviceContext->PSSetShaderResources(0, 1, nullSRVs1);

	gGPUProfiler.End("AAEdgeBlur");
}


void FXAA(
	const TRenderTarget& output,
	const TRenderTarget& input)
{
	gGPUProfiler.Begin("FXAA");

	deviceContext->OMSetRenderTargets(1, &output.rtv, nullptr);

	deviceContext->VSSetShader(gGPUUtilsResources.screenQuadVS, nullptr, 0);
	deviceContext->PSSetShader(aaFXAAPS, nullptr, 0);
	deviceContext->PSSetShaderResources(0, 1, &input.srv);

	float data[4] = { 1.0f / (float)output.width, 1.0f / (float)output.height, 0.0f, 0.0f };
	deviceContext->UpdateSubresource(gGPUUtilsResources.ConstantBuffer(1).buffer, 0, nullptr, data, 0, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &gGPUUtilsResources.ConstantBuffer(1).buffer);

	deviceContext->IASetIndexBuffer(gGPUUtilsResources.screenQuadIB.buffer, DXGI_FORMAT_R16_UINT, 0);

	deviceContext->DrawIndexed(6, 0, 0);

	ID3D11ShaderResourceView* nullSRVs1[] = { nullptr };
	deviceContext->OMSetRenderTargets(0, nullptr, nullptr);
	deviceContext->PSSetShaderResources(0, 1, nullSRVs1);

	gGPUProfiler.End("FXAA");
}


bool Run()
{
	float tickCount = (float)TickCountLow() / 1000.0f;
	float lastFrameTime = application.LastFrameTime();

	static bool temporalAA = true;
	static bool edgeBlurAA = true;
	static bool fxaa = false;
	static int frameID = 0;
	frameID++;

	//

	SVector3 eye;

	float speed = 0.025f * lastFrameTime;
	if (application.IsKeyPressed(EKey::LShift))
		speed *= 4.0f;

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

	if (application.IsKeyDown(EKey::F1))
		temporalAA = !temporalAA;
	if (application.IsKeyDown(EKey::F2))
		edgeBlurAA = !edgeBlurAA;
	if (application.IsKeyDown(EKey::F3))
		fxaa = !fxaa;

	//

	gGPUProfiler.StartFrame();

	float backgroundColor[] = { 0.2f, 0.2f, 0.2f, 1.0f };

	deviceContext->RSSetState(gGPUUtilsResources.ccwRasterizerState);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	SVector3 lightDirection = Normalize(VectorCustom(-2.0f, -8.0f, -2.0f));

	SMatrix viewTransform = MatrixLookAtRH(camera.eye, camera.at, camera.up);
	SMatrix projTransform = MatrixPerspectiveFovRH(EZRange::ZeroToOne, cPi / 3.0f, aspect, 1.0f, 1000.0f);
	SMatrix projTransform_jittered = projTransform;
	if (temporalAA)
	{
		SVector2 subPixelOffset;
		if (frameID % 2 == 0)
			subPixelOffset = VectorCustom(0.25f, -0.25f);
		else
			subPixelOffset = VectorCustom(-0.25f, 0.25f);

		SMatrix subPixelOffsetTransform = MatrixIdentity();
		// image offset
		subPixelOffsetTransform.m[3][0] = subPixelOffset.x / (float)renderTargetWidth;
		subPixelOffsetTransform.m[3][1] = subPixelOffset.y / (float)renderTargetHeight;
		// per-pixel offset
		subPixelOffsetTransform.m[0][0] = 1.0f - subPixelOffset.x / (float)renderTargetWidth;
		subPixelOffsetTransform.m[1][1] = 1.0f - subPixelOffset.y / (float)renderTargetHeight;

		projTransform_jittered = projTransform * subPixelOffsetTransform;
	}
	SMatrix cameraViewProjTransform = viewTransform * projTransform;
	SMatrix cameraViewProjTransform_jittered = viewTransform * projTransform_jittered;

	cameraTransforms[frameID % 2].viewTransform = viewTransform;
	cameraTransforms[frameID % 2].projTransform = projTransform;
	cameraTransforms[frameID % 2].projTransform_jittered = projTransform_jittered;
	cameraTransforms[frameID % 2].viewProjTransform = cameraViewProjTransform;
	cameraTransforms[frameID % 2].viewProjTransform_jittered = cameraViewProjTransform_jittered;

	SetSamplers();

	// render scene
	RenderScene(tickCount, colorRTs[frameID % 2], screenDST, cameraViewProjTransform_jittered);

	TRenderTarget* currentRT = &colorRTs[frameID % 2];

	// AA
	if (temporalAA)
	{
		AATemporal(colorRT_temporalAA, *currentRT, colorRTs[(frameID - 1) % 2], screenDST, cameraTransforms[frameID % 2], cameraTransforms[(frameID - 1) % 2]);
		currentRT = &colorRT_temporalAA;
	}
	if (edgeBlurAA)
	{
		AAEdgeBlur(colorRT_edgeBlurAA, *currentRT);
		currentRT = &colorRT_edgeBlurAA;
	}
	if (fxaa)
	{
		FXAA(colorRT_fxaa, *currentRT);
		currentRT = &colorRT_fxaa;
	}

	// render full-screen quad
	{
		SetViewport(screenWidth, screenHeight);
		deviceContext->OMSetRenderTargets(1, &backBufferRTV, nullptr);

		deviceContext->VSSetShader(gGPUUtilsResources.screenQuadVS, nullptr, 0);
		deviceContext->PSSetShader(gGPUUtilsResources.postprocess_copyPointTexturePS, nullptr, 0);
		deviceContext->PSSetShaderResources(0, 1, &currentRT->srv);

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

	if (application.IsKeyPressed(EKey::F4))
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
	string temp;

	CFile file;
	if (file.Open("config.txt", CFile::EOpenMode::ReadText))
	{
		file.ReadText(temp);
		file.ReadText(fullScreen);
		file.ReadText(temp);
		file.ReadText(screenWidth);
		file.ReadText(temp);
		file.ReadText(screenHeight);

		file.Close();
	}
}


int main()
{
	NSystem::Initialize();
	NImage::Initialize();

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
