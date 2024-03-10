#include "optHandle.h"

#pragma warning(disable : 4996)

OPT::OPT(int argc, char* argv[])
{
	initial();
	int result = load(argc, argv);
	if (result != 0)
	{
		switch (result)
		{
		case -2: help(); break;
		case -3: printf("different bg colors!\n"); break;
		default: printf("parameter error!\n");
		}
		release();
		initial();
	}
}

OPT::~OPT()
{
	release();
}

void OPT::help()
{
	printf("====================\n");
	printf("imageConv\nv1.8 by CBH\n");
	printf("====================\n\n");
	printf("Usage: imageConv <--in> <type> [options...] <--out> <type> [options...]\n\n");
	printf("\n--------------------\nType & Options:\n--------------------\n");
	printf("IMG - image\n");
	printf("-bg <color> : color of background, format of color: r,g,b,a (e.g. 255,255,255,255)\n");
	printf("-type <type> : format of image file: 0 - png, 1 - bmp (default 0, unused in term in)\n");
	printf("-img <name> : name of image file\n");
	printf("\n");
	printf("TMP - tile, map & palette\n");
	printf("-bg <color> : color of background, format of color: r,g,b,a (e.g. 255,255,255,255)\n");
	printf("-tile <name> : name of tile file\n");
	printf("-map <name> : name of map file\n");
	printf("-pal <name> : name of palette file\n");
	printf("-tm <mode> : mode of tile (default 0)\n");
	printf("-cm <number> : mode of color (default 6)\n");
	printf("-cn <mode> : number of color in palette block (default&max 16 when 4bpp, 256 when 8bpp)\n");
	printf("-pm <number> : mode of palette block (default 0)\n");
	printf("-pn <number> : number of palette block (default&max 16 when 4bpp, 1 when 8bpp)\n");
	printf("-pd <mode> : palette padding, 0 - false, 1 - true (default 0)\n");
	printf("-t <number> : times of cluster (default 50)\n");
	printf("-ed <mode> : endian: 0 - little, 1 - big (default 0)\n");
	printf("-rev <mode> : reverse red and blue, 0 - false, 1 - true (default 0)\n");
	printf("-w <number> : width of image (unused as out)\n");
	printf("-h <number> : height of image (unused as out)\n");
	printf("\n");
	printf("IP - index & palette\n");
	printf("-bg <color> : color of background, format of color: r,g,b,a (e.g. 255,255,255,255)\n");
	printf("-idx <name> : name of index file\n");
	printf("-pal <name> : name of palette file\n");
	printf("-cm <number> : mode of color (default 6)\n");
	printf("-cn <mode> : number of color in palette block (default&max 16 when 4bpp, 256 when 8bpp)\n");
	printf("-pm <number> : mode of palette block (default 0)\n");
	printf("-pd <mode> : palette padding, 0 - false, 1 - true (default 0)\n");
	printf("-ed <mode> : endian: 0 - little, 1 - big (default 0)\n");
	printf("-rev <mode> : reverse red and blue, 0 - false, 1 - true (default 0)\n");
	printf("-w <number> : width of image (unused as out)\n");
	printf("-h <number> : height of image (unused as out)\n"); 
	printf("\n");
	printf("TT - tile texture\n");
	printf("-bg <color> : color of background, format of color: r,g,b,a (e.g. 255,255,255,255)\n");
	printf("-tex <name> : name of texture file\n");
	printf("-txm <mode> : mode of texture\n");
	printf("-tm <mode> : mode of tile (default 0)\n");
	printf("-sw <mode> : swizzle: 0 - false, 1 - true (default 0)\n");
	printf("-ed <mode> : endian: 0 - little, 1 - big (default 0)\n");
	printf("-w <number> : width of image (unused as out)\n");
	printf("-h <number> : height of image (unused as out)\n");
	printf("\n");
	printf("--------------------\nMode Parameter:\n--------------------\n");
	printf("type - 0:png, 1:bmp, 2:tga\n");
	printf("tm - 0:8*8, 1:8*4, 2:4*4, 3:4*2, 4:2*2, 5:2*1, 6:1*1\n");
	printf("pm - 0:4bpp, 1 : 8bpp\n");
	printf("txm/cm - 0:I4, 1:I8, 2:IA4, 3:IA8, 4:RGB565, 5:RGB5A3, 6:RGBA8, 7:RGB555, 14:CMPR\n");
}

void OPT::initial()
{
	modeIn = NA;
	modeOut = NA;
	typeIn = NULL;
	typeOut = NULL;
	termIn = (TermInfo *)malloc(0x100);
	termOut = (TermInfo *)malloc(0x100);
	memset(termIn, 0, 0x100);
	memset(termOut, 0, 0x100);
}

void OPT::release()
{
	if (termIn != 0) free(termIn);
	if (termOut != 0) free(termOut);
	termIn = NULL;
	termOut = NULL;
	vector<char *>().swap(termListIn);
	vector<char *>().swap(termListOut);
}

