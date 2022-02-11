#ifndef __FBX_LOAD_MESH_H__
#define __FBX_LOAD_MESH_H__

#include "FBXLoader.h"

#if USE_FBXSDK
namespace ggfbx
{

void CleanupMesh(MeshList& meshList);
void GetMesh(FbxScene* pScene, MeshList& meshList, bool isMirror);

}; // fbx
#endif // USE_FBXSDK

#endif // __FBX_LOAD_MESH_H__