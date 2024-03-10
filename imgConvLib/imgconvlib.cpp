#include "imgConvLib.h"
#include "kmeans.h"
#include <Compressonator.h>
#include "type.h"

#pragma warning(disable : 4996)

using namespace std;

int ImageToTMP(const ImageData &image, vector<vector<unsigned char>> &tileData, vector<unsigned short> &mapData, vector<vector<Color>> &palList, Color bg,
	unsigned char tileMode, unsigned char palMode, unsigned int palNum, unsigned int colNum, unsigned int times, bool bgKeep)
{
	// tileMode: 0: 8*8, 1: 8*4, 2: 4*4, 3: 4*2, 4: 2*2, 5: 2*1, 6: 1*1
	// palMode: 0: 4bit, 1: 8bit
	if (times == 0 || image.size == 0 || tileMode > 6 || palMode > 1 || colNum == 0 || (colNum == 1 && bgKeep)) return -1;

	// initial
	ImageData imgOri(image);
	if (tileData.size() > 0)
	{
		for (size_t i = 0; i < tileData.size(); i++) vector<u8>().swap(tileData[i]);
		vector<vector<u8>>().swap(tileData);
	}
	if (mapData.size() > 0) vector<u16>().swap(mapData);
	if (palList.size() > 0) for (size_t i = 0; i < palList.size(); i++) vector<Color>().swap(palList[i]);
	vector<vector<Color>>(palNum).swap(palList);
	// features calculate
	u32 tileWidth = 8;
	u32 tileHeight = 8;
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
	u32 tileSize = tileWidth * tileHeight;
	u32 dataSize = tileSize;
	if (palMode == 0) dataSize /= 2;
	imgOri.resize((u32)ceil((float)imgOri.width / (float)tileWidth)*tileWidth, (u32)ceil((float)imgOri.height / (float)tileHeight)*tileHeight);
	vector<Tile> tileList;
	u8 recordIndex = 0;
	vector<Color> tempColorRecord(tileSize);
	int tileId = 0;
	bool bgTileTest = bgKeep ? true : false;
	for (size_t j = 0; j < imgOri.size; j++)
	{
		u32 x = tileWidth * ((j / tileSize) % (imgOri.width / tileWidth)) + (j % tileSize) % tileWidth;
		u32 y = tileHeight * ((j / tileSize) / (imgOri.width / tileWidth)) + (j % tileSize) / tileWidth;
		tempColorRecord[recordIndex++] = imgOri.get(x, y);
		if (recordIndex == tileSize)
		{
			Tile tile(tileMode, bg);
			tile.tileId = tileId;
			for (size_t k = 0; k < tileSize; k++) tile.pixel[k] = tempColorRecord[k];
			tile.featureCalc();
			if (tile.pixelNum == 0) bgTileTest = true;
			tileList.push_back(tile);
			recordIndex = 0;
			tileId++;
		}
	}
	if (bgTileTest) tileData.push_back(vector<u8>(dataSize, 0));
	vector<Tile> pts;
	vector<Color> tempColRec;
	for (size_t i = 0; i < tileList.size(); i++)
	{
		if (tileList[i].pixelNum == 0) continue;
		bool addTest = false;
		for (int j = 0; j < tileList[i].colNum; j++)
		{
			bool find = false;
			Color tc = tileList[i].getColor(j);
			for (size_t k = 0; k < tempColRec.size(); k++)
			{
				if (tempColRec[k] == tc)
				{
					find = true;
					break;
				}
			}
			if (find == false)
			{
				addTest = true;
				tempColRec.push_back(tc);
			}
		}
		if (addTest || tileList[i].colNum > 2) pts.push_back(tileList[i]);
	}
	vector<Color>().swap(tempColRec);
	if (pts.size() == 0)
	{
		for (size_t i = 0; i < tileList.size(); i++)
		{
			if (tileList[i].pixelNum == 0) continue;
			pts.push_back(tileList[i]);
		}
	}
	if (pts.size() == 0) return -1;
	u8 palImageData[0x40];
	u32 colDiffRec = numeric_limits<u32>::max();
	vector<int> groupIndexRec(pts.size());
	vector<Color>().swap(tempColorRecord);

	// round 1
	//printf("\nRound 1:\n");
	KMeans km;
	for (u32 circle = 0; circle < times; circle++)
	{
		vector<Tile> outCents;
		km.kmcluster(pts, palList.size(), outCents);
		vector<Tile>().swap(outCents);
		vector<ImageData> resultList(palList.size());
		for (size_t i = 0; i < palList.size(); i++) resultList[i].initial(image.width, image.height, bg);
		for (size_t i = 0; i < pts.size(); i++) pts[i].draw(resultList[pts[i].id()]);
		if (bgKeep) for (size_t i = 0; i < palList.size(); i++) resultList[i].colRemove(bg);
		vector<liq_attr*> attrList(palList.size());
		for (size_t i = 0; i < attrList.size(); i++) attrList[i] = liq_attr_create();
		vector<liq_image*> liqImgList(palList.size());
		vector<liq_result*> liqResList(palList.size());
		for (size_t i = 0; i < palList.size(); i++)
		{
			liqImgList[i] = liq_image_create_rgba(attrList[i], resultList[i].ptr(), image.width, image.height, 0);
			if (bgKeep) liq_set_max_colors(attrList[i], colNum - 1); else liq_set_max_colors(attrList[i], colNum);
			liq_image_quantize(liqImgList[i], attrList[i], &liqResList[i]);
			resultList[i].release();
		}
		vector<ImageData>().swap(resultList);
		u32 colDiffTemp = 0;
		for (size_t i = 0; i < pts.size(); i++)
		{
			liq_attr *liqTileAttr = liq_attr_create();
			liq_image *liqTileImg = liq_image_create_rgba(liqTileAttr, pts[i].pixel, 8, 8, 0);
			liq_write_remapped_image(liqResList[pts[i].id()], liqTileImg, palImageData, 0x40);
			const liq_palette *pal = liq_get_palette(liqResList[pts[i].id()]);
			for (u32 k = 0; k < tileSize; k++)
			{
				if (bgKeep && bg == pts[i].pixel[k]) continue;
				colDiffTemp += abs(pts[i].pixel[k].r - pal->entries[palImageData[k]].r);
				colDiffTemp += abs(pts[i].pixel[k].g - pal->entries[palImageData[k]].g);
				colDiffTemp += abs(pts[i].pixel[k].b - pal->entries[palImageData[k]].b);
				colDiffTemp += abs(pts[i].pixel[k].a - pal->entries[palImageData[k]].a);
			}
			liq_attr_destroy(liqTileAttr);
			liq_image_destroy(liqTileImg);
		}
		if (colDiffTemp < colDiffRec)
		{
			//printf("Times: %d, CD: %d\n", circle, colDiffTemp / pts.size());
			colDiffRec = colDiffTemp;
			for (size_t i = 0; i < pts.size(); i++) groupIndexRec[i] = pts[i].id();
		}
		for (size_t i = 0; i < attrList.size(); i++) liq_attr_destroy(attrList[i]);
		vector<liq_attr*>().swap(attrList);
		for (size_t i = 0; i < liqImgList.size(); i++) liq_image_destroy(liqImgList[i]);
		vector<liq_image*>().swap(liqImgList);
		for (size_t i = 0; i < liqResList.size(); i++) liq_result_destroy(liqResList[i]);
		vector<liq_result*>().swap(liqResList);
	}
	for (size_t i = 0; i < pts.size(); i++) pts[i].setId(groupIndexRec[i]);

	// round 2
	//printf("\nRound 2:\n");
	vector<int> groupIndexRecTemp(pts.size());
	for (u32 circle = 0; circle < times; circle++)
	{
		vector<ImageData> resultList(palList.size());
		for (size_t i = 0; i < palList.size(); i++) resultList[i].initial(image.width, image.height, bg);
		for (size_t i = 0; i < pts.size(); i++) pts[i].draw(resultList[pts[i].id()]);
		if (bgKeep) for (size_t i = 0; i < palList.size(); i++) resultList[i].colRemove(bg);
		vector<liq_attr*> attrList(palList.size());
		for (size_t i = 0; i < attrList.size(); i++) attrList[i] = liq_attr_create();
		vector<liq_image*> liqImgList(palList.size());
		vector<liq_result*> liqResList(palList.size());
		for (size_t i = 0; i < palList.size(); i++)
		{
			liqImgList[i] = liq_image_create_rgba(attrList[i], resultList[i].ptr(), image.width, image.height, 0);
			if (bgKeep) liq_set_max_colors(attrList[i], colNum - 1); else liq_set_max_colors(attrList[i], colNum);
			liq_image_quantize(liqImgList[i], attrList[i], &liqResList[i]);
			resultList[i].release();
		}
		vector<ImageData>().swap(resultList);
		u32 colDiffTemp = 0;
		for (size_t i = 0; i < pts.size(); i++)
		{
			vector<u32> colDiffList(liqResList.size(), 0);
			for (size_t j = 0; j < liqResList.size(); j++)
			{
				liq_attr *liqTileAttr = liq_attr_create();
				liq_image *liqTileImg = liq_image_create_rgba(liqTileAttr, pts[i].pixel, 8, 8, 0);
				liq_write_remapped_image(liqResList[j], liqTileImg, palImageData, 0x40);
				const liq_palette *pal = liq_get_palette(liqResList[j]);
				for (u32 k = 0; k < tileSize; k++)
				{
					if (bgKeep && bg == pts[i].pixel[k]) continue;
					colDiffList[j] += abs(pts[i].pixel[k].r - pal->entries[palImageData[k]].r);
					colDiffList[j] += abs(pts[i].pixel[k].g - pal->entries[palImageData[k]].g);
					colDiffList[j] += abs(pts[i].pixel[k].b - pal->entries[palImageData[k]].b);
					colDiffList[j] += abs(pts[i].pixel[k].a - pal->entries[palImageData[k]].a);
				}
				liq_attr_destroy(liqTileAttr);
				liq_image_destroy(liqTileImg);
			}
			u32 palTest = 0;
			for (size_t j = 0; j < liqResList.size(); j++) if (colDiffList[j] < colDiffList[palTest]) palTest = j;
			colDiffTemp += colDiffList[palTest];
			groupIndexRecTemp[i] = palTest;
			vector<u32>().swap(colDiffList);
		}
		if (colDiffTemp < colDiffRec)
		{
			//printf("Times: %d, CD: %d\n", circle, colDiffTemp / pts.size());
			colDiffRec = colDiffTemp;
			for (size_t i = 0; i < pts.size(); i++) groupIndexRec[i] = pts[i].id();
		}
		for (size_t i = 0; i < pts.size(); i++) pts[i].setId(groupIndexRecTemp[i]);
		for (size_t i = 0; i < attrList.size(); i++) liq_attr_destroy(attrList[i]);
		vector<liq_attr*>().swap(attrList);
		for (size_t i = 0; i < liqImgList.size(); i++) liq_image_destroy(liqImgList[i]);
		vector<liq_image*>().swap(liqImgList);
		for (size_t i = 0; i < liqResList.size(); i++) liq_result_destroy(liqResList[i]);
		vector<liq_result*>().swap(liqResList);
	}
	for (size_t i = 0; i < pts.size(); i++) pts[i].setId(groupIndexRec[i]);
	vector<int>().swap(groupIndexRecTemp);
	vector<int>().swap(groupIndexRec);

	// palette match
	vector<ImageData> resultList(palList.size());
	for (size_t i = 0; i < palList.size(); i++) resultList[i].initial(image.width, image.height, bg);
	for (size_t i = 0; i < pts.size(); i++) pts[i].draw(resultList[pts[i].id()]);
	if (bgKeep) for (size_t i = 0; i < palList.size(); i++) resultList[i].colRemove(bg);
	vector<liq_attr*> attrList(palList.size());
	for (size_t i = 0; i < attrList.size(); i++) attrList[i] = liq_attr_create();
	vector<liq_image*> liqImgList(palList.size());
	vector<liq_result*> liqResList(palList.size());
	for (size_t i = 0; i < palList.size(); i++)
	{
		liqImgList[i] = liq_image_create_rgba(attrList[i], resultList[i].ptr(), image.width, image.height, 0);
		if (bgKeep) liq_set_max_colors(attrList[i], colNum - 1); else liq_set_max_colors(attrList[i], colNum);
		liq_image_quantize(liqImgList[i], attrList[i], &liqResList[i]);
	}
	for (size_t i = 0; i < resultList.size(); i++) resultList[i].release();
	vector<ImageData>().swap(resultList);
	vector<Tile>().swap(pts);
	vector<vector<u32>> groupStat(palList.size());
	for (size_t i = 0; i < tileList.size(); i++)
	{
		vector<u32> colDiffList(liqResList.size(), 0);
		for (size_t j = 0; j < liqResList.size(); j++)
		{
			liq_attr *liqTileAttr = liq_attr_create();
			liq_image *liqTileImg = liq_image_create_rgba(liqTileAttr, tileList[i].pixel, 8, 8, 0);
			liq_write_remapped_image(liqResList[j], liqTileImg, palImageData, 0x40);
			const liq_palette *pal = liq_get_palette(liqResList[j]);
			for (u32 k = 0; k < tileSize; k++)
			{
				if (bgKeep && bg == tileList[i].pixel[k]) continue;
				colDiffList[j] += abs(tileList[i].pixel[k].r - pal->entries[palImageData[k]].r);
				colDiffList[j] += abs(tileList[i].pixel[k].g - pal->entries[palImageData[k]].g);
				colDiffList[j] += abs(tileList[i].pixel[k].b - pal->entries[palImageData[k]].b);
				colDiffList[j] += abs(tileList[i].pixel[k].a - pal->entries[palImageData[k]].a);
			}
			liq_attr_destroy(liqTileAttr);
			liq_image_destroy(liqTileImg);
		}
		u32 palTest = 0;
		for (size_t j = 0; j < liqResList.size(); j++) if (colDiffList[j] < colDiffList[palTest]) palTest = j;
		tileList[i].setId(palTest);
		if (tileList[i].pixelNum > 0) groupStat[palTest].push_back(i);
		vector<u32>().swap(colDiffList);
	}

	// pal & tile construct
	//printf("\nResult:\n");
	colDiffRec = 0;
	vector<vector<u8>> tempTileRecordList(tileList.size(), vector<u8>(dataSize, 0));
	for (size_t i = 0; i < palList.size(); i++) vector<Color>().swap(palList[i]);
	vector<vector<Color>>().swap(palList);
	for (size_t i = 0; i < groupStat.size(); i++)
	{
		if (groupStat[i].size() == 0) continue;
		//printf("Group %d: %d\n", palList.size(), groupStat[i].size());
		const liq_palette *pal = liq_get_palette(liqResList[i]);
		vector<Color> palData;
		if (bgKeep) palData.push_back(bg);
		for (size_t j = 0; j < pal->count; j++)
		{

			Color colTemp = { pal->entries[j].r, pal->entries[j].g, pal->entries[j].b, pal->entries[j].a };
			if (bgKeep && bg == colTemp)
			{
				if (colTemp.b >= 128 && colTemp.b < 255) colTemp.b++;
				else if (colTemp.b < 128 && colTemp.b > 0) colTemp.b--;
				else if (colTemp.b == 255) colTemp.b--;
				else if (colTemp.b == 0) colTemp.b++;
			}
			palData.push_back(colTemp);
		}
		int bgIndex = -1;
		if (!bgKeep)
		{
			for (size_t j = 0; j < palData.size(); j++) if (bg == palData[j])
			{
				bgIndex = j;
				break;
			}
			if (bgIndex != -1)
			{
				palData[bgIndex] = palData[0];
				palData[0] = bg;
			}
		}
		palList.push_back(palData);
		vector<Color>().swap(palData);
		for (size_t j = 0; j < groupStat[i].size(); j++)
		{
			liq_attr *liqTileAttr = liq_attr_create();
			liq_image *liqTileImg = liq_image_create_rgba(liqTileAttr, tileList[groupStat[i][j]].pixel, 8, 8, 0);
			liq_write_remapped_image(liqResList[i], liqTileImg, palImageData, 0x40);
			for (u32 k = 0; k < tileSize; k++)
			{
				if (bgKeep && bg == tileList[groupStat[i][j]].pixel[k])
				{
					palImageData[k] = 0;
					continue;
				}
				colDiffRec += abs(tileList[groupStat[i][j]].pixel[k].r - pal->entries[palImageData[k]].r);
				colDiffRec += abs(tileList[groupStat[i][j]].pixel[k].g - pal->entries[palImageData[k]].g);
				colDiffRec += abs(tileList[groupStat[i][j]].pixel[k].b - pal->entries[palImageData[k]].b);
				colDiffRec += abs(tileList[groupStat[i][j]].pixel[k].a - pal->entries[palImageData[k]].a);
				if (bgKeep) palImageData[k]++;
				else if(bgIndex != -1)
				{
					if (palImageData[k] == bgIndex) palImageData[k] = 0;
					else if (palImageData[k] == 0) palImageData[k] = bgIndex;
				}
			}
			tileList[groupStat[i][j]].setId(palList.size() - 1);
			if (palMode == 1) for (size_t k = 0; k < dataSize; k++) tempTileRecordList[groupStat[i][j]][k] = palImageData[k];
			else for (size_t k = 0; k < dataSize; k++) tempTileRecordList[groupStat[i][j]][k] = (palImageData[k * 2 + 1] & 0xF) << 4 | palImageData[k * 2] & 0xF;
			liq_attr_destroy(liqTileAttr);
			liq_image_destroy(liqTileImg);
		}
	}
	for (size_t i = 0; i < palList.size(); i++) while (palList[i].size() < colNum) palList[i].push_back(bg);
	for (size_t i = 0; i < attrList.size(); i++) liq_attr_destroy(attrList[i]);
	vector<liq_attr*>().swap(attrList);
	for (size_t i = 0; i < liqImgList.size(); i++) liq_image_destroy(liqImgList[i]);
	vector<liq_image*>().swap(liqImgList);
	for (size_t i = 0; i < liqResList.size(); i++) liq_result_destroy(liqResList[i]);
	vector<liq_result*>().swap(liqResList);
	for (size_t i = 0; i < groupStat.size(); i++) vector<u32>().swap(groupStat[i]);
	vector<vector<u32>>().swap(groupStat);

	// index construct
	u32 ttw = tileWidth;
	if (palMode == 0) ttw = tileWidth / 2;
	u32 tth = tileHeight;
	for (size_t i = 0; i < tileList.size(); i++)
	{
		int tileIndex = 0;
		int flip = 0;
		bool find = false;
		for (size_t j = 0; j < tileData.size(); j++)
		{
			// normal
			find = true;
			for (size_t k = 0; k < tileData[j].size(); k++) if (tempTileRecordList[i][k] != tileData[j][k])
			{
				find = false;
				break;
			}
			if (find == true)
			{
				tileIndex = j;
				flip = 0;
				break;
			}
			// horizontal flip
			find = true;
			for (size_t k = 0; k < tileData[j].size(); k++)
			{
				u8 ttc = palMode == 0 ? ((tileData[j][k] & 0xF) << 4 | tileData[j][k] >> 4 & 0xF) : tileData[j][k];
				if (tempTileRecordList[i][k / ttw * ttw + ttw - 1 - k % ttw] != ttc)
				{
					find = false;
					break;
				}
			}
			if (find == true)
			{
				tileIndex = j;
				flip = 1;
				break;
			}
			// vertical flip
			find = true;
			for (size_t k = 0; k < tileData[j].size(); k++)
			{
				if (tempTileRecordList[i][(tth - 1 - k / ttw)*ttw + k % ttw] != tileData[j][k])
				{
					find = false;
					break;
				}
			}
			if (find == true)
			{
				tileIndex = j;
				flip = 2;
				break;
			}
			// hv flip
			find = true;
			for (size_t k = 0; k < tileData[j].size(); k++)
			{
				u8 ttc = palMode == 0 ? ((tileData[j][k] & 0xF) << 4 | tileData[j][k] >> 4 & 0xF) : tileData[j][k];
				if (tempTileRecordList[i][dataSize - k] != ttc)
				{
					find = false;
					break;
				}
			}
			if (find == true)
			{
				tileIndex = j;
				flip = 3;
				break;
			}
		}
		if (find == false)
		{
			tileIndex = tileData.size();
			tileData.push_back(tempTileRecordList[i]);
		}
		mapData.push_back((tileList[i].id() & 0xF) << 12 | (flip & 3) << 10 | (tileIndex & 0x3FF));
	}
	colDiffRec = (colDiffRec / imgOri.size) + 1;
	//printf("CD: %d\n", colDiffRec);
	for (size_t i = 0; i < tempTileRecordList.size(); i++) vector<u8>().swap(tempTileRecordList[i]);
	vector<vector<u8>>().swap(tempTileRecordList);
	imgOri.release();
	return colDiffRec;
}

