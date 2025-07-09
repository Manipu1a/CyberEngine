#include "texture_loader_impl.hpp"
#include "image.h"
#include "graphics/interface/render_device.hpp"
#include "common/graphics_utils.hpp"
#include "texture_utils.h"
#include <algorithm>
#include "math/common.h"
#include "core/file_helper.hpp"
#include "dds_loader.h"

CYBER_BEGIN_NAMESPACE(Cyber)
CYBER_BEGIN_NAMESPACE(TextureLoader)

void get_header_line(const uint8_t*& buffer_pos, char line[256])
{
    char* line_ptr = line;

    uint32_t i;

    for(i = 0;i < 255; ++i)
    {
        if(*buffer_pos == 0 || *buffer_pos == 10 || *buffer_pos == 13)
        {
            ++buffer_pos;
            break;
        }

        *line_ptr++ = *buffer_pos++;
    }

    line[i] = 0;
}

void decompressScanline(uint8_t* out, const uint8_t*& in, uint32_t width)
{
    // minimum and maxmimum scanline length for encoding
	uint32_t MINELEN = 8;
	uint32_t MAXELEN = 0x7fff;

	uint32_t Len = width;

	if(Len < MINELEN || Len > MAXELEN)
	{
		//OldDecompressScanline(out, in, Len);
		return;
	}

	uint8_t r = *in;

	if(r != 2)
	{
		//OldDecompressScanline(out, in, Len);
		return;
	}

	++in;

	uint8_t g = *in++;
	uint8_t b = *in++;
	uint8_t e = *in++;

	if(g != 2 || b & 128)
	{
		*out++ = r; *out++ = g; *out++ = b; *out++ = e;
		//OldDecompressScanline(out, in, Len - 1);
		return;
	}

	for(uint32_t Channel = 0; Channel < 4; ++Channel)
	{
		uint8_t* OutSingleChannel = out + Channel;

		for(uint32_t MultiRunIndex = 0; MultiRunIndex < Len;)
		{
			uint8_t c = *in++;

			if(c > 128)
			{
				uint32_t Count = c & 0x7f;

				c = *in++;

				for(uint32_t RunIndex = 0; RunIndex < Count; ++RunIndex)
				{
					*OutSingleChannel = c;
					OutSingleChannel += 4;
				}			
				MultiRunIndex += Count;
			}
			else
			{
				uint32_t Count = c;

				for(uint32_t RunIndex = 0; RunIndex < Count; ++RunIndex)
				{
					*OutSingleChannel = *in++;
					OutSingleChannel += 4;
				}
				MultiRunIndex += Count;
			}
		}
	}
}

void TextureLoaderImpl::load_from_hdr(const TextureLoadInfo& loadInfo, const uint8_t* data, size_t dataSize)
{
    if(dataSize < 11)
    {
        cyber_assert(false, "HDR data size is too small.");
        return;
    }

    const uint8_t* hdrData = data;

    char Line[256];

    get_header_line(hdrData, Line);

    if(strcmp(Line, "#?RADIANCE"))
    {
        return;
    }

    const uint8_t* rgb_data_start = nullptr;

    for(;;)
    {
        get_header_line(hdrData, Line);
        char* height_str = std::strstr(Line, "-Y");
        char* width_str = std::strstr(Line, "+X");
        
        if(height_str != nullptr && width_str != nullptr)
        {
            *(width_str - 1) = 0;

            m_textureCreateDesc.m_height = atoi(height_str + 3);
            m_textureCreateDesc.m_width = atoi(width_str + 3);

            rgb_data_start = hdrData;
            break;
        }
    }    

    if(rgb_data_start != nullptr)
    {
        std::vector<uint8_t> dds_rgb_data;

        dds_rgb_data.resize(4 + sizeof(DDS_HEADER) +
                            m_textureCreateDesc.m_width * m_textureCreateDesc.m_height * 4);

        uint32_t* dds_magic_number = reinterpret_cast<uint32_t*>(dds_rgb_data.data());
        *dds_magic_number = DDS_MAGIC;

        DDS_HEADER* dds_header = reinterpret_cast<DDS_HEADER*>(dds_rgb_data.data() + 4);

        dds_header->size = sizeof(DDS_HEADER);
        dds_header->flags = DDSF_Caps | DDSF_HEIGHT | DDSF_WIDTH | DDSF_PIXELFORMAT;
        dds_header->width = m_textureCreateDesc.m_width;
        dds_header->height = m_textureCreateDesc.m_height;
        dds_header->caps2 = 0;
        dds_header->mipMapCount = 1;
        dds_header->ddspf.size = sizeof(DDS_PIXELFORMAT);
        dds_header->ddspf.flags = DDS_RGB;
        dds_header->ddspf.RGBBitCount = 32;
        dds_header->ddspf.RBitMask = 0x00FF0000;
        dds_header->ddspf.GBitMask = 0x0000FF00;
        dds_header->ddspf.BBitMask = 0x000000FF;

        uint32_t* dds_rgb_data_ptr = reinterpret_cast<uint32_t*>(dds_rgb_data.data() + 4 + sizeof(DDS_HEADER));

        const uint8_t* file_data_ptr = rgb_data_start;

        for(uint32_t y = 0;y < m_textureCreateDesc.m_height; ++y)
        {
            uint8_t* line = (uint8_t*)&dds_rgb_data_ptr[m_textureCreateDesc.m_width * y];

            decompressScanline(line, file_data_ptr, m_textureCreateDesc.m_width);

            for(uint32_t x = 0; x < m_textureCreateDesc.m_width; ++x)
            {
                std::swap(line[x * 4 + 0], line[x * 4 + 2]);
            }
        }

        load_from_dds(loadInfo, dds_rgb_data.data(), dds_rgb_data.size());
    }
}

CYBER_END_NAMESPACE
CYBER_END_NAMESPACE