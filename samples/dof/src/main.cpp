#include "../../../src/main.h"
#include "../../../src/namespaces.h"


struct SMeshConstantBuffer
{
	SMatrix worldTransform;
	SMatrix viewProjTransform;
} meshConstantBuffer;


bool fullScreen = false;
int screenWidth = 1280;
int screenHeight = 720;

float dofFocalPlaneDistance = 40.0f;
float dofFocusTransitionDistance = 20.0f;
float dofStrength = 1.0f;


CApplication application;

TRenderTarget colorRT;
TRenderTarget cocRT;
TRenderTarget cocRT_x4;
TRenderTarget cocMaxXRT_x4;
TRenderTarget cocMaxRT_x4;
TRenderTarget cocNearBlurXRT_x4;
TRenderTarget cocNearBlurRT_x4;
TRenderTarget colorRT_x4;
TRenderTarget colorMulCoCFarRT_x4;
TRenderTarget dofNearRT_x4;
TRenderTarget dofFarRT_x4;
TRenderTarget dofNearFillRT_x4;
TRenderTarget dofFarFillRT_x4;
TRenderTarget dofCompositeRT;
TDepthStencilTarget depthStencilTarget;

ID3D11VertexShader* meshVS = nullptr;

ID3D11PixelShader* meshPS = nullptr;
ID3D11PixelShader* downsamplePS = nullptr;
ID3D11PixelShader* dofCOCPS = nullptr;
ID3D11PixelShader* dofPS = nullptr;
ID3D11PixelShader* dofFillPS = nullptr;
ID3D11PixelShader* dofCompositePS = nullptr;

NUtils::CMesh sponzaScene;
map<string, NUtils::CTexture> textures;

CCamera camera;


void Log(const string& msg)
{
	cout << msg << endl;
}


void CreateShaders()
{
	MF_ASSERT(CreateVertexShader("../../data/mesh_vs.hlsl", meshVS));

	MF_ASSERT(CreatePixelShader("../../data/mesh_ps.hlsl", meshPS));
	MF_ASSERT(CreatePixelShader("../../data/downsample_ps.hlsl", downsamplePS));
	MF_ASSERT(CreatePixelShader("../../data/dof_coc_ps.hlsl", dofCOCPS));
	MF_ASSERT(CreatePixelShader("../../data/dof_ps.hlsl", dofPS));
	MF_ASSERT(CreatePixelShader("../../data/dof_fill_ps.hlsl", dofFillPS));
	MF_ASSERT(CreatePixelShader("../../data/dof_composite_ps.hlsl", dofCompositePS));
}


void DestroyShaders()
{
	DestroyPixelShader(meshPS);
	DestroyPixelShader(downsamplePS);
	DestroyPixelShader(dofCOCPS);
	DestroyPixelShader(dofPS);
	DestroyPixelShader(dofFillPS);
	DestroyPixelShader(dofCompositePS);

	DestroyVertexShader(meshVS);
}


