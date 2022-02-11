#ifndef __DEBUG_H__
#define __DEBUG_H__

#ifdef _DEBUG
#define USE_DEBUG 1
#else
#define USE_DEBUG 0
#endif

#if USE_DEBUG
#define DMODIFIER
#else
#define DMODIFIER {}
#endif


void InitDebug() DMODIFIER;
void UninitDebug() DMODIFIER;
void UpdateDebug() DMODIFIER;
void DrawDebug() DMODIFIER;

#endif