#pragma once

#define D3D12

#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>
#include "D3D12MemAlloc.h"

// D3D12 Resource Naming
// Auto-enabled in debug builds. Override with:
//   CYBER_D3D12_NO_RESOURCE_NAMES  - force disable in debug
//   CYBER_D3D12_SET_RESOURCE_NAMES - force enable in release
#if defined(_DEBUG) && !defined(CYBER_D3D12_NO_RESOURCE_NAMES)
  #ifndef CYBER_D3D12_SET_RESOURCE_NAMES
    #define CYBER_D3D12_SET_RESOURCE_NAMES
  #endif
#endif

#ifdef CYBER_D3D12_SET_RESOURCE_NAMES
  #define D3D12_SET_RESOURCE_NAME(pResource, wname) \
      do { if(pResource) (pResource)->SetName(wname); } while(0)

  #define D3D12_SET_RESOURCE_NAME_U8(pResource, u8name, fallback) \
      do { if(pResource) { \
          if((u8name) && (u8name)[0] != u8'\0') \
              (pResource)->SetName(u8_to_wstring(u8name).c_str()); \
          else \
              (pResource)->SetName(fallback); \
      }} while(0)
#else
  #define D3D12_SET_RESOURCE_NAME(pResource, wname) ((void)0)
  #define D3D12_SET_RESOURCE_NAME_U8(pResource, u8name, fallback) ((void)0)
#endif
