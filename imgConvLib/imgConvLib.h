#pragma once

#include <vector>

using namespace std;

/* imgMode - 0: png, 1: bmp, 2: tga
   tileMode - 0: 8*8, 1: 8*4, 2: 4*4, 3: 4*2, 4: 2*2, 5: 2*1, 6: 1*1
   palMode - 0: 4bit, 1: 8bit
   hvMode - 0: Raw, 1: H, 2: V, 3: HV */

struct Color
{
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char a;
	bool operator == (const Color &x) const
	{
		return r == x.r && g == x.g && b == x.b && a == x.a;
	}
	bool operator != (const Color &x) const
	{
		return r != x.r || g != x.g || b != x.b || a != x.a;
	}
};

enum TileTexFormat
{
	I4 = 0x00,
	I8 = 0x01,
	IA4 = 0x02,
	IA8 = 0x03,
	RGB565 = 0x04,
	RGB5A3 = 0x05,
	RGBA8 = 0x06,
	RGB555 = 0x07,
	CMPR = 0x0E
};

class ImageData
{
public:
	ImageData();
	ImageData(char *fileName, bool bgFlag = false);
	ImageData(char *fileName, Color bgc);
	ImageData(unsigned int w, unsigned int h, Color bgc = { 0,0,0,0 }, int format = 0);
	ImageData(const ImageData &src);
	~ImageData();

	ImageData operator = (const ImageData &src);
	bool operator == (const ImageData &src) const;
	bool operator != (const ImageData &src) const;

	int write(char *fileName);
	int read(char *fileName, bool bgFlag = false);
	void release();
	void initial();
	void initial(unsigned int w, unsigned int h, Color bgc = { 0,0,0,0 }, int format = 0);
	void setType(int format);
	Color get(unsigned int index) const;
	Color get(unsigned int x, unsigned int y) const;
	void set(Color px, unsigned int index);
	void set(Color px, unsigned int x, unsigned int y);
	void resize(unsigned int w, unsigned int h);
	void colRemove(Color col);
	ImageData flip(bool h, bool v);
	ImageData flip(unsigned int hvMode);
	ImageData clip(unsigned int x, unsigned int y, unsigned int w, unsigned int h);
	int paste(ImageData &src, unsigned int x, unsigned int y, bool cover = 1);
	Color *ptr();
	void clone(ImageData &dst) const;

	float colDev(Color a, Color b);

	unsigned int width;
	unsigned int height;
	unsigned int size;
	unsigned char type;
	unsigned char flag;
	Color bg;

private:
	int readPNG(char *fileName, bool bgFlag);
	int writePNG(char *fileName);

	int readBMP(char *fileName, bool bgFlag);
	int writeBMP(char *fileName);

	int readTGA(char *fileName, bool bgFlag);
	int writeTGA(char *fileName);

	void bgcCalc();

	Color *data;
};

// tile, map and palette
int ImageToTMP(const ImageData &image, vector<vector<unsigned char>> &tileData, vector<unsigned short> &mapData, vector<vector<Color>> &palList, Color bg,
	unsigned char tileMode = 0, unsigned char palMode = 0, unsigned int palNum = 1, unsigned int colNum = 16, unsigned int times = 1, bool bgKeep = false);
int TMPToImage(ImageData &image, const vector<vector<unsigned char>> &tileData, const vector<unsigned short> &mapData, const vector<vector<Color>> &palList,
	unsigned int width, unsigned int height, unsigned char tileMode = 0, unsigned char palMode = 0, unsigned char imgMode = 0);

// index and palette
int ImageToIP(const ImageData &image, vector<unsigned char> &index, vector<Color> &palList, Color bg, unsigned char palMode = 0, unsigned int colNum = 16, bool bgKeep = false);
int IPToImage(ImageData &image, const vector<unsigned char> &index, const vector<Color> &palList,
	unsigned int width, unsigned int height, unsigned char palMode = 0, unsigned char imgMode = 0);

// texture
int ImageToTex(const ImageData &image, vector<unsigned char> &texData, TileTexFormat texMode, unsigned char tileMode = 0, bool swizzle = false, unsigned char endian = 0);
int TexToImage(ImageData &image, const vector<unsigned char> &texData, TileTexFormat texMode, unsigned int width, unsigned int height,
	unsigned char tileMode = 0, bool swizzle = false, unsigned char endian = 0);
int TextureCov(const vector<unsigned char> &src, vector<unsigned char> &dst, TileTexFormat srcMode, TileTexFormat dstMode, unsigned int width, unsigned int height,
	unsigned char srcTileMode, unsigned char dstTileMode, bool srcSwizzle = false, bool dstSwizzle = false, unsigned char srcEndian = 0, unsigned char dstEndian = 0);

// palette
int getBPP(TileTexFormat type);
vector<vector<Color>> ByteToPal(const vector<unsigned char> &srcCol, TileTexFormat colMode, int colNum, int palNum, bool endian=false, bool rev = false);
vector<unsigned char> PalToByte(const vector<vector<Color>> &srcCol, TileTexFormat colMode, bool endian = false, bool rev = false);
int PalMatch(const ImageData &image, const vector<vector<Color>> &palList, vector<unsigned char> &dst, unsigned char palMode, unsigned int def = 0);

// tile
vector<vector<unsigned char>> TileConv(const vector<vector<unsigned char>> &src, unsigned int srcTileCol, 
	unsigned char srcMode, unsigned char dstMode, unsigned char palMode, unsigned char hvMode = 0);
int TileMatch(const vector<vector<unsigned char>> &src, const vector<vector<unsigned char>> &tiles, unsigned int align = 0);
