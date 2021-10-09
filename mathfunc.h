#pragma once
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
	vector<double> coordCenter(vector<vector<double>>& coordList);
	int coordCircleClockwise(vector<vector<double>>& coords);
	int coordCircleClockwise(vector<vector<int>>& icoords);
	void coordDisplacement(vector<vector<double>>& coordList, vector<double> delta);
	double coordDist(vector<double>& origin, vector<double>& test);
	double coordDist(vector<int>& iv1, vector<int>& iv2);
	void coordOnCircle(vector<double>& origin, double& radius, double& angle, vector<double>& coord);
	void coordOnCircle(vector<int>& origin, int& radius, double& angle, vector<int>& coord);
	void coordReflectX(vector<vector<double>>& coordList, double xAxis);
	void coordReflectY(vector<vector<double>>& coordList, double yAxis);
	double hypoteneuse(double& Dx, double& Dy);
	double hypoteneuse(int& Ix, int& Iy);
	double rounding(double dNum, int decimalPlaces);
	double roundingCeil(double dNum);
	double roundingCeil(double dNum, int decimalPlaces);
	double roundingFloor(double dNum);
	double roundingFloor(double dNum, int decimalPlaces);
};
