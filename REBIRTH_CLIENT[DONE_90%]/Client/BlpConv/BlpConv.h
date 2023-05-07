#pragma once

#include <vector>


namespace image {
	struct rgba
	{
		uint8_t b;
		uint8_t g;
		uint8_t r;
		uint8_t a;
	};
	typedef std::vector<uint8_t> buffer;
	typedef std::vector<rgba> pixels;

	namespace blp {
		bool writeblp(const pixels& input, buffer& output, int Width, int Height, int Quality);
		bool __stdcall readblp( const uint8_t * _input, int inputlen, int * _output, unsigned int* Width, unsigned int* Height );
	}
	namespace bmp {
		bool write(const pixels& input, buffer& output, unsigned int width, unsigned int height, unsigned int bitcount = 32);
		bool read(const buffer& input, pixels& output, unsigned int& width, unsigned int& height);
	}
	namespace jpeg {
		bool write(const pixels& input, buffer& output, int Width, int Height, int Quality);
		bool read(const buffer& input, pixels& output, unsigned int Width, unsigned int Height);
	}
}
