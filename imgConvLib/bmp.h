#pragma once

struct BMP_Color
{
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char a;
};

struct BMP_Header
{
	unsigned short magic_stamp;
	unsigned int file_size;
	unsigned int constant;
	unsigned int data_offset;
};

struct INFO_Header
{
	unsigned int header_size;
	unsigned int width;
	unsigned int height;
	unsigned short planes;
	unsigned short bpp;
	unsigned int compression;
	unsigned int pixel;
	unsigned int XPelsPerMeter;
	unsigned int YPelsPerMeter;
	unsigned int clr_used;
	unsigned int clr_important;
};

class Bitmap
{
public:
	Bitmap();
	Bitmap(char *fileName);
	~Bitmap();

	void initial(unsigned int w = 0, unsigned int h = 0, int mode = 0);
	void release();
	int read(char *fileName);
	int write(char *fileName);

	BMP_Header bmp_header;
	INFO_Header info_header;
	unsigned int size;
	BMP_Color *pal;
	BMP_Color *data;
	unsigned char *index;

private:

};
