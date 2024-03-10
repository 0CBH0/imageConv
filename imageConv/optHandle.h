#pragma once

#include <vector>
#include "imgConvLib.h"
#include "type.h"

using namespace std;

enum HandleMode
{
	NA,
	TMP,
	IP,
	IMG,
	TT
};

struct TermInfo
{
	Color bg;
	int bgFlag;
};

struct IMGInfo
{
	Color bg;
	int bgFlag;
	int type;
	char *imgName;
};

struct TMPInfo
{
	Color bg;
	int bgFlag;
	int tileMode;
	int colMode;
	int colNum;
	int palMode;
	int palNum;
	int pad;
	int times;
	int endian;
	int reverse;
	int width;
	int height;
	char *tileName;
	char *mapName;
	char *palName;
};

struct IPInfo
{
	Color bg;
	int bgFlag;
	int colMode;
	int colNum;
	int palMode;
	int pad;
	int endian;
	int reverse;
	int width;
	int height;
	char *indexName;
	char *palName;
};

struct TTInfo
{
	Color bg;
	int bgFlag;
	int texMode;
	int tileMode;
	int swizzle;
	int endian;
	int width;
	int height;
	char *texName;
};

class OPT
{
public:
	OPT(int argc, char* argv[]);
	~OPT();
	void initial();
	void release();
	int load(int argc, char* argv[]);
	int optHandle(char* type, vector<char *> &subTermList, int mode);
	TermInfo *getTerm(int mode);

	HandleMode modeIn;
	HandleMode modeOut;

private:
	int IMGHandle(IMGInfo *term, vector<char *> &subTermList);
	int TMPHandle(TMPInfo *term, vector<char *> &subTermList);
	int IPHandle(IPInfo *term, vector<char *> &subTermList);
	int TTHandle(TTInfo *term, vector<char *> &subTermList);
	void help();

	TermInfo *termIn;
	TermInfo *termOut;
	char *typeIn;
	char *typeOut;
	vector<char *> termListIn;
	vector<char *> termListOut;
};
