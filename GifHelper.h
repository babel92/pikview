#pragma once

#include <cstdint>
#include <fstream>
#include <cmath>
#include <SFML/Graphics.hpp>

class FormatHelper
{
private:
	uint8_t* m_ptr;
	uint8_t* m_pos;
public:
	template<class Tp>
	FormatHelper(Tp* Ptr)
		:m_ptr((uint8_t*)Ptr)
		,m_pos(m_ptr){}

	void Skip(int Count)
	{
		m_pos += Count;
	}

	template <class T>
	void Eat(T* Output, int Count)
	{
		memcpy(Output, m_pos, Count*sizeof(T));
		m_pos += Count*sizeof(T);
	}

	template<>
	void Eat(void* Output, int Count)
	{
		memcpy(Output, m_pos, Count);
		m_pos += Count;
	}

	template <class T>
	void Eat(T& ObjRef)
	{
		memcpy(&ObjRef, m_pos, sizeof(ObjRef));
		m_pos += sizeof(ObjRef);
	}

	void Read(void* Output, int Bytes, int Offset = 0)
	{
		memcpy(Output, m_pos + Offset, Bytes);
	}
};

struct GifHeader
{
	uint8_t header[6];
	uint16_t width;
	uint16_t height;
	uint8_t gctf; // Global color table flag
	uint8_t bci; // Background color index
	uint8_t par; // Pixel aspect ratio
};

struct GifColorTable
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
};

struct GifGCEBlock
{
	uint8_t ei; // 0x21
	uint8_t gcl; // 0xf9
	uint8_t byte_size;
	uint8_t flags;
	uint16_t delay;
	uint8_t trans_color_ind;
	uint8_t block_end; // 0x00
};

struct GifAEBlock
{
	uint8_t ei;
	uint8_t al;
	uint8_t size; // 0x0b
};

struct GifImageBlockHeader
{
	uint8_t header; // 0x21
	uint16_t lp;
	uint16_t tp;
	uint16_t width;
	uint16_t height;
	uint8_t flags;
};

class Bits
{
	std::list<char> m_container;
public:
	Bits(){}
	Bits(uint32_t Number, int Digits)
	{
		for (int i = 0; i < Digits; i++)
		{
			m_container.push_back(Number % 2);
			Number /= 2;
		}
	}
};

class BitExtractor
{
	struct PtrContainer
	{
		uint8_t* Ptr;
		int Size;
	};
	std::vector<PtrContainer> m_stream;
	
	int m_pos_stream;
	int m_pos_byte;
	int m_pos_bit;
public:
	BitExtractor()
		:m_pos_stream(0)
		,m_pos_byte(0)
		,m_pos_bit(0)
	{}
	~BitExtractor()
	{
		for (auto i : m_stream)
			delete[]i.Ptr;
	}
	void FeedMove(uint8_t*&Ptr, int Size)
	{
		PtrContainer p;
		p.Ptr = Ptr;
		p.Size = Size;
		m_stream.push_back(p);
		Ptr = nullptr;
	}
	void FeedCopy(uint8_t*Ptr, int Size)
	{
		PtrContainer p;
		p.Ptr = new uint8_t[Size];
		p.Size = Size;
		m_stream.push_back(p);
	}
	char GetDigit(int Pos)
	{
		
	}
	uint32_t Extract(int Digit)
	{
		uint32_t ret = 0;
		int pos_bit = Digit % 8;
		int pos_byte = m_pos_byte + Digit / 8;
		int pos_stream = m_pos_stream;
		for (; pos_byte > m_stream[pos_stream].Size; pos_stream++)
		{
			pos_byte -= m_stream[pos_stream].Size;
			pos_stream++;
		}
		
	}
};

class GifLZWDecompressor
{
	int m_lzsize;
	uint8_t* m_ptr;
	
public:
	GifLZWDecompressor(int LZWSize)
		:m_lzsize(LZWSize)
		, m_ptr(nullptr)
	{

	}

	
};

class GifHelper
{
private:
	int m_width;
	int m_height;
	int m_frames;
	int m_fps;
	uint8_t** m_ptr;
	sf::FloatRect* m_rect;

public:
	GifHelper();
	~GifHelper();
	int GetFPS()
	{
		return m_fps;
	}
	int GetFrameCount()
	{
		return m_frames;
	}
	const uint8_t* GetFrame(int i)
	{
		return m_ptr[i];
	}
	sf::FloatRect GetFrameRect(int i)
	{
		return m_rect[i];
	}
	int GetWidth()
	{
		return m_width;
	}
	int GetHeight()
	{
		return m_height;
	}
	bool LoadFromFile(const std::wstring& Path);
};

