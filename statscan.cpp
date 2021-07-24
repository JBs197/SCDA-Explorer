#include "stdafx.h"
#include "statscan.h"

void STATSCAN::advanceNextCSVpart()
{
    activeCSVpart++;
    string csvPath = cataPath + "\\" + cataName + "_English_CSV_data (PART ";
    csvPath += to_string(activeCSVpart) + ").csv";
    activeCSV = wf.load(csvPath);
    if (activeCSV.size() < 1) { jf.err("Cannot load CSV part file-sc.advanceNextCSVpart"); }
}
vector<string> STATSCAN::compareGeoListDB(vector<vector<string>>& geoList, vector<string>& dbList)
{
    // Returns a list of GEO_CODEs not already present in the database.
    vector<string> missingGeo;
    unordered_map<string, int> mapDB;
    for (int ii = 0; ii < dbList.size(); ii++)
    {
        mapDB.emplace(dbList[ii], ii);
    }
    for (int ii = 0; ii < geoList.size(); ii++)
    {
        if (!mapDB.count(geoList[ii][0])) 
        { 
            missingGeo.push_back(geoList[ii][0]);
        }
    }
    return missingGeo;
}
void STATSCAN::convertSCgeo(string& geoPathOld)
{
    string geoFile = wf.load(geoPathOld), csvPath, csvFile, csvBuffer, csvLine;
    size_t pos1 = geoPathOld.rfind('\\'), pos2, posEnd;
    string folderPath = geoPathOld.substr(0, pos1), temp, currentAGC, lineAGC;
    string geoPathNew = folderPath + "\\Geo.txt";
    vector<vector<string>> vvsData;
    int index, indexAGC, colIndex;
    pos1 = geoFile.find('\n');
    pos1 = geoFile.find('"', pos1);
    while (pos1 < geoFile.size())
    {
        index = vvsData.size();
        vvsData.push_back(vector<string>(4));
        pos1++;
        pos2 = geoFile.find('"', pos1);
        temp = geoFile.substr(pos1, pos2 - pos1);
        while (temp[0] == '0') { temp.erase(temp.begin()); }
        vvsData[index][0] = temp;
        pos1 = geoFile.find_first_not_of(",\"", pos2 + 1);
        pos2 = geoFile.find('"', pos1);
        vvsData[index][1] = geoFile.substr(pos1, pos2 - pos1);
        pos1 = geoFile.find('"', pos2 + 1);
    }

    vector<string> csvList = wf.get_file_list(folderPath, "*PART*.csv");
    int numPART = csvList.size();
    for (int ii = 1; ii <= numPART; ii++)
    {
        csvPath = folderPath + "\\" + cataName + "_English_CSV_data (PART ";
        csvPath += to_string(ii) + ").csv";
        csvFile = wf.load(csvPath);
        if (ii == 1)
        {
            index = 0;
            posEnd = csvFile.find_first_of("\r\n");
            pos1 = csvFile.find('"');
            pos2 = csvFile.find(',');
            while (pos2 < posEnd)
            {
                temp = csvFile.substr(pos1, pos2 - pos1);
                pos1 = temp.find("ALT_GEO_CODE");
                if (pos1 < temp.size())
                {
                    colIndex = index;
                    break;
                }
                else
                {
                    index++;
                    pos1 = pos2 + 1;
                    pos2 = csvFile.find(',', pos2 + 1);
                }
            }
            pos1 = posEnd;
            for (int jj = 0; jj < colIndex; jj++)
            {
                pos1 = csvFile.find(',', pos1 + 1);
            }
            pos1 = csvFile.find_first_not_of(",\"", pos1);
            pos2 = csvFile.find('"', pos1);
            currentAGC = csvFile.substr(pos1, pos2 - pos1);
            vvsData[0][2] = "1";
            lineAGC = currentAGC;
            indexAGC = 0;
            posEnd = csvFile.find(nl, posEnd + 1);
        }
        else
        {
            posEnd = csvFile.find(nl);
            csvLine = csvBuffer + csvFile.substr(0, posEnd);
            lineAGC = extractCSVLineValue(csvLine, colIndex);
            if (lineAGC != currentAGC)
            {
                vvsData[indexAGC][3] = to_string(ii);
                indexAGC++;
                vvsData[indexAGC][2] = to_string(ii);
                currentAGC = lineAGC;
            }
        }
        
        pos1 = posEnd;
        pos2 = csvFile.find(nl, posEnd + 1);
        while (pos2 < csvFile.size())
        {
            csvLine = csvFile.substr(pos1, pos2 - pos1);
            lineAGC = extractCSVLineValue(csvLine, colIndex);
            if (lineAGC != currentAGC)
            {
                vvsData[indexAGC][3] = to_string(ii);
                indexAGC++;
                vvsData[indexAGC][2] = to_string(ii);
                currentAGC = lineAGC;
            }
            pos1 = pos2;
            pos2 = csvFile.find(nl, pos2 + 1);
        }
        csvBuffer = csvFile.substr(pos1);
    }
    vvsData[indexAGC][3] = to_string(numPART);

    string geoFileNew = "ALT_GEO_CODE, Region Name, PART begin, PART end";
    for (int ii = 0; ii < vvsData.size(); ii++)
    {
        geoFileNew += "\n" + vvsData[ii][0];
        for (int jj = 1; jj < vvsData[ii].size(); jj++)
        {
            geoFileNew += "," + vvsData[ii][jj];
        }
    }
    geoFileNew += "\n\n\n";
    wf.printer(geoPathNew, geoFileNew);
}

string STATSCAN::extractCSVLineValue(string& csvLine, int colIndex)
{
    if (csvLine.size() < 1) { jf.err("No csvLine given-sc.extractCSVLineValue"); }
    size_t pos1 = 0, posLast = csvLine.size() - 1;
    for (int ii = 0; ii < colIndex; ii++)
    {
        pos1 = csvLine.find(',', pos1 + 1);
        if (pos1 > posLast) { jf.err("colIndex exceeds csvLine width-sc.extractCSVLineValue"); }
    }
    pos1++;
    if (csvLine[pos1] == '"') { pos1++; }
    size_t pos2 = csvLine.find(',', pos1);
    if (csvLine[pos2 - 1] == '"') { pos2--; }
    if (pos1 == pos2) { return ""; }  // No value here.
    string sValue = csvLine.substr(pos1, pos2 - pos1);
    return sValue;
}
vector<string> STATSCAN::extractCSVLineValue(string& csvLine, vector<int> colIndex)
{
    if (csvLine.size() < 1) { jf.err("No csvLine given-sc.extractCSVLineValue"); }
    vector<string> sValue(colIndex.size());
    int index = 0, stepsForward;
    bool inside = 0;  // Inside quotation marks.
    size_t pos1 = 0, pos2, posLast = csvLine.size() - 1;
    for (int ii = 0; ii < colIndex.size(); ii++)
    {
        stepsForward = colIndex[ii] - index;
        for (int jj = 0; jj < stepsForward; jj++)
        {
            if (!inside) { pos1 = csvLine.find(',', pos1 + 1); }
            else { pos1 = csvLine.find("\",", pos1 + 1) + 1; }
            if (pos1 > posLast) { jf.err("Stepped beyond column width-sc.extractCSVLineValue"); }
            if (csvLine[pos1 + 1] == '"') { inside = 1; }
            else { inside = 0; }
        }
        index += stepsForward;
        pos1++;
        if (csvLine[pos1] == ',')  // No sValue for this column.
        {
            sValue[ii] = "";
            continue;
        }
        else if (csvLine[pos1] == '"') { pos1++; }
        pos2 = csvLine.find(',', pos1);
        if (pos2 < csvLine.size())
        {
            if (csvLine[pos2 - 1] == '"') { pos2--; }
            sValue[ii] = csvLine.substr(pos1, pos2 - pos1);
        }
        else { sValue[ii] = csvLine.substr(pos1); }
    }
    return sValue;
}