int OPT::load(int argc, char* argv[])
{
	if (argc < 5) return -2;
	if (argc % 2 == 0) return -1;
	vector<char *>().swap(termListIn);
	vector<char *>().swap(termListOut);
	for (int i = 1; i < argc; i+=2)
	{
		if (strcmp(argv[i], "--in") == 0)
		{
			if (typeIn != NULL) return -1;
			typeIn = argv[i + 1];
			int j = i + 2;
			while (j < argc && argv[j][1] != '-')
			{
				if (strlen(argv[j]) > 1 && argv[j][0] == '-')
				{
					termListIn.push_back(argv[j]);
					termListIn.push_back(argv[j + 1]);
				}
				j += 2;
			}
		}
		else if (strcmp(argv[i], "--out") == 0)
		{
			if (typeOut != NULL) return -1;
			typeOut = argv[i + 1];
			int j = i + 2;
			while (j < argc && argv[j][1] != '-')
			{
				if (strlen(argv[j]) > 1 && argv[j][0] == '-')
				{
					termListOut.push_back(argv[j]);
					termListOut.push_back(argv[j + 1]);
				}
				j += 2;
			}
		}
		else if (strcmp(argv[i], "--help") == 0) return -2;
	}
	optHandle(typeIn, termListIn, 0);
	optHandle(typeOut, termListOut, 1);

	// test
	if (termIn->bgFlag == 1 && termOut->bgFlag == 1)
	{
		if (termIn->bg.r != termOut->bg.r || termIn->bg.g != termOut->bg.g || termIn->bg.b != termOut->bg.b || termIn->bg.a != termOut->bg.a) return -3;
	}

	return 0;
}

TermInfo *OPT::getTerm(int mode)
{
	TermInfo *result = NULL;
	if (mode == 0) result = termIn;
	else if (mode == 1) result = termOut;
	return result;
}

int OPT::optHandle(char* type, vector<char *> &subTermList, int mode)
{
	HandleMode hm = NA;
	if (strcmp(type, "IMG") == 0) hm = IMG;
	else if (strcmp(type, "TMP") == 0) hm = TMP;
	else if (strcmp(type, "IP") == 0) hm = IP;
	else if (strcmp(type, "TT") == 0) hm = TT;
	void *term = mode == 0 ? termIn : termOut;
	if (mode == 0) modeIn = hm; else modeOut = hm;
	int result = -1;
	switch (hm)
	{
	case IMG: result = IMGHandle((IMGInfo *)term, subTermList); break;
	case TMP: result = TMPHandle((TMPInfo *)term, subTermList); break;
	case IP: result = IPHandle((IPInfo *)term, subTermList); break;
	case TT: result = TTHandle((TTInfo *)term, subTermList); break;
	default:;
	}
	return result;
}

int OPT::IMGHandle(IMGInfo *term, vector<char *> &subTermList)
{
	term->bgFlag = 0;
	for (size_t i = 0; i < subTermList.size(); i += 2)
	{
		if (strcmp(subTermList[i], "-bg") == 0)
		{
			int r, g, b, a;
			if (sscanf(subTermList[i + 1], "%d,%d,%d,%d", &r, &g, &b, &a) == 4)
			{
				term->bgFlag = 1;
				term->bg.r = r & 0xFF;
				term->bg.g = g & 0xFF;
				term->bg.b = b & 0xFF;
				term->bg.a = a & 0xFF;
			}
		}
		else if (strcmp(subTermList[i], "-img") == 0) term->imgName = subTermList[i + 1];
		else if (strcmp(subTermList[i], "-type") == 0) term->type = atoi(subTermList[i + 1]);
	}
	return 0;
}

int OPT::TMPHandle(TMPInfo *term, vector<char *> &subTermList)
{
	term->bgFlag = 0;
	term->colMode = 6;
	for (size_t i = 0; i < subTermList.size(); i += 2)
	{
		if (strcmp(subTermList[i], "-bg") == 0)
		{
			int r, g, b, a;
			if (sscanf(subTermList[i + 1], "%d,%d,%d,%d", &r, &g, &b, &a) == 4)
			{
				term->bgFlag = 1;
				term->bg.r = r & 0xFF;
				term->bg.g = g & 0xFF;
				term->bg.b = b & 0xFF;
				term->bg.a = a & 0xFF;
			}
		}
		else if (strcmp(subTermList[i], "-tile") == 0) term->tileName = subTermList[i + 1];
		else if (strcmp(subTermList[i], "-map") == 0) term->mapName = subTermList[i + 1];
		else if (strcmp(subTermList[i], "-pal") == 0) term->palName = subTermList[i + 1];
		else if (strcmp(subTermList[i], "-tm") == 0) term->tileMode = atoi(subTermList[i + 1]);
		else if (strcmp(subTermList[i], "-cm") == 0) term->colMode = atoi(subTermList[i + 1]);
		else if (strcmp(subTermList[i], "-cn") == 0) term->colNum = atoi(subTermList[i + 1]);
		else if (strcmp(subTermList[i], "-pm") == 0) term->palMode = atoi(subTermList[i + 1]);
		else if (strcmp(subTermList[i], "-pn") == 0) term->palNum = atoi(subTermList[i + 1]);
		else if (strcmp(subTermList[i], "-pd") == 0) term->pad = atoi(subTermList[i + 1]);
		else if (strcmp(subTermList[i], "-t") == 0) term->times = atoi(subTermList[i + 1]);
		else if (strcmp(subTermList[i], "-ed") == 0) term->endian = atoi(subTermList[i + 1]);
		else if (strcmp(subTermList[i], "-rev") == 0) term->reverse = atoi(subTermList[i + 1]);
		else if (strcmp(subTermList[i], "-w") == 0) term->width = atoi(subTermList[i + 1]);
		else if (strcmp(subTermList[i], "-h") == 0) term->height = atoi(subTermList[i + 1]);
	}
	if (term->times == 0) term->times = 50;
	if ((term->colNum == 0 || term->colNum > 16) && term->palMode == 0) term->colNum = 16;
	if ((term->colNum == 0 || term->colNum > 256) && term->palMode == 1) term->colNum = 256;
	if ((term->palNum == 0 || term->palNum > 16) && term->palMode == 0) term->palNum = 16;
	if ((term->palNum == 0 || term->palNum > 1) && term->palMode == 1) term->palNum = 1;
	if (term->reverse > 0) term->reverse = 1;
	if (term->pad > 0) term->pad = term->palMode == 0 ? 16 : 256;
	return 0;
}

