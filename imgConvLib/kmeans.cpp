#include "kmeans.h"

HSV::HSV()
{
	h = 0;
	s = 0;
	v = 0;
	a = 0;
}

HSV::HSV(Color rgb)
{
	set(rgb);
}

void HSV::set(Color rgb)
{
	a = rgb.a / 255.0;
	double max = rgb.r;
	if (rgb.g > max) max = rgb.g;
	if (rgb.b > max) max = rgb.b;
	double min = rgb.r;
	if (rgb.g < min) min = rgb.g;
	if (rgb.b < min) min = rgb.b;
	double dif = max - min;
	v = max / 255.0;
	if (max == 0 || dif == 0)
	{
		h = 0; s = 0;
		return;
	}
	s = dif / max;
	if (rgb.r == max)
	{
		h = (rgb.g - rgb.b) / dif;
		if (rgb.g < rgb.b) h += 6;
	}
	if (rgb.g == max) h = 2 + (rgb.b - rgb.r) / dif;
	if (rgb.b == max) h = 4 + (rgb.r - rgb.g) / dif;
	h *= 60;
	if (h < 0) h += 360;
	h /= 180.0;
}

Tile::Tile(int mode, Color bgc)
{
	gid = -1;
	tileId = 0;
	imageId = 0;
	pixelNum = 0;
	colNum = 0;
	width = 8;
	height = 8;
	switch (mode)
	{
	case 1: height = 4; break;
	case 2: width = 4; height = 4; break;
	case 3: width = 4; height = 2; break;
	case 4: width = 2; height = 2; break;
	case 5: width = 2; height = 1; break;
	case 6: width = 1; height = 1; break;
	default:;
	}
	size = width * height;
	for (int i = 0; i < 64; i++) pixel[i] = { 0, 0, 0, 0 };
	for (int i = 0; i < 20; i++) features[i] = 0;
	bg = bgc;
}

Tile::Tile(const Tile &other)
{
	gid = other.gid;
	tileId = other.tileId;
	imageId = other.imageId;
	pixelNum = other.pixelNum;
	colNum = other.colNum;
	width = other.width;
	height = other.height;
	size = other.size;
	for (int i = 0; i < 64; i++) pixel[i] = other.pixel[i];
	for (int i = 0; i < 20; i++) features[i] = other[i];
	bg = other.bg;
}

Tile &Tile::operator=(Tile& other)
{
	gid = other.gid;
	tileId = other.tileId;
	imageId = other.imageId;
	pixelNum = other.pixelNum;
	colNum = other.colNum;
	width = other.width;
	height = other.height;
	size = other.size;
	for (int i = 0; i < 64; i++) pixel[i] = other.pixel[i];
	for (int i = 0; i < 20; i++) features[i] = other[i];
	bg = other.bg;
	return *this;
}

void Tile::draw(ImageData &image, int flip)
{
	int index = 0;
	int x = tileId % (image.width / width) * width;
	int y = tileId / (image.width / width) * height;
	int xpos, ypos;
	for (int dy = 0; dy < height; dy++) for (int dx = 0; dx < width; dx++)
	{
		switch (flip)
		{
		case 1:
			// Horizontal
			xpos = x + width - 1 - dx;
			ypos = y + dy;
			break;
		case 2:
			// Vertical
			xpos = x + dx;
			ypos = y + height - 1 - dy;
			break;
		case 3:;
			// Double
			xpos = x + width - 1 - dx;
			ypos = y + height - 1 - dy;
			break;
		default:
			xpos = x + dx;
			ypos = y + dy;
		}
		image.set(pixel[index], xpos, ypos);
		index++;
	}
}

