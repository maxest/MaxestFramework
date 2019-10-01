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


void CreateScene1()
{
	NRayTracer::CMaterial material1;
	material1.lambertianBRDFAlbedo = 0.0f * VectorCustom(1.0f, 1.0f, 1.0f);
	material1.AddBRDF(new CLambertianBRDF(material1.lambertianBRDFAlbedo));
	material1.AddBRDF(new CGlossySpecularBRDF(0.0f));
	material1.transmittance = 0.75f;
	material1.transmittanceEta = 0.5f;
	material1.reflectivity = 0.0f;
	scene.materials.push_back(material1);

	NRayTracer::CMaterial material2;
	material2.lambertianBRDFAlbedo = VectorCustom(1.0f, 1.0f, 0.0f);
	material2.AddBRDF(new CLambertianBRDF(material2.lambertianBRDFAlbedo));
	material2.AddBRDF(new CGlossySpecularBRDF(0.0f));
	material2.transmittance = 0.0f;
	material2.transmittanceEta = 0.0f;
	material2.reflectivity = 0.0f;
	scene.materials.push_back(material2);

	NRayTracer::CMaterial material3;
	material3.lambertianBRDFAlbedo = VectorCustom(1.0f, 1.0f, 1.0f);
	material3.AddBRDF(new CLambertianBRDF(material3.lambertianBRDFAlbedo));
	material3.AddBRDF(new CGlossySpecularBRDF(32.0f));
	material3.transmittance = 0.0f;
	material3.transmittanceEta = 0.0f;
	material3.reflectivity = 0.0f;
	scene.materials.push_back(material3);

	//

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

	SPointLight pointLight;
	//
	pointLight.position = VectorCustom(2.0f, 3.0f, 0.0f);
	pointLight.color = 5.0f * VectorCustom(1.0f, 1.0f, 1.0f);
	scene.pointLights.push_back(pointLight);
	//
	pointLight.position = VectorCustom(-2.0f, 5.0f, -15.0f);
	pointLight.color = 10.0f * VectorCustom(1.0f, 0.0f, 0.0f);
	scene.pointLights.push_back(pointLight);

	SDirLight dirLight;
	dirLight.dir = Normalize(VectorCustom(-1.0f, -1.0f, -1.0f));
	dirLight.color = 2.0f * VectorCustom(1.0f, 1.0f, 1.0f);
	scene.dirLights.push_back(dirLight);

	//

	camera.UpdateFixed(VectorCustom(5.0f, 5.0f, 17.5f), VectorCustom(0.0f, 0.0f, 0.0f));

	//

	rayTracer.maxRecursionDepth = 3;
	rayTracer.ambientConst = 0.0f;
	rayTracer.ambientOcclusionFactor = 0.5f;
	rayTracer.globalIllumination = false;
}


