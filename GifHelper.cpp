#include "stdafx.h"
#include "GifHelper.h"
#include "giflib-5.1.1/lib/gif_lib.h"
#include <cassert>
#include <io.h>
#include <fcntl.h>

GifHelper::GifHelper()
	:m_width(0)
	,m_height(0)
	,m_ptr(nullptr)
	,m_frames(0)
	,m_fps(0)
	,m_rect(nullptr)
{
}


GifHelper::~GifHelper()
{
	for (int i = 0; i < m_frames; i++)
		delete[]m_ptr[i];
	delete[]m_ptr;
	delete[]m_rect;
}

bool GifHelper::LoadFromFile(const std::wstring& Path)
{
	int hgif = _wopen(Path.c_str(), O_RDONLY);
	int err;
	GifFileType* gif = DGifOpenFileHandle(hgif, &err);
	DGifSlurp(gif);
	m_width = gif->SWidth;
	m_height = gif->SHeight;
	m_frames = gif->ImageCount;
	GraphicsControlBlock gcb;
	m_ptr = new uint8_t*[m_frames];
	m_rect = new sf::FloatRect[m_frames];
	for (int i = 0; i < m_frames; i++) {
		DGifSavedExtensionToGCB(gif, i, &gcb);
		m_rect[i].left = gif->SavedImages[i].ImageDesc.Left;
		m_rect[i].top = gif->SavedImages[i].ImageDesc.Top;
		m_rect[i].width = gif->SavedImages[i].ImageDesc.Width;
		m_rect[i].height = gif->SavedImages[i].ImageDesc.Height;
		
		int pixels = gif->SavedImages[i].ImageDesc.Width*gif->SavedImages[i].ImageDesc.Height;
		uint8_t* buf = new uint8_t[pixels * 4];
		m_ptr[i] = buf;
		for (int x = 0; x < pixels; x++)
		{
			GifByteType ind = gif->SavedImages[i].RasterBits[x];
			GifColorType pcolor = gif->SColorMap->Colors[ind];
			buf[4 * x + 0] = pcolor.Red;
			buf[4 * x + 1] = pcolor.Green;
			buf[4 * x + 2] = pcolor.Blue;
			buf[4 * x + 3] = (ind == gcb.TransparentColor) ? 0 : 255;
		}
		int a = 0;
	}
	DGifCloseFile(gif, &err);
	return true;
	/*
	assert(sizeof(GifHeader) == 13);
	assert(sizeof(GifColorTable) == 3);
	assert(sizeof(GifImageBlockHeader) == 10);
	std::ifstream file;
	file.open(Path, std::ios::ate | std::ios::binary);
	if (!file.is_open())
		return false;
	size_t size = file.tellg();
	file.seekg(0);
	char* buf = new char[size];
	file.read(buf, size);

	FormatHelper h(buf);
	h.Eat(m_header);

	if (0x80 & m_header.gctf)
	{
		int t = ((0xe0 & m_header.gctf) >> 5) + 1;
		m_size_ct = 1;
		for (int i = 0; i < t; i++)
			m_size_ct *= 2;
		m_ct = new GifColorTable[m_size_ct];
		h.Eat(m_ct, m_size_ct);
	}

	uint8_t block_type;
	h.Read(&block_type, 1);
	while (block_type != 0x3b) {
		switch (block_type)
		{
		case 0x21:
		{
			// Extensions
			uint8_t ext_type;
			h.Read(&ext_type, 1, 1);
			switch (ext_type)
			{
			case 0xff:
			{
				h.Skip(14);
				uint8_t sb_size;
				h.Eat(sb_size);
				while (sb_size != 0)
				{
					h.Skip(sb_size);
					h.Eat(sb_size);
				}
				break;
			}
			case 0xf9:
			{
				h.Eat(m_gce);
				break;
			}
			case 0xfe:

				break;
			case 0x01:

				break;
			default:
				break;
			}
			break;
		}
		case 0x2c:
		{
			// Image descriptor
			h.Eat(m_ibheader);
			assert(!(m_ibheader.flags & 0x80));
			uint8_t lzw_size;
			h.Eat(lzw_size);
			uint8_t sb_size;
			h.Eat(sb_size);
			while (sb_size != 0)
			{
				h.Skip(sb_size);
				h.Eat(sb_size);
			}
			break;
		}
		}
		h.Read(&block_type, 1);
	}
	delete[]buf;
	return false;*/
}