bool Create()
{
	CreateD3D11(screenWidth, screenHeight, fullScreen);

	gGPUProfiler.Create();
	gGPUUtilsResources.Create("../../../../");

	DXGI_FORMAT colorBufferFormat = DXGI_FORMAT_R11G11B10_FLOAT;
	CreateRenderTarget(screenWidth, screenHeight, colorBufferFormat, 1, colorRT);
	CreateRenderTarget(screenWidth, screenHeight, DXGI_FORMAT_R8G8_UNORM, 1, cocRT);
	CreateRenderTarget(screenWidth/2, screenHeight/2, DXGI_FORMAT_R8G8_UNORM, 1, cocRT_x4);
	CreateRenderTarget(screenWidth/2, screenHeight/2, DXGI_FORMAT_R8_UNORM, 1, cocMaxXRT_x4);
	CreateRenderTarget(screenWidth/2, screenHeight/2, DXGI_FORMAT_R8_UNORM, 1, cocMaxRT_x4);
	CreateRenderTarget(screenWidth/2, screenHeight/2, DXGI_FORMAT_R8_UNORM, 1, cocNearBlurXRT_x4);
	CreateRenderTarget(screenWidth/2, screenHeight/2, DXGI_FORMAT_R8_UNORM, 1, cocNearBlurRT_x4);
	CreateRenderTarget(screenWidth/2, screenHeight/2, colorBufferFormat, 1, colorRT_x4);
	CreateRenderTarget(screenWidth/2, screenHeight/2, colorBufferFormat, 1, colorMulCoCFarRT_x4);
	CreateRenderTarget(screenWidth/2, screenHeight/2, colorBufferFormat, 1, dofNearRT_x4);
	CreateRenderTarget(screenWidth/2, screenHeight/2, colorBufferFormat, 1, dofFarRT_x4);
	CreateRenderTarget(screenWidth/2, screenHeight/2, colorBufferFormat, 1, dofNearFillRT_x4);
	CreateRenderTarget(screenWidth/2, screenHeight/2, colorBufferFormat, 1, dofFarFillRT_x4);
	CreateRenderTarget(screenWidth, screenHeight, colorBufferFormat, 1, dofCompositeRT);
	CreateDepthStencilTarget(screenWidth, screenHeight, 1, depthStencilTarget);

	CreateShaders();

	sponzaScene.CreateFromFile("../../../common/data/sponza/sponza.obj", NUtils::CMesh::EFileType::OBJ, &textures);

	camera.UpdateFixed(VectorCustom(0.0f, 50.0f, 50.0f), VectorCustom(0.0f, 0.0f, 0.0f));

	return true;
}


void Destroy()
{
	for (auto it = textures.begin(); it != textures.end(); it++)
		it->second.Destroy();
	sponzaScene.Destroy();

	DestroyShaders();

	DestroyRenderTarget(colorRT);
	DestroyRenderTarget(cocRT);
	DestroyRenderTarget(cocRT_x4);
	DestroyRenderTarget(cocMaxXRT_x4);
	DestroyRenderTarget(cocMaxRT_x4);
	DestroyRenderTarget(cocNearBlurXRT_x4);
	DestroyRenderTarget(cocNearBlurRT_x4);
	DestroyRenderTarget(colorRT_x4);
	DestroyRenderTarget(colorMulCoCFarRT_x4);
	DestroyRenderTarget(dofNearRT_x4);
	DestroyRenderTarget(dofFarRT_x4);
	DestroyRenderTarget(dofNearFillRT_x4);
	DestroyRenderTarget(dofFarFillRT_x4);
	DestroyRenderTarget(dofCompositeRT);
	DestroyDepthStencilTarget(depthStencilTarget);

	gGPUProfiler.Destroy();
	gGPUUtilsResources.Destroy();

	DestroyD3D11();
}