void CreateScene2()
{
	NRayTracer::CMaterial material1;
	material1.lambertianBRDFAlbedo = VectorCustom(1.0f, 1.0f, 1.0f);
	material1.AddBRDF(new CLambertianBRDF(material1.lambertianBRDFAlbedo));
	material1.AddBRDF( new CGlossySpecularBRDF(0.0f));
	material1.transmittance = 0.0f;
	material1.transmittanceEta = 0.0f;
	material1.reflectivity = 0.0f;
	scene.materials.push_back(material1);

	NRayTracer::CMaterial material2;
	material2.lambertianBRDFAlbedo = VectorCustom(1.0f, 0.0f, 0.0f);
	material2.AddBRDF(new CLambertianBRDF(material2.lambertianBRDFAlbedo));
	material2.AddBRDF(new CGlossySpecularBRDF(0.0f));
	material2.transmittance = 0.0f;
	material2.transmittanceEta = 0.0f;
	material2.reflectivity = 0.0f;
	scene.materials.push_back(material2);

	NRayTracer::CMaterial material3;
	material3.lambertianBRDFAlbedo = VectorCustom(0.0f, 1.0f, 0.0f);
	material3.AddBRDF(new CLambertianBRDF(material3.lambertianBRDFAlbedo));
	material3.AddBRDF(new CGlossySpecularBRDF(0.0f));
	material3.transmittance = 0.0f;
	material3.transmittanceEta = 0.0f;
	material3.reflectivity = 0.0f;
	scene.materials.push_back(material3);

	//

	// room
	{
		NMesh::SMesh mesh;
		ImportASE("../../../../data/meshes/unit_box.ASE", mesh);
		SwapOrdering(mesh);

		NMesh::SMesh whiteWalls;
		whiteWalls.chunks.resize(1);
		whiteWalls.chunks[0].vertices.push_back(mesh.chunks[0].vertices[0]);
		whiteWalls.chunks[0].vertices.push_back(mesh.chunks[0].vertices[1]);
		whiteWalls.chunks[0].vertices.push_back(mesh.chunks[0].vertices[2]);
		whiteWalls.chunks[0].vertices.push_back(mesh.chunks[0].vertices[3]);
		whiteWalls.chunks[0].vertices.push_back(mesh.chunks[0].vertices[4]);
		whiteWalls.chunks[0].vertices.push_back(mesh.chunks[0].vertices[5]);
		whiteWalls.chunks[0].vertices.push_back(mesh.chunks[0].vertices[12]);
		whiteWalls.chunks[0].vertices.push_back(mesh.chunks[0].vertices[13]);
		whiteWalls.chunks[0].vertices.push_back(mesh.chunks[0].vertices[14]);
		whiteWalls.chunks[0].vertices.push_back(mesh.chunks[0].vertices[15]);
		whiteWalls.chunks[0].vertices.push_back(mesh.chunks[0].vertices[16]);
		whiteWalls.chunks[0].vertices.push_back(mesh.chunks[0].vertices[17]);
		whiteWalls.chunks[0].vertices.push_back(mesh.chunks[0].vertices[24]);
		whiteWalls.chunks[0].vertices.push_back(mesh.chunks[0].vertices[25]);
		whiteWalls.chunks[0].vertices.push_back(mesh.chunks[0].vertices[26]);
		whiteWalls.chunks[0].vertices.push_back(mesh.chunks[0].vertices[27]);
		whiteWalls.chunks[0].vertices.push_back(mesh.chunks[0].vertices[28]);
		whiteWalls.chunks[0].vertices.push_back(mesh.chunks[0].vertices[29]);
		ToIndexed(whiteWalls);
		scene.AddMesh(whiteWalls, MatrixScale(10.0f, 10.0f, 10.0f) * MatrixTranslate(0.0f, 0.0f, 0.0f), 0);

		NMesh::SMesh redWallMesh;
		redWallMesh.chunks.resize(1);
		redWallMesh.chunks[0].vertices.push_back(mesh.chunks[0].vertices[30]);
		redWallMesh.chunks[0].vertices.push_back(mesh.chunks[0].vertices[31]);
		redWallMesh.chunks[0].vertices.push_back(mesh.chunks[0].vertices[32]);
		redWallMesh.chunks[0].vertices.push_back(mesh.chunks[0].vertices[33]);
		redWallMesh.chunks[0].vertices.push_back(mesh.chunks[0].vertices[34]);
		redWallMesh.chunks[0].vertices.push_back(mesh.chunks[0].vertices[35]);
		ToIndexed(redWallMesh);
		scene.AddMesh(redWallMesh, MatrixScale(10.0f, 10.0f, 10.0f) * MatrixTranslate(0.0f, 0.0f, 0.0f), 1);

		NMesh::SMesh greenWallMesh;
		greenWallMesh.chunks.resize(1);
		greenWallMesh.chunks[0].vertices.push_back(mesh.chunks[0].vertices[18]);
		greenWallMesh.chunks[0].vertices.push_back(mesh.chunks[0].vertices[19]);
		greenWallMesh.chunks[0].vertices.push_back(mesh.chunks[0].vertices[20]);
		greenWallMesh.chunks[0].vertices.push_back(mesh.chunks[0].vertices[21]);
		greenWallMesh.chunks[0].vertices.push_back(mesh.chunks[0].vertices[22]);
		greenWallMesh.chunks[0].vertices.push_back(mesh.chunks[0].vertices[23]);
		ToIndexed(greenWallMesh);
		scene.AddMesh(greenWallMesh, MatrixScale(10.0f, 10.0f, 10.0f) * MatrixTranslate(0.0f, 0.0f, 0.0f), 2);
	}

	//

	SSpherePrimitive sphere;

	sphere.position = VectorCustom(1.5f, -2.5f, 0.0f);
	sphere.radius = 1.5f;
	sphere.materialIndex = 0;
	scene.spheres.push_back(sphere);

	sphere.position = VectorCustom(-1.5f, -2.5f, -2.5f);
	sphere.radius = 1.5f;
	sphere.materialIndex = 0;
	scene.spheres.push_back(sphere);

	//

	SPointLight pointLight;
	//
	pointLight.position = VectorCustom(0.0f, 4.75f, 0.0f);
	pointLight.color = 10.0f * VectorCustom(1.0f, 1.0f, 1.0f);
	scene.pointLights.push_back(pointLight);

	//

	camera.UpdateFixed(VectorCustom(0.0f, 0.0f, 10.0f), VectorCustom(0.0f, 0.0f, 0.0f));

	//

	rayTracer.maxRecursionDepth = 2;
	rayTracer.ambientConst = 0.0f;
	rayTracer.ambientOcclusionFactor = 0.0f;
	rayTracer.globalIllumination = true;
}


