#ifndef __MACRO_H__
#define __MACRO_H__

#define SAFE_DELETE(p) do{if(p){delete p; p = nullptr;}}while(0)
#define SAFE_DELETE_ARRAY(p) do{if(p){delete[] p; p = nullptr;}}while(0)

#define METER(m)	(m * 1.0f)
#define CMETER(cm)	(METER(cm) * 0.01f)
#define MMETER(mm)	(METER(mm) * 0.001f)

#endif