int OPT::IPHandle(IPInfo *term, vector<char *> &subTermList)
{
	term->bgFlag = 0;
	term->colMode = 6;
	for (size_t i = 0; i < subTermList.size(); i += 2)
	{
		if (strcmp(subTermList[i], "-bg") == 0)
		{
			int r, g, b, a;
			if (sscanf(subTermList[i + 1], "%d,%d,%d,%d", &r, &g, &b, &a) == 4)
			{
				term->bgFlag = 1;
				term->bg.r = r & 0xFF;
				term->bg.g = g & 0xFF;
				term->bg.b = b & 0xFF;
				term->bg.a = a & 0xFF;
			}
		}
		else if (strcmp(subTermList[i], "-idx") == 0) term->indexName = subTermList[i + 1];
		else if (strcmp(subTermList[i], "-pal") == 0) term->palName = subTermList[i + 1];
		else if (strcmp(subTermList[i], "-cm") == 0) term->colMode = atoi(subTermList[i + 1]);
		else if (strcmp(subTermList[i], "-cn") == 0) term->colNum = atoi(subTermList[i + 1]);
		else if (strcmp(subTermList[i], "-pm") == 0) term->palMode = atoi(subTermList[i + 1]);
		else if (strcmp(subTermList[i], "-pd") == 0) term->pad = atoi(subTermList[i + 1]);
		else if (strcmp(subTermList[i], "-ed") == 0) term->endian = atoi(subTermList[i + 1]);
		else if (strcmp(subTermList[i], "-rev") == 0) term->reverse = atoi(subTermList[i + 1]);
		else if (strcmp(subTermList[i], "-w") == 0) term->width = atoi(subTermList[i + 1]);
		else if (strcmp(subTermList[i], "-h") == 0) term->height = atoi(subTermList[i + 1]);
	}
	if ((term->colNum == 0 || term->colNum > 16) && term->palMode == 0) term->colNum = 16;
	if ((term->colNum == 0 || term->colNum > 256) && term->palMode == 1) term->colNum = 256;
	if (term->reverse > 0) term->reverse = 1;
	if (term->pad > 0) term->pad = term->palMode == 0 ? 16 : 256;
	return 0;
}

int OPT::TTHandle(TTInfo *term, vector<char *> &subTermList)
{
	term->bgFlag = 0;
	for (size_t i = 0; i < subTermList.size(); i += 2)
	{
		if (strcmp(subTermList[i], "-bg") == 0)
		{
			int r, g, b, a;
			if (sscanf(subTermList[i + 1], "%d,%d,%d,%d", &r, &g, &b, &a) == 4)
			{
				term->bgFlag = 1;
				term->bg.r = r & 0xFF;
				term->bg.g = g & 0xFF;
				term->bg.b = b & 0xFF;
				term->bg.a = a & 0xFF;
			}
		}
		else if (strcmp(subTermList[i], "-tex") == 0) term->texName = subTermList[i + 1];
		else if (strcmp(subTermList[i], "-txm") == 0) term->texMode = atoi(subTermList[i + 1]);
		else if (strcmp(subTermList[i], "-tm") == 0) term->tileMode = atoi(subTermList[i + 1]);
		else if (strcmp(subTermList[i], "-sw") == 0) term->swizzle = atoi(subTermList[i + 1]);
		else if (strcmp(subTermList[i], "-ed") == 0) term->endian = atoi(subTermList[i + 1]);
		else if (strcmp(subTermList[i], "-w") == 0) term->width = atoi(subTermList[i + 1]);
		else if (strcmp(subTermList[i], "-h") == 0) term->height = atoi(subTermList[i + 1]);
	}
	return 0;
}
