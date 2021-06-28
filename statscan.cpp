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