string STATSCAN::getCSVPath(int PART)
{
    if (metaFile.size() < 1) { jf.err("No init-sc.getCSVPath"); }
    string csvPath = cataPath + "\\" + cataName + "_English_CSV_data (PART ";
    csvPath += to_string(PART) + ").csv";
    return csvPath;
}
int STATSCAN::getGeoCodeIndex(string& sActiveCSVgeocode)
{
    int gci;
    try { gci = mapGeoCode.at(sActiveCSVgeocode); }
    catch (out_of_range) { jf.err("mapGeoCode-sc.getGeoCodeIndex"); }
    return gci;
}
string STATSCAN::getGeoLayer(string geoLayerExternal)
{
    if (mapGeoLayer.size() < 1) { initGeo(); }
    string geoLayerInternal;
    try { geoLayerInternal = mapGeoLayer.at(geoLayerExternal); }
    catch (out_of_range) { jf.err("Geo layer not recognized-sc.getGeoLayer"); }
    return geoLayerInternal;
}

void STATSCAN::init(string cP)
{
    cataPath = cP;
    size_t pos1, pos2;
    pos2 = cataPath.rfind('\\');
    cataName = cataPath.substr(pos2 + 1);
    pos1 = cataPath.rfind('\\', pos2 - 1) + 1;
    cataYear = cataPath.substr(pos1, pos2 - pos1);

    vector<string> vsResult = wf.get_file_list(cataPath, "*English_meta.txt");
    if (vsResult.size() != 1) { jf.err("No meta file-sc.init"); }
    string metaPath = cataPath + "\\" + vsResult[0];
    metaFile = wf.load(metaPath);
    int inum;

    pos1 = metaFile.find("General Information") + 19;
    pos2 = metaFile.find('\n', pos1) + 1;
    string temp = metaFile.substr(pos1, pos2 - pos1);
    pos1 = temp.find('\r');
    if (pos1 < temp.size()) { nl = "\r\n"; nls = 2; }
    else { nl = "\n"; nls = 1; }

    pos1 = metaFile.find_first_not_of("*\r\n", pos2);
    pos2 = metaFile.find(nl, pos1);
    description = metaFile.substr(pos1, pos2 - pos1);
    pos1 = metaFile.find("Topic", pos2) + 5;
    pos1 = metaFile.find_first_not_of(": ", pos1);
    pos2 = metaFile.find(nl, pos1);
    topic = metaFile.substr(pos1, pos2 - pos1);
}
void STATSCAN::initCSV(string activeGeoCode, string sActivePART)
{
    // Initializes all 'activeCSV' variables.
    if (metaFile.size() < 1) { jf.err("No init-sc.initCSV"); }
    size_t pos1, pos2;
    int indexDim = 0, indexDIM = 0;
    activeCSVgeocode = activeGeoCode;
    try { activeCSVpart = stoi(sActivePART); }
    catch (invalid_argument) { jf.err("stoi-sc.initCSV"); }
    string csvPath = getCSVPath(activeCSVpart);
    activeCSV = wf.load(csvPath);
    cataColTitles = loadColTitles();
    for (int ii = 0; ii < cataColTitles.size(); ii++)
    {
        pos1 = cataColTitles[ii].find("Dim:");
        if (pos1 < cataColTitles[ii].size())
        {
            mapDim.emplace(indexDim, ii);
            indexDim++;
        }
        pos1 = cataColTitles[ii].find("DIM:");
        if (pos1 < cataColTitles[ii].size())
        {
            mapDIM.emplace(indexDIM, ii + 1);
            indexDIM++;
        }
        pos1 = cataColTitles[ii].find("ALT_GEO_CODE");
        if (pos1 < cataColTitles[ii].size()) { geoCodeCol = ii; }
        pos1 = cataColTitles[ii].find("GEO_LEVEL");
        if (pos1 < cataColTitles[ii].size()) { geoLevelCol = ii; }
        pos1 = cataColTitles[ii].find("GEO_NAME");
        if (pos1 < cataColTitles[ii].size()) { geoNameCol = ii; }
    }
}
void STATSCAN::initGeo()
{
    string empty;
    if (mapGeoLayer.size() == 0)
    {
        mapGeoLayer.emplace("Canada", empty);
        mapGeoLayer.emplace("Can", empty);
        mapGeoLayer.emplace("Prov.Terr.", "province");
        mapGeoLayer.emplace("Prov.Terr", "province");
        mapGeoLayer.emplace("Provinces Territories", "province");
        mapGeoLayer.emplace("CMACA", "cmaca");
        mapGeoLayer.emplace("CMACA with Provincial Splits", "cmaca");
        mapGeoLayer.emplace("CMACA with Provincial Splits 2016", "cmaca");
        mapGeoLayer.emplace("CMA with Provincial Splits", "cmaca");
        mapGeoLayer.emplace("CT", "ct");
        mapGeoLayer.emplace("CD", "cd");
        mapGeoLayer.emplace("CSD", "csd");
        mapGeoLayer.emplace("FED", "fed");
        mapGeoLayer.emplace("ER", "er");
        mapGeoLayer.emplace("FSA", "fsa");  // No maps available.
        mapGeoLayer.emplace("DA", "da");  // No maps available.
    }
}

