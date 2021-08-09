#pragma once
#include "gdifunc.h"
#include "jfunc.h"

using namespace std;

class MATHFUNC
{
	JFUNC jf;
	const double pi = 3.14159265;

public:
	explicit MATHFUNC() {}
	~MATHFUNC() {}
	double angleBetweenVectors(vector<vector<int>>& vCoords);
	double angleBetweenVectors(vector<vector<double>>& pastPresentFuture);
	int coordCircleClockwise(vector<vector<double>>& coords);
	int coordCircleClockwise(vector<vector<int>>& icoords);
	double coordDist(vector<double>& origin, vector<double>& test);
	double coordDist(vector<int>& iv1, vector<int>& iv2);
	double coordDistPoint(POINT origin, POINT test);
	double coordDistPointSum(POINT& origin, vector<POINT>& testList);
	vector<double> coordDistPointSumList(vector<POINT>& originList, vector<POINT>& testList);
	vector<double> coordDistPointSumList(vector<POINT>& originList, vector<POINT>& testList, int depth);
	void coordOnCircle(vector<double>& origin, double& radius, double& angle, vector<double>& coord);
	void coordOnCircle(vector<int>& origin, int& radius, double& angle, vector<int>& coord);
	double hypoteneuse(double& Dx, double& Dy);
	double hypoteneuse(int& Ix, int& Iy);
	vector<POINT> imgVectorPath(POINT pStart, double angleDeg, vector<POINT>& boundaryTLBR);
};
