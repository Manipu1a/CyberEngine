#pragma once

typedef uint16_t RgFlags16;

typedef uint32_t RgFlags32;

typedef uint64_t RgFlags64;

typedef uint32_t RgIndex32;

typedef enum ERenderGraphResult
{
    /// Successful completion.
    RG_OK = 0,

    /// Failure due to an unspecified error.
    RG_ERROR_UNSPECIFIED = -1,

    /// Failure due to an unrecognized command.
    RG_ERROR_UNRECOGNIZED_COMMAND = -2,

    /// Failure due to an invalid arguments.
    RG_ERROR_INVALID_ARGUMENTS = -3,

    /// Failure due to an invalid data.
    RG_ERROR_INVALID_DATA = -4,

    /// Failure due to an invalid operation.
    RG_ERROR_INVALID_OPERATION = -5,

    /// Failure due to running out of memory.
    RG_ERROR_OUT_OF_MEMORY = -6,

    /// Failure due to not being able to find the specified file.
    RG_ERROR_FILE_NOT_FOUND = -7,

    /// Failure due to an invalid file format.
    RG_ERROR_INVALID_FILE_FORMAT = -8,

    /// Failure due to an invalid file format version being too old.
    RG_ERROR_FILE_FORMAT_VERSION_TOO_OLD = -9,

    /// Failure due to an invalid file format version being too new.
    RG_ERROR_FILE_FORMAT_VERSION_TOO_NEW = -10,

    /// Failure due to an unknown node.
    RG_ERROR_UNKNOWN_NODE = -11,

    /// Failure due to an index being out of its valid bounds.
    RG_ERROR_INDEX_OUT_OF_BOUNDS = -12,

    /// Failure due to a command being already finalized.
    RG_ERROR_COMMAND_ALREADY_FINALIZED = -13,

    /// Failure due to a data layout mismatch between runtime and shader.
    RG_ERROR_DATA_LAYOUT_MISMATCH = -14,

    /// Failure due to a key not being found.
    RG_ERROR_KEY_NOT_FOUND = -15,

    /// Failure due to a key value being duplicated where it is required to be unique.
    RG_ERROR_KEY_DUPLICATED = -16,

    /// Failure due to a feature not being implemented yet.
    RG_ERROR_NOT_IMPLEMENTED = -17,

    /// Failure due to an integer overflow.
    RG_ERROR_INTEGER_OVERFLOW = -18,

    /// Failure due to exclusive rangs overlapping.
    RG_ERROR_RANGE_OVERLAPPING = -19,

    /// Failure due to rpsRenderPipelineValidate finding an invalid pipeline configuration. More details are provided
    /// via output of the device print function.
    RG_ERROR_VALIDATION_FAILED = -20,

    /// Failure due to a compiled RPSL shader program being ill formed. Normally indicates a compiler error.
    RG_ERROR_INVALID_PROGRAM = -21,

    /// Failure due to an RPSL module being incompatible with the current runtime. 
    RG_ERROR_UNSUPPORTED_MODULE_VERSION = -22,

    /// Failure due to a failed type safety check.
    RG_ERROR_TYPE_MISSMATCH = -23,

    /// Failure due to a feature not being supported.
    RG_ERROR_NOT_SUPPORTED = -24,

    /// Failure due to failed a runtime API without direct mapping of the API error code.
    RG_ERROR_RUNTIME_API_ERROR = -25,

    /// Failure due to an RPS library internal error.
    RG_ERROR_INTERNAL_ERROR = -26,

    /// Number of unique error codes.
    RG_ERROR_COUNT = 27
} ERenderGraphResult;