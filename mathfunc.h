#pragma once
#include <limits>
#include "jfunc.h"

using namespace std;

class MATHFUNC
{
	JFUNC jf;
	const double pi = 3.14159265;

public:
	MATHFUNC() {}
	~MATHFUNC() {}
	double angleBetweenVectors(vector<vector<int>>& vCoords);
	double angleBetweenVectors(vector<vector<double>>& pastPresentFuture);
	vector<double> coordCenter(vector<vector<double>>& coordList);
	int coordCircleClockwise(vector<vector<double>>& coords);
	int coordCircleClockwise(vector<vector<int>>& icoords);
	void coordDisplacement(vector<vector<double>>& coordList, vector<double> delta);
	double coordDist(vector<double>& origin, vector<double>& test);
	double coordDist(vector<int>& origin, vector<int>& test);
	void coordOnCircle(vector<double>& origin, double& radius, double& angle, vector<double>& coord);
	void coordOnCircle(vector<int>& origin, int& radius, double& angle, vector<int>& coord);
	void coordReflectX(vector<vector<double>>& coordList, double xAxis);
	void coordReflectY(vector<vector<double>>& coordList, double yAxis);
	double hypotenuse(double Dx, double Dy, double Dx0 = 0.0, double Dy0 = 0.0);
	double hypotenuse(int Ix, int Iy);
	double rounding(double dNum, int decimalPlaces);
	double roundingCeil(double dNum);
	double roundingCeil(double dNum, int decimalPlaces);
	double roundingFloor(double dNum);
	double roundingFloor(double dNum, int decimalPlaces);
	double slope(double Dx, double Dy, double Dx0 = 0.0, double Dy0 = 0.0);
};