int STATSCAN::loadBinGeo(string& filePath, vector<int>& gidList, vector<string>& regionList, vector<string>& layerList, vector<string>& geoLayers)
{
    gidList.clear();
    regionList.clear();
    layerList.clear();
    geoLayers.clear();
    string sfile = jf.load(filePath), temp, sGid, region, sIndent;
    size_t pos1, pos2, pos3;
    int inum, iGid, iIndent;
    pos3 = sfile.rfind("@@Geo URL");
    pos3 = sfile.find_last_not_of('\n', pos3 - 1) + 1;
    pos2 = sfile.rfind("@@Geo Layers");
    pos2 = sfile.find('\n', pos2 + 11);
    pos1 = pos2 + 1;
    pos2 = sfile.find('\n', pos1);
    while (pos2 <= pos3)
    {
        if (pos2 == pos1) { geoLayers.push_back(""); }
        else
        {
            temp = sfile.substr(pos1, pos2 - pos1);
            geoLayers.push_back(temp);
        }
        pos1 = pos2 + 1;
        pos2 = sfile.find('\n', pos1);
    }

    pos3 = sfile.rfind("@@Geo Layers", pos3);
    pos1 = sfile.find_first_of("1234567890");
    pos2 = sfile.find('$', pos1);
    while (pos2 < pos3)
    {
        sGid = sfile.substr(pos1, pos2 - pos1);
        pos1 = pos2 + 1;
        pos2 = sfile.find('$', pos1);
        region = sfile.substr(pos1, pos2 - pos1);
        pos1 = pos2 + 1;
        pos2 = sfile.find('\n', pos1);
        sIndent = sfile.substr(pos1, pos2 - pos1);
        try
        {
            iGid = stoi(sGid);
            iIndent = stoi(sIndent);
        }
        catch (invalid_argument) { jf.err("stoi-sc.loadGeo"); }
        if (iIndent >= geoLayers.size()) { jf.err("geoLayers-sc.loadGeo"); }
        gidList.push_back(iGid);
        regionList.push_back(region);
        layerList.push_back(geoLayers[iIndent]);
        pos1 = pos2 + 1;
        pos2 = sfile.find('$', pos1);
    }

    int loaded = 0;
    if (gidList.size() > 0) { loaded++; }
    if (regionList.size() > 0) { loaded++; }
    if (layerList.size() > 0) { loaded++; }
    if (geoLayers.size() > 0) { loaded++; }
    return loaded;
}
void STATSCAN::loadBookmark(int& iActiveCSVpart, string& sActiveCSVgeocode)
{
    if (metaFile.size() < 1) { jf.err("No init-sc.loadBookmark"); }
    string bmFile, bmPath = cataPath + "\\bookmark.txt";
    size_t pos1, pos2;
    if (wf.file_exist(bmPath))
    {
        bmFile = jf.load(bmPath);
        pos1 = bmFile.find("(PART)");
        if (pos1 > bmFile.size()) { jf.err("Corrupt bookmark-sc.loadBookmark"); }
        pos1 = bmFile.find('\n', pos1) + 1;
        pos2 = bmFile.find('\n', pos1);
        try { iActiveCSVpart = stoi(bmFile.substr(pos1, pos2 - pos1)); }
        catch (invalid_argument) { jf.err("stoi-sc.loadBookmark"); }
        pos1 = bmFile.find("(GEO_CODE)");
        if (pos1 > bmFile.size()) { jf.err("Corrupt bookmark-sc.loadBookmark"); }
        pos1 = bmFile.find('\n', pos1) + 1;
        pos2 = bmFile.find('\n', pos1);
        sActiveCSVgeocode = bmFile.substr(pos1, pos2 - pos1);
    }
    else  // Use default starting values.
    {
        iActiveCSVpart = 1;
        sActiveCSVgeocode = "-1";
        makeBookmark(iActiveCSVpart, sActiveCSVgeocode);
    }
}
vector<string> STATSCAN::loadColTitles()
{
    // Returns the complete (unaltered) first line of a CSV mega-file.
    if (metaFile.size() < 1) { jf.err("No init-sc.loadColTitles"); }
    vector<string> colTitles;
    string csvPath, line, temp, csvFile; 
    csvPath = cataPath + "\\" + cataName + "_English_CSV_data (PART 1).csv";
    csvFile = wf.load(csvPath);
    size_t pos1 = csvFile.find('"');
    size_t pos2 = csvFile.find(nl);
    line = csvFile.substr(pos1, pos2 - pos1);
    pos1 = 0;
    while (pos1 < line.size())
    {
        pos1 = line.find_first_not_of(",\" ", pos1);
        pos2 = line.find('"', pos1);
        temp = line.substr(pos1, pos2 - pos1);
        colTitles.push_back(temp);
        pos1 = line.find(',', pos2);
    }
    return colTitles;
}
vector<vector<string>> STATSCAN::loadGeoList(string geoPath)
{
    // Returns a parsed geoFile, and passively sends a copy to this object, if needed.
    if (metaFile.size() < 1) { jf.err("No init-sc.loadGeoList"); }
    string geoFile = wf.load(geoPath);
    if (geoFile.size() < 1) { jf.err("Empty geoFile-sc.loadGeoList"); }
    mapGeoCode.clear();
    vector<vector<string>> geoList;  // Form [region index][GEO_CODE, Region Name, PART begin, PART end].
    size_t pos1 = geoFile.find_first_of("1234567890"), pos2, posPart, index;
    while (pos1 < geoFile.size())
    {
        index = geoList.size();
        geoList.push_back(vector<string>(4));
        pos2 = geoFile.find(',', pos1);
        geoList[index][0] = geoFile.substr(pos1, pos2 - pos1);
        mapGeoCode.emplace(geoList[index][0], index);
        pos1 = pos2 + 1;
        pos2 = geoFile.find(',', pos1);
        geoList[index][1] = geoFile.substr(pos1, pos2 - pos1);
        if (ignoreSplitRegions)
        {
            posPart = geoList[index][1].find(" part)");
            if (posPart < geoList[index][1].size())
            {
                mapGeoCode.erase(geoList[index][0]);
                geoList.pop_back();
                pos2 = geoFile.find_first_of(nl, pos1);
                pos1 = geoFile.find_first_of("1234567890", pos2);
                continue;
            }
        }
        pos1 = pos2 + 1;
        pos2 = geoFile.find(',', pos1);
        geoList[index][2] = geoFile.substr(pos1, pos2 - pos1);
        pos1 = pos2 + 1;
        pos2 = geoFile.find_first_of(nl, pos1);
        geoList[index][3] = geoFile.substr(pos1, pos2 - pos1);
        pos1 = geoFile.find_first_of("1234567890", pos2);
    }
    if (geo.size() < 1) { geo = geoList; }
    return geoList;
}

string STATSCAN::makeBinBorder(vector<vector<int>>& border)
{
    string binBorder = "//border";
    for (int ii = 0; ii < border.size(); ii++)
    {
        binBorder += "\n" + to_string(border[ii][0]) + "," + to_string(border[ii][1]);
    }
    return binBorder;
}
string STATSCAN::makeBinFrames(vector<vector<vector<int>>>& frames)
{
    string binFrames = "//frames(topLeft@botRight, showing 'maps', 'scale', 'position')";
    for (int ii = 0; ii < frames.size(); ii++)
    {
        binFrames += "\n" + to_string(frames[ii][0][0]) + ",";
        binFrames += to_string(frames[ii][0][1]) + "@" + to_string(frames[ii][1][0]);
        binFrames += "," + to_string(frames[ii][1][1]);
    }
    return binFrames;
}
string STATSCAN::makeBinParent(string parent)
{
    string binParent = "//parent\n" + parent;
    return binParent;
}
string STATSCAN::makeBinParentNew(string binPath, string& sParent)
{
    string binParent = "//parent";
    size_t pos1 = binPath.rfind('\\');
    size_t pos2 = binPath.rfind(".bin");
    string regionName = binPath.substr(pos1 + 1, pos2 - pos1 - 1);
    string geoPath = binPath.substr(0, pos1) + "\\geo layers.txt";
    pos1 = regionName.find("(Canada)");
    if (pos1 < regionName.size())
    {
        binParent += "\nCanada";
        return binParent;
    }
    string geoFile = wf.load(geoPath);
    string regionName8 = jf.asciiToUTF8(regionName);
    pos1 = geoFile.find("$" + regionName8 + "$") + 1;
    if (pos1 > geoFile.size()) { jf.err("Region not found in geo-sc.makeBinParent"); }
    pos1 = geoFile.find('$', pos1) + 1;
    pos2 = geoFile.find_first_of("\r\n", pos1);
    string temp = geoFile.substr(pos1, pos2 - pos1);
    int indent;
    try { indent = stoi(temp); }
    catch (invalid_argument) { jf.err("stoi-sc.makeBinParent"); }
    if (indent == 0)
    {
        binParent += "\nNone";
        return binParent;
    }
    indent--;
    string sIndent = "$" + to_string(indent);
    pos2 = geoFile.rfind(sIndent, pos1);
    pos1 = geoFile.rfind('$', pos2 - 1) + 1;
    temp = geoFile.substr(pos1, pos2 - pos1);
    sParent = temp;
    binParent += "\n" + temp;
    return binParent;
}
string STATSCAN::makeBinPositionGPS(vector<double> position)
{
    string binPosition = "//position(GPS)\n" + to_string(position[0]);
    binPosition += "," + to_string(position[1]);
    return binPosition;
}
string STATSCAN::makeBinPositionPNG(vector<double> position)
{
    string binPosition = "//position(PNG)\n" + to_string(position[0]);
    binPosition += "," + to_string(position[1]);
    return binPosition;
}
string STATSCAN::makeBinScale(double scale)
{
    string binScale = "//scale(pixels per km)\n" + to_string(scale);
    return binScale;
}

