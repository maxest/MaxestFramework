#include "ray_tracer.h"
#include "camera.h"

#include "../../../src/main.h"
#include "../../../src/namespaces.h"


using namespace NRayTracer;


CApplication application;
CJobSystem jobSystem;
COGLTextureRenderer oglTextureRenderer;

uint8* data;
int width = 640;
int height = 480;

NRayTracer::SScene scene;
NMath::CCamera camera;


void Log(const string& msg)
{
	cout << msg << endl;
}


void Create()
{
	Randomize();

	//

	data = new uint8[4 * width * height];

	//

	scene.ambientConst = 0.0f;
	scene.ambientOcclusionFactor = 0.5f;
	scene.Create(8, width, height);

	// floor
	{
		NMesh::SMesh mesh;
		ImportASE("../../../../data/meshes/unit_plane.ASE", mesh);
		ToIndexed(mesh);
		SceneAddMesh(scene, mesh, MatrixScale(100.0f, 100.0f, 1.0f) * MatrixRotateX(-cPi / 2.0f), 2);
	}

	// box
	{
		NMesh::SMesh mesh;
		ImportASE("../../../../data/meshes/unit_box.ASE", mesh);
		ToIndexed(mesh);
		SceneAddMesh(scene, mesh, MatrixTranslate(0.0f, 0.5f, 0.0f), 1);
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

	NRayTracer::SMaterial material;

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

	material.diffuseBRDF = new CLambertianBRDF(VectorCustom(0.1f, 1.0f, 0.1f));
	material.specularBRDF = new CGlossySpecularBRDF(32.0f);
	material.transmittance = 0.0f;
	material.transmittanceEta = 0.0f;
	material.reflectivity = 0.0f;
	scene.materials.push_back(material);

	//

	SPointLight pointLight;
	//
	pointLight.position = VectorCustom(2.0f, 3.0f, 0.0f);
	pointLight.color = 55.0f * VectorCustom(1.0f, 1.0f, 1.0f);
//	scene.pointLights.push_back(pointLight);
	//
	pointLight.position = VectorCustom(8.0f, 5.0f, 0.0f);
	pointLight.color = 55.0f * VectorCustom(1.0f, 1.0f, 1.0f);
//	scene.pointLights.push_back(pointLight);

	SDirLight dirLight;
	dirLight.dir = Normalize(VectorCustom(-1.0f, -1.0f, -1.0f));
	dirLight.color = VectorCustom(1.0f, 1.0f, 1.0f);
	scene.dirLights.push_back(dirLight);

	//

	camera.UpdateFixed(VectorCustom(0.0f, 5.0f, 17.5f), VectorCustom(0.0f, 0.0f, 0.0f));
}


void Destroy()
{
	for (uint i = 0; i < scene.materials.size(); i++)
	{
		delete scene.materials[i].diffuseBRDF;
		delete scene.materials[i].specularBRDF;
	}

	delete[] data;
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

	NRayTracer::SCamera rtCamera;
	UpdateView(rtCamera, camera.eye, camera.at, camera.up);
//	UpdateOrtho(camera, width, height, 10.0f);
	UpdatePerspective(rtCamera, width, height, cPi / 3.0f, 4.0f);
	bool dof = false;
	bool aa = false;

	const int jobsCount = 32;
	CRayTraceJob* jobs[jobsCount];
	for (int i = 0; i < jobsCount; i++)
		jobs[i] = new CRayTraceJob(data, width, i*height/jobsCount, (i+1)*height/jobsCount, scene, rtCamera, dof, aa);

//	for (int i = 0; i < jobsCount; i++)
//		jobs[i]->Do();

	CJobGroup jobGroup;
	for (int i = 0; i < jobsCount; i++)
		jobGroup.AddJob(jobs[i]);
	jobSystem.AddJobGroup(jobGroup);
	jobGroup.Wait();

	for (int i = 0; i < jobsCount; i++)
		delete jobs[i];

	uint64 after = TickCount();
	printf("%llu\n", (after - before));

	oglTextureRenderer.Render(data);

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