int TMPToImage(ImageData &image, const vector<vector<unsigned char>> &tileData, const vector<unsigned short> &mapData, const vector<vector<Color>> &palList,
	unsigned int width, unsigned int height, unsigned char tileMode, unsigned char palMode, unsigned char imgMode)
{
	// tileMode: 0: 8*8, 1: 8*4, 2: 4*4, 3: 4*2, 4: 2*2, 5: 2*1, 6: 1*1
	// palMode: 0: 4bit, 1: 8bit
	if (width * height == 0 || tileMode > 6 || palMode > 1) return -1;

	// features calculate
	u32 tileWidth = 8;
	u32 tileHeight = 8;
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
	u32 tileSize = tileWidth * tileHeight;
	u32 width_ori = width;
	u32 height_ori = height;
	width = (u32)ceil((float)width / (float)tileWidth)*tileWidth;
	height = (u32)ceil((float)height / (float)tileHeight)*tileHeight;

	Tile tile(tileMode);
	image.initial(width, height, { 0,0,0,0 }, imgMode);
	int tileId = 0;
	for (size_t i = 0; i < mapData.size(); i++)
	{
		u8 flip = (mapData[i] >> 10) & 3;
		u8 palGroup = (mapData[i] >> 12) & 0xF;
		u16 tileIndex = mapData[i] & 0x3FF;
		if (tileIndex >= tileData.size() || palGroup >= palList.size())
		{
			image.release();
			return -1;
		}
		if (palMode == 0)
		{
			for (auto term : palList) if (term.size() < 0x10)
			{
				image.release();
				return -1;
			}
			for (int j = 0; j < tile.size / 2; j++)
			{
				u8 ida = tileData[tileIndex][j] & 0xF;
				u8 idb = (tileData[tileIndex][j] >> 4) & 0xF;
				tile.pixel[j * 2] = palList[palGroup][ida];
				tile.pixel[j * 2 + 1] = palList[palGroup][idb];
			}
		}
		else if (palMode == 1)
		{
			for (auto term : palList) if (term.size() < 0x100)
			{
				image.release();
				return -1;
			}
			for (int j = 0; j < tile.size; j++) tile.pixel[j] = palList[palGroup][tileData[tileIndex][j]];
		}
		tile.tileId = tileId++;
		tile.draw(image, flip);
	}
	image.resize(width_ori, height_ori);
	return 0;
}

