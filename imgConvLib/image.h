#pragma once

#include <png.h>
#include <Compressonator.h>
#include "bmp.h"
#include "imgConvLib.h"

class ImageCalc
{
public:
	ImageCalc();
	~ImageCalc();

	float gamma(float x);
	float srgb(float x);
	unsigned char rgb2gray(Color rgb);
	void rgb2lab(Color rgb, float *lab);
	double deg2Rad(double deg);
	float colDev(Color a, Color b);
	Color byte2col(unsigned int srcCol, TileTexFormat srcType, bool rev = false);
	unsigned int col2byte(Color srcCol, TileTexFormat dstType, bool rev = false);

private:

};

class TileTexCov
{
public:
	TileTexCov();
	~TileTexCov();

	int tileCov(const vector<unsigned char> &src, vector<unsigned char> &dst, unsigned int width, unsigned int height,
		unsigned char srcTileMode, unsigned char dstTileMode, bool srcSwizzle = false, bool dstSwizzle = false);
	int texCov(const vector<unsigned char> &src, vector<unsigned char> &dst, TileTexFormat srcMode, TileTexFormat dstMode,
		unsigned int width, unsigned int height, unsigned char tileMode, bool swizzle = false, unsigned char endian = 0);

private:
	int I4ToRGBA8(const vector<unsigned char> &src, vector<unsigned char> &dst,
		unsigned int width, unsigned int height, unsigned int tileWidth, unsigned int tileHeight);
	int RGBA8ToI4(const vector<unsigned char> &src, vector<unsigned char> &dst,
		unsigned int width, unsigned int height, unsigned int tileWidth, unsigned int tileHeight);
	int I8ToRGBA8(const vector<unsigned char> &src, vector<unsigned char> &dst,
		unsigned int width, unsigned int height, unsigned int tileWidth, unsigned int tileHeight);
	int RGBA8ToI8(const vector<unsigned char> &src, vector<unsigned char> &dst,
		unsigned int width, unsigned int height, unsigned int tileWidth, unsigned int tileHeight);
	int IA4ToRGBA8(const vector<unsigned char> &src, vector<unsigned char> &dst,
		unsigned int width, unsigned int height, unsigned int tileWidth, unsigned int tileHeight);
	int RGBA8ToIA4(const vector<unsigned char> &src, vector<unsigned char> &dst,
		unsigned int width, unsigned int height, unsigned int tileWidth, unsigned int tileHeight);
	int IA8ToRGBA8(const vector<unsigned char> &src, vector<unsigned char> &dst,
		unsigned int width, unsigned int height, unsigned int tileWidth, unsigned int tileHeight);
	int RGBA8ToIA8(const vector<unsigned char> &src, vector<unsigned char> &dst,
		unsigned int width, unsigned int height, unsigned int tileWidth, unsigned int tileHeight);
	int RGB565ToRGBA8(const vector<unsigned char> &src, vector<unsigned char> &dst,
		unsigned int width, unsigned int height, unsigned int tileWidth, unsigned int tileHeight);
	int RGBA8ToRGB565(const vector<unsigned char> &src, vector<unsigned char> &dst,
		unsigned int width, unsigned int height, unsigned int tileWidth, unsigned int tileHeight);
	int RGB5A3ToRGBA8(const vector<unsigned char> &src, vector<unsigned char> &dst,
		unsigned int width, unsigned int height, unsigned int tileWidth, unsigned int tileHeight);
	int RGBA8ToRGB5A3(const vector<unsigned char> &src, vector<unsigned char> &dst,
		unsigned int width, unsigned int height, unsigned int tileWidth, unsigned int tileHeight);
	int RGB555ToRGBA8(const vector<unsigned char> &src, vector<unsigned char> &dst,
		unsigned int width, unsigned int height, unsigned int tileWidth, unsigned int tileHeight);
	int RGBA8ToRGB555(const vector<unsigned char> &src, vector<unsigned char> &dst,
		unsigned int width, unsigned int height, unsigned int tileWidth, unsigned int tileHeight);
	int CMPRToRGBA8(const vector<unsigned char> &src, vector<unsigned char> &dst,
		unsigned int width, unsigned int height, unsigned int tileWidth, unsigned int tileHeight, unsigned char endian);
	int RGBA8ToCMPR(const vector<unsigned char> &src, vector<unsigned char> &dst,
		unsigned int width, unsigned int height, unsigned int tileWidth, unsigned int tileHeight, unsigned char endian);
	int TileLowToHigh(vector<unsigned char> &data, TileTexFormat mode);
};
