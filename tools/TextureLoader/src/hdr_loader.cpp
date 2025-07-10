#include "texture_loader_impl.hpp"
#include "image.h"
#include "graphics/interface/render_device.hpp"
#include "common/graphics_utils.hpp"
#include "texture_utils.h"
#include <algorithm>
#include "math/common.h"
#include "core/file_helper.hpp"
#include "dds_loader.h"

#ifndef STB_IMAGE_IMPLEMENTATION_INCLUDED
#define STB_IMAGE_IMPLEMENTATION_INCLUDED
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif

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
    int x, y, channels_in_file;
    //auto* hdr_data = stbi_load_from_memory(data, static_cast<int>(dataSize), &x, &y, &channels_in_file, 0);
    float* hdr_data = stbi_loadf("../../../../samples/pbrdemo/assets/minedump_flats_4k.hdr", &x, &y, &channels_in_file, 0);

    if(hdr_data == nullptr)
    {
        return;
    }
    m_textureCreateDesc.m_dimension = TEX_DIMENSION_2D;
    m_textureCreateDesc.m_width = x;
    m_textureCreateDesc.m_height = y;
    m_textureCreateDesc.m_depth = 1;
    m_textureCreateDesc.m_arraySize = 1;
    m_textureCreateDesc.m_mipLevels = 1;
    m_textureCreateDesc.m_format = channels_in_file == 3 ? TEX_FORMAT_RGB32_FLOAT : TEX_FORMAT_RGBA32_FLOAT;
    m_textureCreateDesc.m_sampleCount = SAMPLE_COUNT_1;

    m_textureSubResData.resize(1);

    m_textureSubResData[0].pData = hdr_data;
    m_textureSubResData[0].stride = static_cast<size_t>(x * sizeof(float) * channels_in_file);
    m_textureSubResData[0].depthStride = static_cast<size_t>(x * y * sizeof(float) * channels_in_file);
    m_textureSubResData[0].srcOffset = 0;
}

CYBER_END_NAMESPACE
CYBER_END_NAMESPACE