void STATSCAN::makeBookmark(int iActiveCSVpart, string sActiveCSVgeocode)
{
    if (metaFile.size() < 1) { jf.err("No init-sc.makeBookmark"); }
    string bmPath = cataPath + "\\bookmark.txt";
    string bmFile = "//LASTSUCCESS(PART)\n" + to_string(iActiveCSVpart);
    bmFile += "\n\n//NEXTREGION(GEO_CODE)\n" + sActiveCSVgeocode + "\n\n";
    jf.printer(bmPath, bmFile);
}

string STATSCAN::makeCreateCensus()
{
    if (metaFile.size() < 1) { jf.err("No init-sc.makeCreateCensus"); }
    string stmt = "CREATE TABLE Census (Year INTEGER PRIMARY KEY);";
    return stmt;
}
vector<string> STATSCAN::makeCreateData(vector<string>& DIMList, vector<string>& dimList)
{
    if (geo.size() < 1) { jf.err("geo not loaded-sc.makeCreateData"); }
    int mapDIMSize = mapDIM.size();
    int mapDimSize = mapDim.size();
    if (mapDIMSize < 1 || mapDimSize < 1) { jf.err("No initCSV-sc.makeCreateData"); }
    vector<string> stmts(geo.size());
    string stmt0 = "CREATE TABLE IF NOT EXISTS \"Census$" + cataYear + "$" + cataName + "$";
    string stmt1 = "\" (\"";
    for (int ii = 0; ii < DIMList.size() - 1; ii++)  // Note: The last DIMList value is a 'dim'.
    {
        if (ii > 0) { stmt1 += ", \""; }
        stmt1 += DIMList[ii] + "\" INT";
    }
    for (int ii = 0; ii < dimList.size(); ii++)
    {
        stmt1 += ", \"" + dimList[ii] + "\" NUMERIC";
    }
    stmt1 += ", UNIQUE(\"";
    for (int ii = 0; ii < DIMList.size() - 1; ii++)
    {
        if (ii > 0) { stmt1 += "\", \""; }
        stmt1 += DIMList[ii];
    }
    for (int ii = 0; ii < dimList.size(); ii++)
    {
        stmt1 += "\", \"" + dimList[ii];
    }
    stmt1 += "\"));";
    for (int ii = 0; ii < geo.size(); ii++)
    {
        stmts[ii] = stmt0 + geo[ii][0] + stmt1;
    }
    return stmts;
}
string STATSCAN::makeCreateGeo()
{
    // Note this table is created with zero 'Ancestor' columns. Those are added
    // during insertion of rows, on an as-needed basis. 
    if (metaFile.size() < 1) { jf.err("No init-sc.makeCreateGeo"); }
    string stmt = "CREATE TABLE IF NOT EXISTS \"Geo$" + cataYear + "$";
    stmt += cataName + "\" (GEO_CODE INTEGER PRIMARY KEY, \"Region Name\" TEXT";
    stmt += ", GEO_LEVEL INT);";
    return stmt;
}
string STATSCAN::makeCreateGeoLayers(string sYear)
{
    string tname = "GeoLayers$" + sYear;
    string stmt = "CREATE TABLE IF NOT EXISTS \"" + tname + "\" (Catalogue TEXT";
    stmt += ", Level0 TEXT, UNIQUE(Catalogue));";
    return stmt;
}
vector<string> STATSCAN::makeCreateInsertDIMIndex(vector<string>& DIMList)
{
    // Note: the last row is a 'Dim', not a 'DIM'.
    if (metaFile.size() < 1) { jf.err("No init-sc.makeCreateInsertDefinitions"); }
    vector<string> defn;
    int index;
    DIMList.clear();
    size_t pos1 = metaFile.find("Definitions"), pos2, posMID;
    posMID = metaFile.find("Member" + nl, pos1);
    while (posMID < metaFile.size())
    {
        index = DIMList.size();
        pos2 = metaFile.rfind('.', posMID) + 1;
        pos1 = metaFile.find_last_of("\r\n", pos2 - 1) + 1;
        defn.push_back(metaFile.substr(pos1, pos2 - pos1));
        pos2 = metaFile.rfind("Definition", pos1);
        pos2 = metaFile.find_last_not_of("\r\n", pos2 - 1) + 1;
        pos1 = metaFile.find_last_of("\r\n", pos2 - 1) + 1;
        DIMList.push_back(metaFile.substr(pos1, pos2 - pos1));
        trimMID(DIMList[index]);
        posMID = metaFile.find("Member", posMID + 1);
    }
    if (DIMList.size() != defn.size()) { jf.err("Size mismatch-sc.makeCreateInsertDefinitions"); }

    vector<string> stmts(1), dirt = { "'" }, soap = { "''" };
    string stmt, tname = "Census$" + cataYear + "$" + cataName + "$DIMIndex";
    stmts[0] = "CREATE TABLE IF NOT EXISTS \"" + tname + "\" (DIMIndex INTEGER ";
    stmts[0] += "PRIMARY KEY, DIM TEXT, Definition TEXT);";
    for (int ii = 0; ii < DIMList.size(); ii++)
    {
        jf.clean(DIMList[ii], dirt, soap);
        jf.clean(defn[ii], dirt, soap);
        stmt = "INSERT OR IGNORE INTO \"" + tname + "\" (DIMIndex, DIM, Definition) ";
        stmt += "VALUES (" + to_string(ii) + ", '" + DIMList[ii] + "', '" + defn[ii] + "');";
        stmts.push_back(stmt);
    }
    return stmts;
}
vector<string> STATSCAN::makeCreateInsertDIM()
{
    // Note: DIMList has form [DIM Index][MID1 name, MID2 name, ...].
    if (metaFile.size() < 1) { jf.err("No init-sc.makeCreateInsertDIM"); }
    vector<string> stmts, nameDIM, dirt = { "'" }, soap = { "''" };
    string temp, stmt, tname;
    vector<int> spaceHistory = { 0 };
    int index, inum, iextra, space, indent, indexDIM = 0;
    size_t pos1 = metaFile.find("Definitions"), pos2, posMID, posMIDnext, posEnd;
    posMID = metaFile.find("Member" + nl, pos1);
    posMIDnext = metaFile.find("Member" + nl, posMID + 1);  // To avoid the last one,
    while (posMIDnext < metaFile.size())                    // which is 'Dim'.
    {
        nameDIM.clear();
        iextra = -1;
        posEnd = metaFile.find(nl + nl + nl, posMID);
        pos1 = metaFile.find_first_of("1234567890", posMID);
        while (pos1 < posEnd)
        {
            pos2 = metaFile.find('.', pos1);
            temp = metaFile.substr(pos1, pos2 - pos1);
            try { inum = stoi(temp); }
            catch (invalid_argument) { jf.err("stoi-sc.makeCreateInsertDIM"); }
            pos2++;
            pos1 = metaFile.find_first_not_of(" ", pos2);
            if (iextra < 0) 
            { 
                iextra = pos1 - pos2; 
                indent = 0; 
            }
            else
            {
                space = pos1 - pos2 - iextra;
                for (int ii = 0; ii < spaceHistory.size(); ii++)
                {
                    if (spaceHistory[ii] == space) { indent = ii; break; }
                    else if (spaceHistory[ii] > space) { jf.err("Inconsistent spacing-sc.makeCreateInsertDIM"); }
                    else if (ii == spaceHistory.size() - 1)
                    {
                        indent = spaceHistory.size();
                        spaceHistory.push_back(space);
                    }
                }
            }
            temp.assign(indent, '+');
            pos2 = metaFile.find(nl, pos1);
            nameDIM.push_back(temp + metaFile.substr(pos1, pos2 - pos1));
            pos1 = metaFile.find_first_of("1234567890", pos2);
        }

        tname = "Census$" + cataYear + "$" + cataName + "$DIM$" + to_string(indexDIM);
        stmt = "CREATE TABLE IF NOT EXISTS \"" + tname;
        stmt += "\" (MID INTEGER PRIMARY KEY, DIM TEXT);";
        stmts.push_back(stmt);

        for (int ii = 0; ii < nameDIM.size(); ii++)
        {
            jf.clean(nameDIM[ii], dirt, soap);
            stmt = "INSERT OR IGNORE INTO \"" + tname + "\" (MID, DIM) VALUES (";
            stmt += to_string(ii + 1) + ", '" + nameDIM[ii] + "');";
            stmts.push_back(stmt);
        }
        indexDIM++;
        posMID = posMIDnext;
        posMIDnext = metaFile.find("Member" + nl, posMID + 1);
    }
    return stmts;
}
vector<string> STATSCAN::makeCreateInsertDim(vector<string>& dimList)
{
    // Note: dimList is an output for the list of dims, to be used later.
    if (metaFile.size() < 1) { jf.err("No init-sc.makeCreateInsertDim"); }
    vector<string> stmts, dirt = { "'" }, soap = { "''" };
    string temp, stmt, tname;
    vector<int> spaceHistory = { 0 };
    int index, inum, iextra = -1, space, indent, indexDim = 0;
    size_t posMID = metaFile.rfind("Member" + nl);
    size_t pos1 = metaFile.find_first_of("1234567890", posMID + 7), pos2;
    dimList.clear();
    while (pos1 < metaFile.size())
    {
        pos2 = metaFile.find('.', pos1);
        temp = metaFile.substr(pos1, pos2 - pos1);
        try { inum = stoi(temp); }
        catch (invalid_argument) { jf.err("stoi-sc.makeCreateInsertDim"); }
        pos2++;
        pos1 = metaFile.find_first_not_of(" ", pos2);
        if (iextra < 0)
        {
            iextra = pos1 - pos2;
            indent = 0;
        }
        else
        {
            space = pos1 - pos2 - iextra;
            for (int ii = 0; ii < spaceHistory.size(); ii++)
            {
                if (spaceHistory[ii] == space) { indent = ii; break; }
                else if (spaceHistory[ii] > space) { jf.err("Inconsistent spacing-sc.makeCreateInsertDim"); }
                else if (ii == spaceHistory.size() - 1)
                {
                    indent = spaceHistory.size();
                    spaceHistory.push_back(space);
                }
            }
        }
        temp.assign(indent, '+');
        pos2 = metaFile.find(nl, pos1);
        dimList.push_back(temp + metaFile.substr(pos1, pos2 - pos1));
        pos1 = metaFile.find_first_of("1234567890", pos2);
    }

    tname = "Census$" + cataYear + "$" + cataName + "$Dim";
    stmt = "CREATE TABLE IF NOT EXISTS \"" + tname;
    stmt += "\" (MID INTEGER PRIMARY KEY, Dim TEXT);";
    stmts.push_back(stmt);

    for (int ii = 0; ii < dimList.size(); ii++)
    {
        jf.clean(dimList[ii], dirt, soap);
        stmt = "INSERT OR IGNORE INTO \"" + tname + "\" (MID, Dim) VALUES (";
        stmt += to_string(ii + 1) + ", '" + dimList[ii] + "');";
        stmts.push_back(stmt);
    }
    return stmts;
}
vector<string> STATSCAN::makeCreateInsertTopic(vector<string>& colTitles)
{
    // Ensure that this catalogue's topic exists as a column title in the year's topic table.
    if (metaFile.size() < 1) { jf.err("No init-sc.makeCreateTopic"); }
    vector<string> stmts, dirt = { "'" }, soap = { "''" };
    string tname = "Census$" + cataYear + "$Topic", stmt, temp;
    int mode = 0;  // 0 = Table does not exist, 1 = Topic column does not exist, 2 = Topic column already exists.
    for (int ii = 0; ii < colTitles.size(); ii++)
    {
        if (colTitles[ii] == topic) { mode = 2; break; }
        else if (ii == colTitles.size() - 1) { mode = 1; }
    }
    temp = topic;
    jf.clean(temp, dirt, soap);
    switch (mode)
    {
    case 0:
    {
        stmt = "CREATE TABLE \"" + tname + "\" (\"" + temp + "\" TEXT, UNIQUE(\"";
        stmt += temp + "\"));";
        stmts.push_back(stmt);
        stmt = "INSERT INTO \"" + tname + "\" (\"" + temp + "\") VALUES ('";
        stmt += cataName + "');";
        stmts.push_back(stmt);
        break;
    }
    case 1:
    {
        colTitles.push_back(topic);
        stmt = "ALTER TABLE \"" + tname + "\" RENAME TO \"old_" + tname + "\";";
        stmts.push_back(stmt);
        stmt = "CREATE TABLE \"" + tname + "\" (\"" + colTitles[0] + "\" TEXT";
        for (int ii = 1; ii < colTitles.size(); ii++)
        {
            stmt += ", \"" + colTitles[ii] + "\" TEXT";
        }
        for (int ii = 0; ii < colTitles.size(); ii++)
        {
            stmt += ", UNIQUE(\"" + colTitles[ii] + "\")";
        }
        stmt += ");";
        stmts.push_back(stmt);
        stmt = "INSERT INTO \"" + tname + "\" SELECT * FROM \"old_" + tname + "\";";
        stmts.push_back(stmt);
        stmt = "DROP TABLE \"old_" + tname + "\";";
        stmts.push_back(stmt);
        stmt = "INSERT OR IGNORE INTO \"" + tname + "\" (\"" + topic + "\")";
        stmt += " VALUES ('" + cataName + "');";
        stmts.push_back(stmt);
        break;
    }
    case 2:
    {
        stmt = "INSERT OR IGNORE INTO \"" + tname + "\" (\"" + temp + "\")";
        stmt += " VALUES ('" + cataName + "');";
        stmts.push_back(stmt);
        break;
    }
    }
    return stmts;
}
string STATSCAN::makeCreateYear()
{
    if (metaFile.size() < 1) { jf.err("No init-sc.makeCreateYear"); }
    string stmt = "CREATE TABLE \"Census$" + cataYear + "\" (Catalogue TEXT,";
    stmt += " Topic TEXT, UNIQUE(Catalogue));";
    return stmt;
}
string STATSCAN::makeInsertCensus()
{
    if (metaFile.size() < 1) { jf.err("No init-sc.makeInsertCensus"); }
    string stmt = "INSERT OR IGNORE INTO Census (Year) VALUES (";
    stmt += cataYear + ");";
    return stmt;
}
vector<string> STATSCAN::makeInsertData(string GEO_CODE, string& geoStmt)
{
    // Note that the first returned string is the template, others are single parameters.
    vector<string> stmt, dirt = { "'" }, soap = { "''" };
    geoStmt.clear();  
    int sizeDIM = mapDIM.size(), sizeDim = mapDim.size();
    string csvLine, csvBuffer, temp;
    string tname = "Census$" + cataYear + "$" + cataName + "$" + GEO_CODE;
    string tnameGeo = "Geo$" + cataYear + "$" + cataName;

    // Make the template statement.
    string stmt0 = "INSERT OR IGNORE INTO \"" + tname + "\" VALUES (";
    for (int jj = 0; jj < sizeDIM; jj++)
    {
        if (jj > 0) { stmt0 += ", "; }
        stmt0 += "@D" + to_string(jj);
    }
    for (int jj = 0; jj < sizeDim; jj++)
    {
        stmt0 += ", @d" + to_string(jj);
    }
    stmt0 += ");";
    stmt.push_back(stmt0);

    // Locate the first line containing the objective GEO_CODE.
    size_t pos1 = activeCSV.find(nl), posStart = 0;
    size_t pos2 = activeCSV.find(nl, pos1 + 1);
    while (pos2 < activeCSV.size())
    {
        csvLine = activeCSV.substr(pos1, pos2 - pos1);
        temp = extractCSVLineValue(csvLine, geoCodeCol);
        while (temp[0] == '0') { temp.erase(temp.begin()); }
        if (temp == GEO_CODE) { posStart = pos1; break; }
        pos1 = pos2;
        pos2 = activeCSV.find(nl, pos1 + 1);
    }
    if (!posStart) { jf.err("Failed to locate first GEO_CODE line-sc.makeInsertData"); }

    // Specify (in order!) which columns are to be extracted.
    vector<int> scoopIndex = {
        geoLevelCol,
        geoNameCol,
        geoCodeCol
    };
    for (int ii = 0; ii < sizeDIM; ii++)
    {
        scoopIndex.push_back(mapDIM.at(ii));
    }
    for (int ii = 0; ii < sizeDim; ii++)
    {
        scoopIndex.push_back(mapDim.at(ii));
    }

    // Extract the desired data, line by line, until a new GEO_CODE is found.
    vector<string> lineData;
    size_t pos3, pos4;
    int index, maxPART;
    try { maxPART = stoi(geo[geo.size() - 1][3]); }
    catch (invalid_argument) { jf.err("stoi-sc.makeInsertData"); }
    pos1 = posStart;
    pos2 = activeCSV.find(nl, pos1 + 1);
    csvLine = activeCSV.substr(pos1, pos2 - pos1);
    while (1)
    {
        lineData = extractCSVLineValue(csvLine, scoopIndex);
        while (lineData[2][0] == '0') { lineData[2].erase(lineData[2].begin()); }
        if (lineData[2] != GEO_CODE) { break; }  // Loop exit.
        pos3 = lineData[1].find('(');
        if (pos3 < lineData[1].size())
        {
            pos4 = lineData[1].find(" part", pos3);
            if (pos4 < lineData[1].size())
            {
                pos1 = pos2;
                pos2 = activeCSV.find(nl, pos1 + 1);
                if (pos2 > activeCSV.size())  // Line is broken by file change.
                {
                    csvBuffer = activeCSV.substr(pos1);
                    if (activeCSVpart < maxPART)
                    {
                        advanceNextCSVpart();
                        pos2 = activeCSV.find_first_of(nl);
                        if (pos2 > 0) { csvLine = csvBuffer + activeCSV.substr(0, pos2); }
                        else { csvLine = csvBuffer; }
                    }
                    else { return stmt; }
                }
                else { csvLine = activeCSV.substr(pos1, pos2 - pos1); }
                continue;
            }
        }

        if (geoStmt.size() < 1)
        {
            jf.clean(lineData[1], dirt, soap);
            geoStmt = "INSERT OR IGNORE INTO \"" + tnameGeo + "\" (GEO_CODE, ";
            geoStmt += "\"Region Name\", GEO_LEVEL) VALUES (" + lineData[2];
            geoStmt += ", '" + lineData[1] + "', " + lineData[0] + ");";
        }

        index = stmt.size();
        stmt.resize(index + sizeDIM + sizeDim);
        for (int ii = 0; ii < sizeDIM; ii++)
        {
            stmt[index + ii] = lineData[3 + ii];
        }
        for (int ii = 0; ii < sizeDim; ii++)
        {
            stmt[index + sizeDIM + ii] = lineData[3 + sizeDIM + ii];
        }

        pos1 = pos2;
        pos2 = activeCSV.find(nl, pos1 + 1);
        if (pos2 > activeCSV.size())  // Line is broken by file change.
        {
            csvBuffer = activeCSV.substr(pos1);
            if (activeCSVpart < maxPART)
            {
                advanceNextCSVpart();
                pos2 = activeCSV.find_first_of(nl);
                if (pos2 > 0) { csvLine = csvBuffer + activeCSV.substr(0, pos2); }
                else { csvLine = csvBuffer; }
            }
            else { return stmt; }
        }
        else { csvLine = activeCSV.substr(pos1, pos2 - pos1); }
    }
    return stmt;
}
vector<string> STATSCAN::makeInsertGeo(vector<vector<string>>& geoList)
{
    if (activeCSVgeocode != "Done") { jf.err("Incomplete geoList-sc.makeCreateGeo"); }
    vector<string> stmts(geoList.size()), geoHistory;
    vector<string> dirt = { "'" }, soap = { "''" };
    string regionName, stmt, stmt0 = "INSERT OR IGNORE INTO \"Census$" + cataYear;
    stmt0 += "$" + cataName + "$Geo\" (GEO_CODE, \"Region Name\", GEO_LEVEL";
    int geoLevel;
    for (int ii = 0; ii < geoList.size(); ii++)
    {
        try { geoLevel = stoi(geoList[ii][2]); }
        catch (invalid_argument) { jf.err("stoi-sc.makeInsertGeo"); }
        if (geoLevel >= geoHistory.size())
        {
            geoHistory.push_back(geoList[ii][0]);
        }
        else
        {
            geoHistory[geoLevel] = geoList[ii][0];
        }
        regionName = geoList[ii][1];
        jf.clean(regionName, dirt, soap);
        stmt = stmt0;
        for (int jj = 0; jj < geoLevel; jj++)
        {
            stmt += ", Ancestor" + to_string(jj);
        }
        stmt += ") VALUES (" + geoList[ii][0] + ", '" + regionName;
        stmt += "', " + geoList[ii][2];
        for (int jj = 0; jj < geoLevel; jj++)
        {
            stmt += ", " + geoHistory[jj];
        }
        stmt += ");";
        stmts[ii] = stmt;
    }
    return stmts;
}
string STATSCAN::makeInsertYear()
{
    if (metaFile.size() < 1) { jf.err("No init-sc.makeInsertCensus"); }
    string temp = topic;
    vector<string> dirt = { "'" }, soap = { "''" };
    jf.clean(temp, dirt, soap);
    string stmt = "INSERT OR IGNORE INTO \"Census$" + cataYear + "\" (";
    stmt += "Catalogue, Topic) VALUES ('" + cataName + "', '" + temp + "');";
    return stmt;
}

