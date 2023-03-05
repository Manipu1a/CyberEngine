#pragma once
#include "cyber_rhi_config.h"

// enums
typedef enum ERHINvAPI_Status
{
    RHI_NVAPI_OK = 0,
    RHI_NVAPI_NONE = 1,
    RHI_NVAPI_ERROR = -1,
    RHI_NVAPI_MAX_ENUM_BIT = 0x7FFFFFFF
} ERHINvAPI_Status;

typedef enum ERHIAGSReturenCode
{
    RHI_AGS_SUCCESS,                    ///< Successful function call
    RHI_AGS_FAILURE,                    ///< Failed to complete call for some unspecified reason
    RHI_AGS_INVALID_ARGS,               ///< Invalid arguments into the function
    RHI_AGS_OUT_OF_MEMORY,              ///< Out of memory when allocating space internally
    RHI_AGS_MISSING_D3D_DLL,            ///< Returned when a D3D dll fails to load
    RHI_AGS_LEGACY_DRIVER,              ///< Returned if a feature is not present in the installed driver
    RHI_AGS_NO_AMD_DRIVER_INSTALLED,    ///< Returned if the AMD GPU driver does not appear to be installed
    RHI_AGS_EXTENSION_NOT_SUPPORTED,    ///< Returned if the driver does not support the requested driver extension
    RHI_AGS_ADL_FAILURE,                ///< Failure in ADL (the AMD Display Library)
    RHI_AGS_DX_FAILURE,                 ///< Failure from DirectX runtime
    RHI_AGS_NONE,
    RHI_AGS_MAX_ENUN_BIT = 0x7FFFFFFF
} ERHIAGSReturenCode;

typedef enum ERHITextureDimension
{
    RHI_TEX_DIMENSION_1D,
    RHI_TEX_DIMENSION_2D,
    RHI_TEX_DIMENSION_2DMS,
    RHI_TEX_DIMENSION_3D,
    RHI_TEX_DIMENSION_CUBE,
    RHI_TEX_DIMENSION_1D_ARRAY,
    RHI_TEX_DIMENSION_2D_ARRAY,
    RHI_TEX_DIMENSION_2DMS_ARRAY,
    RHI_TEX_DIMENSION_CUBE_ARRAY,
    RHI_TEX_DIMENSION_COUNT,
    RHI_TEX_DIMENSION_UNDEFINED,
    RHI_TEX_DIMENSION_MAX_ENUM_BIT = 0x7FFFFFFF
} ERHITextureDimension;

typedef enum ERHIPipelineType
{
    RHI_PIPELINE_TYPE_NONE = 0,
    RHI_PIPELINE_TYPE_COMPUTE,
    RHI_PIPELINE_TYPE_GRAPHICS,
    RHI_PIPELINE_TYPE_RAYTRACING,
    RHI_PIPELINE_TYPE_COUNT,
    RHI_PIPELINE_TYPE_MAX_ENUM_BIT = 0x7FFFFFFF
} ERHIPipelineType;


typedef enum ERHIFenceStatus
{
    RHI_FENCE_STATUS_COMPLETE = 0,
    RHI_FENCE_STATUS_INCOMPLETE,
    RHI_FENCE_STATUS_NOTSUBMITTED,
    RHI_FENCE_STATUS_COUNT,
    RHI_FENCE_STATUS_MAX_ENUM_BIT = 0x7FFFFFFF
} ERHIFenceStatus;

typedef enum ERHIQueryType
{
    RHI_QUERY_TYPE_TIMESTAMP = 0,
    RHI_QUERY_TYPE_PIPELINE_STATISTICS,
    RHI_QUERY_TYPE_OCCLUSION,
    RHI_QUERY_TYPE_COUNT,
    RHI_QUERY_TYPE_MAX_ENUM_BIT = 0x7FFFFFFF
} ERHIQueryType;
