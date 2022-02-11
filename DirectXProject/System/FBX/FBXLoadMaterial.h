#ifndef __FBX_LOAD_MATERIAL_H__
#define __FBX_LOAD_MATERIAL_H__

#include "FBXLoader.h"

#if USE_FBXSDK
namespace ggfbx {

void CleanupMaterial(MaterialList &materialList);
void GetMaterial(FbxScene *pScene, MaterialList &materialList);

}; // fbx
#endif // USE_FBXSDK

#endif // __FBX_LOAD_MATERIAL_H__