#include "../../../src/main.h"
#include "../../../src/namespaces.h"


bool fullScreen = false;
int screenWidth = 800;
int screenHeight = 600;


CApplication application;

SBuffer vertexBuffer;

ID3D11VertexShader* meshVS = nullptr;

ID3D11PixelShader* meshPS = nullptr;


void Log(const string& msg)
{
	cout << msg << endl;
}


struct STransform
{
	string name;
	int parentIndex;
	SVector3 localPosition;
	SQuaternion localRotation;
	SMatrix localTransform;
	SMatrix globalTransform;
};

struct SPose
{
	vector<STransform> transforms;

	void CalculateGlobalTransforms()
	{
		for (uint i = 0; i < transforms.size(); i++)
		{
			if (transforms[i].parentIndex == -1)
				transforms[i].globalTransform = transforms[i].localTransform;
			else
				transforms[i].globalTransform = transforms[i].localTransform * transforms[transforms[i].parentIndex].globalTransform;
		}
	}
};

struct SNode
{
	SVector3 position, prevPosition;
	int transformIndex;
	SVector3 force;
	float mass;

	SNode()
	{
		mass = 1.0f;
	}
};

struct SConstaint
{
	int nodeIndex1;
	int nodeIndex2;
	float distance;

	SConstaint() {}
	SConstaint(int nodeIndex1, int nodeIndex2)
	{
		this->nodeIndex1 = nodeIndex1;
		this->nodeIndex2 = nodeIndex2;
	}
};

struct SCharacter
{
	vector<SNode> nodes;
	vector<SConstaint> constraints;

	void SetPose(const SPose& pose)
	{
		nodes.resize(10);
		nodes[0].transformIndex = 0;
		nodes[1].transformIndex = 17;
		nodes[2].transformIndex = 19;
		nodes[3].transformIndex = 13;
		nodes[4].transformIndex = 15;
		nodes[5].transformIndex = 2;
		nodes[6].transformIndex = 8;
		nodes[7].transformIndex = 10;
		nodes[8].transformIndex = 4;
		nodes[9].transformIndex = 6;

		for (uint i = 0; i < nodes.size(); i++)
		{
			SVector3 forwardAnimationPosition = VectorCustom(
				pose.transforms[nodes[i].transformIndex].globalTransform.m[3][0],
				pose.transforms[nodes[i].transformIndex].globalTransform.m[3][1],
				pose.transforms[nodes[i].transformIndex].globalTransform.m[3][2]);

			nodes[i].position = nodes[i].prevPosition = forwardAnimationPosition;
		}

		constraints.resize(9);
		constraints[0] = SConstaint(0, 1);
		constraints[1] = SConstaint(1, 2);
		constraints[2] = SConstaint(0, 3);
		constraints[3] = SConstaint(3, 4);
		constraints[4] = SConstaint(0, 5);
		constraints[5] = SConstaint(5, 6);
		constraints[6] = SConstaint(6, 7);
		constraints[7] = SConstaint(5, 8);
		constraints[8] = SConstaint(8, 9);
		for (uint i = 0; i < constraints.size(); i++)
			constraints[i].distance = Distance(nodes[constraints[i].nodeIndex1].position, nodes[constraints[i].nodeIndex2].position);
	}
};

vector<SPose> poses;
SCharacter character;


void LoadPoseTransform_Recursive(CFile& file, vector<STransform>& transforms)
{
	STransform transform;

	file.ReadBin(transform.name);

	file.ReadBin((char*)&transform.parentIndex, 4);

	file.ReadBin((char*)&transform.localPosition.x, 4);
	file.ReadBin((char*)&transform.localPosition.y, 4);
	file.ReadBin((char*)&transform.localPosition.z, 4);

	file.ReadBin((char*)&transform.localRotation.x, 4);
	file.ReadBin((char*)&transform.localRotation.y, 4);
	file.ReadBin((char*)&transform.localRotation.z, 4);
	file.ReadBin((char*)&transform.localRotation.w, 4);

	transform.localTransform =
		QuaternionToMatrix(transform.localRotation) *
		MatrixTranslate(transform.localPosition);

	transforms.push_back(transform);

	int childCount;
	file.ReadBin((char*)&childCount, 4);

	for (int i = 0; i < childCount; i++)
		LoadPoseTransform_Recursive(file, transforms);
}


SPose LoadPose(const string& path)
{
	SPose pose;

	CFile file;
	if (file.Open(path, CFile::EOpenMode::ReadBinary))
	{
		LoadPoseTransform_Recursive(file, pose.transforms);
		pose.CalculateGlobalTransforms();
		file.Close();
	}

	return pose;
}


