#include "imageConv.h"

#pragma warning(disable : 4996)

int main(int argc, char* argv[])
{
	OPT options(argc, argv);
	int result = -1;
	if (options.modeIn == NA || options.modeOut == NA) return result;
	else if (options.modeIn == IMG && options.modeOut == IMG) result = IMG2IMG(options);
	else if (options.modeIn == IMG && options.modeOut == TMP) result = IMG2TMP(options);
	else if (options.modeIn == IMG && options.modeOut == IP) result = IMG2IP(options);
	else if (options.modeIn == IMG && options.modeOut == TT) result = IMG2TT(options);
	else if (options.modeIn == TMP && options.modeOut == IMG) result = TMP2IMG(options);
	else if (options.modeIn == IP && options.modeOut == IMG) result = IP2IMG(options);
	else if (options.modeIn == TT && options.modeOut == IMG) result = TT2IMG(options);
	else if (options.modeIn == TT && options.modeOut == TT) result = TT2TT(options);
	if (result == -1) printf(" unsupported!\n");
	return result;
}

int IMG2IMG(OPT &options)
{
	printf("IMG -> IMG\n");
	IMGInfo *termIn = (IMGInfo *)options.getTerm(0);
	IMGInfo *termOut = (IMGInfo *)options.getTerm(1);
	ImageData imgIn(termIn->imgName);
	if (imgIn.size == 0) return -1;
	imgIn.setType(termOut->type);
	int result = imgIn.write(termOut->imgName);
	imgIn.release();
	return result;
}

int IMG2TMP(OPT &options)
{
	printf("IMG -> TMP\n");
	IMGInfo *termIn = (IMGInfo *)options.getTerm(0);
	TMPInfo *termOut = (TMPInfo *)options.getTerm(1);
	ImageData imgIn(termIn->imgName, true);
	if (imgIn.size == 0) return -1;
	Color bg = imgIn.bg;
	if (termIn->bgFlag == 1) bg = termIn->bg;
	else if (termOut->bgFlag == 1) bg = termOut->bg;
	else bg = imgIn.bg;
	vector<vector<u8>> tileData;
	vector<u16> mapData;
	vector<vector<Color>> palList;
	int result = ImageToTMP(imgIn, tileData, mapData, palList, bg, termOut->tileMode, termOut->palMode, termOut->palNum, termOut->colNum, termOut->times, termOut->bgFlag);
	if (result > 0)
	{
		FILE *tile = fopen(termOut->tileName, "wb");
		if (tile != NULL)
		{
			for (size_t i = 0; i < tileData.size(); i++) for (size_t j = 0; j < tileData[i].size(); j++) putc(tileData[i][j], tile);
			fclose(tile);
		}
		FILE *map = fopen(termOut->mapName, "wb");
		if (map != NULL)
		{
			if (termOut->endian) for (size_t i = 0; i < mapData.size(); i++) mapData[i] = ((mapData[i] & 0xFF) << 8) | ((mapData[i] >> 8) & 0xFF);
			for (size_t i = 0; i < mapData.size(); i++) fwrite(&mapData[i], 2, 1, map);
			fclose(map);
		}
		FILE *pal = fopen(termOut->palName, "wb");
		if (pal != NULL)
		{
			for (size_t i = 0; i < palList.size(); i++)
			{
				while (palList[i].size() < (size_t)termOut->colNum) palList[i].push_back({ 0, 0, 0, 0 });
				if (termOut->pad > 0) while (palList[i].size() < (size_t)termOut->pad) palList[i].push_back({ 0, 0, 0, 0 });
			}
			vector<u8> res = PalToByte(palList, (TileTexFormat)termOut->colMode, termOut->endian, termOut->reverse);
			for (size_t i = 0; i < res.size(); i++) putc(res[i], pal);
			vector<u8>().swap(res);
			fclose(pal);
		}
	}
	for (size_t i = 0; i < tileData.size(); i++) vector<u8>().swap(tileData[i]);
	vector<vector<u8>>().swap(tileData);
	for (size_t i = 0; i < palList.size(); i++) vector<Color>().swap(palList[i]);
	vector<vector<Color>>().swap(palList);
	vector<u16>().swap(mapData);
	imgIn.release();
	return result;
}

