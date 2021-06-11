#pragma once

#include <vector>
#include <string>
#include "jfunc.h"

using namespace std;

class MATHFUNC
{
	JFUNC jf;

public:
	explicit MATHFUNC() {}
	~MATHFUNC() {}
	double angleBetweenVectors(vector<vector<int>>& vCoords);
	double angleBetweenVectors(vector<vector<double>>& pastPresentFuture);
	int coordCircleClockwise(vector<vector<double>>& coords);
	int coordCircleClockwise(vector<vector<int>>& icoords);
	double coordDist(vector<double>& origin, vector<double>& test);
	double coordDist(vector<int>& iv1, vector<int>& iv2);
	void coordOnCircle(vector<double>& origin, double& radius, double& angle, vector<double>& coord);
	void coordOnCircle(vector<int>& origin, int& radius, double& angle, vector<int>& coord);
	double hypoteneuse(double& Dx, double& Dy);
	double hypoteneuse(int& Ix, int& Iy);

};