void DOF(float focalPlaneDistance, float focusTransitionRange, float strength, const SMatrix& projTransform)
{
	CProfilerScopedQuery psq("All");

	float kernelScale = 1.0;
	float compositeBlend = 1.0;

	if (strength >= 0.25f)
	{
		kernelScale = strength;
		compositeBlend = 1.0f;
	}
	else
	{
		kernelScale = 0.25f;
		compositeBlend = 4.0f * strength;
	}

	// circle of confusion
	{
		CProfilerScopedQuery psq("CoC generation");

		deviceContext->OMSetRenderTargets(1, &cocRT.rtv, nullptr);

		deviceContext->VSSetShader(gGPUUtilsResources.screenQuadVS, nullptr, 0);
		deviceContext->PSSetShader(dofCOCPS, nullptr, 0);
		deviceContext->PSSetShaderResources(0, 1, &depthStencilTarget.srv);

		deviceContext->IASetIndexBuffer(gGPUUtilsResources.screenQuadIB.buffer, DXGI_FORMAT_R16_UINT, 0);

		float nearBegin = focalPlaneDistance - focusTransitionRange;
		if (nearBegin < 0.0f)
			nearBegin = 0.0f;
		float nearEnd = focalPlaneDistance;
		float farBegin = focalPlaneDistance;
		float farEnd = focalPlaneDistance + focusTransitionRange;

		float buffer[8] =
		{
			nearBegin, nearEnd, farBegin, farEnd,
			projTransform.m[2][2], projTransform.m[3][2], 0.0f, 0.0f
		};
		deviceContext->UpdateSubresource(gGPUUtilsResources.ConstantBuffer(2).buffer, 0, nullptr, &buffer, 0, 0);
		deviceContext->PSSetConstantBuffers(0, 1, &gGPUUtilsResources.ConstantBuffer(2).buffer);

		deviceContext->DrawIndexed(6, 0, 0);
	}

	SetViewport(screenWidth/2, screenHeight/2);

	// downsample
	{
		CProfilerScopedQuery psq("Downsample");

		ID3D11RenderTargetView* downsampleRTVs[] = { colorRT_x4.rtv, colorMulCoCFarRT_x4.rtv, cocRT_x4.rtv };
		deviceContext->OMSetRenderTargets(3, downsampleRTVs, nullptr);

		deviceContext->VSSetShader(gGPUUtilsResources.screenQuadVS, nullptr, 0);
		deviceContext->PSSetShader(downsamplePS, nullptr, 0);
		deviceContext->PSSetShaderResources(0, 1, &colorRT.srv);
		deviceContext->PSSetShaderResources(1, 1, &cocRT.srv);

		deviceContext->IASetIndexBuffer(gGPUUtilsResources.screenQuadIB.buffer, DXGI_FORMAT_R16_UINT, 0);

		SVector4 pixelSize = VectorCustom(2.0f/(float)screenWidth, 2.0f/(float)screenHeight, 0.0f, 0.0f);
		deviceContext->UpdateSubresource(gGPUUtilsResources.ConstantBuffer(1).buffer, 0, nullptr, &pixelSize, 0, 0);
		deviceContext->PSSetConstantBuffers(0, 1, &gGPUUtilsResources.ConstantBuffer(1).buffer);

		deviceContext->DrawIndexed(6, 0, 0);
	}

	// near coc max X
	{
		CProfilerScopedQuery psq("Near CoC Max X");
		gGPUUtilsPostprocessor.MaxX(cocMaxXRT_x4, cocRT_x4, 1, VectorCustom(2.0f/(float)screenWidth, 2.0f/(float)screenHeight), -6, 6);
	}

	// near coc max Y
	{
		CProfilerScopedQuery psq("Near CoC Max Y");
		gGPUUtilsPostprocessor.MaxY(cocMaxRT_x4, cocMaxXRT_x4, 1, VectorCustom(2.0f/(float)screenWidth, 2.0f/(float)screenHeight), -6, 6);
	}

	// near coc blur X
	{
		CProfilerScopedQuery psq("Near CoC Blur X");
		gGPUUtilsPostprocessor.BlurX(cocNearBlurXRT_x4, cocMaxRT_x4, 1, VectorCustom(2.0f/(float)screenWidth, 2.0f/(float)screenHeight), -6, 6);
	}

	// near coc blur Y
	{
		CProfilerScopedQuery psq("Near CoC Blur Y");
		gGPUUtilsPostprocessor.BlurY(cocNearBlurRT_x4, cocNearBlurXRT_x4, 1, VectorCustom(2.0f/(float)screenWidth, 2.0f/(float)screenHeight), -6, 6);
	}

	// computation
	{
		CProfilerScopedQuery psq("Computation");

		ID3D11RenderTargetView* rtvs[] = { dofNearRT_x4.rtv, dofFarRT_x4.rtv };
		deviceContext->OMSetRenderTargets(2, rtvs, nullptr);

		deviceContext->VSSetShader(gGPUUtilsResources.screenQuadVS, nullptr, 0);
		deviceContext->PSSetShader(dofPS, nullptr, 0);
		deviceContext->PSSetShaderResources(0, 1, &cocRT_x4.srv);
		deviceContext->PSSetShaderResources(1, 1, &cocNearBlurRT_x4.srv);
		deviceContext->PSSetShaderResources(2, 1, &colorRT_x4.srv);
		deviceContext->PSSetShaderResources(3, 1, &colorMulCoCFarRT_x4.srv);

		deviceContext->IASetIndexBuffer(gGPUUtilsResources.screenQuadIB.buffer, DXGI_FORMAT_R16_UINT, 0);

		SVector4 pixelSize_dofKernelScale = VectorCustom(2.0f/(float)screenWidth, 2.0f/(float)screenHeight, kernelScale, 0.0f);
		deviceContext->UpdateSubresource(gGPUUtilsResources.ConstantBuffer(1).buffer, 0, nullptr, &pixelSize_dofKernelScale, 0, 0);
		deviceContext->PSSetConstantBuffers(0, 1, &gGPUUtilsResources.ConstantBuffer(1).buffer);

		deviceContext->DrawIndexed(6, 0, 0);
	}

	// fill
	{
		CProfilerScopedQuery psq("Fill");

		ID3D11RenderTargetView* rtvs[] = { dofNearFillRT_x4.rtv, dofFarFillRT_x4.rtv };
		deviceContext->OMSetRenderTargets(2, rtvs, nullptr);

		deviceContext->VSSetShader(gGPUUtilsResources.screenQuadVS, nullptr, 0);
		deviceContext->PSSetShader(dofFillPS, nullptr, 0);
		deviceContext->PSSetShaderResources(0, 1, &cocRT_x4.srv);
		deviceContext->PSSetShaderResources(1, 1, &cocNearBlurRT_x4.srv);
		deviceContext->PSSetShaderResources(2, 1, &dofNearRT_x4.srv);
		deviceContext->PSSetShaderResources(3, 1, &dofFarRT_x4.srv);

		deviceContext->IASetIndexBuffer(gGPUUtilsResources.screenQuadIB.buffer, DXGI_FORMAT_R16_UINT, 0);

		SVector4 pixelSize = VectorCustom(2.0f/(float)screenWidth, 2.0f/(float)screenHeight, 0.0f, 0.0f);
		deviceContext->UpdateSubresource(gGPUUtilsResources.ConstantBuffer(1).buffer, 0, nullptr, &pixelSize, 0, 0);
		deviceContext->PSSetConstantBuffers(0, 1, &gGPUUtilsResources.ConstantBuffer(1).buffer);

		deviceContext->DrawIndexed(6, 0, 0);
	}

	SetViewport(screenWidth, screenHeight);

	// composite
	{
		CProfilerScopedQuery psq("Composite");

		deviceContext->OMSetRenderTargets(1, &dofCompositeRT.rtv, nullptr);

		deviceContext->VSSetShader(gGPUUtilsResources.screenQuadVS, nullptr, 0);
		deviceContext->PSSetShader(dofCompositePS, nullptr, 0);
		deviceContext->PSSetShaderResources(0, 1, &colorRT.srv);
		deviceContext->PSSetShaderResources(1, 1, &cocRT.srv);
		deviceContext->PSSetShaderResources(2, 1, &cocRT_x4.srv);
		deviceContext->PSSetShaderResources(3, 1, &cocNearBlurRT_x4.srv);
		deviceContext->PSSetShaderResources(4, 1, &dofNearFillRT_x4.srv);
		deviceContext->PSSetShaderResources(5, 1, &dofFarFillRT_x4.srv);

		deviceContext->IASetIndexBuffer(gGPUUtilsResources.screenQuadIB.buffer, DXGI_FORMAT_R16_UINT, 0);

		SVector4 pixelSize_dofCompositeBlend = VectorCustom(1.0f/(float)screenWidth, 1.0f/(float)screenHeight, compositeBlend, 0.0f);
		deviceContext->UpdateSubresource(gGPUUtilsResources.ConstantBuffer(1).buffer, 0, nullptr, &pixelSize_dofCompositeBlend, 0, 0);
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

	if (application.IsKeyPressed(EKey::Insert))
		dofFocalPlaneDistance += 0.025f * lastFrameTime;
	if (application.IsKeyPressed(EKey::Delete))
		dofFocalPlaneDistance -= 0.025f * lastFrameTime;

	if (application.IsKeyPressed(EKey::Home))
		dofFocusTransitionDistance += 0.025f * lastFrameTime;
	if (application.IsKeyPressed(EKey::End))
		dofFocusTransitionDistance -= 0.025f * lastFrameTime;

	if (application.IsKeyPressed(EKey::PageUp))
		dofStrength += 0.001f * lastFrameTime;
	if (application.IsKeyPressed(EKey::PageDown))
		dofStrength -= 0.001f * lastFrameTime;
	dofStrength = Saturate(dofStrength);

	//

	gGPUProfiler.StartFrame();

	float backgroundColor[] = { 0.2f, 0.2f, 0.2f, 1.0f };

	deviceContext->RSSetState(gGPUUtilsResources.ccwRasterizerState);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	SMatrix projTransform = MatrixPerspectiveFovRH(EZRange::ZeroToOne, cPi/3.0f, (float)screenWidth/(float)screenHeight, 1.0f, 1000.0f);

	SetViewport(screenWidth, screenHeight);

	SetSamplers();

	// render scene
	{
		deviceContext->OMSetRenderTargets(1, &colorRT.rtv, depthStencilTarget.dsv);
		deviceContext->ClearRenderTargetView(colorRT.rtv, backgroundColor);
		deviceContext->ClearDepthStencilView(depthStencilTarget.dsv, D3D11_CLEAR_DEPTH, 1.0f, 0);

		deviceContext->VSSetShader(meshVS, nullptr, 0);
		deviceContext->PSSetShader(meshPS, nullptr, 0);

		deviceContext->IASetInputLayout(gGPUUtilsResources.posNorUV0UV1InputLayout);

		meshConstantBuffer.worldTransform = MatrixScale(0.1f, 0.1f, 0.1f);
		meshConstantBuffer.viewProjTransform =
			MatrixLookAtRH(camera.eye, camera.at, camera.up) *
			projTransform;
		deviceContext->UpdateSubresource(gGPUUtilsResources.ConstantBuffer(8).buffer, 0, nullptr, &meshConstantBuffer, 0, 0);
		deviceContext->VSSetConstantBuffers(0, 1, &gGPUUtilsResources.ConstantBuffer(8).buffer);

		for (uint i = 0; i < sponzaScene.chunks.size(); i++)
		{
			UINT stride = sizeof(SVertex_PosNorUV0UV1);
			UINT offset = 0;
			deviceContext->IASetVertexBuffers(0, 1, &sponzaScene.chunks[i].vb.buffer, &stride, &offset);
			deviceContext->IASetIndexBuffer(sponzaScene.chunks[i].ib.buffer, DXGI_FORMAT_R16_UINT, 0);

			auto texture = textures.find(sponzaScene.chunks[i].textureFileName);
			if (texture != textures.end())
				deviceContext->PSSetShaderResources(0, 1, &texture->second.texture.srv);

			deviceContext->DrawIndexed(sponzaScene.chunks[i].indicesCount, 0, 0);
		}
	}

	// DoF
	{
		DOF(dofFocalPlaneDistance, dofFocusTransitionDistance, dofStrength, projTransform);
	}

	// render full-screen quad
	{
		deviceContext->OMSetRenderTargets(1, &backBufferRTV, nullptr);

		deviceContext->VSSetShader(gGPUUtilsResources.screenQuadVS, nullptr, 0);
		deviceContext->PSSetShader(gGPUUtilsResources.postprocess_copyPointTexturePS, nullptr, 0);
		deviceContext->PSSetShaderResources(0, 1, &dofCompositeRT.srv);

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
	else
	{
		file.Open("config.txt", CFile::EOpenMode::WriteText);
		
		file.WriteText("FullScreen 0\n");
		file.WriteText("ScreenWidth 1280\n");
		file.WriteText("ScreenHeight 720\n");

		file.Close();
	}
}


int main()
{
	NSystem::Initialize();
	NImage::Initialize();

	LoadConfigFile();

	if (!application.Create(screenWidth, screenHeight, false))
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
