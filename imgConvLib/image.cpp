#include "image.h"

#pragma warning(disable : 4996)

using namespace std;

ImageData::ImageData()
{
	initial();
}

ImageData::ImageData(char *fileName, bool bgFlag)
{
	initial();
	read(fileName, bgFlag);
}

ImageData::ImageData(char *fileName, Color bgc)
{
	initial();
	bg = bgc;
	read(fileName, false);
}

ImageData::ImageData(const ImageData &src)
{
	initial(src.width, src.height, src.bg, src.type);
	flag = src.flag;
	for (unsigned int i = 0; i < size; i++) set(src.data[i], i);
}

ImageData::ImageData(unsigned int w, unsigned int h, Color bgc, int format)
{
	initial(w, h, bgc, format);
}

ImageData::~ImageData()
{
	release();
}

void ImageData::release()
{
	if (data != NULL) free(data);
	initial();
}

void ImageData::setType(int format)
{
	type = format;
}

void ImageData::initial()
{
	width = 0;
	height = 0;
	size = 0;
	type = 0;
	flag = 0;
	bg = { 0,0,0,0 };
	data = NULL;
}

void ImageData::initial(unsigned int w, unsigned int h, Color bgc, int format)
{
	width = w;
	height = h;
	size = w * h;
	bg = bgc;
	type = format;
	data = (Color *)malloc(size * sizeof(Color));
	for (unsigned int i = 0; i < size; i++) data[i] = bg;
}

int ImageData::read(char *fileName, bool bgFlag)
{
	FILE* img = fopen(fileName, "rb");
	if (img == NULL) return -1;
	unsigned int header;
	fread(&header, 4, 1, img);
	fclose(img);
	int res = -1;
	if (header == 0x474E5089) res = readPNG(fileName, bgFlag);
	else if ((header & 0xFFFF) == 0x4D42) res = readBMP(fileName, bgFlag);
	else if (strlen(fileName) > 3)
	{
		unsigned int len = strlen(fileName);
		if (fileName[len - 3] == 't' && fileName[len - 2] == 'g' && fileName[len - 1] == 'a') res = readTGA(fileName, bgFlag);
	}
	if (res < 0) release();
	return res;
}

int ImageData::write(char *fileName)
{
	if (data == NULL || size == 0) return -1;
	if (type == 0) return writePNG(fileName);
	if (type == 1) return writeBMP(fileName);
	if (type == 2) return writeTGA(fileName);
	return -1;
}

Color ImageData::get(unsigned int index) const
{
	return data[index];
}

Color ImageData::get(unsigned int x, unsigned int y) const
{
	return data[y * width + x];
}

void ImageData::set(Color px, unsigned int index)
{
	if (index >= size) return;
	data[index] = px;
}

void ImageData::set(Color px, unsigned int x, unsigned int y)
{
	if (x >= width || y >= height) return;
	data[y * width + x] = px;
}

void ImageData::resize(unsigned int w, unsigned int h)
{
	if (w == 0 || h == 0) return;
	if (w == width && h == height) return;
	unsigned int w_ori = width;
	unsigned int w_min = width;
	if (w_min > w) w_min = w;
	unsigned int h_ori = height;
	unsigned int h_min = height;
	if (h_min > h) h_min = h;
	Color *data_ori = data;
	width = w;
	height = h;
	size = w * h;
	data = (Color *)malloc(size * sizeof(Color));
	for (unsigned int i = 0; i < size; i++) data[i] = bg;
	for (unsigned int i = 0; i < h_min; i++) for (unsigned int j = 0; j < w_min; j++) data[i * width + j] = data_ori[i * w_ori + j];
	free(data_ori);
}

bool ImageData::operator == (const ImageData &src) const
{
	if (src.width != width || src.height != height) return false;
	for (unsigned int i = 0; i < size; i++) if (src.get(i, i) != get(i)) return false;
	return true;
}

bool ImageData::operator != (const ImageData &src) const
{
	if (src.width != width || src.height != height) return true;
	for (unsigned int i = 0; i < size; i++) if (src.get(i, i) != get(i)) return true;
	return false;
}

Color *ImageData::ptr()
{
	return data;
}

void ImageData::bgcCalc()
{
	vector<Color> colRec;
	vector<unsigned int> stat;
	for (unsigned int i = 0; i < size; i++)
	{
		bool find = false;
		for (size_t j = 0; j < colRec.size(); j++)
		{
			if (data[i] == colRec[j])
			{
				find = true;
				stat[j]++;
				break;
			}
		}
		if (find == false)
		{
			colRec.push_back(data[i]);
			stat.push_back(1);
		}
	}
	int index = 0;
	for (size_t j = 0; j < colRec.size(); j++) if (stat[j] > stat[index]) index = j;
	bg = colRec[index];
	vector<Color>().swap(colRec);
	vector<unsigned int>().swap(stat);
}

void ImageData::clone(ImageData &dst) const
{
	dst.initial(width, height, bg, type);
	dst.flag = flag;
	for (unsigned int i = 0; i < size; i++) dst.set(data[i], i);
}

ImageData ImageData::operator = (const ImageData &src)
{
	initial(src.width, src.height, src.bg, src.type);
	flag = src.flag;
	for (unsigned int i = 0; i < size; i++) set(src.data[i], i);
}

void ImageData::colRemove(Color col)
{
	vector<Color> colRec;
	vector<unsigned int> stat;
	for (unsigned int i = 0; i < size; i++)
	{
		if (data[i] == col) continue;
		bool find = false;
		for (size_t j = 0; j < colRec.size(); j++)
		{
			if (data[i] == colRec[j])
			{
				find = true;
				stat[j]++;
				break;
			}
		}
		if (find == false)
		{
			colRec.push_back(data[i]);
			stat.push_back(1);
		}
	}
	if (colRec.size() == 0) return;
	int index = 0;
	for (size_t j = 0; j < colRec.size(); j++) if (stat[j] > stat[index]) index = j;
	for (unsigned int i = 0; i < size; i++) if (data[i] == col) data[i] = colRec[index];
	vector<Color>().swap(colRec);
	vector<unsigned int>().swap(stat);
}

ImageData ImageData::flip(bool h, bool v)
{
	ImageData res(width, height, bg, type);
	unsigned int dstX, dstY;
	for (unsigned int i = 0; i < width; i++)
	{
		dstX = h ? width - i - 1 : i;
		for (unsigned int j = 0; j < height; j++)
		{
			dstY = v ? height - j - 1 : j;
			res.set(get(i, j), dstX, dstY);
		}
	}
	return res;
}

ImageData ImageData::flip(unsigned int hvMode)
{
	bool h = hvMode & 1, v = (hvMode >> 1) & 1;
	ImageData res(width, height, bg, type);
	unsigned int dstX, dstY;
	for (unsigned int i = 0; i < width; i++)
	{
		dstX = h ? width - i - 1 : i;
		for (unsigned int j = 0; j < height; j++)
		{
			dstY = v ? height - j - 1 : j;
			res.set(get(i, j), dstX, dstY);
		}
	}
	return res;
}

ImageData ImageData::clip(unsigned int x, unsigned int y, unsigned int w, unsigned int h)
{
	ImageData res(w, h, bg, type);
	if (res.size == 0 || x + w > width || y + h > height) return res;
	for (unsigned int i = 0; i < w; i++) for (unsigned int j = 0; j < h; j++) res.set(get(x + i, y + j), i, j);
	return res;
}

int ImageData::paste(ImageData &src, unsigned int x, unsigned int y, bool cover)
{
	if (x + src.width > width || y + src.height > height) return -1;
	if (cover == 0)
	{
		for (unsigned int i = 0; i < src.width; i++) for (unsigned int j = 0; j < src.height; j++)
		{
			Color col = get(i + x, j + y);
			if (col != src.get(i, j) && col != bg && src.get(i, j) != bg) return 1;
		}
	}
	for (unsigned int i = 0; i < src.width; i++) for (unsigned int j = 0; j < src.height; j++) if (src.get(i, j) != bg) set(src.get(i, j), i + x, j + y);
	return 0;
}

int ImageData::readPNG(char *fileName, bool bgFlag)
{
	FILE *pic_fp = fopen(fileName, "rb");
	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
	png_infop info_ptr = png_create_info_struct(png_ptr);
	setjmp(png_jmpbuf(png_ptr));
	png_init_io(png_ptr, pic_fp);
	png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_EXPAND, 0);
	int channels = png_get_channels(png_ptr, info_ptr);
	int bit_depth = png_get_bit_depth(png_ptr, info_ptr);
	int color_type = png_get_color_type(png_ptr, info_ptr);

	type = 0;
	width = png_get_image_width(png_ptr, info_ptr);
	height = png_get_image_height(png_ptr, info_ptr);
	size = width * height;
	if (data != NULL) free(data);
	data = (Color *)malloc(size * sizeof(Color));

	unsigned int i, j, temp, pos = 0;
	png_bytep *row_pointers = png_get_rows(png_ptr, info_ptr);
	if (channels == 4 || color_type == PNG_COLOR_TYPE_RGB_ALPHA)
	{
		temp = 4 * width;
		for (i = 0; i < height; i++)
		{
			for (j = 0; j < temp; j += 4)
			{
				data[pos].r = row_pointers[i][j];
				data[pos].g = row_pointers[i][j + 1];
				data[pos].b = row_pointers[i][j + 2];
				data[pos].a = row_pointers[i][j + 3];
				++pos;
			}
		}
	}
	else if (channels == 3 || color_type == PNG_COLOR_TYPE_RGB)
	{
		temp = 3 * width;
		for (i = 0; i < height; i++)
		{
			for (j = 0; j < temp; j += 3)
			{
				data[pos].r = row_pointers[i][j];
				data[pos].g = row_pointers[i][j + 1];
				data[pos].b = row_pointers[i][j + 2];
				data[pos].a = 0xFF;
				++pos;
			}
		}
	}
	else return -1;

	if (bgFlag) bgcCalc();

	png_destroy_read_struct(&png_ptr, &info_ptr, 0);
	fclose(pic_fp);
	return 0;
}