int ImageToIP(const ImageData &image, vector<unsigned char> &index, vector<Color> &palList, Color bg, unsigned char palMode, unsigned int colNum, bool bgKeep)
{
	// palMode: 0: 4bit, 1: 8bit
	if ((palMode == 0 && colNum > 16) || (palMode == 1 && colNum > 256) || colNum == 0 || (colNum == 1 && bgKeep) || image.size == 0 || palMode > 1) return -1;
	ImageData imgOri(image);
	ImageData imgTemp(image);
	if (bgKeep) imgTemp.colRemove(bg);
	liq_attr *liqAttr = liq_attr_create();
	liq_image *liqImage = liq_image_create_rgba(liqAttr, imgTemp.ptr(), imgTemp.width, imgTemp.height, 0);
	if (bgKeep) liq_set_max_colors(liqAttr, colNum - 1); else liq_set_max_colors(liqAttr, colNum);
	liq_result *liqResult;
	liq_image_quantize(liqImage, liqAttr, &liqResult);
	vector<Color>().swap(palList);
	if (bgKeep) palList.push_back(bg);
	const liq_palette *pal = liq_get_palette(liqResult);
	liq_attr *liqAttrRemap = liq_attr_create();
	liq_image *liqImgRemap = liq_image_create_rgba(liqAttrRemap, imgOri.ptr(), imgOri.width, imgOri.height, 0);
	u8 *tmpData = (u8 *)malloc(imgOri.size);
	liq_write_remapped_image(liqResult, liqImgRemap, tmpData, imgOri.width * imgOri.height);
	for (u32 i = 0; i < pal->count; i++)
	{
		Color colTemp = { pal->entries[i].r, pal->entries[i].g, pal->entries[i].b, pal->entries[i].a };
		if (bgKeep && bg == colTemp)
		{
			if (colTemp.b >= 128 && colTemp.b < 255) colTemp.b++;
			else if (colTemp.b < 128 && colTemp.b > 0) colTemp.b--;
			else if (colTemp.b == 255) colTemp.b--;
			else if (colTemp.b == 0) colTemp.b++;
		}
		palList.push_back(colTemp);
	}
	if (bgKeep) for (u32 i = 0; i < imgOri.size; i++)
	{
		Color colTemp = imgOri.get(i);
		if (bg == colTemp) tmpData[i] = 0; else tmpData[i]++;
	}
	else
	{
		int bgIndexChange = -1;
		for (u32 i = 0; i < palList.size(); i++) if (bg == palList[i])
		{
			bgIndexChange = i;
			break;
		}
		if (bgIndexChange != -1)
		{
			for (u32 i = 0; i < imgOri.size; i++) if (tmpData[i] == bgIndexChange) tmpData[i] = 0; else if (tmpData[i] == 0) tmpData[i] = bgIndexChange;
			palList[bgIndexChange] = palList[0];
			palList[0] = bg;
		}
	}
	if (palMode == 0)
	{
		vector<u8>(imgOri.size / 2).swap(index);
		for (size_t i = 0; i < index.size(); i++) index[i] = (tmpData[i * 2 + 1] & 0xF) << 4 | (tmpData[i * 2] & 0xF);
	}
	else
	{
		vector<u8>(imgOri.size).swap(index);
		for (size_t i = 0; i < index.size(); i++) index[i] = tmpData[i];
	}
	free(tmpData);
	liq_attr_destroy(liqAttr);
	liq_attr_destroy(liqAttrRemap);
	liq_image_destroy(liqImage);
	liq_image_destroy(liqImgRemap);
	imgTemp.release();
	imgOri.release();
	return palList.size();
}

