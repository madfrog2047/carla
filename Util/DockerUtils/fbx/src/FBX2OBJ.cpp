/****************************************************************************************

   Copyright (C) 2015 Autodesk, Inc.
   All rights reserved.

   Use of this software is subject to the terms of the Autodesk license agreement
   provided at the time of installation or download, or which otherwise accompanies
   this software in either electronic or hard copy form.

****************************************************************************************/

#include <fbxsdk.h>

// declare global
FbxManager*   gSdkManager = NULL;

// materials
FbxSurfacePhong* gMatRoad;
FbxSurfacePhong* gMatSidewalk;
FbxSurfacePhong* gMatCross;
FbxSurfacePhong* gMatGrass;
FbxSurfacePhong* gMatBlock;

#ifdef IOS_REF
	#undef  IOS_REF
	#define IOS_REF (*(gSdkManager->GetIOSettings()))
#endif

// Create a material that will be applied to a polygon
FbxSurfacePhong* CreateMaterial(FbxScene* pScene, char *name)
{
    // Create material
    FbxSurfacePhong* lMaterial = FbxSurfacePhong::Create(pScene, name);

    return lMaterial;
}

bool StartsWith(const char *name, const char *str)
{
    size_t lenName = strlen(name);
    size_t lenStr = strlen(str);

    if (lenName == 0 || lenStr == 0 || lenStr != lenName) return false;

    return (memcmp(name, str, lenStr) == 0);
}

void SetMaterials(FbxNode* pNode)
{
	if (!pNode) return;

    FbxSurfacePhong* mat = gMatBlock;

    // only for mesh nodes
    FbxMesh* lMesh = pNode->GetMesh();
    if(lMesh)
    {
        // remove
        pNode->RemoveAllMaterials();
        // check nomenclature
        const char *name = pNode->GetName();
        if (StartsWith(name, "Road_Road"))
            mat = gMatRoad;
        else if (StartsWith(name, "Road_Marking"))
            mat = gMatRoad;
        else if (StartsWith(name, "Road_Curb"))
            mat = gMatRoad;
        else if (StartsWith(name, "Road_Gutter"))
            mat = gMatRoad;
		else if (StartsWith(name, "Road_Sidewalk"))
			mat = gMatSidewalk;
		else if (StartsWith(name, "Road_Crosswalk"))
			mat = gMatCross;
		else if (StartsWith(name, "Road_Grass"))
			mat = gMatGrass;

        printf("Node %s : %s\n", name, mat->GetName());
        pNode->AddMaterial(mat);
    }

    //recursively traverse each node in the scene
    int i, lCount = pNode->GetChildCount();
    for (i = 0; i < lCount; i++)
    {
        SetMaterials(pNode->GetChild(i));
    }
}

// Creates an importer object, and uses it to
// import a file into a scene.
bool LoadScene(
               FbxManager* pSdkManager,  // Use this memory manager...
               FbxScene* pScene,            // to import into this scene
               const char* pFilename         // the data from this file.
               )
{
    int lFileMajor, lFileMinor, lFileRevision;
    int lSDKMajor,  lSDKMinor,  lSDKRevision;
    //int i, lAnimStackCount;
    bool lStatus;
    //char lPassword[1024];

    // Get the version number of the FBX files generated by the
    // version of FBX SDK that you are using.
    FbxManager::GetFileFormatVersion(lSDKMajor, lSDKMinor, lSDKRevision);

    // Create an importer.
    FbxImporter* lImporter = FbxImporter::Create(pSdkManager,"");
    
    // Initialize the importer by providing a filename.
    const bool lImportStatus = lImporter->Initialize(pFilename, -1, pSdkManager->GetIOSettings() );

    // Get the version number of the FBX file format.
    lImporter->GetFileVersion(lFileMajor, lFileMinor, lFileRevision);

    if( !lImportStatus )  // Problem with the file to be imported
    {
        FbxString error = lImporter->GetStatus().GetErrorString();
        printf("Call to FbxImporter::Initialize() failed.");
        printf("Error returned: %s", error.Buffer());

        if (lImporter->GetStatus().GetCode() == FbxStatus::eInvalidFileVersion)
        {
            printf("FBX version number for this FBX SDK is %d.%d.%d",
                lSDKMajor, lSDKMinor, lSDKRevision);
            printf("FBX version number for file %s is %d.%d.%d",
                pFilename, lFileMajor, lFileMinor, lFileRevision);
        }

        return false;
    }

    // printf("FBX version number for this FBX SDK is %d.%d.%d",lSDKMajor, lSDKMinor, lSDKRevision);

    if (lImporter->IsFBX())
    {
        IOS_REF.SetBoolProp(IMP_FBX_MATERIAL,        false);
        IOS_REF.SetBoolProp(IMP_FBX_TEXTURE,         false);
        IOS_REF.SetBoolProp(IMP_FBX_LINK,            false);
        IOS_REF.SetBoolProp(IMP_FBX_SHAPE,           false);
        IOS_REF.SetBoolProp(IMP_FBX_GOBO,            false);
        IOS_REF.SetBoolProp(IMP_FBX_ANIMATION,       false);
        IOS_REF.SetBoolProp(IMP_FBX_GLOBAL_SETTINGS, false);
    }

    // Import the scene.
    lStatus = lImporter->Import(pScene);

    // Destroy the importer
    lImporter->Destroy();

    return lStatus;
}

