#pragma once

#include <time.h>
#include <math.h>
#include <libimagequant.h>
#include "imgConvLib.h"

struct HSV
{
	HSV();
	HSV(Color rgb);
	void set(Color rgb);
	double h;
	double s;
	double v;
	double a;
};

class Tile
{
public:
	Tile(int mode = 0, Color bgc = { 0, 0, 0, 0 });
	Tile(const Tile &other);

	Tile &operator=(Tile &other);
	double &operator[](int idx) { return features[idx]; }
	const double &operator[](int idx) const { return features[idx]; }
	int id() { return gid; }
	void setId(int id) { gid = id; }
	double get(int idx) { return features[idx]; }
	Color getColor(int id) { return id >= 0 && id < colNum ? pixel[colors[id]] : bg; }
	void draw(ImageData &image, int flip = 0);
	void featureCalc();

	int pixelNum;
	Color bg;
	Color pixel[0x40];
	int tileId;
	int imageId;
	int colNum;
	int width;
	int height;
	int size;

private:
	int gid;
	double features[20];
	int colors[0x40];
};

class KMeans
{
public:
	KMeans() { srand((unsigned)time(0)); }
	double randf(double m) { return m * rand() / (RAND_MAX - 1.); }
	double dist(Tile &a, Tile &b);

	int nearest(Tile &pt, vector<Tile> &cents, double &d);
	void kpp(vector<Tile> &pts, vector<Tile> &cents);
	void kmcluster(vector<Tile> &pts, int k, vector<Tile> &outCents);
};
