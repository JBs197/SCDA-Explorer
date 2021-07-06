#include "stdafx.h"
#include "mathfunc.h"

double MATHFUNC::angleBetweenVectors(vector<vector<int>>& vCoords)
{
	// Input has form [point][xCoord, yCoord].
	vector<vector<double>> pastPresentFuture;
	jf.toDouble(vCoords, pastPresentFuture);
	double theta = angleBetweenVectors(pastPresentFuture);
	return theta;
}
double MATHFUNC::angleBetweenVectors(vector<vector<double>>& pastPresentFuture)
{
	// Input has form [point 0, point 1, point 2][xCoord, yCoord].
	vector<double> triangleSides(3);  // 01, 12, 02
	triangleSides[0] = coordDist(pastPresentFuture[0], pastPresentFuture[1]);
	triangleSides[1] = coordDist(pastPresentFuture[1], pastPresentFuture[2]);
	triangleSides[2] = coordDist(pastPresentFuture[0], pastPresentFuture[2]);
	double cosPhi = (pow(triangleSides[0], 2.0) + pow(triangleSides[1], 2.0) - pow(triangleSides[2], 2.0)) / (2.0 * triangleSides[0] * triangleSides[1]);
	if (abs(cosPhi) > 1.0 && abs(cosPhi) <= 1.1)  // 10% error tolerance is arbitrary.
	{
		if (cosPhi > 0.0) { cosPhi = 0.9999; }  // Four significant digits is arbitrary.
		else { cosPhi = -0.9999; }
	}
	double phi = 180.0 / 3.1415926535 * acos(cosPhi);
	int clockwise = coordCircleClockwise(pastPresentFuture);
	double theta;  // Range [0.0, 360.0)
	if (clockwise == 1)
	{
		theta = 180.0 - phi;
	}
	else if (clockwise == 0)
	{
		theta = 180.0 + phi;
	}
	else if (clockwise == 2)
	{
		theta = 0.0;
	}
	else if (clockwise == 3)
	{
		theta = 180.0;
	}
	else { jf.err("Indeterminate clockwise-mf.angleBetweenVectors"); }
	return theta;
}
int MATHFUNC::coordCircleClockwise(vector<vector<double>>& coords)
{
	// NOTE: Image coordinates use a reversed y-axis (positive points down) !
	//       7
	//     2 | 3
	//   6---+---4      <--- Quadrant diagram.
	//     1 | 0
	//       5
	double baseDx = coords[1][0] - coords[0][0];
	double baseDy = coords[1][1] - coords[0][1];
	double testDx = coords[2][0] - coords[1][0];
	double testDy = coords[2][1] - coords[1][1];
	if (baseDx == 0.0 && baseDy == 0.0) { jf.err("same coordinate-mf.coordCircleClockwise"); }
	if (testDx == 0.0 && testDy == 0.0) { jf.err("same coordinate-mf.coordCircleClockwise"); }
	int baseQuadrant, testQuadrant;  // [0,3] are real quadrants, and [4,7] are horizontal/vertical perpendiculars.
	if (baseDx > 0.0)
	{
		if (baseDy > 0.0) { baseQuadrant = 0; }
		else if (baseDy < 0.0) { baseQuadrant = 3; }
		else { baseQuadrant = 4; }
	}
	else if (baseDx < 0.0)
	{
		if (baseDy > 0.0) { baseQuadrant = 1; }
		else if (baseDy < 0.0) { baseQuadrant = 2; }
		else { baseQuadrant = 6; }
	}
	else
	{
		if (baseDy >= 0.0) { baseQuadrant = 5; }
		else { baseQuadrant = 7; }
	}
	if (testDx > 0.0)
	{
		if (testDy > 0.0) { testQuadrant = 0; }
		else if (testDy < 0.0) { testQuadrant = 3; }
		else { testQuadrant = 4; }
	}
	else if (testDx < 0.0)
	{
		if (testDy > 0.0) { testQuadrant = 1; }
		else if (testDy < 0.0) { testQuadrant = 2; }
		else { testQuadrant = 6; }
	}
	else
	{
		if (testDy >= 0.0) { testQuadrant = 5; }
		else { testQuadrant = 7; }
	}

	// Easy quadrants cases (no angle needed).
	switch (baseQuadrant)
	{
	case 0:
		if (testQuadrant == 5 || testQuadrant == 1 || testQuadrant == 6) { return 1; }
		if (testQuadrant == 4 || testQuadrant == 3 || testQuadrant == 7) { return 0; }
		break;
	case 1:
		if (testQuadrant == 6 || testQuadrant == 2 || testQuadrant == 7) { return 1; }
		if (testQuadrant == 5 || testQuadrant == 0 || testQuadrant == 4) { return 0; }
		break;
	case 2:
		if (testQuadrant == 7 || testQuadrant == 3 || testQuadrant == 4) { return 1; }
		if (testQuadrant == 6 || testQuadrant == 1 || testQuadrant == 5) { return 0; }
		break;
	case 3:
		if (testQuadrant == 4 || testQuadrant == 0 || testQuadrant == 5) { return 1; }
		if (testQuadrant == 7 || testQuadrant == 2 || testQuadrant == 6) { return 0; }
		break;
	case 4:
		if (testQuadrant == 0 || testQuadrant == 5 || testQuadrant == 1) { return 1; }
		if (testQuadrant == 3 || testQuadrant == 7 || testQuadrant == 2) { return 0; }
		if (testQuadrant == 4) { return 2; }
		if (testQuadrant == 6) { return 3; }
		break;
	case 5:
		if (testQuadrant == 1 || testQuadrant == 6 || testQuadrant == 2) { return 1; }
		if (testQuadrant == 0 || testQuadrant == 4 || testQuadrant == 3) { return 0; }
		if (testQuadrant == 5) { return 2; }
		if (testQuadrant == 7) { return 3; }
		break;
	case 6:
		if (testQuadrant == 2 || testQuadrant == 7 || testQuadrant == 3) { return 1; }
		if (testQuadrant == 1 || testQuadrant == 5 || testQuadrant == 0) { return 0; }
		if (testQuadrant == 6) { return 2; }
		if (testQuadrant == 4) { return 3; }
		break;
	case 7:
		if (testQuadrant == 3 || testQuadrant == 4 || testQuadrant == 0) { return 1; }
		if (testQuadrant == 2 || testQuadrant == 6 || testQuadrant == 1) { return 0; }
		if (testQuadrant == 7) { return 2; }
		if (testQuadrant == 5) { return 3; }
		break;
	}

	// Same/opposite quadrants cases.
	double phi = abs(atan(baseDy / baseDx));
	double theta = abs(atan(testDy / testDx));
	switch (baseQuadrant)
	{
	case 0:
		if (testQuadrant == 0)
		{
			if (theta > phi) { return 1; }
			else { return 0; }
		}
		else if (testQuadrant == 2)
		{
			if (theta > phi) { return 0; }
			else { return 1; }
		}
		else { jf.err("wrong quadrants-mf.coordCircleClockwise"); }
		break;
	case 1:
		if (testQuadrant == 1)
		{
			if (theta > phi) { return 0; }
			else { return 1; }
		}
		else if (testQuadrant == 3)
		{
			if (theta > phi) { return 1; }
			else { return 0; }
		}
		else { jf.err("wrong quadrants-mf.coordCircleClockwise"); }
		break;
	case 2:
		if (testQuadrant == 2)
		{
			if (theta > phi) { return 1; }
			else { return 0; }
		}
		else if (testQuadrant == 0)
		{
			if (theta > phi) { return 0; }
			else { return 1; }
		}
		else { jf.err("wrong quadrants-mf.coordCircleClockwise"); }
		break;
	case 3:
		if (testQuadrant == 3)
		{
			if (theta > phi) { return 0; }
			else { return 1; }
		}
		else if (testQuadrant == 1)
		{
			if (theta > phi) { return 1; }
			else { return 0; }
		}
		else { jf.err("wrong quadrants-mf.coordCircleClockwise"); }
		break;
	}

	return -1;
}
int MATHFUNC::coordCircleClockwise(vector<vector<int>>& icoords)
{
	vector<vector<double>> coords;
	jf.toDouble(icoords, coords);
	int answer = coordCircleClockwise(coords);
	return answer;
}
double MATHFUNC::coordDist(vector<double>& origin, vector<double>& test)
{
	double xTemp = pow(test[0] - origin[0], 2.0);
	double yTemp = pow(test[1] - origin[1], 2.0);
	double radius = sqrt(xTemp + yTemp);
	return radius;
}
double MATHFUNC::coordDist(vector<int>& iv1, vector<int>& iv2)
{
	vector<double> origin = { (double)iv1[0], (double)iv1[1] };
	vector<double> test = { (double)iv2[0], (double)iv2[1] };
	double dist = coordDist(origin, test);
	return dist;
}
double MATHFUNC::coordDistPoint(POINT origin, POINT test)
{
	double xTemp = pow((double)(test.x - origin.x), 2.0);
	double yTemp = pow((double)(test.y - origin.y), 2.0);
	double radius = sqrt(xTemp + yTemp);
	return radius;
}
double MATHFUNC::coordDistPointSum(POINT& origin, vector<POINT>& testList)
{
	double dsum = 0.0;
	for (int ii = 0; ii < testList.size(); ii++)
	{
		dsum += coordDistPoint(origin, testList[ii]);
	}
	return dsum;
}
void MATHFUNC::coordDistPointSumList(vector<POINT>& originList, vector<POINT>& testList, vector<double>& resultList)
{
	resultList.resize(originList.size());
	for (int ii = 0; ii < resultList.size(); ii++)
	{
		resultList[ii] = coordDistPointSum(originList[ii], testList);
	}
}
void MATHFUNC::coordOnCircle(vector<double>& origin, double& radius, double& angle, vector<double>& coord)
{
	// NOTE: Angle is measured in degrees, starting from north, 
	// and travelling clockwise.
	if (radius < 0.0) { jf.err("negative radius-mf.coordOnCircle"); }
	if (angle < 0.0 || angle >= 360.0) { jf.err("angle out of bounds-mf.coordOnCircle"); }
	double angleRad = angle * 3.1415926535 / 180.0;
	double xCoord = radius * sin(angleRad);
	double yCoord = radius * cos(angleRad);
	coord.resize(2);
	coord[0] = origin[0] + xCoord;
	coord[1] = origin[1] - yCoord;
}
void MATHFUNC::coordOnCircle(vector<int>& origin, int& radius, double& angle, vector<int>& coord)
{
	vector<double> originD = { (double)origin[0], (double)origin[1] };
	double radiusD = (double)radius;
	vector<double> coordD;
	coordOnCircle(originD, radiusD, angle, coordD);
	coord.resize(2);
	coord[0] = int(round(coordD[0]));
	coord[1] = int(round(coordD[1]));
}
double MATHFUNC::hypoteneuse(double& Dx, double& Dy)
{
	double hypo = sqrt(pow(Dx, 2.0) + pow(Dy, 2.0));
	return hypo;
}
double MATHFUNC::hypoteneuse(int& Ix, int& Iy)
{
	double Dx = (double)Ix;
	double Dy = (double)Iy;
	double hypo = sqrt(pow(Dx, 2.0) + pow(Dy, 2.0));
	return hypo;
}

