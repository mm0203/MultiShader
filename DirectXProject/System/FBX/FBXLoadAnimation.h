#ifndef __FBX_LOAD_ANIMATION_H__
#define __FBX_LOAD_ANIMATION_H__

#include "FBXLoader.h"

#if USE_FBXSDK
namespace ggfbx {

void CleanupAnimation(AnimationInfo &anime);
void GetAnimation(FbxScene *pScene, AnimationInfo &anime, bool isMirror);

}; // fbx
#endif // USE_FBXSDK

#endif // __FBX_LOAD_ANIMATION_H__