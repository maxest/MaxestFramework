#include "ray_tracer.h"
#include "camera.h"

#include "../../../src/main.h"
#include "../../../src/namespaces.h"


using namespace NRayTracer;


CApplication application;
CJobSystem jobSystem;
COGLTextureRenderer oglTextureRenderer;

int width = 640;
int height = 480;

NRayTracer::CScene scene;
NMath::CCamera camera;
NRayTracer::CRayTracer rayTracer;


void Log(const string& msg)
{
	cout << msg << endl;
}


void Create()
{
	Randomize();

	//

	scene.ambientConst = 0.0f;
	scene.ambientOcclusionFactor = 0.5f;

	// floor
	{
		NMesh::SMesh mesh;
		ImportASE("../../../../data/meshes/unit_plane.ASE", mesh);
		ToIndexed(mesh);
		scene.AddMesh(mesh, MatrixScale(100.0f, 100.0f, 1.0f) * MatrixRotateX(-cPi / 2.0f), 2);
	}

	// box
	{
		NMesh::SMesh mesh;
		ImportASE("../../../../data/meshes/unit_box.ASE", mesh);
		ToIndexed(mesh);
		scene.AddMesh(mesh, MatrixTranslate(0.0f, 0.5f, 0.0f), 1);
	}

	//

	SSpherePrimitive sphere;

	sphere.position = VectorCustom(-7.5f, 3.0f, 0.0f);
	sphere.radius = 2.5f;
	sphere.materialIndex = 0;
	scene.spheres.push_back(sphere);

	sphere.position = VectorCustom(7.5f, 3.0f, 0.0f);
	sphere.radius = 2.5f;
	sphere.materialIndex = 1;
	scene.spheres.push_back(sphere);

	//

	NRayTracer::CMaterial material;

	material.diffuseBRDF = new CLambertianBRDF(0.0f * VectorCustom(1.0f, 1.0f, 1.0f));
	material.specularBRDF = new CGlossySpecularBRDF(0.0f);
	material.transmittance = 0.75f;
	material.transmittanceEta = 0.5f;
	material.reflectivity = 0.0f;
	scene.materials.push_back(material);

	material.diffuseBRDF = new CLambertianBRDF(VectorCustom(1.0f, 1.0f, 0.0f));
	material.specularBRDF = new CGlossySpecularBRDF(0.0f);
	material.transmittance = 0.0f;
	material.transmittanceEta = 0.0f;
	material.reflectivity = 0.0f;
	scene.materials.push_back(material);

	material.diffuseBRDF = new CLambertianBRDF(VectorCustom(1.0f, 1.0f, 1.0f));
	material.specularBRDF = new CGlossySpecularBRDF(32.0f);
	material.transmittance = 0.0f;
	material.transmittanceEta = 0.0f;
	material.reflectivity = 0.0f;
	scene.materials.push_back(material);

	//

	SPointLight pointLight;
	//
	pointLight.position = VectorCustom(2.0f, 3.0f, 0.0f);
	pointLight.color = 50.0f * VectorCustom(1.0f, 1.0f, 1.0f);
	scene.pointLights.push_back(pointLight);
	//
	pointLight.position = VectorCustom(-2.0f, 5.0f, -15.0f);
	pointLight.color = 500.0f * VectorCustom(1.0f, 0.0f, 0.0f);
	scene.pointLights.push_back(pointLight);

	SDirLight dirLight;
	dirLight.dir = Normalize(VectorCustom(-1.0f, -1.0f, -1.0f));
	dirLight.color = VectorCustom(1.0f, 1.0f, 1.0f);
	scene.dirLights.push_back(dirLight);

	//

	camera.UpdateFixed(VectorCustom(0.0f, 5.0f, 17.5f), VectorCustom(0.0f, 0.0f, 0.0f));

	//

	rayTracer.Create(width, height, scene);
}


void Destroy()
{
	rayTracer.Destroy();

	for (uint i = 0; i < scene.materials.size(); i++)
		scene.materials[i].Destroy();
}


bool Run()
{
	SVector3 eye;

	float speed = 0.02f;
	if (application.IsKeyPressed(EKey::LShift))
		speed = 10.0f * 0.08f;

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

//	camera.UpdateFree(eye);

	//

/*	static float angle = 0.0f;
	angle += 0.1f;
	float xx = 8.0f*cosf(angle);
	float zz = 8.0f*sinf(angle);
	scene.lights[1].position.x = xx;
	scene.lights[1].position.z = zz;*/

	//

	uint64 before = TickCount();

	NRayTracer::CCamera rtCamera;
	rtCamera.UpdateView(camera.eye, camera.at, camera.up);
//	rtCamera.SetOrtho(width, height, 10.0f);
	rtCamera.SetPerspective(cPi / 3.0f, width, height, 4.0f);
	bool dof = false;
	bool aa = false;

	uint8* outputData = rayTracer.Render(&jobSystem, rtCamera, dof, aa);

	uint64 after = TickCount();
	cout << after - before << endl;

	oglTextureRenderer.Render(outputData);

	if (application.IsKeyPressed(EKey::Escape))
		return false;

	return true;
}


int main()
{
	NSystem::Initialize();
	NImage::Initialize();

	if (!application.Create(width, height, false))
		return 1;
	application.SetMouseWrapping(true);
	application.ShowCursor(false);
	jobSystem.Create(16);
	oglTextureRenderer.Create(width, height);

	Create();
	application.Run(Run);
	Destroy();

	oglTextureRenderer.Destroy();
	jobSystem.Destroy();
	application.Destroy();

	NSystem::Deinitialize();
	NImage::Deinitialize();

	return 0;
}