void CreateScene3()
{
	NRayTracer::CMaterial material1;
	material1.lambertianBRDFAlbedo = VectorCustom(1.0f, 1.0f, 1.0f);
	material1.AddBRDF(new CLambertianBRDF(material1.lambertianBRDFAlbedo));
	material1.AddBRDF(new CGlossySpecularBRDF(32.0f));
	material1.transmittance = 0.0f;
	material1.transmittanceEta = 0.0f;
	material1.reflectivity = 1.0f;
	scene.materials.push_back(material1);

	NRayTracer::CMaterial material2;
	material2.lambertianBRDFAlbedo = VectorCustom(1.0f, 1.0f, 0.0f);
	material2.AddBRDF(new CLambertianBRDF(material2.lambertianBRDFAlbedo));
	material2.AddBRDF(new CGlossySpecularBRDF(0.0f));
	material2.transmittance = 0.0f;
	material2.transmittanceEta = 0.0f;
	material2.reflectivity = 0.0f;
	scene.materials.push_back(material2);

	//

	// floor
	{
		NMesh::SMesh mesh;
		ImportASE("../../../../data/meshes/unit_plane.ASE", mesh);
		ToIndexed(mesh);
		scene.AddMesh(mesh, MatrixScale(100.0f, 100.0f, 1.0f) * MatrixRotateX(-cPi / 2.0f), 0);
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
	sphere.materialIndex = 1;
	scene.spheres.push_back(sphere);

	sphere.position = VectorCustom(7.5f, 3.0f, 0.0f);
	sphere.radius = 2.5f;
	sphere.materialIndex = 1;
	scene.spheres.push_back(sphere);

	//

	SDirLight dirLight;
	dirLight.dir = Normalize(VectorCustom(-1.0f, -1.0f, -1.0f));
	dirLight.color = 2.0f * VectorCustom(1.0f, 1.0f, 1.0f);
	scene.dirLights.push_back(dirLight);

	//

	camera.UpdateFixed(VectorCustom(5.0f, 5.0f, 17.5f), VectorCustom(0.0f, 0.0f, 0.0f));

	//

	rayTracer.maxRecursionDepth = 2;
	rayTracer.ambientConst = 0.25f;
	rayTracer.ambientOcclusionFactor = 0.0f;
	rayTracer.globalIllumination = false;
}


void Create()
{
	Randomize();
	rayTracer.Create(width, height, scene);
	CreateScene3();
}


void Destroy()
{
	rayTracer.Destroy();
	scene.Destroy();
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

	if (application.IsKeyPressed(EKey::Space))
		camera.UpdateFree(eye);

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

	uint8* outputData = rayTracer.Render(&jobSystem, rtCamera);

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
	application.MouseShowCursor(false);
	application.MouseSetRelativeMode(true);
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
