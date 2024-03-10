#include <stdio.h>
#include <stdlib.h>
#include "bmp.h"

#pragma warning(disable : 4996)

Bitmap::Bitmap()
{
	initial();
}

Bitmap::Bitmap(char *fileName)
{
	initial();
	read(fileName);
}

Bitmap::~Bitmap()
{
	release();
}

void Bitmap::initial(unsigned int w, unsigned int h, int mode)
{
	// mode - 0:24bitmap 1:16pal 2:256pal
	bmp_header.magic_stamp = 0x4D42;
	bmp_header.constant = 0;
	info_header.header_size = 0x28;
	info_header.width = w;
	info_header.height = h;
	info_header.planes = 1;
	info_header.compression = 0;
	info_header.XPelsPerMeter = 0;
	info_header.YPelsPerMeter = 0;
	info_header.clr_used = 0;
	info_header.clr_important = 0;
	size = w * h;
	if (mode == 1)
	{
		bmp_header.file_size = 0x76;
		bmp_header.data_offset = 0x76;
		info_header.bpp = 4;
	}
	else if (mode == 2)
	{
		bmp_header.file_size = 0x436;
		bmp_header.data_offset = 0x436;
		info_header.bpp = 8;
	}
	else
	{
		bmp_header.file_size = 0x36;
		bmp_header.data_offset = 0x36;
		info_header.bpp = 24;
	}
	info_header.pixel = w * h * info_header.bpp / 8;
	bmp_header.file_size += info_header.pixel;
	pal = NULL;
	data = NULL;
	index = NULL;
}

void Bitmap::release()
{
	if (pal != NULL) free(pal);
	if (data != NULL) free(data);
	if (index != NULL) free(index);
	initial();
}

int Bitmap::read(char *fileName)
{
	FILE *bmp = fopen(fileName, "rb");
	if (bmp == NULL) return -1;
	fseek(bmp, 0, 2);
	if (ftell(bmp) <= 0x36) { fclose(bmp); return -1; }
	rewind(bmp);

	fread(&bmp_header.magic_stamp, 2, 1, bmp);
	fread(&bmp_header.file_size, 4, 1, bmp);
	fread(&bmp_header.constant, 4, 1, bmp);
	fread(&bmp_header.data_offset, 4, 1, bmp);
	fread(&info_header.header_size, 4, 1, bmp);
	fread(&info_header.width, 4, 1, bmp);
	fread(&info_header.height, 4, 1, bmp);
	fread(&info_header.planes, 2, 1, bmp);
	fread(&info_header.bpp, 2, 1, bmp);
	fread(&info_header.compression, 4, 1, bmp);
	fread(&info_header.pixel, 4, 1, bmp);
	fread(&info_header.XPelsPerMeter, 4, 1, bmp);
	fread(&info_header.YPelsPerMeter, 4, 1, bmp);
	fread(&info_header.clr_used, 4, 1, bmp);
	fread(&info_header.clr_important, 4, 1, bmp);

	info_header.pixel = info_header.width * info_header.height * info_header.bpp / 8;
	if(info_header.compression != 0) { fclose(bmp); return -2; }
	size = info_header.width * info_header.height;
	data = (BMP_Color *)malloc(size * sizeof(BMP_Color));
	if (info_header.bpp == 4)
	{
		index = (unsigned char *)malloc(size);
		pal = (BMP_Color *)malloc(16 * sizeof(BMP_Color));
		for (int i = 0; i < 16; i++)
		{
			pal[i].b = getc(bmp);
			pal[i].g = getc(bmp);
			pal[i].r = getc(bmp);
			pal[i].a = getc(bmp);
			pal[i].a = 0xFF;
		}
		fseek(bmp, bmp_header.data_offset, 0);
		for (unsigned int i = 0; i < size; i += 2)
		{
			unsigned int pos = (info_header.height - i / info_header.width - 1) * info_header.width + i % info_header.width;
			unsigned char dp = getc(bmp);
			index[pos] = dp >> 4 & 0xF;
			index[pos + 1] = dp & 0xF;
			data[pos] = pal[index[pos]];
			data[pos + 1] = pal[index[pos + 1]];
		}
	}
	else if (info_header.bpp == 8)
	{
		index = (unsigned char *)malloc(size);
		pal = (BMP_Color *)malloc(256 * sizeof(BMP_Color));
		for (int i = 0; i < 256; i++)
		{
			pal[i].b = getc(bmp);
			pal[i].g = getc(bmp);
			pal[i].r = getc(bmp);
			pal[i].a = getc(bmp);
			pal[i].a = 0xFF;
		}
		fseek(bmp, bmp_header.data_offset, 0);
		for (unsigned int i = 0; i < size; i++)
		{
			unsigned int pos = (info_header.height - i / info_header.width - 1) * info_header.width + i % info_header.width;
			index[pos] = getc(bmp);
			data[pos] = pal[index[pos]];
		}
	}
	else if (info_header.bpp == 16)
	{
		unsigned int dp = 0;
		fseek(bmp, bmp_header.data_offset, 0);
		for (unsigned int i = 0; i < size; i++)
		{
			unsigned int pos = (info_header.height - i / info_header.width - 1) * info_header.width + i % info_header.width;
			fread(&dp, 2, 1, bmp);
			data[pos].b = (dp & 0x1F) * 255 / 31;
			data[pos].g = (dp >> 5 & 0x1F) * 255 / 31;
			data[pos].r = (dp >> 10 & 0x1F) * 255 / 31;
			data[pos].a = 0xFF;
		}
	}
	else if (info_header.bpp == 24)
	{
		fseek(bmp, bmp_header.data_offset, 0);
		for (unsigned int i = 0; i < size; i++)
		{
			unsigned int pos = (info_header.height - i / info_header.width - 1) * info_header.width + i % info_header.width;
			data[pos].b = getc(bmp);
			data[pos].g = getc(bmp);
			data[pos].r = getc(bmp);
			data[pos].a = 0xFF;
		}
	}
	else
	{
		fclose(bmp);
		release();
		return -3;
	}
	fclose(bmp);
	return 0;
}