SPose BlendPoses(const vector<SPose>& poses, const vector<float>& weights, int technique)
{
	MF_ASSERT(poses.size() == weights.size());
	MF_ASSERT(poses.size() >= 2);

	SPose newPose;
	size_t transformsCount = poses[0].transforms.size();

	for (uint i = 0; i < transformsCount; i++)
	{
		int parentIndex = poses[0].transforms[i].parentIndex;

		SVector3 localPosition = cVector3Zero;
		for (uint j = 0; j < poses.size(); j++)
			localPosition += weights[j] * poses[j].transforms[i].localPosition;

		SQuaternion localRotation;
		if (technique == 0)
		{
			localRotation = cQuaternionIdentity;

			// rotation order should be inversed to match what I did in Unity
			for (uint j = 0; j < poses.size(); j++)
				localRotation *= Slerp(cQuaternionIdentity, poses[j].transforms[i].localRotation, weights[j]);
		}
		else if (technique == 1)
		{
			localRotation = cQuaternionZero;

			for (uint j = 0; j < poses.size(); j++)
				localRotation += weights[j] * poses[j].transforms[i].localRotation;
		}
		else if (technique == 2)
		{
			// https://www.gamedev.net/forums/topic/645242-quaternions-and-animation-blending-questions/

			float weightsSum = 0.0f;
			localRotation = cQuaternionIdentity;

			for (uint j = 0; j < poses.size(); j++)
			{
				if (weights[j] > cEpsilon4)
				{
					weightsSum += weights[j];
					float weight = weights[j] / weightsSum;

					localRotation = Slerp(localRotation, poses[j].transforms[i].localRotation, weight);
					localRotation = Normalize(localRotation);
				}
			}
		}

		localRotation = Normalize(localRotation);

		STransform transform;
		transform.parentIndex = parentIndex;
		transform.localPosition = localPosition;
		transform.localRotation = localRotation;
		transform.localTransform =
			QuaternionToMatrix(localRotation) *
			MatrixTranslate(localPosition);
		newPose.transforms.push_back(transform);
	}

	return newPose;
}
SPose BlendPoses(const SPose& pose1, const SPose& pose2, float t)
{
	vector<SPose> poses;
	vector<float> weights;

	poses.push_back(pose1);
	poses.push_back(pose2);
	weights.push_back(1.0f - t);
	weights.push_back(t);

	return BlendPoses(poses, weights, 2);
}


SPose NewPose(float interp)
{
	float lastFrameTime = application.LastFrameTime();

	static int blendPosesTechniqueIndex = 2;
	if (application.IsKeyDown(EKey::F1))
		blendPosesTechniqueIndex = 0;
	if (application.IsKeyDown(EKey::F2))
		blendPosesTechniqueIndex = 1;
	if (application.IsKeyDown(EKey::F3))
		blendPosesTechniqueIndex = 2;

	int index1 = (int)interp % 4;
	int index2 = (index1 + 1) % 4;
	float progress = interp - index1;
	vector<float> weights;
	{
		for (int j = 0; j < 4; j++)
			weights.push_back(0.0f);
		weights[index1] = 1.0f - progress;
		weights[index2] = progress;
	}
	return BlendPoses(poses, weights, blendPosesTechniqueIndex);
}


void SetPose(vector<SNode>& nodes, const SPose& pose)
{
	for (uint i = 0; i < nodes.size(); i++)
	{
		SVector3 forwardAnimationPosition = VectorCustom(
			pose.transforms[nodes[i].transformIndex].globalTransform.m[3][0],
			pose.transforms[nodes[i].transformIndex].globalTransform.m[3][1],
			pose.transforms[nodes[i].transformIndex].globalTransform.m[3][2]);

		nodes[i].position = nodes[i].prevPosition = forwardAnimationPosition;
	}
}


void CreateSimulation()
{
	poses.resize(4);
	poses[0] = LoadPose("../../data/poses/pose1.asset");
	poses[1] = LoadPose("../../data/poses/pose2.asset");
	poses[2] = LoadPose("../../data/poses/pose3.asset");
	poses[3] = LoadPose("../../data/poses/pose4.asset");

	character.SetPose(poses[0]);
}