int IPToImage(ImageData &image, const vector<unsigned char> &index, const vector<Color> &palList,
	unsigned int width, unsigned int height, unsigned char palMode, unsigned char imgMode)
{
	// palMode: 0: 4bit, 1: 8bit
	unsigned int size = width * height;
	if (palMode == 0) size /= 2;
	if (index.size() == 0 || size != index.size() || palMode > 1) return -1;
	image.initial(width, height, { 0,0,0,0 }, imgMode);
	if (palMode == 0)
	{
		if (palList.size() < 0x10)
		{
			image.release();
			return -1;
		}
		for (size_t i = 0; i < index.size(); i++)
		{
			u8 ida = index[i] & 0xF;
			u8 idb = (index[i] >> 4) & 0xF;
			image.set(palList[ida], i * 2);
			image.set(palList[idb], i * 2 + 1);
		}
	}
	else if (palMode == 1)
	{
		if (palList.size() < 0x100)
		{
			image.release();
			return -1;
		}
		for (size_t i = 0; i < index.size(); i++) image.set(palList[index[i]], i);
	}
	return 0;
}

int ImageToTex(const ImageData &image, vector<unsigned char> &texData, TileTexFormat texMode, unsigned char tileMode, bool swizzle, unsigned char endian)
{
	// tileMode: 0: 8*8, 1: 8*4, 2: 4*4, 3: 4*2, 4: 2*2, 5: 2*1, 6: 1*1
	if (image.size == 0 || tileMode > 6) return -1;

	// features calculate
	ImageData imgOri(image);
	u32 tileWidth = 8;
	u32 tileHeight = 8;
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
	u32 tileSize = tileWidth * tileHeight;
	imgOri.resize((u32)ceil((float)imgOri.width / (float)tileWidth)*tileWidth, (u32)ceil((float)imgOri.height / (float)tileHeight)*tileHeight);
	vector<u8> dst;
	u8 recordIndex = 0;
	vector<Color> tempColorRecord(tileSize);
	for (size_t i = 0; i < imgOri.size; i++)
	{
		u32 x = tileWidth * ((i / tileSize) % (imgOri.width / tileWidth)) + (i % tileSize) % tileWidth;
		u32 y = tileHeight * ((i / tileSize) / (imgOri.width / tileWidth)) + (i % tileSize) / tileWidth;
		tempColorRecord[recordIndex++] = imgOri.get(x, y);
		if (recordIndex == tileSize)
		{
			for (size_t j = 0; j < tileSize; j++)
			{
				dst.push_back(tempColorRecord[j].a);
				dst.push_back(tempColorRecord[j].r);
			}
			for (size_t j = 0; j < tileSize; j++)
			{
				dst.push_back(tempColorRecord[j].g);
				dst.push_back(tempColorRecord[j].b);
			}
			recordIndex = 0;
		}
	}
	int result = TextureCov(dst, texData, RGBA8, texMode, imgOri.width, imgOri.height, tileMode, tileMode, 0, swizzle, 0, endian);
	vector<u8>().swap(dst);
	vector<Color>().swap(tempColorRecord);
	imgOri.release();
	return result;
}

