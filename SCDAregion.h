#pragma once
#include <QPainterPath>
#include <string>
#include <vector>
#include "jlog.h";

class SCDAregion
{
	void err(std::string message);

public:
	SCDAregion() {}
	~SCDAregion() = default;

	std::string geoCode, name;
	std::vector<std::string> vsGeoLayer;
	std::vector<std::pair<double, double>> vBorder, vFrame;

	void load(std::vector<std::vector<std::string>>& vvsBorder, std::vector<std::vector<std::string>>& vvsFrame);
	void makePath(QPainterPath& qpPath, double width, double height, std::vector<std::pair<double, double>> parentTLBR = {});
};