int IMG2IP(OPT &options)
{
	printf("IMG -> IP\n");
	IMGInfo *termIn = (IMGInfo *)options.getTerm(0);
	IPInfo *termOut = (IPInfo *)options.getTerm(1);
	ImageData imgIn(termIn->imgName, true);
	if (imgIn.size == 0) return -1;
	Color bg = imgIn.bg;
	if (termIn->bgFlag == 1) bg = termIn->bg;
	else if (termOut->bgFlag == 1) bg = termOut->bg;
	else bg = imgIn.bg;
	vector<u8> indexData;
	vector<Color> palData;
	int result = ImageToIP(imgIn, indexData, palData, bg, termOut->palMode, termOut->colNum, termOut->bgFlag);
	if (result > 0)
	{
		FILE *index = fopen(termOut->indexName, "wb");
		if (index != NULL)
		{
			for (size_t i = 0; i < indexData.size(); i++) putc(indexData[i], index);
			fclose(index);
		}
		FILE *pal = fopen(termOut->palName, "wb");
		if (pal != NULL)
		{
			while (palData.size() < (size_t)termOut->colNum) palData.push_back({ 0, 0, 0, 0 });
			if (termOut->pad > 0) while (palData.size() < (size_t)termOut->pad) palData.push_back({ 0, 0, 0, 0 });
			vector<u8> res = PalToByte(vector<vector<Color>>(1, palData), (TileTexFormat)termOut->colMode, termOut->endian, termOut->reverse);
			for (size_t i = 0; i < res.size(); i++) putc(res[i], pal);
			vector<u8>().swap(res);
			fclose(pal);
		}
	}
	vector<u8>().swap(indexData);
	vector<Color>().swap(palData);
	imgIn.release();
	return result;
}

int IMG2TT(OPT &options)
{
	printf("IMG -> TT\n");
	IMGInfo *termIn = (IMGInfo *)options.getTerm(0);
	TTInfo *termOut = (TTInfo *)options.getTerm(1);
	ImageData imgIn(termIn->imgName, true);
	if (imgIn.size == 0) return -1;
	Color bg = imgIn.bg;
	if (termIn->bgFlag == 1) bg = termIn->bg;
	else if (termOut->bgFlag == 1) bg = termOut->bg;
	vector<u8> texData;
	int result = ImageToTex(imgIn, texData, TileTexFormat(termOut->texMode), termOut->tileMode, termOut->swizzle, termOut->endian);
	if (result == 0)
	{
		FILE *tex = fopen(termOut->texName, "wb");
		if (tex != NULL)
		{
			for (size_t i = 0; i < texData.size(); i++) putc(texData[i], tex);
			fclose(tex);
		}
	}
	vector<u8>().swap(texData);
	imgIn.release();
	return result;
}

int TMP2IMG(OPT &options)
{
	printf("TMP -> IMG\n");
	TMPInfo *termIn = (TMPInfo *)options.getTerm(0);
	IMGInfo *termOut = (IMGInfo *)options.getTerm(1);
	int size = termIn->width * termIn->height;
	if (termIn->palMode == 0) size /= 2;
	if (size == 0 || termIn->tileMode > 2 || termIn->palMode > 1)return -1;

	u32 tileWidth = 8;
	u32 tileHeight = 8;
	switch (termIn->tileMode)
	{
	case 1: tileHeight = 4; break;
	case 2: tileWidth = 4; tileHeight = 4; break;
	default:;
	}
	int tileSize = tileWidth * tileHeight;
	if (termIn->palMode == 0) tileSize /= 2;

	FILE *tile = fopen(termIn->tileName, "rb");
	if (tile == NULL) return -1;
	fseek(tile, 0, 2);
	int tileNum = ftell(tile) / tileSize;
	rewind(tile);
	vector<vector<u8>> tileData(tileNum, vector<u8>(tileSize));
	for (int i = 0; i < tileNum; i++) for (int j = 0; j < tileSize; j++) tileData[i][j] = getc(tile);
	fclose(tile);
	FILE *map = fopen(termIn->mapName, "rb");
	if (map == NULL) return -1;
	fseek(map, 0, 2);
	int fileCount = ftell(map);
	rewind(map);
	int mapNum = size / tileSize;
	vector<u16> mapData(mapNum, 0);
	for (int i = 0; i < mapNum; i++)
	{
		if (fileCount <= 0) break;
		mapData[i] = getc(map);
		mapData[i] = termIn->endian == 1 ? (mapData[i] << 8) | getc(map) : (getc(map) << 8) | mapData[i];
		fileCount -= 2;
	}
	fclose(map);
	FILE *pal = fopen(termIn->palName, "rb");
	if (pal == NULL) return -1;
	fseek(pal, 0, 2);
	vector<u8> palData(ftell(pal));
	rewind(pal);
	for (size_t i = 0; i < palData.size(); i++) palData[i] = getc(pal);
	fclose(pal);
	vector<vector<Color>> palList = ByteToPal(palData, (TileTexFormat)termIn->colMode, termIn->colNum, termIn->palNum, termIn->endian, termIn->reverse);
	vector<u8>().swap(palData);
	ImageData imgOut;
	TMPToImage(imgOut, tileData, mapData, palList, termIn->width, termIn->height, termIn->tileMode, termIn->palMode, termOut->type);
	int result = imgOut.write(termOut->imgName);
	for (size_t i = 0; i < tileData.size(); i++) vector<u8>().swap(tileData[i]);
	vector<vector<u8>>().swap(tileData);
	vector<u16>().swap(mapData);
	for (size_t i = 0; i < palList.size(); i++) vector<Color>().swap(palList[i]);
	vector<vector<Color>>().swap(palList);
	imgOut.release();
	return result;
}