int TexToImage(ImageData &image, const vector<unsigned char> &texData, TileTexFormat texMode,
	unsigned int width, unsigned int height, unsigned char tileMode, bool swizzle, unsigned char endian)
{
	// tileMode: 0: 8*8, 1: 8*4, 2: 4*4, 3: 4*2, 4: 2*2, 5: 2*1, 6: 1*1
	if (width * height == 0 || tileMode > 6) return -1;

	// features calculate
	u32 tileWidth = 8;
	u32 tileHeight = 8;
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
	u32 tileSize = tileWidth * tileHeight;
	u32 width_ori = width;
	u32 height_ori = height;
	width = (u32)ceil((float)width / (float)tileWidth)*tileWidth;
	height = (u32)ceil((float)height / (float)tileHeight)*tileHeight;

	vector<u8> dst(width * height * 4);
	image.initial(width, height);
	if (TextureCov(texData, dst, texMode, RGBA8, width, height, tileMode, tileMode, swizzle, 0, endian, 0) != 0) return -1;
	u32 halfSize = tileSize * 2;
	vector<u8> tempTileRecord(tileSize * 4);
	for (size_t i = 0; i < dst.size(); i += tempTileRecord.size())
	{
		for (size_t j = 0; j < tempTileRecord.size(); j++) tempTileRecord[j] = dst[i + j];
		for (size_t j = 0; j < tileSize; j++)
		{
			u32 index = i / 4 + j;
			u32 x = tileWidth * ((index / tileSize) % (image.width / tileWidth)) + (index % tileSize) % tileWidth;
			u32 y = tileHeight * ((index / tileSize) / (image.width / tileWidth)) + (index % tileSize) / tileWidth;
			image.set({ tempTileRecord[j * 2 + 1], tempTileRecord[j * 2 + halfSize], tempTileRecord[j * 2 + halfSize + 1], tempTileRecord[j * 2] }, x, y);
		}
	}
	vector<u8>().swap(dst);
	vector<u8>().swap(tempTileRecord);
	image.resize(width_ori, height_ori);
	return 0;
}

