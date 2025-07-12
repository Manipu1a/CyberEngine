#include "interface/texture.hpp"

CYBER_BEGIN_NAMESPACE(Cyber)
CYBER_BEGIN_NAMESPACE(RenderObject)

MipLevelProperties compute_mip_level_properties(const RenderObject::TextureCreateDesc& load_info, uint32_t mip_level)
{
    MipLevelProperties mip_props;
    const auto& fmt_attribs = get_texture_format_attribs(load_info.m_format);
    mip_props.logical_width = std::max(1u, load_info.m_width >> mip_level);
    mip_props.logical_height = std::max(1u, load_info.m_height >> mip_level);
    mip_props.depth = std::max(1u, load_info.m_depth >> mip_level);
    if(fmt_attribs.component_type == COMPONENT_TYPE_COMPRESSED)
    {
        mip_props.storage_width = align_up(mip_props.logical_width, fmt_attribs.block_width);
        mip_props.storage_height = align_up(mip_props.logical_height, fmt_attribs.block_height);
        mip_props.row_size = uint64_t(mip_props.storage_width) / (uint32_t)fmt_attribs.block_width * (uint32_t)fmt_attribs.component_size;
        mip_props.depth_slice_size = mip_props.storage_height / uint32_t(fmt_attribs.block_height) * mip_props.row_size;
        mip_props.mip_size = mip_props.depth_slice_size * mip_props.depth;
    }
    else
    {
        mip_props.storage_width = mip_props.logical_width;
        mip_props.storage_height = mip_props.logical_height;
        mip_props.row_size = mip_props.storage_width * fmt_attribs.component_size * fmt_attribs.num_components;
        mip_props.depth_slice_size = mip_props.storage_height * mip_props.row_size;
        mip_props.mip_size = mip_props.depth_slice_size * mip_props.depth;
    }
    return mip_props;
}

CYBER_END_NAMESPACE
CYBER_END_NAMESPACE