int ImageData::writePNG(char *fileName)
{
	FILE *fp = fopen(fileName, "wb");
	if (!fp) return -1;

	png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	png_infop info_ptr = png_create_info_struct(png_ptr);
	setjmp(png_jmpbuf(png_ptr));
	png_init_io(png_ptr, fp);
	setjmp(png_jmpbuf(png_ptr));
	png_byte color_type = PNG_COLOR_TYPE_RGB_ALPHA;
	png_set_IHDR(png_ptr, info_ptr, width, height, 8, color_type, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
	png_write_info(png_ptr, info_ptr);

	setjmp(png_jmpbuf(png_ptr));
	unsigned int temp = 4 * width;
	unsigned int pos = 0;
	png_bytep *row_pointers = (png_bytep*) new char[height * sizeof(png_bytep)];
	for (unsigned int i = 0; i < height; i++)
	{
		row_pointers[i] = (png_bytep) new char[temp * sizeof(unsigned char)];
		for (unsigned int j = 0; j < temp; j += 4)
		{
			row_pointers[i][j] = data[pos].r;
			row_pointers[i][j + 1] = data[pos].g;
			row_pointers[i][j + 2] = data[pos].b;
			row_pointers[i][j + 3] = data[pos].a;
			++pos;
		}
	}
	png_write_image(png_ptr, row_pointers);

	setjmp(png_jmpbuf(png_ptr));
	png_write_end(png_ptr, NULL);

	for (unsigned int i = 0; i < height; i++) delete[]row_pointers[i];
	delete[]row_pointers;
	fclose(fp);
	return 0;
}

int ImageData::readBMP(char *fileName, bool bgFlag)
{
	Bitmap bmp(fileName);
	if (bmp.size == 0) return -1;

	type = 1;
	width = bmp.info_header.width;
	height = bmp.info_header.height;
	size = width * height;
	if (data != NULL) free(data);
	data = (Color *)malloc(size * sizeof(Color));
	for (unsigned int i = 0; i < size; i++)
	{
		data[i].r = bmp.data[i].r;
		data[i].g = bmp.data[i].g;
		data[i].b = bmp.data[i].b;
		data[i].a = bmp.data[i].a;
	}

	if (bgFlag)
	{
		if (bmp.info_header.bpp == 4 || bmp.info_header.bpp == 8)
		{
			bg.r = bmp.pal[0].r;
			bg.g = bmp.pal[0].g;
			bg.b = bmp.pal[0].b;
			bg.a = bmp.pal[0].a;
		}
		else bgcCalc();
	}
	bmp.release();
	return 0;
}

int ImageData::writeBMP(char *fileName)
{
	Bitmap bmp;
	bmp.initial(width, height);
	bmp.data = (BMP_Color *)malloc(size * sizeof(BMP_Color));
	for (unsigned int i = 0; i < size; i++)
	{
		bmp.data[i].r = data[i].r;
		bmp.data[i].g = data[i].g;
		bmp.data[i].b = data[i].b;
		bmp.data[i].a = data[i].a;
	}
	int result = bmp.write(fileName);
	bmp.release();
	return result;
}

int ImageData::readTGA(char *fileName, bool bgFlag)
{
	FILE *pic_fp = fopen(fileName, "rb");
	if (!pic_fp) return -1;
	unsigned char imgInfoSize = getc(pic_fp);
	unsigned char palType = getc(pic_fp);
	unsigned char imgType = getc(pic_fp);
	if (palType != 0 || imgType != 2)
	{
		fclose(pic_fp);
		return -1;
	}
	for (unsigned int t = 0; t < 9; t++) if(getc(pic_fp) != 0)
	{
		fclose(pic_fp);
		return -1;
	}
	type = 2;
	width = 0;
	fread(&width, 2, 1, pic_fp);
	height = 0;
	fread(&height, 2, 1, pic_fp);
	size = width * height;
	if (data != NULL) free(data);
	data = (Color *)malloc(size * sizeof(Color));
	unsigned char bit_depth = getc(pic_fp);
	unsigned char col_alpha = getc(pic_fp);
	unsigned char start = col_alpha >> 5;
	col_alpha &= 0xF;
	if(start > 1)
	{
		fclose(pic_fp);
		return -1;
	}
	fseek(pic_fp, imgInfoSize, 1);
	unsigned int i, j, px_data, pos;
	if (bit_depth == 16)
	{
		for (i = 0; i < height; i++)
		{
			pos = start == 1 ? i * width : (height - i - 1) * width;
			for (j = 0; j < width; j++)
			{
				px_data = 0;
				fread(&px_data, 2, 1, pic_fp);
				data[pos].r = px_data & 0x1F;
				data[pos].g = px_data >> 5 & 0x1F;
				data[pos].b = px_data >> 10 & 0x1F;
				if (col_alpha == 0 || px_data >> 15 == 1) data[pos].a = 0xFF; else data[pos].a = 0;
				pos++;
			}
		}
	}
	else if (bit_depth == 24)
	{
		for (i = 0; i < height; i++)
		{
			pos = start == 1 ? i * width : (height - i - 1) * width;
			for (j = 0; j < width; j++)
			{
				data[pos].b = getc(pic_fp);
				data[pos].g = getc(pic_fp);
				data[pos].r = getc(pic_fp);
				data[pos].a = 0xFF;
				pos++;
			}
		}
	}
	else if (bit_depth == 32)
	{
		for (i = 0; i < height; i++)
		{
			pos = start == 1 ? i * width : (height - i - 1) * width;
			for (j = 0; j < width; j++)
			{
				data[pos].b = getc(pic_fp);
				data[pos].g = getc(pic_fp);
				data[pos].r = getc(pic_fp);
				data[pos].a = getc(pic_fp);
				pos++;
			}
		}
	}
	else
	{
		fclose(pic_fp);
		return -1;
	}
	if (bgFlag) bgcCalc();
	fclose(pic_fp);
	return 0;
}

int ImageData::writeTGA(char *fileName)
{
	FILE *fp = fopen(fileName, "wb");
	if (!fp) return -1;
	putc(0, fp);
	putc(0, fp);
	putc(2, fp);
	for (unsigned int t = 0; t < 9; t++) putc(0, fp);
	fwrite(&width, 2, 1, fp);
	fwrite(&height, 2, 1, fp);
	putc(32, fp);
	putc(8, fp);
	unsigned int i, j, pos;
	for (i = 0; i < height; i++)
	{
		pos = (height - i - 1) * width;
		for (j = 0; j < width; j++)
		{
			putc(data[pos].b, fp);
			putc(data[pos].g, fp);
			putc(data[pos].r, fp);
			putc(data[pos].a, fp);
			pos++;
		}
	}
	fclose(fp);
	return 0;
}

float ImageData::colDev(Color a, Color b)
{
	ImageCalc ic;
	return ic.colDev(a, b);
}

ImageCalc::ImageCalc()
{

}

ImageCalc::~ImageCalc()
{

}

float ImageCalc::colDev(Color a, Color b)
{
	float labA[3], labB[3];
	rgb2lab(a, labA);
	rgb2lab(b, labB);

	double k_L = 1.0, k_C = 1.0, k_H = 1.0;
	double deg360InRad = deg2Rad(360.0);
	double deg180InRad = deg2Rad(180.0);
	double pow25To7 = 6103515625.0;

	double C1 = sqrt((labA[1] * labA[1]) + (labA[2] * labA[2]));
	double C2 = sqrt((labB[1] * labB[1]) + (labB[2] * labB[2]));
	double barC = (C1 + C2) / 2.0;
	double G = 0.5 * (1 - sqrt(pow(barC, 7) / (pow(barC, 7) + pow25To7)));
	double a1Prime = (1.0 + G) * labA[1];
	double a2Prime = (1.0 + G) * labB[1];
	double CPrime1 = sqrt((a1Prime * a1Prime) + (labA[2] * labA[2]));
	double CPrime2 = sqrt((a2Prime * a2Prime) + (labB[2] * labB[2]));
	double hPrime1;
	if (labA[2] == 0 && a1Prime == 0) hPrime1 = 0.0;
	else
	{
		hPrime1 = atan2(labA[2], a1Prime);
		if (hPrime1 < 0) hPrime1 += deg360InRad;
	}
	double hPrime2;
	if (labB[2] == 0 && a2Prime == 0) hPrime2 = 0.0;
	else
	{
		hPrime2 = atan2(labB[2], a2Prime);
		if (hPrime2 < 0) hPrime2 += deg360InRad;
	}

	double deltaLPrime = labB[0] - labA[0];
	double deltaCPrime = CPrime2 - CPrime1;
	double deltahPrime;
	double CPrimeProduct = CPrime1 * CPrime2;
	if (CPrimeProduct == 0) deltahPrime = 0;
	else
	{
		deltahPrime = hPrime2 - hPrime1;
		if (deltahPrime < -deg180InRad) deltahPrime += deg360InRad;
		else if (deltahPrime > deg180InRad) deltahPrime -= deg360InRad;
	}
	double deltaHPrime = 2.0 * sqrt(CPrimeProduct) * sin(deltahPrime / 2.0);

	double barLPrime = (labA[0] + labB[0]) / 2.0;
	double barCPrime = (CPrime1 + CPrime2) / 2.0;
	double barhPrime, hPrimeSum = hPrime1 + hPrime2;
	if (CPrime1 * CPrime2 == 0)barhPrime = hPrimeSum;
	else
	{
		if (fabs(hPrime1 - hPrime2) <= deg180InRad) barhPrime = hPrimeSum / 2.0;
		else
		{
			if (hPrimeSum < deg360InRad) barhPrime = (hPrimeSum + deg360InRad) / 2.0;
			else barhPrime = (hPrimeSum - deg360InRad) / 2.0;
		}
	}
	double T = 1.0 - (0.17 * cos(barhPrime - deg2Rad(30.0))) + (0.24 * cos(2.0 * barhPrime)) +
		(0.32 * cos((3.0 * barhPrime) + deg2Rad(6.0))) - (0.20 * cos((4.0 * barhPrime) - deg2Rad(63.0)));
	double deltaTheta = deg2Rad(30.0) * exp(-pow((barhPrime - deg2Rad(275.0)) / deg2Rad(25.0), 2.0));
	double R_C = 2.0 * sqrt(pow(barCPrime, 7.0) / (pow(barCPrime, 7.0) + pow25To7));
	double S_L = 1 + ((0.015 * pow(barLPrime - 50.0, 2.0)) / sqrt(20 + pow(barLPrime - 50.0, 2.0)));
	double S_C = 1 + (0.045 * barCPrime);
	double S_H = 1 + (0.015 * barCPrime * T);
	double R_T = (-sin(2.0 * deltaTheta)) * R_C;

	double deltaE = sqrt(pow(deltaLPrime / (k_L * S_L), 2.0) + pow(deltaCPrime / (k_C * S_C), 2.0) +
		pow(deltaHPrime / (k_H * S_H), 2.0) + (R_T * (deltaCPrime / (k_C * S_C)) * (deltaHPrime / (k_H * S_H))));
	return (float)deltaE;
}

float ImageCalc::gamma(float x)
{
	return x > 0.04045f ? powf((x + 0.055f) / 1.055f, 2.4f) : (x / 12.92f);
}

float ImageCalc::srgb(float x)
{
	return x > 0.0031308f ? powf(x, 1 / 2.4f) * 1.055f - 0.055f : (x * 12.92f);
}

unsigned char ImageCalc::rgb2gray(Color rgb)
{
	float gray = 0.2126f * gamma(rgb.r / 255.0f) + 0.7152f * gamma(rgb.g / 255.0f) + 0.0722f * gamma(rgb.b / 255.0f);
	return unsigned char(round(srgb(gray) * 255.0f));
}

void ImageCalc::rgb2lab(Color rgb, float *lab)
{
	float RR = gamma(rgb.r / 255.0f);
	float GG = gamma(rgb.g / 255.0f);
	float BB = gamma(rgb.b / 255.0f);
	float X = (0.4124564f * RR + 0.3575761f * GG + 0.1804375f * BB) / 0.950456f;
	float Y = (0.2126729f * RR + 0.7151522f * GG + 0.0721750f * BB) / 1.0f;
	float Z = (0.0193339f * RR + 0.1191920f * GG + 0.9503041f * BB) / 1.088754f;
	float fX = X > 0.008856f ? pow(X, 1.0f / 3.0f) : 7.787f * X + 16.0f / 116.0f;
	float fY = Y > 0.008856f ? pow(Y, 1.0f / 3.0f) : 7.787f * Y + 16.0f / 116.0f;
	float fZ = Z > 0.008856f ? pow(Z, 1.0f / 3.0f) : 7.787f * Z + 16.0f / 116.0f;

	lab[0] = 116.0f * fY - 16.0f;
	lab[0] = lab[0] > 0.0f ? lab[0] : 0.0f;
	lab[1] = 500.0f * (fX - fY);
	lab[2] = 200.0f * (fY - fZ);
}

double ImageCalc::deg2Rad(double deg)
{
	return (deg * (3.14159265358979323846 / 180.0));
}

Color ImageCalc::byte2col(unsigned int srcCol, TileTexFormat srcType, bool rev)
{
	Color col;
	switch (srcType)
	{
	case RGBA8:
		col.a = (srcCol >> 24) & 0xFF;
		col.r = (srcCol >> 16) & 0xFF;
		col.g = (srcCol >> 8) & 0xFF;
		col.b = srcCol & 0xFF;
		break;
	case RGB565:
		col.a = 0xFF;
		col.r = (((srcCol >> 11) & 0x1F) * 0x08) & 0xFF;
		col.g = (((srcCol >> 5) & 0x3F) * 0x04) & 0xFF;
		col.b = ((srcCol & 0x1F) * 0x08) & 0xFF;
		break;
	case RGB5A3:
		if (srcCol < 0x8000)
		{
			col.a = (((srcCol >> 12) & 0x07) * 0x20) & 0xFF;
			col.r = (((srcCol >> 8) & 0x0F) * 0x11) & 0xFF;
			col.g = (((srcCol >> 4) & 0x0F) * 0x11) & 0xFF;
			col.b = ((srcCol & 0x0F) * 0x11) & 0xFF;
		}
		else
		{
			col.a = 0xFF;
			col.r = (((srcCol >> 10) & 0x1F) * 0x08) & 0xFF;
			col.g = (((srcCol >> 5) & 0x1F) * 0x08) & 0xFF;
			col.b = ((srcCol & 0x1F) * 0x08) & 0xFF;
		}
		break;
	case RGB555:
		col.a = 0xFF;
		col.r = (((srcCol >> 10) & 0x1F) * 0x08) & 0xFF;
		col.g = (((srcCol >> 5) & 0x1F) * 0x08) & 0xFF;
		col.b = ((srcCol & 0x1F) * 0x08) & 0xFF;
		break;
	case I4:
		col.a = 0xFF;
		col.r = (srcCol & 0x0F) * 0x11;
		col.g = col.r;
		col.b = col.r;
		break;
	case I8:
		col.a = 0xFF;
		col.r = srcCol & 0xFF;
		col.g = col.r;
		col.b = col.r;
		break;
	case IA4:
		col.a = ((srcCol & 0xF0) >> 4) * 0x11;
		col.r = (srcCol & 0x0F) * 0x11;
		col.g = col.r;
		col.b = col.r;
		break;
	case IA8:
		col.a = (srcCol >> 8) & 0xFF;
		col.r = srcCol & 0xFF;
		col.g = col.r;
		col.b = col.r;
		break;
	default: col = { 0 ,0 ,0 ,0 };
	};
	if (rev) col = { col.b, col.g, col.r, col.a };
	return col;
}

unsigned int ImageCalc::col2byte(Color srcCol, TileTexFormat dstType, bool rev)
{
	unsigned int col;
	if (rev) srcCol = { srcCol.b, srcCol.g, srcCol.r, srcCol.a };
	switch (dstType)
	{
	case RGBA8:
		col = (srcCol.a << 24) | (srcCol.r << 16) | (srcCol.g << 8) | srcCol.b;
		break;
	case RGB565:
		col = (((srcCol.r / 0x08) & 0x1F) << 11) | (((srcCol.g / 0x04) & 0x3F) << 5) | (srcCol.b / 0x08) & 0x1F;
		break;
	case RGB555:
		col = (((srcCol.r / 0x08) & 0x1F) << 10) | (((srcCol.g / 0x08) & 0x1F) << 5) | ((srcCol.b / 0x08) & 0x1F);
		break;
	case RGB5A3:
		if (srcCol.a == 0xFF) col = 0x8000 | (((srcCol.r / 0x08) & 0x1F) << 10) | (((srcCol.g / 0x08) & 0x1F) << 5) | ((srcCol.b / 0x08) & 0x1F);
		else col = (((srcCol.a / 0x20) & 0x07) << 12) | ((srcCol.r / 0x11) << 8) | ((srcCol.g / 0x11) << 4) | (srcCol.b / 0x11);
		break;
	case I4:
		col = rgb2gray(srcCol) / 0x11;
		break;
	case I8:
		col = rgb2gray(srcCol);
		break;
	case IA4:
		col = ((srcCol.a / 0x11) << 4) | (rgb2gray(srcCol) / 0x11);
		break;
	case IA8:
		col = (srcCol.a << 8) | rgb2gray(srcCol);
		break;
	default: col = 0;
	};
	return col;
}

TileTexCov::TileTexCov()
{

}

TileTexCov::~TileTexCov()
{

}

int TileTexCov::I4ToRGBA8(const vector<unsigned char> &src, vector<unsigned char> &dst,
	unsigned int width, unsigned int height, unsigned int tileWidth, unsigned int tileHeight)
{
	unsigned int srcDepth = 4;
	unsigned int dstDepth = 32;
	unsigned int texSize = width * height;
	unsigned int tileSize = tileWidth * tileHeight;
	unsigned int srcSize = texSize * srcDepth / 8;
	unsigned int dstSize = texSize * dstDepth / 8;
	unsigned int halfSize = tileSize * 2;
	if (src.size() < srcSize) return -1;

	vector<unsigned char>().swap(dst);
	vector<unsigned char> srcTileRecord(tileSize * srcDepth / 8);
	vector<unsigned char> dstTileRecord(tileSize * dstDepth / 8);
	for (size_t i = 0; i < srcSize; i += srcTileRecord.size())
	{
		for (size_t j = 0; j < srcTileRecord.size(); j++) srcTileRecord[j] = src[i + j];
		unsigned char value = 0;
		for (size_t j = 0; j < srcTileRecord.size(); j++)
		{
			value = ((srcTileRecord[j] & 0xF0) >> 4) * 0x11;
			dstTileRecord[j * 4] = value;
			dstTileRecord[j * 4 + 1] = value;
			dstTileRecord[j * 4 + halfSize] = value;
			dstTileRecord[j * 4 + halfSize + 1] = value;
			value = (srcTileRecord[j] & 0x0F) * 0x11;
			dstTileRecord[j * 4 + 2] = value;
			dstTileRecord[j * 4 + 3] = value;
			dstTileRecord[j * 4 + halfSize + 2] = value;
			dstTileRecord[j * 4 + halfSize + 3] = value;
		}
		for (size_t j = 0; j < dstTileRecord.size(); j++) dst.push_back(dstTileRecord[j]);
	}
	vector<unsigned char>().swap(srcTileRecord);
	vector<unsigned char>().swap(dstTileRecord);
	return 0;
}

int TileTexCov::RGBA8ToI4(const vector<unsigned char> &src, vector<unsigned char> &dst,
	unsigned int width, unsigned int height, unsigned int tileWidth, unsigned int tileHeight)
{
	unsigned int srcDepth = 32;
	unsigned int dstDepth = 4;
	unsigned int texSize = width * height;
	unsigned int tileSize = tileWidth * tileHeight;
	unsigned int srcSize = texSize * srcDepth / 8;
	unsigned int dstSize = texSize * dstDepth / 8;
	unsigned int halfSize = tileSize * 2;
	if (src.size() < srcSize) return -1;

	ImageCalc ic;
	vector<unsigned char>().swap(dst);
	vector<unsigned char> srcTileRecord(tileSize * srcDepth / 8);
	vector<unsigned char> dstTileRecord(tileSize * dstDepth / 8);
	for (size_t i = 0; i < srcSize; i += srcTileRecord.size())
	{
		for (size_t j = 0; j < srcTileRecord.size(); j++) srcTileRecord[j] = src[i + j];
		unsigned char value = 0;
		for (size_t j = 0; j < dstTileRecord.size(); j++)
		{
			
			value = (ic.rgb2gray({ srcTileRecord[j * 4 + 1], srcTileRecord[j * 4 + halfSize], srcTileRecord[j * 4 + halfSize + 1], 0xFF }) * srcTileRecord[j * 4] / 0x10EF) << 4;
			value |= ic.rgb2gray({ srcTileRecord[j * 4 + 3], srcTileRecord[j * 4 + halfSize + 2], srcTileRecord[j * 4 + halfSize + 3], 0xFF }) * srcTileRecord[j * 4 + 2] / 0x10EF;
			dstTileRecord[j] = value;
		}
		for (size_t j = 0; j < dstTileRecord.size(); j++) dst.push_back(dstTileRecord[j]);
	}
	vector<unsigned char>().swap(srcTileRecord);
	vector<unsigned char>().swap(dstTileRecord);
	return 0;
}

int TileTexCov::I8ToRGBA8(const vector<unsigned char> &src, vector<unsigned char> &dst,
	unsigned int width, unsigned int height, unsigned int tileWidth, unsigned int tileHeight)
{
	unsigned int srcDepth = 8;
	unsigned int dstDepth = 32;
	unsigned int texSize = width * height;
	unsigned int tileSize = tileWidth * tileHeight;
	unsigned int srcSize = texSize * srcDepth / 8;
	unsigned int dstSize = texSize * dstDepth / 8;
	unsigned int halfSize = tileSize * 2;
	if (src.size() < srcSize) return -1;

	vector<unsigned char>().swap(dst);
	vector<unsigned char> srcTileRecord(tileSize * srcDepth / 8);
	vector<unsigned char> dstTileRecord(tileSize * dstDepth / 8);
	for (size_t i = 0; i < srcSize; i += srcTileRecord.size())
	{
		for (size_t j = 0; j < srcTileRecord.size(); j++) srcTileRecord[j] = src[i + j];
		unsigned char value = 0;
		for (size_t j = 0; j < srcTileRecord.size(); j++)
		{
			value = srcTileRecord[j];
			dstTileRecord[j * 2] = 0xFF;
			dstTileRecord[j * 2 + 1] = value;
			dstTileRecord[j * 2 + halfSize] = value;
			dstTileRecord[j * 2 + halfSize + 1] = value;
		}
		for (size_t j = 0; j < dstTileRecord.size(); j++) dst.push_back(dstTileRecord[j]);
	}
	vector<unsigned char>().swap(srcTileRecord);
	vector<unsigned char>().swap(dstTileRecord);
	return 0;
}

int TileTexCov::RGBA8ToI8(const vector<unsigned char> &src, vector<unsigned char> &dst,
	unsigned int width, unsigned int height, unsigned int tileWidth, unsigned int tileHeight)
{
	unsigned int srcDepth = 32;
	unsigned int dstDepth = 8;
	unsigned int texSize = width * height;
	unsigned int tileSize = tileWidth * tileHeight;
	unsigned int srcSize = texSize * srcDepth / 8;
	unsigned int dstSize = texSize * dstDepth / 8;
	unsigned int halfSize = tileSize * 2;
	if (src.size() < srcSize) return -1;

	ImageCalc ic;
	vector<unsigned char>().swap(dst);
	vector<unsigned char> srcTileRecord(tileSize * srcDepth / 8);
	vector<unsigned char> dstTileRecord(tileSize * dstDepth / 8);
	for (size_t i = 0; i < srcSize; i += srcTileRecord.size())
	{
		for (size_t j = 0; j < srcTileRecord.size(); j++) srcTileRecord[j] = src[i + j];
		unsigned char value = 0;
		for (size_t j = 0; j < dstTileRecord.size(); j++)
		{
			dstTileRecord[j] = ic.rgb2gray({ srcTileRecord[j * 2 + 1], srcTileRecord[j * 2 + halfSize], srcTileRecord[j * 2 + halfSize + 1], 0xFF });
		}
		for (size_t j = 0; j < dstTileRecord.size(); j++) dst.push_back(dstTileRecord[j]);
	}
	vector<unsigned char>().swap(srcTileRecord);
	vector<unsigned char>().swap(dstTileRecord);
	return 0;
}

int TileTexCov::IA4ToRGBA8(const vector<unsigned char> &src, vector<unsigned char> &dst,
	unsigned int width, unsigned int height, unsigned int tileWidth, unsigned int tileHeight)
{
	unsigned int srcDepth = 8;
	unsigned int dstDepth = 32;
	unsigned int texSize = width * height;
	unsigned int tileSize = tileWidth * tileHeight;
	unsigned int srcSize = texSize * srcDepth / 8;
	unsigned int dstSize = texSize * dstDepth / 8;
	unsigned int halfSize = tileSize * 2;
	if (src.size() < srcSize) return -1;

	vector<unsigned char>().swap(dst);
	vector<unsigned char> srcTileRecord(tileSize * srcDepth / 8);
	vector<unsigned char> dstTileRecord(tileSize * dstDepth / 8);
	for (size_t i = 0; i < srcSize; i += srcTileRecord.size())
	{
		for (size_t j = 0; j < srcTileRecord.size(); j++) srcTileRecord[j] = src[i + j];
		unsigned char value = 0;
		for (size_t j = 0; j < srcTileRecord.size(); j++)
		{
			value = ((srcTileRecord[j] & 0xF0) >> 4) * 0x11;
			dstTileRecord[j * 2] = value;
			value = (srcTileRecord[j] & 0x0F) * 0x11;
			dstTileRecord[j * 2 + 1] = value;
			dstTileRecord[j * 2 + halfSize] = value;
			dstTileRecord[j * 2 + halfSize + 1] = value;
		}
		for (size_t j = 0; j < dstTileRecord.size(); j++) dst.push_back(dstTileRecord[j]);
	}
	vector<unsigned char>().swap(srcTileRecord);
	vector<unsigned char>().swap(dstTileRecord);
	return 0;
}

int TileTexCov::RGBA8ToIA4(const vector<unsigned char> &src, vector<unsigned char> &dst,
	unsigned int width, unsigned int height, unsigned int tileWidth, unsigned int tileHeight)
{
	unsigned int srcDepth = 32;
	unsigned int dstDepth = 8;
	unsigned int texSize = width * height;
	unsigned int tileSize = tileWidth * tileHeight;
	unsigned int srcSize = texSize * srcDepth / 8;
	unsigned int dstSize = texSize * dstDepth / 8;
	unsigned int halfSize = tileSize * 2;
	if (src.size() < srcSize) return -1;

	ImageCalc ic;
	vector<unsigned char>().swap(dst);
	vector<unsigned char> srcTileRecord(tileSize * srcDepth / 8);
	vector<unsigned char> dstTileRecord(tileSize * dstDepth / 8);
	for (size_t i = 0; i < srcSize; i += srcTileRecord.size())
	{
		for (size_t j = 0; j < srcTileRecord.size(); j++) srcTileRecord[j] = src[i + j];
		unsigned char value = 0;
		for (size_t j = 0; j < dstTileRecord.size(); j++)
		{
			value = (srcTileRecord[j * 2] / 0x11) << 4;
			value |= ic.rgb2gray({ srcTileRecord[j * 2 + 1], srcTileRecord[j * 2 + halfSize], srcTileRecord[j * 2 + halfSize + 1], 0xFF }) / 0x11;
			dstTileRecord[j] = value;
		}
		for (size_t j = 0; j < dstTileRecord.size(); j++) dst.push_back(dstTileRecord[j]);
	}
	vector<unsigned char>().swap(srcTileRecord);
	vector<unsigned char>().swap(dstTileRecord);
	return 0;
}

int TileTexCov::IA8ToRGBA8(const vector<unsigned char> &src, vector<unsigned char> &dst,
	unsigned int width, unsigned int height, unsigned int tileWidth, unsigned int tileHeight)
{
	unsigned int srcDepth = 16;
	unsigned int dstDepth = 32;
	unsigned int texSize = width * height;
	unsigned int tileSize = tileWidth * tileHeight;
	unsigned int srcSize = texSize * srcDepth / 8;
	unsigned int dstSize = texSize * dstDepth / 8;
	unsigned int halfSize = tileSize * 2;
	if (src.size() < srcSize) return -1;

	vector<unsigned char>().swap(dst);
	vector<unsigned char> srcTileRecord(tileSize * srcDepth / 8);
	vector<unsigned char> dstTileRecord(tileSize * dstDepth / 8);
	for (size_t i = 0; i < srcSize; i += srcTileRecord.size())
	{
		for (size_t j = 0; j < srcTileRecord.size(); j++) srcTileRecord[j] = src[i + j];
		unsigned char value = 0;
		for (size_t j = 0; j < srcTileRecord.size(); j += 2)
		{
			value = srcTileRecord[j];
			dstTileRecord[j] = value;
			value = srcTileRecord[j + 1];
			dstTileRecord[j + 1] = value;
			dstTileRecord[j + halfSize] = value;
			dstTileRecord[j + halfSize + 1] = value;
		}
		for (size_t j = 0; j < dstTileRecord.size(); j++) dst.push_back(dstTileRecord[j]);
	}
	vector<unsigned char>().swap(srcTileRecord);
	vector<unsigned char>().swap(dstTileRecord);
	return 0;
}

int TileTexCov::RGBA8ToIA8(const vector<unsigned char> &src, vector<unsigned char> &dst,
	unsigned int width, unsigned int height, unsigned int tileWidth, unsigned int tileHeight)
{
	unsigned int srcDepth = 32;
	unsigned int dstDepth = 16;
	unsigned int texSize = width * height;
	unsigned int tileSize = tileWidth * tileHeight;
	unsigned int srcSize = texSize * srcDepth / 8;
	unsigned int dstSize = texSize * dstDepth / 8;
	unsigned int halfSize = tileSize * 2;
	if (src.size() < srcSize) return -1;

	ImageCalc ic;
	vector<unsigned char>().swap(dst);
	vector<unsigned char> srcTileRecord(tileSize * srcDepth / 8);
	vector<unsigned char> dstTileRecord(tileSize * dstDepth / 8);
	for (size_t i = 0; i < srcSize; i += srcTileRecord.size())
	{
		for (size_t j = 0; j < srcTileRecord.size(); j++) srcTileRecord[j] = src[i + j];
		unsigned char value = 0;
		for (size_t j = 0; j < dstTileRecord.size(); j += 2)
		{
			dstTileRecord[j] = srcTileRecord[j];
			dstTileRecord[j + 1] = ic.rgb2gray({ srcTileRecord[j + 1], srcTileRecord[j + halfSize], srcTileRecord[j + halfSize + 1], 0xFF });
		}
		for (size_t j = 0; j < dstTileRecord.size(); j++) dst.push_back(dstTileRecord[j]);
	}
	vector<unsigned char>().swap(srcTileRecord);
	vector<unsigned char>().swap(dstTileRecord);
	return 0;
}

int TileTexCov::RGB565ToRGBA8(const vector<unsigned char> &src, vector<unsigned char> &dst,
	unsigned int width, unsigned int height, unsigned int tileWidth, unsigned int tileHeight)
{
	unsigned int srcDepth = 16;
	unsigned int dstDepth = 32;
	unsigned int texSize = width * height;
	unsigned int tileSize = tileWidth * tileHeight;
	unsigned int srcSize = texSize * srcDepth / 8;
	unsigned int dstSize = texSize * dstDepth / 8;
	unsigned int halfSize = tileSize * 2;
	if (src.size() < srcSize) return -1;

	ImageCalc ic;
	vector<unsigned char>().swap(dst);
	vector<unsigned char> srcTileRecord(tileSize * srcDepth / 8);
	vector<unsigned char> dstTileRecord(tileSize * dstDepth / 8);
	for (size_t i = 0; i < srcSize; i += srcTileRecord.size())
	{
		for (size_t j = 0; j < srcTileRecord.size(); j++) srcTileRecord[j] = src[i + j];
		unsigned short value = 0;
		for (size_t j = 0; j < srcTileRecord.size(); j += 2)
		{
			value = (srcTileRecord[j] << 8) | srcTileRecord[j + 1];
			dstTileRecord[j] = 0xFF;
			dstTileRecord[j + 1] = (((value >> 11) & 0x1F) * 0x08) & 0xFF;
			dstTileRecord[j + halfSize] = (((value >> 5) & 0x3F) * 0x04) & 0xFF;
			dstTileRecord[j + halfSize + 1] = ((value & 0x1F) * 0x08) & 0xFF;
		}
		for (size_t j = 0; j < dstTileRecord.size(); j++) dst.push_back(dstTileRecord[j]);
	}
	vector<unsigned char>().swap(srcTileRecord);
	vector<unsigned char>().swap(dstTileRecord);
	return 0;
}

int TileTexCov::RGBA8ToRGB565(const vector<unsigned char> &src, vector<unsigned char> &dst,
	unsigned int width, unsigned int height, unsigned int tileWidth, unsigned int tileHeight)
{
	unsigned int srcDepth = 32;
	unsigned int dstDepth = 16;
	unsigned int texSize = width * height;
	unsigned int tileSize = tileWidth * tileHeight;
	unsigned int srcSize = texSize * srcDepth / 8;
	unsigned int dstSize = texSize * dstDepth / 8;
	unsigned int halfSize = tileSize * 2;
	if (src.size() < srcSize) return -1;

	ImageCalc ic;
	vector<unsigned char>().swap(dst);
	vector<unsigned char> srcTileRecord(tileSize * srcDepth / 8);
	vector<unsigned char> dstTileRecord(tileSize * dstDepth / 8);
	for (size_t i = 0; i < srcSize; i += srcTileRecord.size())
	{
		for (size_t j = 0; j < srcTileRecord.size(); j++) srcTileRecord[j] = src[i + j];
		unsigned short value = 0;
		for (size_t j = 0; j < dstTileRecord.size(); j += 2)
		{
			value = ((srcTileRecord[j + 1] / 0x08) & 0x1F) << 11;
			value |= ((srcTileRecord[j + halfSize] / 0x04) & 0x3F) << 5;
			value |= (srcTileRecord[j + halfSize + 1] / 0x08) & 0x1F;
			dstTileRecord[j] = (value >> 8) & 0xFF;
			dstTileRecord[j + 1] = value & 0xFF;
		}
		for (size_t j = 0; j < dstTileRecord.size(); j++) dst.push_back(dstTileRecord[j]);
	}
	vector<unsigned char>().swap(srcTileRecord);
	vector<unsigned char>().swap(dstTileRecord);
	return 0;
}

int TileTexCov::RGB5A3ToRGBA8(const vector<unsigned char> &src, vector<unsigned char> &dst,
	unsigned int width, unsigned int height, unsigned int tileWidth, unsigned int tileHeight)
{
	unsigned int srcDepth = 16;
	unsigned int dstDepth = 32;
	unsigned int texSize = width * height;
	unsigned int tileSize = tileWidth * tileHeight;
	unsigned int srcSize = texSize * srcDepth / 8;
	unsigned int dstSize = texSize * dstDepth / 8;
	unsigned int halfSize = tileSize * 2;
	if (src.size() < srcSize) return -1;

	ImageCalc ic;
	vector<unsigned char>().swap(dst);
	vector<unsigned char> srcTileRecord(tileSize * srcDepth / 8);
	vector<unsigned char> dstTileRecord(tileSize * dstDepth / 8);
	for (size_t i = 0; i < srcSize; i += srcTileRecord.size())
	{
		for (size_t j = 0; j < srcTileRecord.size(); j++) srcTileRecord[j] = src[i + j];
		unsigned short value = 0;
		for (size_t j = 0; j < srcTileRecord.size(); j += 2)
		{
			value = (srcTileRecord[j] << 8) | srcTileRecord[j + 1];
			if (value < 0x8000)
			{
				dstTileRecord[j] = (((value >> 12) & 0x07) * 0x20) & 0xFF;
				dstTileRecord[j + 1] = (((value >> 8) & 0x0F) * 0x11) & 0xFF;
				dstTileRecord[j + halfSize] = (((value >> 4) & 0x0F) * 0x11) & 0xFF;
				dstTileRecord[j + halfSize + 1] = ((value & 0x0F) * 0x11) & 0xFF;
			}
			else
			{
				dstTileRecord[j] = 0xFF;
				dstTileRecord[j + 1] = (((value >> 10) & 0x1F) * 0x08) & 0xFF;
				dstTileRecord[j + halfSize] = (((value >> 5) & 0x1F) * 0x08) & 0xFF;
				dstTileRecord[j + halfSize + 1] = ((value & 0x1F) * 0x08) & 0xFF;
			}
		}
		for (size_t j = 0; j < dstTileRecord.size(); j++) dst.push_back(dstTileRecord[j]);
	}
	vector<unsigned char>().swap(srcTileRecord);
	vector<unsigned char>().swap(dstTileRecord);
	return 0;
}

int TileTexCov::RGBA8ToRGB5A3(const vector<unsigned char> &src, vector<unsigned char> &dst,
	unsigned int width, unsigned int height, unsigned int tileWidth, unsigned int tileHeight)
{
	unsigned int srcDepth = 32;
	unsigned int dstDepth = 16;
	unsigned int texSize = width * height;
	unsigned int tileSize = tileWidth * tileHeight;
	unsigned int srcSize = texSize * srcDepth / 8;
	unsigned int dstSize = texSize * dstDepth / 8;
	unsigned int halfSize = tileSize * 2;
	if (src.size() < srcSize) return -1;

	ImageCalc ic;
	vector<unsigned char>().swap(dst);
	vector<unsigned char> srcTileRecord(tileSize * srcDepth / 8);
	vector<unsigned char> dstTileRecord(tileSize * dstDepth / 8);
	for (size_t i = 0; i < srcSize; i += srcTileRecord.size())
	{
		for (size_t j = 0; j < srcTileRecord.size(); j++) srcTileRecord[j] = src[i + j];
		unsigned short value = 0;
		for (size_t j = 0; j < dstTileRecord.size(); j += 2)
		{
			if (srcTileRecord[j] == 0xFF)
			{
				value = 0x8000;
				value |= ((srcTileRecord[j + 1] / 0x08) & 0x1F) << 10;
				value |= ((srcTileRecord[j + halfSize] / 0x08) & 0x1F) << 5;
				value |= (srcTileRecord[j + halfSize + 1] / 0x08) & 0x1F;
			}
			else
			{
				value = ((srcTileRecord[j] / 0x20) & 0x07) << 12;
				value |= ((srcTileRecord[j + 1] / 0x11) & 0x0F) << 8;
				value |= ((srcTileRecord[j + halfSize] / 0x11) & 0x0F) << 4;
				value |= (srcTileRecord[j + halfSize + 1] / 0x11) & 0x0F;
			}
			dstTileRecord[j] = (value >> 8) & 0xFF;
			dstTileRecord[j + 1] = value & 0xFF;
		}
		for (size_t j = 0; j < dstTileRecord.size(); j++) dst.push_back(dstTileRecord[j]);
	}
	vector<unsigned char>().swap(srcTileRecord);
	vector<unsigned char>().swap(dstTileRecord);
	return 0;
}

int TileTexCov::RGB555ToRGBA8(const vector<unsigned char> &src, vector<unsigned char> &dst,
	unsigned int width, unsigned int height, unsigned int tileWidth, unsigned int tileHeight)
{
	unsigned int srcDepth = 16;
	unsigned int dstDepth = 32;
	unsigned int texSize = width * height;
	unsigned int tileSize = tileWidth * tileHeight;
	unsigned int srcSize = texSize * srcDepth / 8;
	unsigned int dstSize = texSize * dstDepth / 8;
	unsigned int halfSize = tileSize * 2;
	if (src.size() < srcSize) return -1;

	ImageCalc ic;
	vector<unsigned char>().swap(dst);
	vector<unsigned char> srcTileRecord(tileSize * srcDepth / 8);
	vector<unsigned char> dstTileRecord(tileSize * dstDepth / 8);
	for (size_t i = 0; i < srcSize; i += srcTileRecord.size())
	{
		for (size_t j = 0; j < srcTileRecord.size(); j++) srcTileRecord[j] = src[i + j];
		unsigned short value = 0;
		for (size_t j = 0; j < srcTileRecord.size(); j += 2)
		{
			value = (srcTileRecord[j] << 8) | srcTileRecord[j + 1];
			dstTileRecord[j] = 0xFF;
			dstTileRecord[j + 1] = (((value >> 10) & 0x1F) * 255 / 31) & 0xFF;
			dstTileRecord[j + halfSize] = (((value >> 5) & 0x1F) * 255 / 31) & 0xFF;
			dstTileRecord[j + halfSize + 1] = ((value & 0x1F) * 255 / 31) & 0xFF;
		}
		for (size_t j = 0; j < dstTileRecord.size(); j++) dst.push_back(dstTileRecord[j]);
	}
	vector<unsigned char>().swap(srcTileRecord);
	vector<unsigned char>().swap(dstTileRecord);
	return 0;
}

int TileTexCov::RGBA8ToRGB555(const vector<unsigned char> &src, vector<unsigned char> &dst,
	unsigned int width, unsigned int height, unsigned int tileWidth, unsigned int tileHeight)
{
	unsigned int srcDepth = 32;
	unsigned int dstDepth = 16;
	unsigned int texSize = width * height;
	unsigned int tileSize = tileWidth * tileHeight;
	unsigned int srcSize = texSize * srcDepth / 8;
	unsigned int dstSize = texSize * dstDepth / 8;
	unsigned int halfSize = tileSize * 2;
	if (src.size() < srcSize) return -1;

	ImageCalc ic;
	vector<unsigned char>().swap(dst);
	vector<unsigned char> srcTileRecord(tileSize * srcDepth / 8);
	vector<unsigned char> dstTileRecord(tileSize * dstDepth / 8);
	for (size_t i = 0; i < srcSize; i += srcTileRecord.size())
	{
		for (size_t j = 0; j < srcTileRecord.size(); j++) srcTileRecord[j] = src[i + j];
		unsigned short value = 0;
		for (size_t j = 0; j < dstTileRecord.size(); j += 2)
		{
			value |= ((srcTileRecord[j + 1] / 0x08) & 0x1F) << 10;
			value |= ((srcTileRecord[j + halfSize] / 0x08) & 0x1F) << 5;
			value |= (srcTileRecord[j + halfSize + 1] / 0x08) & 0x1F;
			dstTileRecord[j] = (value >> 8) & 0xFF;
			dstTileRecord[j + 1] = value & 0xFF;
		}
		for (size_t j = 0; j < dstTileRecord.size(); j++) dst.push_back(dstTileRecord[j]);
	}
	vector<unsigned char>().swap(srcTileRecord);
	vector<unsigned char>().swap(dstTileRecord);
	return 0;
}

int TileTexCov::CMPRToRGBA8(const vector<unsigned char> &src, vector<unsigned char> &dst, unsigned int width, unsigned int height,
	unsigned int tileWidth, unsigned int tileHeight, unsigned char endian)
{
	unsigned int srcDepth = 4;
	unsigned int dstDepth = 32;
	unsigned int texSize = width * height;
	unsigned int tileSize = tileWidth * tileHeight;
	unsigned int srcSize = texSize * srcDepth / 8;
	unsigned int dstSize = texSize * dstDepth / 8;
	unsigned int halfSize = tileSize * 2;
	if (tileWidth < 4 || tileHeight < 4 || src.size() < srcSize) return -1;

	vector<unsigned char>().swap(dst);
	CMP_Texture srcTexture = { 0 };
	srcTexture.dwSize = sizeof(srcTexture);
	srcTexture.dwWidth = tileWidth;
	srcTexture.dwHeight = tileHeight;
	srcTexture.dwPitch = tileWidth;
	srcTexture.format = CMP_FORMAT_BC1;
	srcTexture.dwDataSize = tileSize * srcDepth / 8;
	srcTexture.pData = (CMP_BYTE*)malloc(srcTexture.dwDataSize);
	CMP_Texture dstTexture = { 0 };
	dstTexture.dwSize = sizeof(dstTexture);
	dstTexture.dwWidth = tileWidth;
	dstTexture.dwHeight = tileHeight;
	dstTexture.dwPitch = tileWidth;
	dstTexture.format = CMP_FORMAT_RGBA_8888;
	dstTexture.dwDataSize = tileSize * dstDepth / 8;
	dstTexture.pData = (CMP_BYTE*)malloc(dstTexture.dwDataSize);
	CMP_CompressOptions options = { 0 };
	options.dwSize = sizeof(options);
	options.fquality = (CMP_FLOAT)1.0;
	options.nAlphaThreshold = 128;
	options.bDXT1UseAlpha = true;
	options.nCompressionSpeed = CMP_Speed_Normal;
	options.dwnumThreads = 1;
	vector<unsigned char> dstTileRecord(tileSize * dstDepth / 8);
	for (size_t i = 0; i < srcSize; i += srcTexture.dwDataSize)
	{
		vector<unsigned char> dstTile;
		for (size_t j = 0; j < srcTexture.dwDataSize; j++) dstTile.push_back(src[i + j]);
		if (endian == 1) TileLowToHigh(dstTile, CMPR);
		for (size_t j = 0; j < srcTexture.dwDataSize; j++) srcTexture.pData[j] = dstTile[j];
		vector<unsigned char>().swap(dstTile);
		CMP_ConvertTexture(&srcTexture, &dstTexture, &options, NULL);
		for (size_t j = 0; j < dstTexture.dwDataSize; j += 4)
		{
			dstTileRecord[j / 2] = dstTexture.pData[j + 3];
			dstTileRecord[j / 2 + 1] = dstTexture.pData[j];
			dstTileRecord[j / 2 + halfSize] = dstTexture.pData[j + 1];
			dstTileRecord[j / 2 + halfSize + 1] = dstTexture.pData[j + 2];
		}
		for (size_t j = 0; j < dstTileRecord.size(); j++) dst.push_back(dstTileRecord[j]);
	}
	free(srcTexture.pData);
	free(dstTexture.pData);
	vector<unsigned char>().swap(dstTileRecord);
	return 0;
}

int TileTexCov::RGBA8ToCMPR(const vector<unsigned char> &src, vector<unsigned char> &dst, unsigned int width, unsigned int height,
	unsigned int tileWidth, unsigned int tileHeight, unsigned char endian)
{
	unsigned int srcDepth = 32;
	unsigned int dstDepth = 4;
	unsigned int texSize = width * height;
	unsigned int tileSize = tileWidth * tileHeight;
	unsigned int srcSize = texSize * srcDepth / 8;
	unsigned int dstSize = texSize * dstDepth / 8;
	unsigned int halfSize = tileSize * 2;
	if (tileWidth < 4 || tileHeight < 4 || src.size() < srcSize) return -1;

	vector<unsigned char>().swap(dst);
	CMP_Texture srcTexture = { 0 };
	srcTexture.dwSize = sizeof(srcTexture);
	srcTexture.dwWidth = tileWidth;
	srcTexture.dwHeight = tileHeight;
	srcTexture.dwPitch = tileWidth;
	srcTexture.format = CMP_FORMAT_RGBA_8888;
	srcTexture.dwDataSize = tileSize * srcDepth / 8;
	srcTexture.pData = (CMP_BYTE*)malloc(srcTexture.dwDataSize);
	memset(srcTexture.pData, 0, sizeof(CMP_BYTE) * srcTexture.dwDataSize);
	CMP_Texture dstTexture = { 0 };
	dstTexture.dwSize = sizeof(dstTexture);
	dstTexture.dwWidth = tileWidth;
	dstTexture.dwHeight = tileHeight;
	dstTexture.dwPitch = tileWidth;
	dstTexture.format = CMP_FORMAT_BC1;
	dstTexture.dwDataSize = tileSize * dstDepth / 8;
	dstTexture.pData = (CMP_BYTE*)malloc(dstTexture.dwDataSize);
	memset(dstTexture.pData, 0, sizeof(CMP_BYTE) * dstTexture.dwDataSize);
	CMP_CompressOptions options = { 0 };
	options.dwSize = sizeof(options);
	options.fquality = (CMP_FLOAT)1.0;
	options.nAlphaThreshold = 128;
	options.bDXT1UseAlpha = true;
	options.nCompressionSpeed = CMP_Speed_Normal;
	options.dwnumThreads = 1;
	vector<unsigned char> srcTileRecord(tileSize * srcDepth / 8);
	for (size_t i = 0; i < srcSize; i += srcTexture.dwDataSize)
	{
		for (size_t j = 0; j < srcTexture.dwDataSize; j++) srcTileRecord[j] = src[i + j];
		for (size_t j = 0; j < srcTexture.dwDataSize; j += 4)
		{
			srcTexture.pData[j] = srcTileRecord[j / 2 + 1];
			srcTexture.pData[j + 1] = srcTileRecord[j / 2 + halfSize];
			srcTexture.pData[j + 2] = srcTileRecord[j / 2 + halfSize + 1];
			srcTexture.pData[j + 3] = srcTileRecord[j / 2];
		}
		CMP_ConvertTexture(&srcTexture, &dstTexture, &options, NULL);
		vector<unsigned char> dstTile;
		for (size_t j = 0; j < dstTexture.dwDataSize; j++) dstTile.push_back(dstTexture.pData[j]);
		if (endian == 1) TileLowToHigh(dstTile, CMPR);
		for (size_t j = 0; j < dstTexture.dwDataSize; j++) dst.push_back(dstTile[j]);
		vector<unsigned char>().swap(dstTile);
	}
	free(srcTexture.pData);
	free(dstTexture.pData);
	vector<unsigned char>().swap(srcTileRecord);
	return 0;
}

int TileTexCov::tileCov(const vector<unsigned char> &src, vector<unsigned char> &dst, unsigned int width, unsigned int height,
	unsigned char srcTileMode, unsigned char dstTileMode, bool srcSwizzle, bool dstSwizzle)
{
	// tileMode: 0: 8*8, 1: 8*4, 2: 4*4, 3: 4*2, 4: 2*2, 5: 2*1, 6: 1*1
	if (width * height == 0 || srcTileMode > 6 || dstTileMode > 6) return -1;
	if (srcTileMode == dstTileMode)
	{
		vector<unsigned char>(src.size()).swap(dst);
		copy(src.begin(), src.end(), dst.begin());
		return 0;
	}
	unsigned int dataSize = width * height * 4;
	if (src.size() < dataSize) return -1;

	// features calculate
	unsigned int srcTileWidth = 8;
	unsigned int srcTileHeight = 8;
	switch (srcTileMode)
	{
	case 1: srcTileHeight = 4; break;
	case 2: srcTileWidth = 4; srcTileHeight = 4; break;
	case 3: srcTileWidth = 4; srcTileHeight = 2; break;
	case 4: srcTileWidth = 2; srcTileHeight = 2; break;
	case 5: srcTileWidth = 2; srcTileHeight = 1; break;
	case 6: srcTileWidth = 1; srcTileHeight = 1; break;
	default:;
	}
	unsigned int dstTileWidth = 8;
	unsigned int dstTileHeight = 8;
	switch (dstTileMode)
	{
	case 1: dstTileHeight = 4; break;
	case 2: dstTileWidth = 4; dstTileHeight = 4; break;
	case 3: dstTileWidth = 4; dstTileHeight = 2; break;
	case 4: dstTileWidth = 2; dstTileHeight = 2; break;
	case 5: dstTileWidth = 2; dstTileHeight = 1; break;
	case 6: dstTileWidth = 1; dstTileHeight = 1; break;
	default:;
	}
	if (width % srcTileWidth != 0 || width % dstTileWidth != 0 || height % srcTileHeight != 0 || height % dstTileHeight != 0) return -1;
	unsigned int srcTileSize = srcTileWidth * srcTileHeight;
	unsigned int srcHalfSize = srcTileSize * 2;
	unsigned int dstTileSize = dstTileWidth * dstTileHeight;
	unsigned int dstHalfSize = dstTileSize * 2;

	ImageData image;
	image.initial(width, height);
	vector<unsigned char> tempTileRecord(srcTileSize * 4);
	for (size_t i = 0; i < dataSize; i += tempTileRecord.size())
	{
		for (size_t j = 0; j < tempTileRecord.size(); j++) tempTileRecord[j] = src[i + j];
		for (size_t j = 0; j < srcTileSize; j++)
		{
			unsigned int index = i / 4 + j;
			unsigned int x = srcTileWidth * ((index / srcTileSize) % (image.width / srcTileWidth)) + (index % srcTileSize) % srcTileWidth;
			unsigned int y = srcTileHeight * ((index / srcTileSize) / (image.width / srcTileWidth)) + (index % srcTileSize) / srcTileWidth;
			image.set({ tempTileRecord[j * 2 + 1], tempTileRecord[j * 2 + srcHalfSize], tempTileRecord[j * 2 + srcHalfSize + 1], tempTileRecord[j * 2] }, x, y);
		}
	}
	vector<unsigned char>().swap(dst);
	unsigned char recordIndex = 0;
	vector<Color> tempColorRecord(dstTileSize);
	for (size_t i = 0; i < image.size; i++)
	{
		unsigned int x = dstTileWidth * ((i / dstTileSize) % (image.width / dstTileWidth)) + (i % dstTileSize) % dstTileWidth;
		unsigned int y = dstTileHeight * ((i / dstTileSize) / (image.width / dstTileWidth)) + (i % dstTileSize) / dstTileWidth;
		tempColorRecord[recordIndex++] = image.get(x, y);
		if (recordIndex == dstTileSize)
		{
			for (size_t j = 0; j < dstTileSize; j++)
			{
				dst.push_back(tempColorRecord[j].a);
				dst.push_back(tempColorRecord[j].r);
			}
			for (size_t j = 0; j < dstTileSize; j++)
			{
				dst.push_back(tempColorRecord[j].g);
				dst.push_back(tempColorRecord[j].b);
			}
			recordIndex = 0;
		}
	}
	vector<unsigned char>().swap(tempTileRecord);
	vector<Color>().swap(tempColorRecord);
	image.release();
	return 0;
}

int TileTexCov::texCov(const vector<unsigned char> &src, vector<unsigned char> &dst, TileTexFormat srcMode, TileTexFormat dstMode,
	unsigned int width, unsigned int height, unsigned char tileMode, bool swizzle, unsigned char endian)
{
	// tileMode: 0: 8*8, 1: 8*4, 2: 4*4, 3: 4*2, 4: 2*2, 5: 2*1, 6: 1*1
	if (width * height == 0 || tileMode > 6) return -1;
	if (srcMode == dstMode)
	{
		vector<unsigned char>(src.size()).swap(dst);
		copy(src.begin(), src.end(), dst.begin());
		return 0;
	}

	// features calculate
	unsigned int tileWidth = 8;
	unsigned int tileHeight = 8;
	switch (tileMode)
	{
	case 1: tileHeight = 4; break;
	case 2: tileWidth = 4; tileHeight = 4; break;
	case 3: tileWidth = 4; tileHeight = 2; break;
	case 4: tileWidth = 2; tileHeight = 2; break;
	case 5: tileWidth = 2; tileHeight = 1; break;
	case 6: tileWidth = 1; tileHeight = 1; break;
	default:;
	}
	if (width % tileWidth != 0 || height % tileHeight != 0) return -1;
	if (srcMode == I4 && dstMode == RGBA8) return I4ToRGBA8(src, dst, width, height, tileWidth, tileHeight);
	if (srcMode == I8 && dstMode == RGBA8) return I8ToRGBA8(src, dst, width, height, tileWidth, tileHeight);
	if (srcMode == IA4 && dstMode == RGBA8) return IA4ToRGBA8(src, dst, width, height, tileWidth, tileHeight);
	if (srcMode == IA8 && dstMode == RGBA8) return IA8ToRGBA8(src, dst, width, height, tileWidth, tileHeight);
	if (srcMode == RGB565 && dstMode == RGBA8) return RGB565ToRGBA8(src, dst, width, height, tileWidth, tileHeight);
	if (srcMode == RGB5A3 && dstMode == RGBA8) return RGB5A3ToRGBA8(src, dst, width, height, tileWidth, tileHeight);
	if (srcMode == RGB555 && dstMode == RGBA8) return RGB555ToRGBA8(src, dst, width, height, tileWidth, tileHeight);
	if (srcMode == CMPR && dstMode == RGBA8) return CMPRToRGBA8(src, dst, width, height, tileWidth, tileHeight, endian);
	if (srcMode == RGBA8 && dstMode == I4) return RGBA8ToI4(src, dst, width, height, tileWidth, tileHeight);
	if (srcMode == RGBA8 && dstMode == I8) return RGBA8ToI8(src, dst, width, height, tileWidth, tileHeight);
	if (srcMode == RGBA8 && dstMode == IA4) return RGBA8ToIA4(src, dst, width, height, tileWidth, tileHeight);
	if (srcMode == RGBA8 && dstMode == IA8) return RGBA8ToIA8(src, dst, width, height, tileWidth, tileHeight);
	if (srcMode == RGBA8 && dstMode == RGB565) return RGBA8ToRGB565(src, dst, width, height, tileWidth, tileHeight);
	if (srcMode == RGBA8 && dstMode == RGB5A3) return RGBA8ToRGB5A3(src, dst, width, height, tileWidth, tileHeight);
	if (srcMode == RGBA8 && dstMode == RGB555) return RGBA8ToRGB555(src, dst, width, height, tileWidth, tileHeight);
	if (srcMode == RGBA8 && dstMode == CMPR) return RGBA8ToCMPR(src, dst, width, height, tileWidth, tileHeight, endian);
	return -1;
}

int TileTexCov::TileLowToHigh(vector<unsigned char> &data, TileTexFormat mode)
{
	if (mode == CMPR)
	{
		unsigned char temp;
		vector<unsigned char> src(data);
		for (size_t i = 0; i < src.size(); i += 8)
		{
			data[i] = src[i + 1];
			data[i + 1] = src[i];
			data[i + 2] = src[i + 3];
			data[i + 3] = src[i + 2];
			for (int k = 4; k < 8; k++)
			{
				temp = src[i + k];
				data[i + k] = (temp & 0x3) << 6 | (temp & 0xC) << 2 | (temp & 0x30) >> 2 | (temp & 0xC0) >> 6;
			}
		}
	}
	return 0;
}

int TextureCov(const vector<unsigned char> &src, vector<unsigned char> &dst, TileTexFormat srcMode, TileTexFormat dstMode, unsigned int width, unsigned int height,
	unsigned char srcTileMode, unsigned char dstTileMode, bool srcSwizzle, bool dstSwizzle, unsigned char srcEndian, unsigned char dstEndian)
{
	TileTexCov ttc;
	vector<unsigned char> temp;
	if (ttc.texCov(src, dst, srcMode, RGBA8, width, height, srcTileMode, srcSwizzle, srcEndian) != 0) return -1;
	if (ttc.tileCov(dst, temp, width, height, srcTileMode, dstTileMode, srcSwizzle, dstSwizzle) != 0) return -1;
	if (ttc.texCov(temp, dst, RGBA8, dstMode, width, height, dstTileMode, dstSwizzle, dstEndian) != 0) return -1;
	vector<unsigned char>().swap(temp);
	return 0;
}

int getBPP(TileTexFormat type)
{
	switch (type)
	{
	case I4: return 4;
	case I8: return 8;
	case IA4: return 8;
	case IA8: return 16;
	case RGB565: return 16;
	case RGB5A3: return 16;
	case RGBA8: return 32;
	case RGB555: return 16;
	default: return 0;
	}
}

vector<vector<Color>> ByteToPal(const vector<unsigned char> &srcCol, TileTexFormat colMode, int colNum, int palNum, bool endian, bool rev)
{
	ImageCalc ic;
	vector<vector<Color>> palList(palNum, vector<Color>(colNum, { 0, 0, 0, 0 }));
	if (colMode > 7 || palNum == 0 || colNum == 0) return palList;
	int bpp = getBPP(colMode) / 8;
	unsigned int col = 0, srcPos = 0;
	if (bpp == 0) for (int i = 0; i < palNum; i++) for (int j = 0; j < colNum; j += 2)
	{
		if (srcPos >= srcCol.size()) break;
		col = srcCol[srcPos++];
		palList[i][j] = ic.byte2col(col & 0x0F, colMode, rev);
		palList[i][j + 1] = ic.byte2col((col >> 4) & 0x0F, colMode, rev);
	}
	else for (int i = 0; i < palNum; i++) for (int j = 0; j < colNum; j++)
	{
		if (srcPos >= srcCol.size()) break;
		col = 0;
		if (endian) for (int k = 0; k < bpp; k++) col = (col << 8) | srcCol[srcPos + k];
		else for (int k = bpp; k > 0; k--) col = (col << 8) | srcCol[srcPos + k - 1];
		srcPos += bpp;
		palList[i][j] = ic.byte2col(col, colMode, rev);
	}
	return palList;
}

vector<unsigned char> PalToByte(const vector<vector<Color>> &srcCol, TileTexFormat colMode, bool endian, bool rev)
{
	ImageCalc ic;
	vector<unsigned char> dstCol;
	if (colMode > 7 || srcCol.size() == 0 || srcCol[0].size() == 0) return dstCol;
	int bpp = getBPP(colMode) / 8;
	unsigned int col = 0;
	if (bpp == 0) for (size_t i = 0; i < srcCol.size(); i++) for (size_t j = 0; j < srcCol[i].size(); j += 2)
	{
		dstCol.push_back(((ic.col2byte(srcCol[i][j], colMode, rev) & 0x0F) << 4) | (ic.col2byte(srcCol[i][j + 1], colMode, rev) & 0x0F));
	}
	else for (size_t i = 0; i < srcCol.size(); i++) for (size_t j = 0; j < srcCol[i].size(); j++)
	{
		col = ic.col2byte(srcCol[i][j], colMode, rev);
		if (endian) for (int k = bpp - 1; k >= 0; k--) dstCol.push_back((col >> (k * 8)) & 0xFF);
		else for (int k = 0; k < bpp; k++) dstCol.push_back((col >> (k * 8)) & 0xFF);
	}
	return dstCol;
}