void Tile::featureCalc()
{
	vector<Color> colRec;
	vector<Color> pixRec;
	for (int i = 0; i < size; i++)
	{
		if (bg == pixel[i]) continue;
		bool colFind = false;
		for (size_t j = 0; j < colRec.size(); j++)
		{
			if (colRec[j] == pixel[i])
			{
				colFind = true;
				break;
			}
		}
		if (colFind == false)
		{
			colors[colRec.size()] = i;
			colRec.push_back(pixel[i]);
			pixRec.push_back(pixel[i]);
			HSV hsv(pixel[i]);
			features[0] += hsv.h;
			features[1] += hsv.s;
			features[2] += hsv.v;
			features[3] += hsv.a;
			if (hsv.h < features[12]) features[12] = hsv.h;
			if (hsv.s < features[13]) features[13] = hsv.s;
			if (hsv.v < features[14]) features[14] = hsv.v;
			if (hsv.a < features[15]) features[15] = hsv.a;
			if (hsv.h > features[16]) features[16] = hsv.h;
			if (hsv.s > features[17]) features[17] = hsv.s;
			if (hsv.v > features[18]) features[18] = hsv.v;
			if (hsv.a > features[19]) features[19] = hsv.a;
		}
		/*pixRec(pixel[i]);
		HSV hsv(pixel[i]);
		features[0] += hsv.h;
		features[1] += hsv.s;
		features[2] += hsv.v;
		features[3] += hsv.a;
		if (hsv.h < features[12]) features[12] = hsv.h;
		if (hsv.s < features[13]) features[13] = hsv.s;
		if (hsv.v < features[14]) features[14] = hsv.v;
		if (hsv.a < features[15]) features[15] = hsv.a;
		if (hsv.h > features[16]) features[16] = hsv.h;
		if (hsv.s > features[17]) features[17] = hsv.s;
		if (hsv.v > features[18]) features[18] = hsv.v;
		if (hsv.a > features[19]) features[19] = hsv.a;*/
	}
	colNum = colRec.size();
	vector<Color>().swap(colRec);
	if (pixRec.size() > 0)
	{
		pixelNum = pixRec.size();
		features[0] /= pixelNum;
		features[1] /= pixelNum;
		features[2] /= pixelNum;
		features[3] /= pixelNum;
		for (size_t i = 0; i < pixRec.size(); i++)
		{
			HSV hsv(pixRec[i]);
			features[4] += pow(fabs(hsv.h - features[0]), 2);
			features[5] += pow(fabs(hsv.s - features[1]), 2);
			features[6] += pow(fabs(hsv.v - features[2]), 2);
			features[7] += pow(fabs(hsv.a - features[3]), 2);
			features[8] += pow(fabs(hsv.h - features[0]), 3);
			features[9] += pow(fabs(hsv.s - features[1]), 3);
			features[10] += pow(fabs(hsv.v - features[2]), 3);
			features[11] += pow(fabs(hsv.a - features[3]), 3);
		}
		features[4] = pow(features[4] / pixelNum, 0.5);
		features[5] = pow(features[5] / pixelNum, 0.5);
		features[6] = pow(features[6] / pixelNum, 0.5);
		features[7] = pow(features[7] / pixelNum, 0.5);
		features[8] = pow(features[8] / pixelNum, 1.0 / 3);
		features[9] = pow(features[9] / pixelNum, 1.0 / 3);
		features[10] = pow(features[10] / pixelNum, 1.0 / 3);
		features[11] = pow(features[11] / pixelNum, 1.0 / 3);
	}
	vector<Color>().swap(pixRec);
}

double KMeans::dist(Tile &a, Tile &b)
{
	double len = 0;
	for (int i = 0; i < 20; ++i) len += (a[i] - b[i]) * (a[i] - b[i]);
	return len;
}

int KMeans::nearest(Tile &pt, vector<Tile> &cents, double &d)
{
	int i, min_i;
	double d1, min_d;
	min_d = HUGE_VAL;
	min_i = pt.id();
	for (i = 0; i < (int)cents.size(); ++i)
	{
		Tile c = cents[i];
		if (min_d > (d1 = dist(c, pt)))
		{
			min_d = d1;
			min_i = i;
		}
	}
	d = min_d;
	return min_i;
}

void KMeans::kpp(vector<Tile> &pts, vector<Tile> &cents)
{
	double sum = 0;
	vector<double> d;
	d.resize(pts.size());
	cents[0] = pts[rand() % pts.size()];
	vector<Tile> tmpCents;
	tmpCents.push_back(cents[0]);
	for (int k = 1; k < (int)cents.size(); ++k)
	{
		sum = 0;
		for (int i = 0; i < (int)pts.size(); ++i)
		{
			nearest(pts[i], tmpCents, d[i]);
			sum += d[i];
		}
		sum = randf(sum);
		for (int i = 0; i < (int)pts.size(); ++i)
		{
			if ((sum -= d[i]) > 0)	continue;
			cents[k] = pts[i];
			tmpCents.push_back(cents[k]);
			break;
		}
	}
	for (int i = 0; i < (int)pts.size(); ++i)
	{
		int id = nearest(pts[i], cents, *(new double));
		pts[i].setId(id);
	}
}

void KMeans::kmcluster(vector<Tile> &pts, int k, vector<Tile> &outCents)
{
	if (outCents.size() <= 0) outCents.resize(k);
	kpp(pts, outCents);
	int changed;
	do
	{
		for (int i = 0; i < (int)outCents.size(); ++i)
		{
			for (int j = 0; j < 20; ++j) outCents[i][j] = 0;
			outCents[i].setId(0);
		}
		vector<int> cnt(k, 0);
		for (int i = 0; i < (int)pts.size(); ++i)
		{
			int k = pts[i].id();
			for (int j = 0; j < 20; ++j) outCents[k][j] += pts[i][j];
			cnt[k]++;
		}
		for (int i = 0; i < (int)outCents.size(); ++i)
		{
			for (int j = 0; j < 20; ++j) outCents[i][j] /= cnt[i];
		}
		changed = 0;
		for (int i = 0; i < (int)pts.size(); ++i)
		{
			int min_i = nearest(pts[i], outCents, *(new double));
			if (min_i != pts[i].id())
			{
				changed++;
				pts[i].setId(min_i);
			}
		}
	} while (changed > 0.001 * pts.size());
	for (int i = 0; i < (int)outCents.size(); ++i) outCents[i].setId(i);
}
