#include "statscan.h"

void STATSCAN::init(string cP)
{
    cataPath = cP;
    vector<string> vsResult = wf.get_file_list(cataPath, "*English_meta.txt");
    if (vsResult.size() != 1) { jf.err("No meta file-sc.init"); }
    string metaPath = cataPath + "\\" + vsResult[0];
    metaFile = wf.load(metaPath);
    size_t pos1, pos2;
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
    pos1 = metaFile.find("Source:", pos2);
    pos2 = metaFile.find("Census", pos1);
    pos2 = metaFile.find_last_of("1234567890", pos2) + 1;
    pos1 = metaFile.rfind(' ', pos2 - 1) + 1;
    cataYear = metaFile.substr(pos1, pos2 - pos1);
    try { inum = stoi(cataYear); }
    catch (invalid_argument) { jf.err("stoi-sc.init"); }
    pos2 = metaFile.find(nl, pos2);
    pos1 = metaFile.find("Catalogue", pos1) + 9;
    temp = metaFile.substr(pos1, pos2 - pos1);
    pos1 = temp.find("no.");
    if (pos1 > temp.size())
    {
        pos1 = temp.find("Number");
        if (pos1 > temp.size()) { jf.err("Parsing cataName-sc.init"); }
        else { pos1 += 6; }
    }
    else { pos1 += 3; }
    cataName = temp.substr(pos1);
    while (cataName.front() == ' ') { cataName.erase(cataName.begin()); }
    while (cataName.back() == '.') { cataName.pop_back(); }
}
void STATSCAN::initCSV()
{
    if (metaFile.size() < 1) { jf.err("No init-sc.initCSV"); }
    string bmFile, bmPath = cataPath + "\\bookmark.txt", csvPath;
    size_t pos1, pos2;
    if (wf.file_exist(bmPath))
    {
        bmFile = jf.load(bmPath);
        pos1 = bmFile.find("PART");
        pos1 = bmFile.find('\n', pos1) + 1;
        pos2 = bmFile.find('\n', pos1);
        try { activeCSVpart = stoi(bmFile.substr(pos1, pos2 - pos1)); }
        catch (invalid_argument) { jf.err("stoi-sc.initCSV"); }
        pos1 = bmFile.find("GEO_CODE");
        pos1 = bmFile.find('\n', pos1) + 1;
        pos2 = bmFile.find('\n', pos1);
        try { activeCSVgeocode = stoi(bmFile.substr(pos1, pos2 - pos1)); }
        catch (invalid_argument) { jf.err("stoi-sc.initCSV"); }
        csvPath = cataPath + "\\" + cataName + "_English_CSV_data (PART ";
        csvPath += to_string(activeCSVpart) + ").csv";
    }
    else
    {
        bmFile = "//LASTSUCCESS(PART)\n1\n\n//NEXTREGION(GEO_CODE)\n-1\n\n";
        jf.printer(bmPath, bmFile);
        csvPath = cataPath + "\\" + cataName + "_English_CSV_data (PART 1).csv";
        activeCSVpart = 1;
        activeCSVgeocode = -1;
    }
    activeCSV = wf.load(csvPath);
    cataColTitles = loadColTitles();
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
    vector<vector<string>> geoList;  // Form [region index][GEO_CODE, Region Name].
    size_t pos1 = geoFile.find_first_of("1234567890"), pos2, index;
    while (pos1 < geoFile.size())
    {
        index = geoList.size();
        geoList.push_back(vector<string>(2));
        pos2 = geoFile.find('"', pos1);
        geoList[index][0] = geoFile.substr(pos1, pos2 - pos1);
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
string STATSCAN::makeCreateCensus()
{
    if (metaFile.size() < 1) { jf.err("No init-sc.makeCreateCensus"); }
    string stmt = "CREATE TABLE Census (Year INTEGER PRIMARY KEY);";
    return stmt;
}
vector<string> STATSCAN::makeCreateData()
{
    if (cataColTitles.size() < 1) { initCSV(); }
    vector<string> stmts;
    string temp, tname, stmt;
    vector<int> maxDIM;
    size_t pos1, pos2;
    for (int ii = 0; ii < cataColTitles.size(); ii++)
    {
        pos1 = cataColTitles[ii].find("DIM:");
        if (pos1 > cataColTitles[ii].size()) { continue; }
        pos1 = cataColTitles[ii].rfind('(') + 1;
        if (pos1 > cataColTitles[ii].size()) { jf.err("No maxDIM found-sc.makeCreateData"); }
        pos2 = cataColTitles[ii].find(')', pos1);
        temp = cataColTitles[ii].substr(pos1, pos2 - pos1);
        try { maxDIM.push_back(stoi(temp)); }
        catch (invalid_argument) { jf.err("stoi-sc.makeCreateData"); }
    }
    int maxDim = -1, index, numTables = 1;
    for (int ii = 0; ii < maxDIM.size(); ii++)
    {
        numTables *= maxDIM[ii];
    }
    for (int ii = 0; ii < cataColTitles.size(); ii++)
    {
        pos1 = cataColTitles[ii].find("Dim:");
        if (pos1 > cataColTitles[ii].size()) { continue; }
        pos1 = cataColTitles[ii].rfind("Member ID:");
        if (pos1 > cataColTitles[ii].size()) { continue; }       
        pos1 = cataColTitles[ii].rfind('(') + 1;
        if (pos1 > cataColTitles[ii].size()) { continue; }
        pos2 = cataColTitles[ii].find(')', pos1);
        temp = cataColTitles[ii].substr(pos1, pos2 - pos1);
        try { maxDim = stoi(temp); break; }
        catch (invalid_argument) { jf.err("stoi-sc.makeCreateData"); }
    }
    if (maxDim < 0) { jf.err("No maxDim found-sc.makeCreateData"); }
    vector<int> paramDIM(maxDIM.size(), 1);
    bool letmeout = 0;
    while (!letmeout)
    {
        tname = "Census$" + cataYear + "$" + cataName;
        for (int ii = 0; ii < paramDIM.size(); ii++)
        {
            tname += "$" + to_string(paramDIM[ii]);
        }
        tname += "$Data";
        stmt = "CREATE TABLE IF NOT EXISTS \"" + tname;
        stmt += "\" (GEO_CODE INTEGER PRIMARY KEY";
        for (int ii = 1; ii <= maxDim; ii++)
        {
            stmt += ", MID" + to_string(ii) + " NUMERIC";
        }
        stmt += ");";
        stmts.push_back(stmt);

        index = paramDIM.size() - 1;
        paramDIM[index]++;
        while (1)
        {
            if (paramDIM[index] > maxDIM[index])
            {
                paramDIM[index] = 1;
                if (index > 0)
                {
                    index--;
                    paramDIM[index]++;
                }
                else
                {
                    letmeout = 1;
                    break;
                }
            }
            else { break; }
        }
    }
    if (stmts.size() != numTables) { jf.err("numTables mismatch-sc.makeCreateData"); }
    return stmts;
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

    vector<string> stmts(1);
    string stmt, tname = "Census$" + cataYear + "$" + cataName + "$Definitions";
    stmts[0] = "CREATE TABLE IF NOT EXISTS \"" + tname;
    stmts[0] += "\" (DIM TEXT, Definition TEXT, UNIQUE(DIM));";
    for (int ii = 0; ii < dim.size(); ii++)
    {
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
    vector<string> dims;
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
        posEnd = metaFile.find(nl + nl + nl + nl, posMID);
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
vector<string> STATSCAN::makeInsertData(int GEO_CODE, vector<vector<string>>& geoList)
{
    // Returns all the insert statements for ONE region (given by GEO_CODE).
    if (metaFile.size() < 1) { jf.err("No init-sc.makeInsertData"); }
    vector<string> stmts, vsData, listMID;
    string csvPath, temp, stmt, tname, csvBuffer;
    if (activeCSVpart < 0) 
    {
        csvPath = cataPath + "\\" + cataName + "_English_CSV_data (PART 1).csv";
        activeCSV = wf.load(csvPath);
        activeCSVpart = 1;
        cataColTitles = loadColTitles();
    }
    string search = cataYear + ",\"" + to_string(GEO_CODE) + "\",";
    size_t pos1 = activeCSV.find(search);
    while (pos1 > activeCSV.size()) 
    { 
        activeCSVpart++;
        csvPath = cataPath + "\\" + cataName + "_English_CSV_data (PART ";
        csvPath += to_string(activeCSVpart) + ").csv";
        activeCSV = wf.load(csvPath);
        if (activeCSV.size() < 1) { jf.err("Cannot load CSV part file-sc.makeInsertData"); }
        pos1 = activeCSV.find(search);
    }
    int nextGeo, geoDone = 0, numDim = 0, colIndex;
    pos1 += 6;
    size_t pos2 = activeCSV.find('"', pos1), pos3, pos4;
    string sGeoCode = activeCSV.substr(pos1, pos2 - pos1);
    try { nextGeo = stoi(sGeoCode); }
    catch (invalid_argument) { jf.err("stoi-sc.makeInsertData"); }
    
    unordered_set<int> usMID, usDim;
    for (int ii = 0; ii < cataColTitles.size(); ii++)
    {
        pos3 = cataColTitles[ii].find("Dim:");
        if (pos3 < cataColTitles[ii].size()) { usDim.emplace(ii); }
        pos3 = cataColTitles[ii].find("Member ID:");
        if (pos3 < cataColTitles[ii].size()) { usMID.emplace(ii); }
    }

    while (nextGeo == GEO_CODE)
    {
        if (!geoDone)
        {
            pos1 = activeCSV.find_first_of("1234567890", pos2);
            pos2 = activeCSV.find('"', pos1);
            for (int ii = 0; ii < geoList.size(); ii++)
            {
                if (geoList[ii][0] == sGeoCode)
                {
                    geoList[ii].push_back(activeCSV.substr(pos1, pos2 - pos1));
                    geoDone = 1;
                    break;
                }
                else if (ii == geoList.size() - 1) { jf.err("Failed to add GEO_LEVEL-sc.makeInsertData"); }
            }
            pos2 = activeCSV.rfind('"', pos1 - 2);
            pos1 = activeCSV.rfind('"', pos2 - 1) + 1;
        }
        pos3 = activeCSV.find(nl, pos2);
        vsData.clear();
        vsData.resize(1);
        vsData[0] = to_string(GEO_CODE);

        pos1 = activeCSV.find(',', pos2);
        colIndex = 1;
        while (pos1 < pos3)
        {
            colIndex++;
            if (usMID.count(colIndex))
            {
                pos1 = activeCSV.find_first_not_of(", ", pos1);
                pos2 = activeCSV.find(',', pos1);
                listMID.push_back(activeCSV.substr(pos1, pos2 - pos1));
                pos1 = pos2;
            }
            else if (usDim.count(colIndex))
            {
                pos1 = activeCSV.find_first_not_of(", ", pos1);
                pos2 = activeCSV.find(',', pos1);
                vsData.push_back(activeCSV.substr(pos1, pos2 - pos1));
                pos1 = pos2;
            }
            else { pos1 = activeCSV.find(',', pos1 + 1); }
        }

        tname = "Census$" + cataYear + "$" + cataName;
        for (int ii = 0; ii < listMID.size(); ii++)
        {
            tname += "$" + listMID[ii];
        }
        tname += "$Data";

        stmt = "INSERT OR IGNORE INTO \"" + tname + "\" (GEO_CODE";
        for (int ii = 0; ii < vsData.size(); ii++)
        {
            stmt += ", MID" + to_string(ii + 1);
        }
        stmt += ") VALUES (" + to_string(GEO_CODE);
        for (int ii = 0; ii < vsData.size(); ii++)
        {
            stmt += ", " + vsData[ii];
        }
        stmt += ");";
        stmts.push_back(stmt);

        pos4 = activeCSV.find(nl, pos3 + 1);
        if (pos4 < activeCSV.size())  // The next line is available.
        {
            pos1 = activeCSV.find('"', pos3) + 1;
            pos2 = activeCSV.find('"', pos1);
            sGeoCode = activeCSV.substr(pos1, pos2 - pos1);
            try { nextGeo = stoi(sGeoCode); }
            catch (invalid_argument) { jf.err("stoi-sc.makeInsertData"); }
        }
        else  // The next line is broken (from CSV mega-file splitting). 
        {
            pos1 = activeCSV.find_first_not_of(nl, pos3);
            csvBuffer = activeCSV.substr(pos1);
            activeCSVpart++;
            csvPath = cataPath + "\\" + cataName + "_English_CSV_data (PART ";
            csvPath += to_string(activeCSVpart) + ").csv";
            activeCSV = wf.load(csvPath);
            if (activeCSV.size() < 1) { jf.err("Cannot load CSV part file-sc.makeInsertData"); }
            pos3 = activeCSV.find(nl);
            csvBuffer += activeCSV.substr(0, pos3);


        }

    }

}
string STATSCAN::makeInsertYear()
{
    if (metaFile.size() < 1) { jf.err("No init-sc.makeInsertCensus"); }
    string stmt = "INSERT OR IGNORE INTO \"Census$" + cataYear + "\" (";
    stmt += "Catalogue) VALUES ('" + cataName + "');";
    return stmt;
}
string STATSCAN::processCSVline(int& GEO_CODE, string csvLine, vector<vector<string>>& geoList)
{
    // RESUME HERE.
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
