#ifndef __FBX_LOAD_BONE_H__
#define __FBX_LOAD_BONE_H__

#include "FBXLoader.h"

#if USE_FBXSDK
namespace ggfbx {

void CleanupBone(BoneListInfo &boneListInfo);
void GetBone(FbxScene *pScene, BoneListInfo &boneListInfo, bool isMirror);

}; // fbx
#endif // USE_FBXSDK

#endif // __FBX_LOAD_BONE_H__