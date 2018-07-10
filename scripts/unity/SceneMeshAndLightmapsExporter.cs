using UnityEngine;
using UnityEditor;
using System.Collections;
using System.IO;
using System.Collections.Generic;


class SceneMeshAndLightmapsExporter : EditorWindow
{
    [MenuItem("Window/SceneMeshAndLightmapsExporter")]


    public static void ShowWindow()
    {
        EditorWindow.GetWindow(typeof(SceneMeshAndLightmapsExporter));
    }


    void SetTextureReadable(string texturePath, bool state)
    {
        TextureImporter ti = AssetImporter.GetAtPath(texturePath) as TextureImporter;
        TextureImporterSettings tis = new TextureImporterSettings();
        ti.ReadTextureSettings(tis);
        tis.readable = state;
        ti.SetTextureSettings(tis);
        AssetDatabase.ImportAsset(texturePath);
    }


    int ExportLightmaps(string outputDir)
    {
        int colorLightmapsCount = 0;
        int dirLightmapsCount = 0;
        object[] objs = Resources.FindObjectsOfTypeAll(typeof(Texture2D));

        for (int i = 0; i < objs.Length; i++)
        {
            Texture2D texture = (Texture2D)objs[i];
            string texturePath = AssetDatabase.GetAssetPath(texture);

            if (texturePath.IndexOf("Lightmap") >= 0)
            {
                string textureFileName = Path.GetFileName(texturePath);

                if (texturePath.IndexOf("comp_light") >= 0)
                {
                    colorLightmapsCount++;

                    SetTextureReadable(texturePath, true);

                    byte[] bytes = texture.EncodeToPNG();
                    File.WriteAllBytes(outputDir + textureFileName + ".png", bytes);

                    SetTextureReadable(texturePath, false);
                }
                else if (texturePath.IndexOf("comp_dir") >= 0)
                {
                    dirLightmapsCount++;

                    File.Copy(texturePath, outputDir + textureFileName);
                }
            }
        }

        Debug.Assert(colorLightmapsCount == dirLightmapsCount);

        return colorLightmapsCount;
    }


    GameObject[] StaticGameObjectsWithMeshes(int lightmapsCount)
    {
        GameObject[] gos1 = GameObject.FindObjectsOfType<GameObject>();
        List<GameObject> gos2 = new List<GameObject>();

        foreach (GameObject go in gos1)
        {
            if (go.isStatic)
            {
                MeshRenderer mr = go.GetComponent<MeshRenderer>();
                MeshFilter mf = go.GetComponent<MeshFilter>();

                if (mr && mf && mf.sharedMesh.uv2 != null && mf.sharedMesh.uv2.Length == mf.sharedMesh.uv.Length && mr.lightmapIndex >= 0 && mr.lightmapIndex < lightmapsCount)
                {
                    gos2.Add(go);
                }
            }
        }

        return gos2.ToArray();
    }


    void ExportSceneMesh(string outputDir, int lightmapsCount)
    {
        GameObject[] gos = StaticGameObjectsWithMeshes(lightmapsCount);
        
        BinaryWriter bw = new BinaryWriter(File.Open(outputDir + "mesh.dat", FileMode.Create));

        bw.Write(lightmapsCount);

        int chunksCount = 0;
        for (int i = 0; i < gos.Length; i++)
        {
            GameObject go = gos[i];
            MeshFilter mf = go.GetComponent<MeshFilter>();

            chunksCount += mf.sharedMesh.subMeshCount;
        }
        bw.Write(chunksCount);

        for (int i = 0; i < gos.Length; i++)
        {
            GameObject go = gos[i];
            MeshFilter mf = go.GetComponent<MeshFilter>();
            MeshRenderer mr = go.GetComponent<MeshRenderer>();

            for (int j = 0; j < mf.sharedMesh.subMeshCount; j++)
            {
                string path = AssetDatabase.GetAssetPath(mr.sharedMaterials[j].GetTexture("_MainTex"));
                if (path != "")
                {
                    string newPath = outputDir + path;

                    if (!File.Exists(newPath))
                    {
                        string dir = Path.GetDirectoryName(newPath);
                        if (!Directory.Exists(dir))
                            Directory.CreateDirectory(dir);
                        File.Copy(path, newPath);
                    }
                }
                bw.Write((uint)path.Length);
                for (int k = 0; k < path.Length; k++)
                {
                    bw.Write(path[k]);
                }

                bw.Write((int)mr.lightmapIndex);
                bw.Write(mr.lightmapScaleOffset.x);
                bw.Write(mr.lightmapScaleOffset.y);
                bw.Write(mr.lightmapScaleOffset.z);
                bw.Write(mr.lightmapScaleOffset.w);
                bw.Write((int)mf.sharedMesh.vertexCount);
                bw.Write((int)mf.sharedMesh.GetIndexCount(j));
                for (int k = 0; k < mf.sharedMesh.vertexCount; k++)
                {
                    Vector3 position = mf.sharedMesh.vertices[k];
                    position = go.transform.TransformPoint(position);
                    Vector3 normal = mf.sharedMesh.normals[k];
                    normal = go.transform.TransformVector(normal);

                    bw.Write(position.x);
                    bw.Write(position.y);
                    bw.Write(position.z);
                    bw.Write(normal.x);
                    bw.Write(normal.y);
                    bw.Write(normal.z);
                    bw.Write(mf.sharedMesh.uv[k].x);
                    bw.Write(mf.sharedMesh.uv[k].y);
                    bw.Write(mf.sharedMesh.uv2[k].x);
                    bw.Write(mf.sharedMesh.uv2[k].y);
                }
                for (int k = 0; k < mf.sharedMesh.GetIndexCount(j); k++)
                {
                    bw.Write((ushort)mf.sharedMesh.GetIndices(j)[k]);
                }
            }
        }

        bw.Close();
    }


    void OnGUI()
    {
        if (GUILayout.Button("Export selected meshes and lightmaps"))
        {
            string dir = "E:/";

            int lightmapsCount = ExportLightmaps(dir);
            ExportSceneMesh(dir, lightmapsCount); 
        }
    }
}