int IP2IMG(OPT &options)
{
	printf("IP -> IMG\n");
	IPInfo *termIn = (IPInfo *)options.getTerm(0);
	IMGInfo *termOut = (IMGInfo *)options.getTerm(1);
	int size = termIn->width * termIn->height;
	if (termIn->palMode == 0) size /= 2;
	if (size == 0) return -1;
	FILE *index = fopen(termIn->indexName, "rb");
	if (index == NULL) return -1;
	vector<u8> indexData(size);
	for (int i = 0; i < size; i++) indexData[i] = getc(index);
	fclose(index);
	FILE *pal = fopen(termIn->palName, "rb");
	if (pal == NULL) return -1;
	fseek(pal, 0, 2);
	vector<u8> palData(ftell(pal));
	rewind(pal);
	for (size_t i = 0; i < palData.size(); i++) palData[i] = getc(pal);
	fclose(pal);
	vector<vector<Color>> palList = ByteToPal(palData, (TileTexFormat)termIn->colMode, termIn->colNum, 1, termIn->endian, termIn->reverse);
	vector<u8>().swap(palData);
	ImageData imgOut;
	IPToImage(imgOut, indexData, palList[0], termIn->width, termIn->height, termIn->palMode, termOut->type);
	int result = imgOut.write(termOut->imgName);
	vector<u8>().swap(indexData);
	for (size_t i = 0; i < palList.size(); i++) vector<Color>().swap(palList[i]);
	vector<vector<Color>>().swap(palList);
	imgOut.release();
	return result;
}

int TT2IMG(OPT &options)
{
	printf("TT -> IMG\n");
	TTInfo *termIn = (TTInfo *)options.getTerm(0);
	IMGInfo *termOut = (IMGInfo *)options.getTerm(1);
	u32 size = termIn->width * termIn->height;
	if (size == 0)return -1;
	FILE *tex = fopen(termIn->texName, "rb");
	if (tex == NULL) return -1;
	fseek(tex, 0, 2);
	size = (unsigned int)ftell(tex);
	rewind(tex);
	vector<u8> texData(size);
	for (u32 i = 0; i < size; i++) texData[i] = getc(tex);
	fclose(tex);
	ImageData imgOut;
	TexToImage(imgOut, texData, TileTexFormat(termIn->texMode), termIn->width, termIn->height, termIn->tileMode, termIn->swizzle, termIn->endian);
	int result = imgOut.write(termOut->imgName);
	vector<u8>().swap(texData);
	imgOut.release();
	return result;
}

int TT2TT(OPT &options)
{
	printf("TT -> TT\n");
	TTInfo *termIn = (TTInfo *)options.getTerm(0);
	TTInfo *termOut = (TTInfo *)options.getTerm(1);
	u32 size = termIn->width * termIn->height;
	if (size == 0)return -1;
	FILE *tex = fopen(termIn->texName, "rb");
	if (tex == NULL) return -1;
	fseek(tex, 0, 2);
	size = (unsigned int)ftell(tex);
	rewind(tex);
	vector<u8> srcData(size);
	for (u32 i = 0; i < size; i++) srcData[i] = getc(tex);
	fclose(tex);

	vector<u8> dstData;
	int result = TextureCov(srcData, dstData, TileTexFormat(termIn->texMode), TileTexFormat(termOut->texMode),
		TileTexFormat(termIn->width), TileTexFormat(termIn->height), termIn->tileMode, termOut->tileMode, 
		termIn->swizzle, termOut->swizzle, termIn->endian, termOut->endian);
	if (result == 0)
	{
		FILE *tex = fopen(termOut->texName, "wb");
		if (tex != NULL)
		{
			for (size_t i = 0; i < dstData.size(); i++) putc(dstData[i], tex);
			fclose(tex);
		}
	}
	vector<u8>().swap(srcData);
	vector<u8>().swap(dstData);
	return result;
}