unordered_map<string, string> STATSCAN::mapGeoCodeToPart(vector<vector<string>>& geoList)
{
    // Return an unordered_map connecting GEO_CODE->PART. PART is the CSV wherein 
    // that GEO_CODE begins (not necessarily ends). 
    unordered_map<string, string> mapGeoPart;
    for (int ii = 0; ii < geoList.size(); ii++)
    {
        mapGeoPart.emplace(geoList[ii][0], geoList[ii][2]);
    }
    return mapGeoPart;
}
vector<vector<string>> STATSCAN::parseNavSearch(string& navSearchBlob)
{
    vector<vector<string>> navSearch;
    size_t pos1 = navSearchBlob.find_first_of("\r\n");
    size_t pos2 = navSearchBlob.find('\n', pos1) + 1;
    nl = navSearchBlob.substr(pos1, pos2 - pos1);
    nls = nl.size();
    pos1 = navSearchBlob.find_last_of("1234567890", pos1);
    pos2 = navSearchBlob.find_first_not_of("1234567890", pos1);
    string temp = navSearchBlob.substr(pos1, pos2 - pos1);
    int navSearchTerms, index;
    try { navSearchTerms = stoi(temp); }
    catch (invalid_argument) { jf.err("stoi-MainWindow.initGUI"); }
    pos1 = navSearchBlob.find(nl, pos2) + nls;
    while (pos1 < navSearchBlob.size())
    {
        index = navSearch.size();
        navSearch.push_back(vector<string>(navSearchTerms));
        for (int ii = 0; ii < navSearchTerms - 1; ii++)
        {
            pos2 = navSearchBlob.find(',', pos1);
            navSearch[index][ii] = navSearchBlob.substr(pos1, pos2 - pos1);
            pos1 = pos2 + 1;
        }
        pos2 = navSearchBlob.find(nl, pos1);
        navSearch[index][navSearchTerms - 1] = navSearchBlob.substr(pos1, pos2 - pos1);
        pos1 = navSearchBlob.find_first_not_of("\r\n", pos2);
    }
    return navSearch;
}