int PalMatch(const ImageData &image, const vector<vector<Color>> &palList, vector<unsigned char> &dst, unsigned char palMode, unsigned int def)
{
	if (palMode > 1 || image.size == 0 || image.size % 2 != 0 || palList.size() == 0 || palList[0].size() == 0) return -1;
	if (def >= palList.size()) def = 0;
	if (palMode == 0)
	{
		for (auto term : palList) if (term.size() > 0x10) return -1;
		if (dst.size() == image.size / 2) 
		{
			bool test = true;
			for (size_t i = 0; i < dst.size(); i++)
			{
				unsigned int a = dst[i] & 0xF, b = dst[i] >> 4 & 0xF;
				if (a >= palList[def].size() || b >= palList[def].size() || palList[def][a] != image.get(i * 2) || palList[def][b] != image.get(i * 2 + 1))
				{
					test = false;
					break;
				}
			}
			if (test) return def;
		}
	}
	else if (dst.size() == image.size)
	{
		bool test = true;
		for (size_t i = 0; i < dst.size(); i++) if (dst[i] >= palList[def].size() || palList[def][dst[i]] != image.get(i))
		{
			test = false;
			break;
		}
		if (test) return def;
	}
	ImageData imageMod;
	image.clone(imageMod);
	vector<liq_attr*> attrList(palList.size());
	for (size_t i = 0; i < attrList.size(); i++) attrList[i] = liq_attr_create();
	vector<liq_image*> liqImgList(palList.size());
	vector<liq_result*> liqResList(palList.size());
	vector<vector<u8>> mapResList(palList.size(), vector<u8>(imageMod.size, 0));
	vector<u32> colDiffList(liqResList.size(), 0);
	u8 *mapRes = (u8*)malloc(imageMod.size);
	for (size_t i = 0; i < palList.size(); i++)
	{
		liqImgList[i] = liq_image_create_rgba(attrList[i], imageMod.ptr(), imageMod.width, imageMod.height, 0);
		for (size_t j = 0; j < palList[i].size(); j++) liq_image_add_fixed_color(liqImgList[i], { palList[i][j].r, palList[i][j].g, palList[i][j].b, palList[i][j].a });
		liq_set_max_colors(attrList[i], palList[i].size());
		liq_image_quantize(liqImgList[i], attrList[i], &liqResList[i]);
		liq_write_remapped_image(liqResList[i], liqImgList[i], mapRes, image.size);
		for (u32 j = 0; j < imageMod.size; j++)
		{
			colDiffList[i] += abs(imageMod.get(j).r - palList[i][mapRes[j]].r);
			colDiffList[i] += abs(imageMod.get(j).g - palList[i][mapRes[j]].g);
			colDiffList[i] += abs(imageMod.get(j).b - palList[i][mapRes[j]].b);
			colDiffList[i] += abs(imageMod.get(j).a - palList[i][mapRes[j]].a);
			mapResList[i][j] = mapRes[j];
		}
	}
	u32 palTest = def;
	for (size_t i = 0; i < palList.size(); i++) if (colDiffList[i] < colDiffList[palTest]) palTest = i;
	if (palMode == 0)
	{
		vector<u8>().swap(dst);
		for (size_t i = 0; i < mapResList[palTest].size(); i += 2) dst.push_back(((mapResList[palTest][i + 1] & 0xF) << 4 | mapResList[palTest][i] & 0xF) & 0xFF);
	}
	else mapResList[palTest].swap(dst);
	imageMod.release();
	vector<u32>().swap(colDiffList);
	for (size_t i = 0; i < attrList.size(); i++) liq_attr_destroy(attrList[i]);
	vector<liq_attr*>().swap(attrList);
	for (size_t i = 0; i < liqImgList.size(); i++) liq_image_destroy(liqImgList[i]);
	vector<liq_image*>().swap(liqImgList);
	for (size_t i = 0; i < liqResList.size(); i++) liq_result_destroy(liqResList[i]);
	vector<liq_result*>().swap(liqResList);
	for (size_t i = 0; i < mapResList.size(); i++) vector<u8>().swap(mapResList[i]);
	vector<vector<u8>>().swap(mapResList);
	return palTest;
}

