
    struct CYBER_GRAPHICS_API RHIInstanceCreateDesc
    {
        bool enable_debug_layer;
        bool enable_gpu_based_validation;
        bool enable_set_name;
    };

    // Objects
    struct CYBER_GRAPHICS_API RHIInstance
    {
        ERHIBackend mBackend;
        ERHINvAPI_Status mNvAPIStatus;
        ERHIAGSReturenCode mAgsStatus;
        bool mEnableSetName;
    };
    