vector<vector<int>> STATSCAN::readBinBorder(string& binFile)
{
    vector<vector<int>> border;
    size_t pos1 = binFile.find("//border"), nls;
    if (pos1 > binFile.size()) { return border; }
    size_t pos2 = binFile.find_first_of("1234567890", pos1);
    string temp = binFile.substr(pos1, pos2 - pos1), nl, sx, sy;
    pos2 = temp.find('\r');
    if (pos2 > temp.size()) { nl = "\n"; nls = 1; }
    else { nl = "\r\n"; nls = 2; }
    temp = nl + nl;
    size_t posEnd = binFile.find(temp, pos1);
    if (posEnd > binFile.size())
    {
        if (nls == 1)
        {
            temp = "\r\n\r\n";
            posEnd = binFile.find(temp, pos1);
            if (posEnd > binFile.size()) { jf.err("No posEnd found-sc.readBinBorder"); }
        }
        else
        {
            temp = "\n\n";
            posEnd = binFile.find(temp, pos1);
            if (posEnd > binFile.size()) { jf.err("No posEnd found-sc.readBinBorder"); }
        }
    }

    int ix, iy;
    pos1 = binFile.find_first_of("1234567890", pos1);
    while (pos1 < posEnd)
    {
        pos2 = binFile.find(',', pos1);
        sx = binFile.substr(pos1, pos2 - pos1);
        pos1 = pos2 + 1;
        pos2 = binFile.find(nl, pos1);
        sy = binFile.substr(pos1, pos2 - pos1);
        try
        {
            ix = stoi(sx);
            iy = stoi(sy);
        }
        catch (invalid_argument) { jf.err("stoi-sc.readBinBorder"); }
        border.push_back({ ix, iy });
        pos1 = pos2 + nls;
    }
    return border;
}
vector<vector<vector<int>>> STATSCAN::readBinFrames(string& binFile)
{
    vector<vector<vector<int>>> frames;
    size_t pos1 = binFile.find("//frames"), nls;
    if (pos1 > binFile.size()) { return frames; }
    size_t pos2 = binFile.find_first_of("1234567890", pos1);
    string temp = binFile.substr(pos1, pos2 - pos1), nl, sx, sy;
    pos2 = temp.find('\r');
    if (pos2 > temp.size()) { nl = "\n"; nls = 1; }
    else { nl = "\r\n"; nls = 2; }
    temp = nl + nl;
    size_t posEnd = binFile.find(temp, pos1);
    if (posEnd > binFile.size()) { jf.err("No posEnd found-sc.readBinFrames"); }

    int ix, iy, index;
    pos1 = binFile.find_first_of("1234567890", pos1);
    while (pos1 < posEnd)
    {
        index = frames.size();
        frames.push_back(vector<vector<int>>());
        pos2 = binFile.find(',', pos1);
        sx = binFile.substr(pos1, pos2 - pos1);
        pos1 = pos2 + 1;
        pos2 = binFile.find('@', pos1);
        sy = binFile.substr(pos1, pos2 - pos1);
        try
        {
            ix = stoi(sx);
            iy = stoi(sy);
        }
        catch (invalid_argument) { jf.err("stoi-sc.readBinFrames"); }
        frames[index].push_back({ ix, iy });

        pos1 = pos2 + 1;
        pos2 = binFile.find(',', pos1);
        sx = binFile.substr(pos1, pos2 - pos1);
        pos1 = pos2 + 1;
        pos2 = binFile.find(nl, pos1);
        sy = binFile.substr(pos1, pos2 - pos1);
        try
        {
            ix = stoi(sx);
            iy = stoi(sy);
        }
        catch (invalid_argument) { jf.err("stoi-sc.readBinFrames"); }
        frames[index].push_back({ ix, iy });
        pos1 = pos2 + nls;
    }
    return frames;
}
string STATSCAN::readBinParent(string& binFile)
{
    string parent = "";
    size_t pos1 = binFile.find("//parent");
    if (pos1 > binFile.size()) { return parent; }
    pos1 = binFile.find('\n', pos1) + 1;
    size_t pos2 = binFile.find_first_of("\r\n", pos1);
    parent = binFile.substr(pos1, pos2 - pos1);
    return parent;
}
vector<double> STATSCAN::readBinPositionGPS(string& binFile)
{
    vector<double> position = { -181.0, -181.0 };
    size_t pos1 = binFile.find("//position(GPS)");
    if (pos1 > binFile.size()) { return position; }
    pos1 = binFile.find('\n', pos1) + 1;
    size_t pos2 = binFile.find(',', pos1);
    string latitude = binFile.substr(pos1, pos2 - pos1);
    pos1 = pos2 + 1;
    pos2 = binFile.find_first_of("\r\n", pos1);
    string longitude = binFile.substr(pos1, pos2 - pos1);
    position.resize(2);
    try
    {
        position[0] = stod(latitude);
        position[1] = stod(longitude);
    }
    catch (invalid_argument) { jf.err("stod-sc.readBinPosition"); }
    return position;
}
vector<double> STATSCAN::readBinPositionPNG(string& binFile)
{
    vector<double> position = { -1.0, -1.0 };
    size_t pos1 = binFile.find("//position(PNG)");
    if (pos1 > binFile.size()) { return position; }
    pos1 = binFile.find('\n', pos1) + 1;
    size_t pos2 = binFile.find(',', pos1);
    string xPercent = binFile.substr(pos1, pos2 - pos1);
    pos1 = pos2 + 1;
    pos2 = binFile.find_first_of("\r\n", pos1);
    string yPercent = binFile.substr(pos1, pos2 - pos1);
    position.resize(2);
    try
    {
        position[0] = stod(xPercent);
        position[1] = stod(yPercent);
    }
    catch (invalid_argument) { jf.err("stod-sc.readBinPosition"); }
    return position;
}
double STATSCAN::readBinScale(string& binFile)
{
    double scale = -1.0;
    size_t pos1 = binFile.find("//scale(pixels per km)");
    if (pos1 > binFile.size()) { return scale; }
    pos1 = binFile.find('\n', pos1) + 1;
    size_t pos2 = binFile.find_first_of("\r\n", pos1);
    string sScale = binFile.substr(pos1, pos2 - pos1);
    try { scale = stod(sScale); }
    catch (invalid_argument) { jf.err("stod-sc.readBinScale"); }
    return scale;
}