int Bitmap::write(char *fileName)
{
	FILE *bmp = fopen(fileName, "wb");
	if (bmp == NULL) return -1;
	fwrite(&bmp_header.magic_stamp, 2, 1, bmp);
	fwrite(&bmp_header.file_size, 4, 1, bmp);
	fwrite(&bmp_header.constant, 4, 1, bmp);
	fwrite(&bmp_header.data_offset, 4, 1, bmp);
	fwrite(&info_header.header_size, 4, 1, bmp);
	fwrite(&info_header.width, 4, 1, bmp);
	fwrite(&info_header.height, 4, 1, bmp);
	fwrite(&info_header.planes, 2, 1, bmp);
	fwrite(&info_header.bpp, 2, 1, bmp);
	fwrite(&info_header.compression, 4, 1, bmp);
	fwrite(&info_header.pixel, 4, 1, bmp);
	fwrite(&info_header.XPelsPerMeter, 4, 1, bmp);
	fwrite(&info_header.YPelsPerMeter, 4, 1, bmp);
	fwrite(&info_header.clr_used, 4, 1, bmp);
	fwrite(&info_header.clr_important, 4, 1, bmp);
	if (info_header.bpp == 4)
	{
		for (int i = 0; i < 16; i++)
		{
			putc(pal[i].b, bmp);
			putc(pal[i].g, bmp);
			putc(pal[i].r, bmp);
			putc(0x00, bmp);
		}
		for (unsigned int i = 0; i < size; i += 2)
		{
			unsigned int pos = (info_header.height - i / info_header.width - 1) * info_header.width + i % info_header.width;
			unsigned char dp = (index[pos] << 4 | index[pos + 1]) & 0xFF;
			putc(dp, bmp);
		}
	}
	else if (info_header.bpp == 8)
	{
		for (int i = 0; i < 256; i++)
		{
			putc(pal[i].b, bmp);
			putc(pal[i].g, bmp);
			putc(pal[i].r, bmp);
			putc(0x00, bmp);
		}
		for (unsigned int i = 0; i < size; i++)
		{
			unsigned int pos = (info_header.height - i / info_header.width - 1) * info_header.width + i % info_header.width;
			putc(index[pos], bmp);
		}
	}
	else if (info_header.bpp == 24)
	{
		for (unsigned int i = 0; i < size; i++)
		{
			unsigned int pos = (info_header.height - i / info_header.width - 1) * info_header.width + i % info_header.width;
			putc(data[pos].b, bmp);
			putc(data[pos].g, bmp);
			putc(data[pos].r, bmp);
		}
	}
	else { fclose(bmp); return -3; }
	fclose(bmp);
	return 0;
}
