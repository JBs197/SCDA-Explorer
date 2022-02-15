#pragma once
#include <QPainterPath>
#include <string>
#include <vector>
#include "jlog.h";

class SCDAregion : public QPainterPath
{
	Q_OBJECT

private:
	void err(std::string message);

public:
	SCDAregion() {}
	~SCDAregion() = default;

	std::string geoCode, name;
	std::vector<std::string> vsGeoLayer;
	std::vector<std::pair<double, double>> vBorder, vFrame;

	void load(std::vector<std::vector<std::string>>& vvsBorder, std::vector<std::vector<std::string>>& vvsFrame);


};
