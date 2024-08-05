#ifndef _PLATFORM_H_
#define _PLATFORM_H_

#define _CRT_SECURE_NO_WARNINGS

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#endif

#include <Windows.h>

#include <cstdint>
#include <cstdio>
#include <cwchar>
#include <memory>
#include <new>
#include <vector>
#include <unordered_map>
#include <array>

#include <DirectXMath.h>
#include <DirectXPackedVector.h>


#ifdef _DEBUG
#include <crtdbg.h>
#endif

#endif /* _PLATFORM_H_ */