void STATSCAN::trimMID(string& MID)
{
    size_t pos1 = MID.find('('), pos2;
    string temp;
    int inum;
    while (pos1 < MID.size())
    {
        pos2 = MID.find(')', pos1);
        temp = MID.substr(pos1 + 1, pos2 - pos1 - 1);
        try { inum = stoi(temp); }
        catch (invalid_argument)
        {
            pos1 = MID.find('(', pos2);
            continue;
        }
        MID.erase(MID.begin() + pos1, MID.begin() + pos2 + 1);
        pos1 = MID.find('(', pos1);
    }
    while (MID.back() == ' ') { MID.pop_back(); }
}
void STATSCAN::uptickBookmark(vector<vector<string>>& geoList)
{
    int geoCodeIndex = getGeoCodeIndex(activeCSVgeocode);
    string nextGeoCode;
    if (geoCodeIndex < geoList.size() - 1)
    {
        nextGeoCode = geoList[geoCodeIndex + 1][0];
    }
    else
    {
        nextGeoCode = "Done";
    }
    activeCSVgeocode = nextGeoCode;
    makeBookmark(activeCSVpart, nextGeoCode);
}

string STATSCAN::urlCatalogue(string sYear, string sCata)
{
    string urlCata, urlTemp;
    wstring wPage, wTemp, wCata;
    size_t pos1, pos2;
    int iYear;
    try { iYear = stoi(sYear); }
    catch (invalid_argument) { jf.err("stoi-sc.urlCatalogue"); }
    if (iYear >= 2016)
    {
        urlTemp = "www12.statcan.gc.ca/datasets/Index-eng.cfm?Temporal=";
        urlTemp += to_string(iYear);
        wPage = wf.browseW(urlTemp);
        wCata = jf.utf8to16(sCata);
        wTemp = L"title=\"Dataset " + wCata;
        pos2 = wPage.find(wTemp);
        pos1 = wPage.find(L"PID=", pos2) + 4;
        pos2 = wPage.find_first_not_of(L"1234567890", pos1);
        wCata = wPage.substr(pos1, pos2 - pos1);
        urlTemp = jf.utf16to8(wCata);
        urlCata = "www12.statcan.gc.ca/census-recensement/2016/dp-pd";
        urlCata += "/dt-td/Rp-eng.cfm?LANG=E&APATH=3&DETAIL=0&DIM=0";
        urlCata += "&FL=A&FREE=0&GC=0&GID=0&GK=0&GRP=1&PID=";
        urlCata += urlTemp + "&PRID=10&PTYPE=109445&S=0&SHOWALL=0";
        urlCata += "&SUB=0&Temporal=" + to_string(iYear) + "&THEME=123";
        urlCata += "&VID=0&VNAMEE=&VNAMEF=";
    }
    return urlCata;
}
string STATSCAN::urlCSVDownload(string sYear, string sCata)
{
    string url;
    int iYear;
    try { iYear = stoi(sYear); }
    catch (invalid_argument) { jf.err("stoi-sc.urlCSVDownload"); }
    if (iYear >= 2016)
    {
        string urlTemp = "www12.statcan.gc.ca/datasets/Index-eng.cfm?Temporal=" + sYear;
        wstring wPage = wf.browseW(urlTemp);
        wstring wCata = jf.utf8to16(sCata);
        wstring wTemp = L"title=\"Dataset " + wCata;
        size_t pos2 = wPage.find(wTemp);
        size_t pos1 = wPage.find(L"PID=", pos2) + 4;
        pos2 = wPage.find_first_not_of(L"1234567890", pos1);
        wTemp = wPage.substr(pos1, pos2 - pos1);
        url = "www12.statcan.gc.ca/census-recensement/2016/dp-pd/dt-td/";
        url += "CompDataDownload.cfm?LANG=E&PID=" + jf.utf16to8(wTemp);
        url += "&OFT=CSV";
    }
    return url;
}
string STATSCAN::urlGeo(string urlCata)
{
    size_t pos1 = urlCata.find('?') + 1;
    string url = urlCata.substr(0, pos1) + "TABID=6&";
    url += urlCata.substr(pos1);
    url += "&D1=0&D2=0&D3=0&D4=0&D5=0&D6=0";
    return url;
}
string STATSCAN::urlYear(string syear)
{
    string url = "www12.statcan.gc.ca/datasets/index-eng.cfm?Temporal=" + syear;
    return url;
}