vector<vector<unsigned char>> TileConv(const vector<vector<unsigned char>> &src, unsigned int srcTileCol, 
	unsigned char srcMode, unsigned char dstMode, unsigned char palMode, unsigned char hvMode)
{
	if (srcMode > 6 || dstMode > 6 || src.size() == 0 || src.size() % srcTileCol != 0) return vector<vector<u8>>();
	u32 srcTileWidth = 8, srcTileHeight = 8;
	switch (srcMode)
	{
	case 1: srcTileHeight = 4; break;
	case 2: srcTileWidth = 4; srcTileHeight = 4; break;
	case 3: srcTileWidth = 4; srcTileHeight = 2; break;
	case 4: srcTileWidth = 2; srcTileHeight = 2; break;
	case 5: srcTileWidth = 2; srcTileHeight = 1; break;
	case 6: srcTileWidth = 1; srcTileHeight = 1; break;
	default:;
	}
	u32 dstTileWidth = 8, dstTileHeight = 8;
	switch (dstMode)
	{
	case 1: dstTileHeight = 4; break;
	case 2: dstTileWidth = 4; dstTileHeight = 4; break;
	case 3: dstTileWidth = 4; dstTileHeight = 2; break;
	case 4: dstTileWidth = 2; dstTileHeight = 2; break;
	case 5: dstTileWidth = 2; dstTileHeight = 1; break;
	case 6: dstTileWidth = 1; dstTileHeight = 1; break;
	default:;
	}
	if (palMode == 0)
	{
		srcTileWidth /= 2;
		dstTileWidth /= 2;
	}
	if (srcTileWidth == 0 || dstTileWidth == 0 || srcTileWidth * srcTileHeight != src[0].size())  return vector<vector<u8>>();
	u32 srcWidth = srcTileWidth * srcTileCol;
	u32 srcHeight = srcTileHeight * src.size() / srcTileCol;
	if (srcWidth % dstTileWidth != 0 || srcHeight % dstTileHeight != 0 || srcWidth * srcHeight != src.size()*src[0].size()) return vector<vector<u8>>();
	u32 dstTileCol = srcWidth / dstTileWidth;
	vector<vector<u8>> dst(dstTileCol*srcHeight / dstTileHeight, vector<u8>(dstTileWidth*dstTileHeight, 0));
	unsigned int srcId, srcPos, dstId, dstPos, dstX, dstY;
	for (u32 x = 0; x < srcWidth; x++) for (u32 y = 0; y < srcHeight; y++)
	{
		dstX = hvMode & 1 ? srcWidth - x - 1 : x;
		dstY = (hvMode >> 1) & 1 ? srcHeight - y - 1 : y;
		srcId = (y / srcTileHeight)*srcTileCol + x / srcTileWidth;
		srcPos = (y % srcTileHeight)*srcTileWidth + x % srcTileWidth;
		dstId = (dstY / dstTileHeight)*dstTileCol + dstX / dstTileWidth;
		dstPos = (dstY % dstTileHeight)*dstTileWidth + dstX % dstTileWidth;
		dst[dstId][dstPos] = src[srcId][srcPos];
	}
	return dst;
}

int TileMatch(const vector<vector<unsigned char>> &src, const vector<vector<unsigned char>> &tiles, unsigned int align)
{
	if (src.size() == 0 || src.size() > tiles.size() || src[0].size() != tiles[0].size()) return -1;
	int index = -1;
	for (size_t i = 0; i <= tiles.size() - src.size(); i++)
	{
		if (align > 0 && i % align != 0) continue;
		index = i;
		for (size_t j = 0; j < src.size(); j++)
		{
			for (size_t t = 0; t < src[j].size(); t++) if (tiles[i + j][t] != src[j][t])
			{
				index = -1;
				break;
			}
			if (index == -1) break;
		}
		if (index == i) return index;
	}
	return -1;
}