void UpdateSimulation(float lastFrameTime)
{
	lastFrameTime *= 0.001f;

	static float interp = 0.0f;
	if (application.IsKeyPressed(EKey::A))
		interp -= 2.0f * lastFrameTime;
	if (application.IsKeyPressed(EKey::D))
		interp += 2.0f * lastFrameTime;
	if (interp >= (float)4)
		interp = 0.0f;
	if (interp < 0.0f)
		interp = (float)4 - 0.001f;
	SPose newPose = NewPose(interp);

	newPose.CalculateGlobalTransforms();

	for (uint i = 0; i < character.nodes.size(); i++)
	{
		SVector3 forwardAnimationPosition = VectorCustom(
			newPose.transforms[character.nodes[i].transformIndex].globalTransform.m[3][0],
			newPose.transforms[character.nodes[i].transformIndex].globalTransform.m[3][1],
			newPose.transforms[character.nodes[i].transformIndex].globalTransform.m[3][2]);

		SVector3 temp = character.nodes[i].position;
		character.nodes[i].position = MoveTowards(character.nodes[i].position, forwardAnimationPosition, 50.0f*lastFrameTime);
		character.nodes[i].prevPosition += character.nodes[i].position - temp; // this cancels velocity induction that happens in the next phase (Verlet)

		character.nodes[i].force = cVector3Zero;
	}

	if (application.IsKeyPressed(EKey::F))
	{
		character.nodes[8].force = VectorCustom(-500.0f, 0.0f, 0.0f);
		character.nodes[9].force = VectorCustom(-500.0f, 0.0f, 0.0f);
	}

	for (uint i = 0; i < character.nodes.size(); i++)
	{
		SVector3 temp = character.nodes[i].position;

		SVector3 velocity = character.nodes[i].position - character.nodes[i].prevPosition;
		if (character.nodes[i].force == cVector3Zero) // dampen/friction
			velocity = velocity * Pow(0.01f, lastFrameTime);

		character.nodes[i].position += velocity + character.nodes[i].force*lastFrameTime*lastFrameTime;
		character.nodes[i].prevPosition = temp;
	}

	// constraints
	for (int i = 0; i < 100; i++)
	{
		for (int j = 0; j < character.constraints.size(); j++)
		{
			SVector3 diff = character.nodes[character.constraints[j].nodeIndex1].position - character.nodes[character.constraints[j].nodeIndex2].position;
			float d = Length(diff);

			if (d <= 0.0001f)
				continue;

			float im1 = 1.0f / character.nodes[character.constraints[j].nodeIndex1].mass;
			float im2 = 1.0f / character.nodes[character.constraints[j].nodeIndex2].mass;

			// difference scalar
			float difference = (character.constraints[j].distance - d) / (d * (im1 + im2));

			SVector3 translate = diff * difference;
			character.nodes[character.constraints[j].nodeIndex1].position += im1 * translate;
			character.nodes[character.constraints[j].nodeIndex2].position -= im2 * translate;
		}

		//	nodes[2].position = VectorCustom(4.6f, -8.7f, 1.55f);
		//	nodes[4].position = VectorCustom(-4.7f, -7.9f, -1.15f);
	}
}


void CreateShaders()
{
	MF_ASSERT(CreateVertexShader("../../data/mesh_vs.hlsl", meshVS));

	MF_ASSERT(CreatePixelShader("../../data/mesh_ps.hlsl", meshPS));
}


void DestroyShaders()
{
	DestroyPixelShader(meshPS);

	DestroyVertexShader(meshVS);
}


bool Create()
{
	CreateSimulation();

	CreateD3D11(screenWidth, screenHeight, false);

	gGPUProfiler.Create();
	gGPUUtilsResources.Create("../../../../");

	CreateVertexBuffer(true, nullptr, 2*sizeof(SVertex_Pos), vertexBuffer);

	CreateShaders();

	return true;
}


void Destroy()
{
	DestroyShaders();

	DestroyBuffer(vertexBuffer);

	gGPUProfiler.Destroy();
	gGPUUtilsResources.Destroy();

	DestroyD3D11();
}


void DrawLine(const SVector3& v1, const SVector3& v2)
{
	SVertex_Pos vertices[2];
	vertices[0].position = v1;
	vertices[1].position = v2;
	UpdateBuffer(vertexBuffer, (uint8*)vertices, 2 * sizeof(SVertex_Pos));
	deviceContext->Draw(2, 0);
}


bool Run()
{
	float lastFrameTime = application.LastFrameTime();

	//

	UpdateSimulation(lastFrameTime);

	//

	gGPUProfiler.StartFrame();

	float backgroundColor[] = { 0.2f, 0.2f, 0.2f, 1.0f };

	deviceContext->RSSetState(gGPUUtilsResources.ccwRasterizerState);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	SMatrix viewProjTransform =
		MatrixLookAtLH(VectorCustom(0.0f, 0.0f, -50.0f), VectorCustom(0.0f, 0.0f, 0.0f), VectorCustom(0.0f, 1.0f, 0.0f)) *
		MatrixPerspectiveFovLH(EZRange::ZeroToOne, cPi/3.0f, (float)screenWidth/(float)screenHeight, 1.0f, 300.0f);

	deviceContext->UpdateSubresource(gGPUUtilsResources.ConstantBuffer(4).buffer, 0, nullptr, (void*)&viewProjTransform, 0, 0);

	SetSamplers();

	SetViewport(screenWidth, screenHeight);
	deviceContext->OMSetRenderTargets(1, &backBufferRTV, nullptr);
	deviceContext->ClearRenderTargetView(backBufferRTV, backgroundColor);

	deviceContext->IASetInputLayout(gGPUUtilsResources.posInputLayout);
	deviceContext->VSSetShader(meshVS, nullptr, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &gGPUUtilsResources.ConstantBuffer(4).buffer);
	deviceContext->PSSetShader(meshPS, nullptr, 0);

	UINT stride = sizeof(SVertex_Pos);
	UINT offset = 0;
	deviceContext->IASetVertexBuffers(0, 1, &vertexBuffer.buffer, &stride, &offset);
	
	for (uint i = 0; i < character.constraints.size(); i++)
		DrawLine(character.nodes[character.constraints[i].nodeIndex1].position, character.nodes[character.constraints[i].nodeIndex2].position);

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