// Exports a scene to a file
bool SaveScene(
               FbxManager* pSdkManager,
               FbxScene* pScene,
               const char* pFilename,
               int pFileFormat,
               bool pEmbedMedia
               )
{
    //int lMajor, lMinor, lRevision;
    bool lStatus = true;

    // Create an exporter.
    FbxExporter* lExporter = FbxExporter::Create(pSdkManager, "");

    // show file formats available
    // {
    //     //Try to export in ASCII if possible
    //     int lFormatIndex, lFormatCount = pSdkManager->GetIOPluginRegistry()->GetWriterFormatCount();

    //     for (lFormatIndex=0; lFormatIndex<lFormatCount; lFormatIndex++)
    //     {
    //         // if (pSdkManager->GetIOPluginRegistry()->WriterIsFBX(lFormatIndex))
    //         {
    //             FbxString lDesc = pSdkManager->GetIOPluginRegistry()->GetWriterFormatDescription(lFormatIndex);
    //             printf("\n%d) %s", lFormatIndex, lDesc);
    //         }
    //     }
    //     printf("\n");
    // }

    pFileFormat = pSdkManager->GetIOPluginRegistry()->FindWriterIDByDescription("Alias OBJ (*.obj)");
    
    // Initialize the exporter by providing a filename.
    if(lExporter->Initialize(pFilename, pFileFormat, pSdkManager->GetIOSettings()) == false)
    {
        printf("Call to FbxExporter::Initialize() failed.");
        printf("Error returned: %s", lExporter->GetStatus().GetErrorString());
        return false;
    }

    // FbxManager::GetFileFormatVersion(lMajor, lMinor, lRevision);
    // printf("FBX version number for this FBX SDK is %d.%d.%d",lMajor, lMinor, lRevision);

    if (pSdkManager->GetIOPluginRegistry()->WriterIsFBX(pFileFormat))
    {
        // Export options determine what kind of data is to be imported.
        // The default (except for the option eEXPORT_TEXTURE_AS_EMBEDDED)
        // is true, but here we set the options explicitly.
        IOS_REF.SetBoolProp(EXP_FBX_MATERIAL,        true);
        IOS_REF.SetBoolProp(EXP_FBX_TEXTURE,         true);
        IOS_REF.SetBoolProp(EXP_FBX_EMBEDDED,        pEmbedMedia);
        IOS_REF.SetBoolProp(EXP_FBX_SHAPE,           true);
        IOS_REF.SetBoolProp(EXP_FBX_GOBO,            true);
        IOS_REF.SetBoolProp(EXP_FBX_ANIMATION,       true);
        IOS_REF.SetBoolProp(EXP_FBX_GLOBAL_SETTINGS, true);
    }

    // get root node of the fbx scene
    FbxNode* lRootNode = pScene->GetRootNode();

    // rotate
    lRootNode->LclRotation.Set(FbxVector4(-90.0, 0.0, 0.0));

    // set materials following nomenclature of RoadRunner
    SetMaterials(lRootNode);

    // Export the scene.
    lStatus = lExporter->Export(pScene);

    // Destroy the exporter.
    lExporter->Destroy();

    return lStatus;
}

int main(int argc, char **argv)
{
    // Creates an instance of the SDK manager.
    gSdkManager = FbxManager::Create();

	// create an IOSettings object
	FbxIOSettings * ios = FbxIOSettings::Create(gSdkManager, IOSROOT );
	gSdkManager->SetIOSettings(ios);

    // ImportExport(argv[1], argv[2], -1);
	// Create a scene
	FbxScene* lScene = FbxScene::Create(gSdkManager,"");

    // import
    bool r = LoadScene(gSdkManager, lScene, argv[1]);
    if(!r) 
    {
        printf("------- Import failed ----------------------------");
        // Destroy the scene
		lScene->Destroy();
        return 0;
    }


    gMatRoad = CreateMaterial(lScene, "road");
    gMatSidewalk = CreateMaterial(lScene, "sidewalk");
    gMatCross = CreateMaterial(lScene, "crosswalk");
    gMatGrass = CreateMaterial(lScene, "grass");
    gMatBlock = CreateMaterial(lScene, "block");

    // export
    r = SaveScene(gSdkManager, lScene, argv[2], -1,	false);
    if(!r)
        printf("------- Export failed ----------------------------");

	// destroy the scene
	lScene->Destroy();

    // Destroys an instance of the SDK manager
    if (gSdkManager) gSdkManager->Destroy();

    return 1;
}