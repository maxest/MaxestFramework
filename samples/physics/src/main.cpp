#include "../../../src/main.h"
#include "../../../src/namespaces.h"


bool fullScreen = false;
int screenWidth = 800;
int screenHeight = 600;


CApplication application;
CRigidBody rigidbody;


void Log(const string& msg)
{
	cout << msg << endl;
}


bool Create()
{
	CreateD3D11(screenWidth, screenHeight, false);

	gGPUProfiler.Create();
	gGPUUtilsResources.Create("../../../../");
	gGPUUtilsDebugRenderer.Create("../../../../");

	return true;
}


void Destroy()
{
	gGPUProfiler.Destroy();
	gGPUUtilsResources.Destroy();
	gGPUUtilsDebugRenderer.Destroy();

	DestroyD3D11();
}


bool Run()
{
	float lastFrameTime = application.LastFrameTime();

	//

	gGPUProfiler.StartFrame();

	float backgroundColor[] = { 0.2f, 0.2f, 0.2f, 1.0f };

	deviceContext->RSSetState(gGPUUtilsResources.ccwRasterizerState);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	float forceStrength = 0.00001f;
	SVector3 forcePoint_local = VectorCustom(0.5f, 0.0f, 0.0f);
	if (application.IsKeyPressed(EKey::A))
		rigidbody.AddForce_Local(VectorCustom(-forceStrength, 0.0f, 0.0f), forcePoint_local);
	if (application.IsKeyPressed(EKey::D))
		rigidbody.AddForce_Local(VectorCustom(forceStrength, 0.0f, 0.0f), forcePoint_local);
	if (application.IsKeyPressed(EKey::W))
		rigidbody.AddForce_Local(VectorCustom(0.0f, 0.0f, -forceStrength), forcePoint_local);
	if (application.IsKeyPressed(EKey::S))
		rigidbody.AddForce_Local(VectorCustom(0.0f, 0.0f, forceStrength), forcePoint_local);
	if (application.IsKeyPressed(EKey::E))
		rigidbody.AddForce_Local(VectorCustom(0.0f, forceStrength, 0.0f), forcePoint_local);
	if (application.IsKeyPressed(EKey::Q))
		rigidbody.AddForce_Local(VectorCustom(0.0f, -forceStrength, 0.0f), forcePoint_local);
	rigidbody.Integrate(lastFrameTime);

	SMatrix worldTransform = QuaternionToMatrix(rigidbody.orientation) * MatrixTranslate(rigidbody.position);
	SMatrix viewProjTransform =
		MatrixLookAtRH(VectorCustom(15.0f, 15.0f, 15.0f), VectorCustom(0.0f, 0.0f, 0.0f), VectorCustom(0.0f, 1.0f, 0.0f)) *
		MatrixPerspectiveFovRH(EZRange::ZeroToOne, cPi/3.0f, (float)screenWidth/(float)screenHeight, 1.0f, 300.0f);

	SetSamplers();

	SetViewport(screenWidth, screenHeight);
	deviceContext->OMSetRenderTargets(1, &backBufferRTV, nullptr);
	deviceContext->ClearRenderTargetView(backBufferRTV, backgroundColor);

	gGPUUtilsDebugRenderer.RenderBox(worldTransform, viewProjTransform);

	swapChain->Present(1, 0);

	gGPUProfiler.EndFrame();
	gGPUProfiler.StopProfiling();
	if (!gGPUProfiler.isProfiling)
		gGPUProfiler.StartProfiling();

	//

	if (application.IsKeyPressed(EKey::Escape))
		return false;

	//

	return true;
}


int main()
{
	NSystem::Initialize();
	NImage::Initialize();

	if (!application.Create(screenWidth, screenHeight, fullScreen))
		return 1;

	Create();
	application.Run(Run);
	Destroy();

	application.Destroy();

	NSystem::Deinitialize();
	NImage::Deinitialize();

	return 0;
}
