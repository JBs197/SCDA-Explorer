#include "statscan.h"

void STATSCAN::advanceNextCSVpart()
{
    activeCSVpart++;
    string csvPath = cataPath + "\\" + cataName + "_English_CSV_data (PART ";
    csvPath += to_string(activeCSVpart) + ").csv";
    activeCSV = wf.load(csvPath);
    if (activeCSV.size() < 1) { jf.err("Cannot load CSV part file-sc.advanceNextCSVpart"); }
}
void STATSCAN::convertSCgeo(string& geoPathOld)
{
    string geoFile = wf.load(geoPathOld), csvPath, csvFile, csvBuffer, csvLine;
    size_t pos1 = geoPathOld.rfind('\\'), pos2, posEnd;
    string folderPath = geoPathOld.substr(0, pos1), temp, currentAGC;
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
        vvsData[index][0] = geoFile.substr(pos1, pos2 - pos1);
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
            for (int ii = 0; ii < colIndex; ii++)
            {
                pos1 = csvFile.find(',', pos1 + 1);
            }
            pos1 = csvFile.find_first_not_of(",\"", pos1);
            pos2 = csvFile.find('"', pos1);
            currentAGC = csvFile.substr(pos1, pos2 - pos1);
            vvsData[0][2] = currentAGC;
            indexAGC = 0;
        }
        else
        {
            posEnd = csvFile.find(nl);
            csvLine = csvBuffer + csvFile.substr(0, posEnd);
            // RESUME HERE.
        }
    }

    string geoFileNew = "ALT_GEO_CODE, Region Name, PART begin, PART end\n";
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
    int numCol = cataColTitles.size(), index = 0, stepsForward;
    size_t pos1 = 0, pos2, posLast = csvLine.size() - 1;
    for (int ii = 0; ii < colIndex.size(); ii++)
    {
        stepsForward = colIndex[ii] - index;
        for (int jj = 0; jj < stepsForward; jj++)
        {
            pos1 = csvLine.find(',', pos1 + 1);
            if (pos1 > posLast) { jf.err("Stepped beyond column width-sc.extractCSVLineValue"); }
        }
        index += stepsForward;
        pos1++;
        if (csvLine[pos1] == ',')  // No sValue for this column.
        {
            sValue[ii] = "";
            continue;
        }
        else if (csvLine[pos1] == '"') { pos1++; }
        if (ii < colIndex.size() - 1)
        {
            pos2 = csvLine.find(',', pos1);
            if (csvLine[pos2 - 1] == '"') { pos2--; }
            sValue[ii] = csvLine.substr(pos1, pos2 - pos1);
        }
        else { sValue[ii] = csvLine.substr(pos1); }
    }
    return sValue;
}
int STATSCAN::getGeoCodeIndex(string& sActiveCSVgeocode)
{
    int gci;
    try { gci = mapGeoCode.at(sActiveCSVgeocode); }
    catch (out_of_range) { jf.err("mapGeoCode-sc.getGeoCodeIndex"); }
    return gci;
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
int STATSCAN::initCSV(vector<vector<string>>& geoList)
{
    // Initializes all 'activeCSV' variables. Returns number of regions already completed.
    if (metaFile.size() < 1) { jf.err("No init-sc.initCSV"); }
    size_t pos1, pos2;
    loadBookmark(activeCSVpart, activeCSVgeocode);
    string csvPath = cataPath + "\\" + cataName + "_English_CSV_data (PART ";
    csvPath += to_string(activeCSVpart) + ").csv";
    activeCSV = wf.load(csvPath);
    cataColTitles = loadColTitles();

    int indexDim = 0, indexDIM = 0, geoListIndex;
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
    }

    if (activeCSVgeocode == "-1")
    {
        pos1 = activeCSV.find(nl) + nls;
        pos2 = activeCSV.find(nl, pos1);
        string csvLine = activeCSV.substr(pos1, pos2 - pos1);
        activeCSVgeocode = extractCSVLineValue(csvLine, geoCodeCol);
        makeBookmark(activeCSVpart, activeCSVgeocode);
    }
    geoListIndex = getGeoCodeIndex(activeCSVgeocode);
    return geoListIndex;
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
    if (metaFile.size() < 1) { jf.err("No init-sc.loadGeoList"); }
    string geoFile = wf.load(geoPath);
    if (geoFile.size() < 1) { jf.err("Empty geoFile-sc.loadGeoList"); }
    mapGeoCode.clear();
    vector<vector<string>> geoList;  // Form [region index][GEO_CODE, Region Name].
    size_t pos1 = geoFile.find_first_of("1234567890"), pos2, index;
    while (pos1 < geoFile.size())
    {
        index = geoList.size();
        geoList.push_back(vector<string>(2));
        pos2 = geoFile.find('"', pos1);
        geoList[index][0] = geoFile.substr(pos1, pos2 - pos1);
        mapGeoCode.emplace(geoList[index][0], index);
        pos1 = geoFile.find_first_not_of("\", ", pos2);
        pos2 = geoFile.find('"', pos1);
        geoList[index][1] = geoFile.substr(pos1, pos2 - pos1);
        pos1 = geoFile.find(nl, pos2);
        pos1 = geoFile.find_first_of("1234567890", pos1);
    }
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
vector<string> STATSCAN::makeCreateData(vector<vector<string>>& geoList)
{
    int mapDIMSize = mapDIM.size();
    int mapDimSize = mapDim.size();
    if (mapDIMSize < 1 || mapDimSize < 1) { jf.err("No initCSV-sc.makeCreateData"); }
    vector<string> stmts(geoList.size());
    string stmt0 = "CREATE TABLE IF NOT EXISTS \"Census$" + cataYear + "$" + cataName + "$";
    string stmt1 = "$Data\" (";
    for (int ii = 0; ii < mapDIMSize; ii++)
    {
        if (ii > 0) { stmt1 += ", "; }
        stmt1 += "DIM" + to_string(ii) + " INT";
    }
    for (int ii = 0; ii < mapDimSize; ii++)
    {
        stmt1 += ", MID" + to_string(ii + 1) + " INT";
    }
    stmt1 += ", UNIQUE(";
    for (int ii = 0; ii < mapDIMSize; ii++)
    {
        if (ii > 0) { stmt1 += ", "; }
        stmt1 += "DIM" + to_string(ii);
    }
    for (int ii = 0; ii < mapDimSize; ii++)
    {
        stmt1 += ", MID" + to_string(ii + 1);
    }
    stmt1 += "));";
    for (int ii = 0; ii < geoList.size(); ii++)
    {
        stmts[ii] = stmt0 + geoList[ii][0] + stmt1;
    }
    return stmts;
}
string STATSCAN::makeCreateGeo(vector<vector<string>>& geoList)
{
    if (activeCSVgeocode != "Done") { jf.err("Incomplete geoList-sc.makeCreateGeo"); }
    string stmt = "CREATE TABLE IF NOT EXISTS \"Census$" + cataYear + "$";
    stmt += cataName + "$Geo\" (GEO_CODE INTEGER PRIMARY KEY, \"Region Name\" TEXT";
    stmt += ", GEO_LEVEL INT";
    for (int ii = 0; ii < geoMaxLevel; ii++)
    {
        stmt += ", Ancestor" + to_string(ii) + " INT";
    }
    stmt += ");";
    return stmt;
}
vector<string> STATSCAN::makeCreateInsertCatalogue()
{
    if (cataColTitles.size() < 1) { cataColTitles = loadColTitles(); }
    vector<string> stmts, DIMs;
    string temp, tname = "Census$" + cataYear + "$" + cataName;
    string stmt = "CREATE TABLE IF NOT EXISTS \"" + tname;
    stmt += "\" (\"Dimension Index\" INTEGER PRIMARY KEY, DIM TEXT);";
    stmts.push_back(stmt);
    size_t index = 0, pos1;
    for (int ii = 0; ii < cataColTitles.size(); ii++)
    {
        pos1 = cataColTitles[ii].find("DIM:");
        if (pos1 > cataColTitles[ii].size()) { continue; }
        pos1 += 4;
        pos1 = cataColTitles[ii].find_first_not_of(' ', pos1);
        temp = cataColTitles[ii].substr(pos1);
        trimMID(temp);
        DIMs.push_back(temp);
    }
    for (int ii = 0; ii < DIMs.size(); ii++)
    {
        stmt = "INSERT OR IGNORE INTO \"" + tname + "\" (\"Dimension Index\", DIM)";
        stmt += " VALUES (" + to_string(ii) + ", '" + DIMs[ii] + "');";
        stmts.push_back(stmt);
    }
    return stmts;
}
vector<string> STATSCAN::makeCreateInsertDefinitions()
{
    if (metaFile.size() < 1) { jf.err("No init-sc.makeCreateInsertDefinitions"); }
    vector<string> dim, defn;
    int index;
    size_t pos1 = metaFile.find("Definitions"), pos2, posMID;
    posMID = metaFile.find("Member", pos1);
    while (posMID < metaFile.size())
    {
        index = dim.size();
        pos2 = metaFile.rfind(nl + nl + nl, posMID);
        pos1 = metaFile.find_last_of("\r\n", pos2 - 1) + 1;
        defn.push_back(metaFile.substr(pos1, pos2 - pos1));
        pos2 = metaFile.rfind("Definition", pos1);
        pos2 = metaFile.find_last_not_of("\r\n", pos2 - 1) + 1;
        pos1 = metaFile.find_last_of("\r\n", pos2 - 1) + 1;
        dim.push_back(metaFile.substr(pos1, pos2 - pos1));
        trimMID(dim[index]);
        posMID = metaFile.find("Member", posMID + 1);
    }
    if (dim.size() != defn.size()) { jf.err("Size mismatch-sc.makeCreateInsertDefinitions"); }

    vector<string> stmts(1), dirt = { "'" }, soap = { "''" };
    string stmt, tname = "Census$" + cataYear + "$" + cataName + "$Definitions";
    stmts[0] = "CREATE TABLE IF NOT EXISTS \"" + tname;
    stmts[0] += "\" (DIM TEXT, Definition TEXT, UNIQUE(DIM));";
    for (int ii = 0; ii < dim.size(); ii++)
    {
        jf.clean(dim[ii], dirt, soap);
        jf.clean(defn[ii], dirt, soap);
        stmt = "INSERT OR IGNORE INTO \"" + tname + "\" (DIM, Definition) VALUES ('";
        stmt += dim[ii] + "', '" + defn[ii] + "');";
        stmts.push_back(stmt);
    }
    return stmts;
}
vector<vector<string>> STATSCAN::makeCreateInsertDIM()
{
    // Return form [DIM index][create, insert, ...].
    if (metaFile.size() < 1) { jf.err("No init-sc.makeCreateInsertDIM"); }
    vector<vector<string>> stmts;
    vector<string> dims, dirt = { "'" }, soap = { "''" };
    string dim, temp, stmt, tname;
    vector<int> spaceHistory = { 0 };
    int index, inum, iextra, space, indent;
    size_t pos1 = metaFile.find("Definitions"), pos2, posMID, posEnd;
    posMID = metaFile.find("Member", pos1);
    while (posMID < metaFile.size())
    {
        index = stmts.size();
        stmts.push_back(vector<string>());
        dims.clear();
        iextra = -1;
        pos2 = metaFile.rfind("Definition", posMID);
        pos2 = metaFile.find_last_not_of("\r\n", pos2 - 1) + 1;
        pos1 = metaFile.rfind(nl, pos2 - 1) + nls;
        dim = metaFile.substr(pos1, pos2 - pos1);
        trimMID(dim);
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
            dims.push_back(temp + metaFile.substr(pos1, pos2 - pos1));
            pos1 = metaFile.find_first_of("1234567890", pos2);
        }

        tname = "Census$" + cataYear + "$" + cataName + "$" + dim;
        stmt = "CREATE TABLE IF NOT EXISTS \"" + tname;
        stmt += "\" (MID INTEGER PRIMARY KEY, DIM TEXT);";
        stmts[index].push_back(stmt);
        for (int ii = 0; ii < dims.size(); ii++)
        {
            jf.clean(dims[ii], dirt, soap);
            stmt = "INSERT OR IGNORE INTO \"" + tname + "\" (MID, DIM) VALUES (";
            stmt += to_string(ii + 1) + ", '" + dims[ii] + "');";
            stmts[index].push_back(stmt);
        }
        posMID = metaFile.find("Member", posMID + 1);
    }
    return stmts;
}
vector<string> STATSCAN::makeCreateInsertTopic(vector<string>& colTitles)
{
    // Ensure that this catalogue's topic exists as a column title in the year's topic table.
    if (metaFile.size() < 1) { jf.err("No init-sc.makeCreateTopic"); }
    vector<string> stmts;
    string stmt, tname = "Census$" + cataYear + "$Topic";
    int mode = 0;  // 0 = Table does not exist, 1 = Topic column does not exist, 2 = Topic column already exists.
    for (int ii = 0; ii < colTitles.size(); ii++)
    {
        if (colTitles[ii] == topic) { mode = 2; break; }
        else if (ii == colTitles.size() - 1) { mode = 1; }
    }
    switch (mode)
    {
    case 0:
    {
        stmt = "CREATE TABLE \"" + tname + "\" (\"" + topic + "\" TEXT);";
        stmts.push_back(stmt);
        stmt = "INSERT INTO \"" + tname + "\" (\"" + topic + "\") VALUES ('";
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
        stmt = "INSERT OR IGNORE INTO \"" + tname + "\" (\"" + topic + "\")";
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
    stmt += " UNIQUE(Catalogue));";
    return stmt;
}
string STATSCAN::makeInsertCensus()
{
    if (metaFile.size() < 1) { jf.err("No init-sc.makeInsertCensus"); }
    string stmt = "INSERT OR IGNORE INTO Census (Year) VALUES (";
    stmt += cataYear + ");";
    return stmt;
}
vector<string> STATSCAN::makeInsertData(vector<vector<string>>& geoList)
{
    // Return form [stmt template, param00, param01, ..., param10, param11, ...]
    vector<string> insertData;
    string csvLine, csvBuffer, geoLevel;
    string tname = "Census$" + cataYear + "$" + cataName;
    tname += "$" + activeCSVgeocode + "$Data";
    string stmt = "INSERT OR IGNORE INTO \"" + tname + "\" VALUES (";
    for (int jj = 0; jj < mapDIM.size(); jj++)
    {
        if (jj > 0) { stmt += ", "; }
        stmt += "@D" + to_string(jj);
    }
    for (int jj = 0; jj < mapDim.size(); jj++)
    {
        stmt += ", @M" + to_string(jj + 1);
    }
    stmt += ");";
    insertData.push_back(stmt);

    string search = ",\"" + activeCSVgeocode + "\",";
    size_t pos1 = activeCSV.find(search);
    if (pos1 > activeCSV.size()) { jf.err("Geocode not found-sc.makeInsertData"); }
    pos1 = activeCSV.rfind(nl, pos1) + nls;
    size_t pos2 = activeCSV.find_first_of(nl, pos1);
    if (pos2 < activeCSV.size())
    {
        csvLine = activeCSV.substr(pos1, pos2 - pos1);
    }
    else
    {
        csvBuffer = activeCSV.substr(pos1);
        advanceNextCSVpart();
        pos2 = activeCSV.find_first_of(nl);
        if (pos2 > 0) { csvLine = csvBuffer + activeCSV.substr(0, pos2); }
        else { csvLine = csvBuffer; }
    }
    int error = processCSVline(csvLine, insertData), geoCodeIndex, iGeoLevel;
    if (!error) // Add this region's geo level to the geo list.
    {
        if (geoLevelCol < 0) { jf.err("No initCSV-sc.makeInsertData"); }
        geoLevel = extractCSVLineValue(csvLine, geoLevelCol);
        geoCodeIndex = getGeoCodeIndex(activeCSVgeocode);
        geoList[geoCodeIndex].push_back(geoLevel);
        try { iGeoLevel = stoi(geoLevel); }
        catch (invalid_argument) { jf.err("stoi-sc.makeInsertData"); }
        if (iGeoLevel > geoMaxLevel) { geoMaxLevel = iGeoLevel; }
    }
    while (!error)
    {
        pos1 = activeCSV.find_first_not_of(nl, pos2);
        if (pos1 > activeCSV.size())
        {
            advanceNextCSVpart();
            pos1 = activeCSV.find_first_not_of(nl, pos2);
        }
        pos2 = activeCSV.find_first_of(nl, pos1);
        if (pos2 < activeCSV.size())
        {
            csvLine = activeCSV.substr(pos1, pos2 - pos1);
        }
        else
        {
            csvBuffer = activeCSV.substr(pos1);
            advanceNextCSVpart();
            pos2 = activeCSV.find_first_of(nl);
            if (pos2 > 0) { csvLine = csvBuffer + activeCSV.substr(0, pos2); }
            else { csvLine = csvBuffer; }
        }
        error = processCSVline(csvLine, insertData);
    }
    return insertData;

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
    string stmt = "INSERT OR IGNORE INTO \"Census$" + cataYear + "\" (";
    stmt += "Catalogue) VALUES ('" + cataName + "');";
    return stmt;
}
int STATSCAN::processCSVline(string& csvLine, vector<string>& insertData)
{
    // Establish whether this new line is still within our active GEO_CODE.
    int numDIM = mapDIM.size();
    int numDim = mapDim.size();
    if (numDIM < 1 || numDim < 1) { jf.err("No initCSV-sc.processCSVline"); }
    string lineGeoCode = extractCSVLineValue(csvLine, geoCodeCol);
    if (lineGeoCode != activeCSVgeocode) { return 1; }

    vector<int> scoopIndex;
    for (int ii = 0; ii < numDIM; ii++)
    {
        try { scoopIndex.push_back(mapDIM.at(ii)); }
        catch (out_of_range) { jf.err("mapMID-sc.processCSVline"); }
    }
    for (int ii = 0; ii < numDim; ii++)
    {
        try { scoopIndex.push_back(mapDim.at(ii)); }
        catch (out_of_range) { jf.err("mapDim-sc.processCSVline"); }
    }
    vector<string> vsLine = extractCSVLineValue(csvLine, scoopIndex);
    int dataIndex = insertData.size();
    insertData.resize(dataIndex + vsLine.size());
    for (int ii = 0; ii < vsLine.size(); ii++)
    {
        insertData[dataIndex + ii] = vsLine[ii];
    }
    return 0;
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
