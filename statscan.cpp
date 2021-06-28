#include "statscan.h"

using namespace std;

int STATSCAN::cata_init(string& sample_csv)
{
    size_t pos1 = cata_path.rfind('\\'), finalTextVar;
    size_t pos2 = cata_path.rfind('\\', pos1 - 1) + 1;
    cataName = cata_path.substr(pos2, pos1 - pos2);
    cata_name = cata_path.substr(pos1 + 1);
    cata_desc = extract_description(sample_csv);
    text_vars = extract_text_vars(sample_csv, finalTextVar);
    column_titles = extract_column_titles(sample_csv, finalTextVar);
    int damaged_val;  // Unneeded for init.
    rows = extract_rows(sample_csv, damaged_val, finalTextVar);
    linearized_titles = linearize_row_titles(rows, column_titles);
    if (text_vars.size() + linearized_titles.size() >= columnLimit) { return -1; }

    insert_primary_template = make_insert_primary_template(cata_name, text_vars, linearized_titles);
    create_csv_table_template = make_create_csv_table_template(column_titles);
    insert_csv_row_template = make_insert_csv_row_template(column_titles);

    tree_from_indent(csv_tree, rows);
    int tg_row_col;
    subtable_names_template = make_subtable_names_template(cata_name, tg_row_col, csv_tree, rows);

    binParameter = { "frames","scale","position","parent","border" };

    return tg_row_col;
}
void STATSCAN::cleanURL(string& url)
{
    size_t pos1 = url.find("&amp;");
    while (pos1 < url.size())
    {
        url.replace(pos1, 5, "&");
        pos1 = url.find("&amp;", pos1);
    }
}
vector<string> STATSCAN::disassembleNameCSV(string& fileName)
{
    // Form [sCata, GID, Region Name].
    vector<string> output(3);
    size_t pos1 = fileName.find('(') + 1;
    output[0] = fileName.substr(0, pos1 - 2);
    size_t pos2 = fileName.find(')', pos1);
    output[1] = fileName.substr(pos1, pos2 - pos1);
    pos1 = pos2 + 2;
    pos2 = fileName.rfind(".csv");
    output[2] = fileName.substr(pos1, pos2 - pos1);
    return output;
}
void STATSCAN::downloadCatalogue(SWITCHBOARD& sbgui)
{
    vector<int> mycomm;
    vector<vector<int>> comm_gui;
    thread::id myid = this_thread::get_id();
    sbgui.answer_call(myid, mycomm);
    vector<string> prompt = sbgui.get_prompt();  // Form [syear, sname].       
    int iYear, iCata;
    try {
        iYear = stoi(prompt[0]);
        iCata = stoi(prompt[1].substr(8));
    }
    catch (invalid_argument& ia) { err("stoi-sc.downloadCatalogue"); }

    // Read the geo list, downloading a new one if necessary.
    if (navSearch.size() < 1) { navSearch = navAsset(); }
    initGeo();
    string folderPath = sroot + "\\" + prompt[0] + "\\" + prompt[1];
    wf.makeDir(folderPath);
    string geoPath = folderPath + "\\" + prompt[1] + " geo list.bin";
    string zipPath = folderPath + "\\" + prompt[1] + " entireCata.zip";
    string doNotDownload = folderPath + "\\" + prompt[1] + " do not download.txt";
    string geoPage, geoURL;
    if (wf.file_exist(doNotDownload))
    {
        mycomm[0] = 1;
        sbgui.update(myid, mycomm);
        return;
    }
    else if (!wf.file_exist(geoPath))
    {
        if (wf.file_exist(zipPath))
        {
            mycomm[0] = 1;
            sbgui.update(myid, mycomm);
            return;
        }
        downloadGeoList(prompt[0], prompt[1], geoPage);
        if (wf.file_exist(zipPath)) 
        {
            zf.unzip(zipPath);
            mycomm[0] = 1;
            sbgui.update(myid, mycomm);
            return;
        }
        else if (wf.file_exist(doNotDownload))
        {
            mycomm[0] = 1;
            sbgui.update(myid, mycomm);
            return;
        }
    }
    else if (!testGeoList(geoPath))
    {
        downloadGeoList(prompt[0], prompt[1], geoPage);
    }
    unordered_map<string, string> mapGeo;
    vector<vector<string>> geoAll = readGeo(geoPath, mapGeo);
    if (geoPage.size() < 1)
    {
        geoURL = geoAll[geoAll.size() - 1][0];
        geoPage = wf.browseS(geoURL);
    }
    
    // Make a list of CSVs already in local storage, and compare it to the online CSV list.
    vector<string> csvLocal = wf.get_file_list(folderPath, "*.csv");
    vector<vector<string>> csvLocalDis(csvLocal.size());
    for (int ii = 0; ii < csvLocalDis.size(); ii++)
    {
        csvLocalDis[ii] = disassembleNameCSV(csvLocal[ii]);
    }
    vector<int> activeColumn = { 0, 1 };
    vector<vector<string>> difference = jf.compareList(geoAll, csvLocalDis, activeColumn);
    if (difference[1].size() > 0) { err("Local CSV not in geo list-sc.downloadCatalogue"); }
    difference[0].erase(difference[0].begin() + difference[0].size() - 2, difference[0].end());

    // Download all missing CSVs. 
    mycomm[2] = difference[0].size();
    comm_gui = sbgui.update(myid, mycomm);
    string urlCataDL, csvPath, csvFile, regionName;
    for (int ii = 0; ii < difference[0].size(); ii++)
    {
        urlCataDL = urlCataDownload(iYear, geoPage, difference[0][ii]);
        try { regionName = mapGeo.at(difference[0][ii]); }
        catch (out_of_range& oor) { err("mapGeo-sc.downloadCatalogue"); }
        csvPath = folderPath + "\\" + prompt[1] + " (" + difference[0][ii];
        csvPath += ") " + regionName + ".csv";
        wf.download(urlCataDL, csvPath);
        mycomm[1]++;
        sbgui.update(myid, mycomm);
    }
    mycomm[0] = 1;
    sbgui.update(myid, mycomm);
}
void STATSCAN::downloadGeoList(string sYear, string sName, string& geoPage)
{
    int iYear;
    try { iYear = stoi(sYear); }
    catch (invalid_argument& ia) { err("stoi-sc.downloadGeoList"); }
    string geoPath = sroot + "\\" + sYear + "\\" + sName;
    geoPath += "\\" + sName + " geo list.bin";
    string urlCata = urlCatalogue(iYear, sName);
    string urlGeo = urlGeoList(iYear, urlCata);
    geoPage = wf.browseS(urlGeo);
    if (testFileNotFound(geoPage))
    {
        string entireCataPath = sroot + "\\" + sYear + "\\" + sName;
        entireCataPath += "\\" + sName + " entireCata.zip";
        if (iYear >= 2016)
        {
            urlGeo = urlEntireTableDownload(iYear, urlCata);
            vector<unsigned char> zipFile = wf.browseUC(urlGeo);
            jf.printer(entireCataPath, zipFile);
            return;
        }
        else { err("No coding for early iYear-sc.downloadGeoList"); }
    }
    vector<string> geoTemp = jf.textParser(geoPage, navSearch[6]);
    if (testCanadaOnly(geoTemp[0]))
    {
        string entireCataPath = sroot + "\\" + sYear + "\\" + sName;
        entireCataPath += "\\" + sName + " entireCata.zip";
        if (iYear >= 2016)
        {
            urlGeo = urlEntireTableDownload(iYear, urlCata);
            vector<unsigned char> zipFile = wf.browseUC(urlGeo);
            jf.printer(entireCataPath, zipFile);
            return;
        }
        else { err("No coding for early iYear-sc.downloadGeoList"); }
    }
    vector<string> geoLayers = makeGeoLayers(geoTemp[0]);
    if (geoLayers[0] == "fsa")
    {
        string doNotDownload = sroot + "\\" + sYear + "\\" + sName;
        doNotDownload += "\\" + sName + " do not download.txt";
        string fsa = "fsa";
        jf.printer(doNotDownload, fsa);
        return;
    }
    vector<string> geoLinkNames = jf.textParser(geoPage, navSearch[2]);
    string geoList = makeGeoList(geoLinkNames, geoLayers, urlGeo);
    jf.printer(geoPath, geoList);
}
void STATSCAN::downloadMaps(SWITCHBOARD& sbgui)
{
    vector<int> mycomm;
    vector<vector<int>> comm_gui;
    thread::id myid = this_thread::get_id();
    sbgui.answer_call(myid, mycomm);
    vector<string> prompt = sbgui.get_prompt();  // Form [syear, sname].
    initGeo();
    int iYear;
    try { iYear = stoi(prompt[0]); }
    catch (invalid_argument& ia) { err("stoi-sc.downloadMaps"); }
    string cataURL = urlCatalogue(iYear, prompt[1]);
    string geoListURL = urlGeoList(iYear, cataURL);
    string geoPage = wf.browseS(geoListURL);
    vector<string> geoTemp = jf.textParser(geoPage, navSearch[6]);
    vector<string> geoLayers = makeGeoLayers(geoTemp[0]);
    vector<string> geoLinkNames = jf.textParser(geoPage, navSearch[2]);
    // RESUME HERE.
    int bbq = 1;
}
void STATSCAN::err(string func)
{
    jf.err(func);
}

vector<string> STATSCAN::extract_column_titles(string& sfile, size_t& finalTextVar)
{
    vector<string> column_titles;
    string temp1;
    size_t pos1, pos2, pos_nl1, pos_nl2;
    int spaces, indent;
    vector<int> space_history = { 0 };

    pos_nl1 = sfile.find('\n', finalTextVar);
    pos_nl2 = sfile.find('\n', pos_nl1 + 1);
    pos1 = sfile.find('"', pos_nl1);
    do
    {
        pos2 = sfile.find('"', pos1 + 1);
        if (pos2 < pos_nl2)
        {
            temp1 = sfile.substr(pos1 + 1, pos2 - pos1 - 1);
            spaces = sclean(temp1, 0);
            for (int ii = 0; ii < space_history.size(); ii++)
            {
                if (spaces == space_history[ii])
                {
                    indent = ii;
                    break;
                }
                else if (ii == space_history.size() - 1)
                {
                    indent = space_history.size();
                    space_history.push_back(spaces);
                }
            }
            temp1.insert(0, indent, '+');
            column_titles.push_back(temp1);
            pos1 = sfile.find('"', pos2 + 1);
        }
    } while (pos1 < pos_nl2);

    if (column_titles.size() < 2)
    {
        multi_column = 0;
        column_titles.resize(2);
        column_titles[0] = "";
        column_titles[1] = "Value";
    }
    else { multi_column = 1; }

    return column_titles;
}
void STATSCAN::extract_csv_branches(vector<string>& file_list)
{
    // Given a list of file names (not paths), extract the unique portions and store them locally.

    size_t pos1;
    csv_branches.resize(file_list.size());
    for (int ii = 0; ii < file_list.size(); ii++)
    {
        pos1 = file_list[ii].find('(');
        csv_branches[ii] = file_list[ii].substr(pos1);
    }
}
string STATSCAN::extract_description(string& sfile)
{
    size_t pos1 = sfile.find('"');
    size_t pos2 = sfile.find('"', pos1 + 1);
    string description = sfile.substr(pos1 + 1, pos2 - pos1 - 1);
    return description;
}
void STATSCAN::extract_gid_list(vector<string>& file_list)
{
    // Given a list of file names (not paths), extract the GIDs and store them locally.

    size_t pos1, pos2;
    gid_list.resize(file_list.size());
    for (int ii = 0; ii < file_list.size(); ii++)
    {
        pos1 = file_list[ii].find('(');
        pos2 = file_list[ii].find(')', pos1 + 1);
        gid_list[ii] = file_list[ii].substr(pos1 + 1, pos2 - pos1 - 1);
    }
}
vector<vector<string>> STATSCAN::extract_rows(string& sfile, int& damaged, size_t& finalTextVar)
{
    // Returns a 2D vector of the form [row index][row title, row val1, row val2, ...].

    vector<vector<string>> rows;
    string line, temp1;
    size_t pos1, pos2, pos3, pos_nl1, pos_nl2;
    char math;
    damaged = 0;

    if (finalTextVar == 0)
    {
        pos1 = 0;
        while (1)
        {
            pos1 = sfile.find('=', pos1 + 1);
            if (pos1 > sfile.size()) { break; }  // Loop exit.
            math = sfile[pos1 - 1];
            if (math == '<' || math == '>') { continue; }  // Ignore cases where '=' is used in row titles.
            finalTextVar = pos1;
        }
    }
    
    pos_nl1 = sfile.find('\n', finalTextVar);
    pos_nl2 = sfile.find('\n', pos_nl1 + 1);    
    if (multi_column)
    {
        pos_nl1 = pos_nl2;
        pos_nl2 = sfile.find('\n', pos_nl1 + 1);
    }

    vector<int> space_history = { 0 };
    vector<int> diff, mins;
    int spaces, indent, inum, min, index, rindex;
    int old_spaces = 0;
    do                                                   // For every line...
    {
        line = sfile.substr(pos_nl1, pos_nl2 - pos_nl1);
        pos1 = line.find('"');
        pos2 = line.find('"', pos1 + 1);
        temp1 = line.substr(pos1 + 1, pos2 - pos1 - 1);
        spaces = sclean(temp1, 0);
        if (temp1 == "Note") { break; }  // Primary loop exit.
        rindex = rows.size();
        rows.push_back(vector<string>(1));

        // This portion of the code is problematic. Stats Canada was inconsistent in how 
        // they indented their rows using single char spaces (even within the same file).
        if (spaces == old_spaces)
        {
            indent = space_history.size() - 1;
        }
        else if (spaces > old_spaces)
        {
            indent = space_history.size();
            space_history.push_back(spaces);
            old_spaces = spaces;
        }
        else
        {
            inum = old_spaces - spaces;
            if (inum == 1)
            {
                indent = space_history.size() - 1;  // Pretend it's the same as before...
            }
            else
            {
                for (int ii = space_history.size() - 1; ii >= 0; ii--)
                {
                    if (space_history[ii] == spaces)
                    {
                        indent = ii;
                        while (ii < space_history.size() - 1)
                        {
                            space_history.pop_back();
                        }
                        old_spaces = spaces;
                        break;
                    }
                    else if (ii == 0)   // Tell the program to guess. What could possibly go wrong?
                    {
                        diff.resize(space_history.size());
                        for (int jj = 0; jj < space_history.size(); jj++)
                        {
                            diff[jj] = abs(spaces - space_history[jj]);
                        }
                        min = diff[0];
                        mins = { 0 };
                        for (int jj = 1; jj < space_history.size(); jj++)
                        {
                            if (diff[jj] < min)
                            {
                                min = diff[jj];
                                mins = { jj };
                            }
                            else if (diff[jj] == min)
                            {
                                mins.push_back(jj);
                            }
                        }
                        if (mins.size() == 1)
                        {
                            indent = mins[0];
                            while (indent < space_history.size() - 1)
                            {
                                space_history.pop_back();
                            }
                            old_spaces = spaces;
                        }
                        else
                        {
                            // Give up.
                            err("Failed to parse spaces-sc.extract_rows");
                        }
                    }
                }
            }
        }
        temp1.insert(0, indent, '+');
        rows[rindex][0] = temp1;

        pos2 = line.find(',', pos2);
        do                                // For every data value on this line...
        {
            pos1 = pos2;
            pos2 = line.find(',', pos1 + 1);
            if (pos2 > line.size())  // If we have reached the last value on this line...
            {
                pos3 = line.find("..", pos1 + 1);  // ... check for a damaged value.
                if (pos3 < line.size())
                {
                    damaged++;
                    rows[rindex].push_back("..");
                    break;
                }
                pos3 = line.find('x', pos1 + 1);  // ... check for a damaged value.
                if (pos3 < line.size())
                {
                    damaged++;
                    rows[rindex].push_back("x");
                    break;
                }
                pos3 = line.find('F', pos1 + 1);  // ... check for a damaged value.
                if (pos3 < line.size())
                {
                    damaged++;
                    rows[rindex].push_back("F");
                    break;
                }

                pos3 = line.find_last_of("1234567890") + 1;
                pos1 = line.find_last_of(" ,", pos3 - 1) + 1;
                temp1 = line.substr(pos1, pos3 - pos1);
                rows[rindex].push_back(temp1);
            }
            else
            {
                temp1 = line.substr(pos1 + 1, pos2 - pos1 - 1);
                if (temp1 == ".." || temp1 == "...") { damaged++; }
                else if (temp1 == "x" || temp1 == "F") { damaged++; }
                else if (jf.is_numeric(temp1))
                {
                    rows[rindex].push_back(temp1);
                }
            }
        } while (pos2 < line.size());

        pos_nl1 = pos_nl2;
        pos_nl2 = sfile.find('\n', pos_nl1 + 1);
    } while (pos_nl2 < sfile.size());  // Secondary loop exit.
    
    return rows;
}
vector<vector<string>> STATSCAN::extract_text_vars(string& sfile, size_t& finalTextVar)
{
	vector<vector<string>> text_vars;
	size_t pos1, pos2;
	char math;
	string temp1;

	pos1 = 0;
	while (1)
	{
		pos1 = sfile.find('=', pos1 + 1);
		if (pos1 > sfile.size()) { break; }  // Loop exit.
		math = sfile[pos1 - 1];
		if (math == '<' || math == '>') { continue; }  // Ignore cases where '=' is used in row titles.
        pos2 = sfile.rfind('"', pos1);
        temp1 = sfile.substr(pos2, pos1 - pos2);
        pos2 = temp1.find_first_of("[]");
        if (pos2 < temp1.size()) { continue; }

        finalTextVar = pos1;
        text_vars.push_back(vector<string>(2));
		pos2 = sfile.rfind('"', pos1);
		temp1 = sfile.substr(pos2 + 1, pos1 - pos2 - 1);
        sclean(temp1, 0);
        text_vars[text_vars.size() - 1][0] = temp1;
        
        pos2 = sfile.find('"', pos1);
        temp1 = sfile.substr(pos1 + 1, pos2 - pos1 - 1);
        sclean(temp1, 0);
        text_vars[text_vars.size() - 1][1] = temp1;
	}
    return text_vars;
}
vector<vector<string>> STATSCAN::extractTitles(string& csvFile)
{
    // Return has form [row, column][title0, title1, ... ].
    vector<vector<string>> titles(2, vector<string>());
    vector<int> diff, mins, space_history = { 0 };
    string temp, title;
    bool multiColumn;  // Before parsing, we must know this.
    size_t pos1 = 0, pos2, posFinalEq, posEnd;
    while (1)
    {
        pos1 = csvFile.find('=', pos1 + 1);
        if (pos1 > csvFile.size()) { break; }  // Loop exit.
        if (csvFile[pos1 - 1] == '<' || csvFile[pos1 - 1] == '>') { continue; }  // Ignore cases where '=' is used in row titles.
        pos2 = csvFile.rfind('"', pos1);
        temp = csvFile.substr(pos2, pos1 - pos2);
        pos2 = temp.find_first_of("[]");       // Sometimes equality symbols are
        if (pos2 < temp.size()) { continue; }  // used in the [num] footnotes.
        posFinalEq = pos1;
    }
    pos1 = csvFile.find('\n', posFinalEq);
    pos2 = csvFile.find('\n', pos1 + 1);
    temp = csvFile.substr(pos1, pos2 - pos1);
    int inum, min, indent, spaces, old_spaces = 0, count = 0;
    for (int ii = 0; ii < temp.size(); ii++)
    {
        if (temp[ii] == '"') { count++; }
    }
    if (count == 2) { multiColumn = 0; }
    else if (count > 2) { multiColumn = 1; }
    else { jf.err("Quotation marks-sc.extractTitles"); }

    // Extract the row titles.
    posEnd = csvFile.find("\"Note\"");
    if (posEnd > csvFile.size()) { jf.err("Failed to find CSV stop point-sc.extractTitles"); }
    pos1 = csvFile.find('\n', posFinalEq) + 2;
    if (multiColumn) { pos1 = csvFile.find('\n', pos1) + 2; }
    while (pos1 < posEnd)
    {
        pos2 = csvFile.find('"', pos1);
        title = csvFile.substr(pos1, pos2 - pos1);
        spaces = sclean(title, 0);

        // This next portion of the code is problematic. Stats Canada was 
        // inconsistent in how they indented their rows using single char 
        // spaces (even within the same file).
        if (spaces == old_spaces)
        {
            indent = space_history.size() - 1;
        }
        else if (spaces > old_spaces)
        {
            indent = space_history.size();
            space_history.push_back(spaces);
            old_spaces = spaces;
        }
        else
        {
            inum = old_spaces - spaces;
            if (inum == 1)
            {
                indent = space_history.size() - 1;  // Pretend it's the same as before...
            }
            else
            {
                for (int ii = space_history.size() - 1; ii >= 0; ii--)
                {
                    if (space_history[ii] == spaces)
                    {
                        indent = ii;
                        while (ii < space_history.size() - 1)
                        {
                            space_history.pop_back();
                        }
                        old_spaces = spaces;
                        break;
                    }
                    else if (ii == 0)   // Tell the program to guess. What could possibly go wrong?
                    {
                        diff.resize(space_history.size());
                        for (int jj = 0; jj < space_history.size(); jj++)
                        {
                            diff[jj] = abs(spaces - space_history[jj]);
                        }
                        min = diff[0];
                        mins = { 0 };
                        for (int jj = 1; jj < space_history.size(); jj++)
                        {
                            if (diff[jj] < min)
                            {
                                min = diff[jj];
                                mins = { jj };
                            }
                            else if (diff[jj] == min)
                            {
                                mins.push_back(jj);
                            }
                        }
                        if (mins.size() == 1)
                        {
                            indent = mins[0];
                            while (indent < space_history.size() - 1)
                            {
                                space_history.pop_back();
                            }
                            old_spaces = spaces;
                        }
                        else
                        {
                            // Give up.
                            jf.err("Failed to parse spaces-sc.extractTitles");
                        }
                    }
                }
            }
        }

        temp.assign(indent, '+');
        titles[0].push_back(temp + title);
    }

    // Extract the column titles (if there are any).
    if (multiColumn)
    {
        space_history = { 0 };
        pos1 = csvFile.find('\n', posFinalEq) + 2;
        posEnd = csvFile.find('\n', pos1);
        while (pos1 < posEnd)
        {
            pos2 = csvFile.find('"', pos1);
            title = csvFile.substr(pos1, pos2 - pos1);
            spaces = sclean(title, 2);  // Mode 2 to eliminate number references.
            if (spaces == old_spaces)
            {
                indent = space_history.size() - 1;
            }
            else if (spaces > old_spaces)
            {
                indent = space_history.size();
                space_history.push_back(spaces);
                old_spaces = spaces;
            }
            else
            {
                inum = old_spaces - spaces;
                if (inum == 1)
                {
                    indent = space_history.size() - 1;  // Pretend it's the same as before...
                }
                else
                {
                    for (int ii = space_history.size() - 1; ii >= 0; ii--)
                    {
                        if (space_history[ii] == spaces)
                        {
                            indent = ii;
                            while (ii < space_history.size() - 1)
                            {
                                space_history.pop_back();
                            }
                            old_spaces = spaces;
                            break;
                        }
                        else if (ii == 0)   // Tell the program to guess. What could possibly go wrong?
                        {
                            diff.resize(space_history.size());
                            for (int jj = 0; jj < space_history.size(); jj++)
                            {
                                diff[jj] = abs(spaces - space_history[jj]);
                            }
                            min = diff[0];
                            mins = { 0 };
                            for (int jj = 1; jj < space_history.size(); jj++)
                            {
                                if (diff[jj] < min)
                                {
                                    min = diff[jj];
                                    mins = { jj };
                                }
                                else if (diff[jj] == min)
                                {
                                    mins.push_back(jj);
                                }
                            }
                            if (mins.size() == 1)
                            {
                                indent = mins[0];
                                while (indent < space_history.size() - 1)
                                {
                                    space_history.pop_back();
                                }
                                old_spaces = spaces;
                            }
                            else
                            {
                                // Give up.
                                jf.err("Failed to parse spaces-sc.extractTitles");
                            }
                        }
                    }
                }
            }
            temp.assign(indent, '+');
            titles[1].push_back(temp + title);
        }
    }

    return titles;
}
string STATSCAN::geoLinkToRegionUrl(string& urlGeoList, string& geoLink)
{
    size_t pos1 = urlGeoList.rfind('/') + 1;
    string mapUrl = urlGeoList.substr(0, pos1);
    pos1 = geoLink.find("href=\"") + 6;
    size_t pos2 = geoLink.find("\" ", pos1);
    string temp = geoLink.substr(pos1, pos2 - pos1);
    cleanURL(temp);
    mapUrl += temp;
    return mapUrl;
}
string STATSCAN::get_cata_desc()
{
    return cata_desc;
}
string STATSCAN::get_cata_name()
{
    return cata_name;
}
vector<string> STATSCAN::get_column_titles()
{
    return column_titles;
}
string STATSCAN::get_create_csv_table_template()
{
    return create_csv_table_template;
}
vector<vector<int>> STATSCAN::get_csv_tree()
{
    return csv_tree;
}
string STATSCAN::get_gid(int index)
{
    return gid_list[index];
}
vector<string> STATSCAN::get_gid_list()
{
    return gid_list;
}
string STATSCAN::get_insert_csv_row_template()
{
    return insert_csv_row_template;
}
string STATSCAN::get_insert_primary_template()
{
    return insert_primary_template;
}
vector<string> STATSCAN::getLayerSelected(string& sfile)
{
    // This function is meant to be used with a geo list webpage.
    size_t pos2 = sfile.find("<div id=\"geo-download\"");
    size_t pos1 = sfile.rfind("<strong>", pos2) + 8;
    pos2 = sfile.find("</strong>", pos1);
    string layerDesc = sfile.substr(pos1, pos2 - pos1);
    layerDesc.push_back('/');
    int numLayers = 0;
    for (int ii = 0; ii < layerDesc.size(); ii++)
    {
        if (layerDesc[ii] == '/') { numLayers++; }
    }
    vector<string> layerCodes(numLayers);
    pos2 = 0;
    string temp;
    for (int ii = 0; ii < numLayers; ii++)
    {
        pos1 = pos2;
        pos2 = layerDesc.find('/', pos1);
        temp = layerDesc.substr(pos1, pos2 - pos1);
        try { layerCodes[ii] = mapGeoLayers.at(temp); }
        catch (out_of_range& oor) { err("mapGeoLayers-sc.getLayerSelected"); }
        pos2++;
    }
    return layerCodes;
}
int STATSCAN::get_num_subtables()
{
    return subtable_names_template.size();
}
vector<int> STATSCAN::getSmallGeoIndex(string sParent, vector<string>& regionList, vector<string>& layerList)
{
    vector<int> startStop = { -1, -1 };
    string parentLayer;
    for (int ii = 0; ii < regionList.size(); ii++)
    {
        if (startStop[0] < 0 && regionList[ii] == sParent)
        {
            startStop[0] = ii;
            parentLayer = layerList[ii];
        }
        else if (startStop[0] >= 0 && layerList[ii] == parentLayer)
        {
            startStop[1] = ii - 1;
            break;
        }
    }
    if (startStop[0] < 0) { jf.err("Cannot find parent name-sc.getSmallGeoIndex"); }
    else if (startStop[1] < 0) { startStop[1] = regionList.size() - 1; }
    return startStop;
}
string STATSCAN::get_subtable_name_template(int index)
{
    return subtable_names_template[index];
}
void STATSCAN::initGeo()
{
    string empty;
    if (mapGeoLayers.size() == 0)
    {
        mapGeoLayers.emplace("Canada", empty);
        mapGeoLayers.emplace("Can", empty);
        mapGeoLayers.emplace("Prov.Terr.", "province");
        mapGeoLayers.emplace("Prov.Terr", "province");
        mapGeoLayers.emplace("Provinces Territories", "province");
        mapGeoLayers.emplace("CMACA", "cmaca");
        mapGeoLayers.emplace("CMACA with Provincial Splits", "cmaca");
        mapGeoLayers.emplace("CMACA with Provincial Splits 2016", "cmaca");
        mapGeoLayers.emplace("CMA with Provincial Splits", "cmaca");
        mapGeoLayers.emplace("CT", "ct");
        mapGeoLayers.emplace("CD", "cd");
        mapGeoLayers.emplace("CSD", "csd");
        mapGeoLayers.emplace("FED", "fed");
        mapGeoLayers.emplace("ER", "er");
        mapGeoLayers.emplace("FSA", "fsa");  // No maps available.
        mapGeoLayers.emplace("DA", "da");  // No maps available.
    }
}
int STATSCAN::loadGeo(string& filePath, vector<int>& gidList, vector<string>& regionList, vector<string>& layerList, vector<string>& geoLayers)
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
vector<string> STATSCAN::linearize_row_titles(vector<vector<string>>& rows, vector<string>& column_titles)
{
    // Produces a list of unique titles from the indented row titles which do not include ancestors.
    vector<string> family_line;
    string base, title;
    int indent, old_indent;

    vector<string> linearized_titles;
    old_indent = 0;
    if (multi_column)
    {
        for (int ii = 0; ii < rows.size(); ii++)
        {
            base.clear();
            indent = 0;
            while (rows[ii][0][indent] == '+')
            {
                indent++;
            }
            if (indent > old_indent)
            {
                if (ii > 0)
                {
                    family_line.push_back(rows[ii - 1][0]);
                }
                old_indent = indent;
            }
            else if (indent < old_indent)
            {
                for (int jj = 0; jj < old_indent - indent; jj++)
                {
                    family_line.pop_back();
                }
                old_indent = indent;
            }
            for (int jj = 0; jj < family_line.size(); jj++)
            {
                base.append(family_line[jj]);
                base += "@";
            }
            base.append(rows[ii][0]);

            for (int jj = 1; jj < column_titles.size(); jj++)
            {
                // Note: the first column title is dropped, as it is not useful.
                title = base + "@";  // The 'at' symbol separates rows from columns.
                title += column_titles[jj];
                linearized_titles.push_back(title);
            }
        }
    }
    else
    {
        for (int ii = 0; ii < rows.size(); ii++)
        {
            base.clear();
            indent = 0;
            while (rows[ii][0][indent] == '+')
            {
                indent++;
            }
            if (indent > old_indent)
            {
                if (ii > 0)
                {
                    family_line.push_back(rows[ii - 1][0]);
                }
                old_indent = indent;
            }
            else if (indent < old_indent)
            {
                for (int jj = 0; jj < old_indent - indent; jj++)
                {
                    family_line.pop_back();
                }
                old_indent = indent;
            }
            for (int jj = 0; jj < family_line.size(); jj++)
            {
                base.append(family_line[jj]);
                base += "@";
            }
            base.append(rows[ii][0]);
            linearized_titles.push_back(base);
        }
    }

    return linearized_titles;
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

string STATSCAN::makeCreateCSV(vector<string> param)
{
    string stmt = "CREATE TABLE IF NOT EXISTS \"" + param[0] + "$" + param[1];
    stmt += "\" (\"" + param[2] + "\" TEXT";
    for (int ii = 3; ii < param.size(); ii++)
    {
        stmt += ", \"" + param[ii] + "\" NUMERIC";
    }
    stmt += ");";
    return stmt;
}
string STATSCAN::makeCreateGeo(string cataName)
{
    // Note that this table must have unique rows.
    string stmt = "CREATE TABLE IF NOT EXISTS \"Geo$" + cataName;
    stmt += "\" (GID INTEGER PRIMARY KEY, \"Region Name\" TEXT, Indent INT, ";
    stmt += "UNIQUE(GID, \"Region Name\", Indent));";
    return stmt;
}
string STATSCAN::makeCreateGeoLayers(string cataName)
{
    // Note that this table must have unique rows.
    string stmt = "CREATE TABLE IF NOT EXISTS \"Geo_Layers$" + cataName;
    stmt += "\" (Layers TEXT, UNIQUE(Layers));";
    return stmt;
}
string STATSCAN::makeCreateRowColTitle(string cataName)
{
    // Note that this table must have unique rows.
    string stmt = "CREATE TABLE IF NOT EXISTS \"RowColTitle$" + cataName;
    stmt += "\" (Row TEXT, Column TEXT, UNIQUE(Row));";
    return stmt;
}
vector<string> STATSCAN::makeCreateTCatalogue(string& csvFile, string& tname, vector<string>& listParam)
{
    vector<string> stmts;
    size_t pos1 = csvFile.find('"') + 1, posEq;
    size_t pos2 = csvFile.find(" for ", pos1);
    string line, varLeft, varRight;
    string desc = csvFile.substr(pos1, pos2 - pos1);
    pos1 = csvFile.find('"', pos2);
    pos1 = csvFile.find('"', pos1 + 1) + 1;
    pos2 = csvFile.find('"', pos1);
    while (1)
    {
        line = csvFile.substr(pos1, pos2 - pos1);
        posEq = line.find('=');
        if (posEq > line.size()) { break; }
        pos1 = line.rfind("Geography", posEq);
        if (pos1 < line.size())
        {
            pos1 = csvFile.find('"', pos2 + 1) + 1;
            pos2 = csvFile.find('"', pos1);
            continue;
        }
        pos1 = line.rfind("(GNR)", posEq);
        if (pos1 < line.size())
        {
            pos1 = csvFile.find('"', pos2 + 1) + 1;
            pos2 = csvFile.find('"', pos1);
            continue;
        }
        varLeft = line.substr(0, posEq);
        while (varLeft.back() == ' ') { varLeft.pop_back(); }
        pos1 = desc.find(varLeft);
        if (pos1 > desc.size()) { jf.err("Description variables-sc.makeCreateTCatalogue"); }
        varRight = line.substr(posEq + 1);
        while (varRight.front() == ' ') { varRight.erase(varRight.begin()); }
        desc.replace(pos1, varLeft.size(), varRight);
        pos1 = csvFile.find('"', pos2 + 1) + 1;
        pos2 = csvFile.find('"', pos1);
    }
    pos1 = desc.find('(');
    int inum;
    while (pos1 < desc.size())
    {
        pos2 = desc.find(')', pos1);
        line = desc.substr(pos1 + 1, pos2 - pos1 - 1);
        try { inum = stoi(line); }
        catch (invalid_argument) { inum = -1; }
        if (inum >= 0)
        {
            desc.erase(desc.begin() + pos1, desc.begin() + pos2 + 1);
        }
        pos1 = desc.find('(', pos2);
    }
    pos1 = 0;
    pos2 = desc.find(',');
    while (pos2 < desc.size())
    {
        line = desc.substr(pos1, pos2 - pos1);
        while (line.back() == ' ') { line.pop_back(); }
        listParam.push_back(line);
        pos1 = desc.find_first_not_of(", ", pos2);
        pos2 = desc.find(',', pos1);
    }
    pos2 = desc.find(" and ", pos1);
    if (pos2 < desc.size())
    {
        line = desc.substr(pos1, pos2 - pos1);
        while (line.back() == ' ') { line.pop_back(); }
        listParam.push_back(line);
        pos1 = pos2 + 5;
        line = desc.substr(pos1);
        while (line.back() == ' ') { line.pop_back(); }
        listParam.push_back(line);
    }

    tname = "TCatalogue";
    stmts.resize(listParam.size() + 1);
    stmts[0] = "CREATE TABLE IF NOT EXISTS \"" + tname;
    stmts[0] += "\" (Year INTEGER PRIMARY KEY);";
    for (int ii = 1; ii <= listParam.size(); ii++)
    {
        tname += "$" + listParam[ii - 1];
        stmts[ii] = "CREATE TABLE IF NOT EXISTS \"" + tname;
        stmts[ii] += "\" (Topic" + to_string(ii) + " TEXT";
        stmts[ii] += ", UNIQUE(Topic" + to_string(ii) + "));";
    }
    return stmts;
}
string STATSCAN::makeCreateTG_Region(string cataName, int numCol)
{
    // Note that this table must have unique rows.
    string stmt = "CREATE TABLE IF NOT EXISTS \"TG_Region$" + cataName;
    stmt += "\" (GID INTEGER PRIMARY KEY, \"Region Name\" TEXT, ";
    for (int ii = 2; ii < numCol; ii++)
    {
        stmt += "param" + to_string(ii) + " INT, ";
    }
    stmt += "UNIQUE(GID));";
    return stmt;
}
vector<string> STATSCAN::makeCreateTMap(vector<string>& stmtsTMI)
{
    vector<string> stmts;
    string coreDir, tname, stmt;
    size_t pos1, pos2;
    for (int ii = 0; ii < stmtsTMI.size(); ii++)
    {
        pos1 = stmtsTMI[ii].find("VALUES");
        pos1 = stmtsTMI[ii].find('\'', pos1) + 1;
        pos2 = stmtsTMI[ii].rfind('\'');
        coreDir = stmtsTMI[ii].substr(pos1, pos2 - pos1);
        
        tname = coreDir + "$scale";
        stmt = "CREATE TABLE IF NOT EXISTS \"" + tname; 
        stmt += "\" (\"Pixels Per KM\" NUMERIC);";
        stmts.push_back(stmt);

        tname = coreDir + "$parent";
        stmt = "CREATE TABLE IF NOT EXISTS \"" + tname;
        stmt += "\" (\"Region Name\" TEXT);";
        stmts.push_back(stmt);

        tname = coreDir + "$position";
        stmt = "CREATE TABLE IF NOT EXISTS \"" + tname;
        stmt += "\" (PNG NUMERIC, GPS NUMERIC);";
        stmts.push_back(stmt);

        tname = coreDir + "$border";
        stmt = "CREATE TABLE IF NOT EXISTS \"" + tname;
        stmt += "\" (xCoord NUMERIC, yCoord NUMERIC);";
        stmts.push_back(stmt);
    }
    return stmts;
}
string STATSCAN::makeCreateTMapIndex(string cataName)
{
    // Note that this table must have unique rows.
    string stmt = "CREATE TABLE IF NOT EXISTS \"TMapIndex$" + cataName;
    stmt += "\" (GID INTEGER PRIMARY KEY, \"Map Path\" TEXT, UNIQUE(GID));";
    return stmt;
}

vector<string> STATSCAN::makeInsertCSV(vector<string> param, string& csvFile)
{
    size_t finalTextVar = 0;
    int damaged;
    vector<vector<string>> rows = extract_rows(csvFile, damaged, finalTextVar);
    vector<string> stmts(rows.size());
    for (int ii = 0; ii < stmts.size(); ii++)
    {
        stmts[ii] = "INSERT OR IGNORE INTO \"" + param[0] + "$" + param[1] + "\" (\"";
        for (int jj = 2; jj < param.size(); jj++)
        {
            stmts[ii] += param[jj] + "\", ";
        }
        stmts[ii].pop_back();
        stmts[ii].pop_back();
        stmts[ii] += ") VALUES ('" + rows[ii][0] + "'";
        for (int jj = 1; jj < rows[ii].size(); jj++)
        {
            stmts[ii] += ", " + rows[ii][jj];
        }
        stmts[ii] += ");";
    }
    return stmts;
}
vector<string> STATSCAN::makeInsertGeo(string& geoFile, string cataName)
{
    vector<string> stmts;
    string gid, name, indent, stmt;
    vector<string> dirt = { "'" }, soap = { "''" };
    size_t pos1 = geoFile.find_first_of("1234567890");
    size_t pos2 = geoFile.find('$', pos1);
    size_t pos3 = geoFile.rfind("@@Geo Layers");
    while (pos2 < pos3)
    {
        gid = geoFile.substr(pos1, pos2 - pos1);
        pos1 = pos2 + 1;
        pos2 = geoFile.find('$', pos1);
        name = geoFile.substr(pos1, pos2 - pos1);
        jf.clean(name, dirt, soap);
        pos1 = pos2 + 1;
        pos2 = geoFile.find_first_not_of("1234567890", pos1);
        indent = geoFile.substr(pos1, pos2 - pos1);
        stmt = "INSERT OR IGNORE INTO \"Geo$" + cataName + "\" (GID, ";
        stmt += "\"Region Name\", Indent) VALUES (" + gid + ", '" + name;
        stmt += "', " + indent + ");";
        stmts.push_back(stmt);
        pos2 = geoFile.find('\n', pos2);
        pos1 = pos2 + 1;
        pos2 = geoFile.find('$', pos1);
    }
    return stmts;
}
vector<string> STATSCAN::makeInsertGeoLayers(string& geoFile, string cataName, vector<string>& geoLayers)
{
    vector<string> stmts;
    string temp;
    size_t pos3 = geoFile.rfind("@@Geo URL");
    pos3 = geoFile.find_last_not_of("\r\n", pos3 - 1) + 1;
    size_t pos1 = geoFile.rfind("@@Geo Layers") + 12;
    size_t pos2 = geoFile.find('\n', pos1);
    if (pos1 == pos2)  // No carriage return char.
    {
        pos1 = pos2 + 1;
        while (pos1 < pos3)
        {
            pos2 = geoFile.find('\n', pos1);
            if (pos1 == pos2) { geoLayers.push_back("canada"); }
            else
            {
                temp = geoFile.substr(pos1, pos2 - pos1);
                geoLayers.push_back(temp);
            }
            pos1 = pos2 + 1;
        }
    }
    else  // Geo file has carriage return chars.
    {
        pos1 = pos2 + 1;
        while (pos1 < pos3)
        {
            pos2 = geoFile.find("\r\n", pos1);
            if (pos1 == pos2) { geoLayers.push_back("canada"); }
            else
            {
                temp = geoFile.substr(pos1, pos2 - pos1);
                geoLayers.push_back(temp);
            }
            pos1 = pos2 + 2;
        }
    }
    stmts.resize(geoLayers.size());
    for (int ii = 0; ii < stmts.size(); ii++)
    {
        stmts[ii] = "INSERT OR IGNORE INTO \"Geo_Layers$" + cataName;
        stmts[ii] += "\" (Layers) VALUES ('" + geoLayers[ii] + "');";
    }
    return stmts;
}
vector<string> STATSCAN::makeInsertRowColTitle(string cataName, string& csvFile)
{
    vector<vector<string>> titles = extractTitles(csvFile);  // Form [row, column][title0, title1, ... ].
    vector<int> indentHistory = { -1 };
    vector<string> rowTitles(titles[0].size()), colTitles(titles[1].size());
    string temp, stmt;
    int indent, count;
    size_t pos1, pos2;
    for (int ii = 0; ii < rowTitles.size(); ii++)
    {
        indent = 0;
        while (titles[0][ii][indent] == '+') { indent++; }
        if (indent >= indentHistory.size()) { indentHistory.push_back(ii); }
        else { indentHistory[indent] = ii; }

        for (int jj = 0; jj <= indent; jj++)
        {
            temp = titles[0][indentHistory[jj]];
            if (jj == 0 && indent == 0)  // Single title, nothing to attach.
            {
                rowTitles[ii] = temp;
            }
            else if (jj == 0)  // Begin a new title, adding more later.
            {
                pos1 = temp.find_first_not_of('+');
                pos2 = temp.find("Total");
                if (pos1 == pos2)
                {
                    pos1 += 5;
                    pos2 = temp.find_first_not_of(" -", pos1);
                    rowTitles[ii] = temp.substr(pos2);
                }
                else { rowTitles[ii] = temp.substr(pos1); }
            }
            else if (jj < indent)  // Build upon the existing title, and adding more later.
            {
                pos1 = temp.find_first_not_of('+');
                pos2 = temp.find("Total");
                if (pos1 == pos2)
                {
                    pos1 += 5;
                    pos2 = temp.find_first_not_of(" -", pos1);
                    rowTitles[ii] += "(" + temp.substr(pos2);
                }
                else { rowTitles[ii] += "(" + temp.substr(pos1); }
            }
            else  // Build upon the existing title, and conclude it.
            {
                pos1 = temp.find_first_not_of('+');
                rowTitles[ii] += "(" + temp.substr(pos1);
                count = 0;
                for (int kk = 0; kk < rowTitles[ii].size(); kk++)
                {
                    if (rowTitles[ii][kk] == '(') { count++; }
                    else if (rowTitles[ii][kk] == ')') { count--; }
                }
                if (count < 0) { jf.err("Parentheses mismatch-sc.makeCreateRowColTitle"); }
                while (count > 0)
                {
                    rowTitles[ii].push_back(')');
                    count--;
                }
            }
        }
    }
    for (int ii = 0; ii < colTitles.size(); ii++)
    {
        if (ii < 2)
        {
            colTitles[ii] = titles[1][ii];
            continue;
        }
        if (titles[1][ii].front() != '+')
        {
            colTitles[ii] = titles[1][ii];
            continue;
        }

        pos1 = titles[1][1].find("Total") + 5;
        if (pos1 > titles[1][1].size()) { pos1 = 0; }
        pos2 = titles[1][1].find_first_not_of(" -", pos1);
        colTitles[ii] = titles[1][1].substr(pos2) + "(";
        pos1 = titles[1][ii].find_first_not_of('+');
        colTitles[ii] += titles[1][ii].substr(pos1) + ")";
    }

    vector<string> stmts, dirt = { "'" }, soap = { "''" };
    if (rowTitles.size() >= colTitles.size())
    {
        for (int ii = 0; ii < rowTitles.size(); ii++)
        {
            jf.clean(rowTitles[ii], dirt, soap);
            stmt = "INSERT OR IGNORE INTO \"RowColTitle$" + cataName;
            stmt += "\" (Row, Column) VALUES ('" + rowTitles[ii] + "', '";
            if (ii < colTitles.size())
            {
                jf.clean(colTitles[ii], dirt, soap);
                stmt += colTitles[ii] + "');";
            }
            else { stmt += "');"; }
            stmts.push_back(stmt);
        }
    }
    else
    {
        for (int ii = 0; ii < colTitles.size(); ii++)
        {
            jf.clean(colTitles[ii], dirt, soap);
            stmt = "INSERT OR IGNORE INTO \"RowColTitle$" + cataName;
            stmt += "\" (Row, Column) VALUES ('";
            if (ii < rowTitles.size())
            {
                jf.clean(rowTitles[ii], dirt, soap);
                stmt += rowTitles[ii] + "', '" + colTitles[ii] + "');";
            }
            else { stmt += "', '" + colTitles[ii] + "');"; }
            stmts.push_back(stmt);
        }
    }
    return stmts;
}
vector<string> STATSCAN::makeInsertTCatalogue(string cataName, string tname, vector<string>& listParam)
{
    vector<string> stmts(listParam.size() + 1);
    size_t pos1;
    for (int ii = stmts.size() - 1; ii > 0; ii--)
    {
        stmts[ii] = "INSERT OR IGNORE INTO \"" + tname + "\" (Topic"; 
        stmts[ii] += to_string(ii) + ") VALUES ('";
        if (ii == listParam.size())
        {
            stmts[ii] += cataName + "');";
        }
        else
        {
            stmts[ii] += listParam[ii] + "');";
        }
        pos1 = tname.rfind('$');
        tname = tname.substr(0, pos1);
    }
    stmts[0] = "INSERT OR IGNORE INTO TCatalogue (Year) VALUES (";
    stmts[0] += listParam[0] + ");";
    return stmts;
}
vector<string> STATSCAN::makeInsertTG_Region(string cataName, string& geoFile)
{
    vector<string> stmts, dirt = { "'" }, soap = { "''" };
    string gid, name, temp, stmt;
    vector<int> indentHistory;
    int indent, GID;
    size_t posEnd = geoFile.rfind("@@Geo Layers");
    posEnd = geoFile.find_last_of("1234567890", posEnd) + 1;
    size_t pos1 = geoFile.find_first_of("1234567890"), pos2;
    while (pos1 < posEnd)
    {
        pos2 = geoFile.find('$', pos1);
        gid = geoFile.substr(pos1, pos2 - pos1);
        pos1 = pos2 + 1;
        pos2 = geoFile.find('$', pos1);
        name = geoFile.substr(pos1, pos2 - pos1);
        jf.clean(name, dirt, soap);
        pos1 = pos2 + 1;
        pos2 = geoFile.find_first_not_of("1234567890", pos1);
        temp = geoFile.substr(pos1, pos2 - pos1);
        try 
        { 
            indent = stoi(temp); 
            GID = stoi(gid);
        }
        catch (invalid_argument) { jf.err("stoi-sc.makeInsertTG_Region"); }
        if (indent >= indentHistory.size()) { indentHistory.push_back(GID); }
        else { indentHistory[indent] = GID; }

        stmt = "INSERT OR IGNORE INTO \"TG_Region$" + cataName; 
        stmt += "\" (GID, \"Region Name\"";
        for (int ii = 0; ii < indent; ii++)
        {
            stmt += ", param" + to_string(ii + 2);
        }
        stmt += ") VALUES (" + gid + ", '" + name + "'";
        for (int ii = 0; ii < indent; ii++)
        {
            stmt += ", " + to_string(indentHistory[ii]);
        }
        stmt += ");";
        stmts.push_back(stmt);
    }
    return stmts;
}
vector<string> STATSCAN::makeInsertTMap(vector<string>& stmtsTMI)
{
    vector<string> stmts, dirt = { "$", "''" }, soap = { "\\", "'" };
    string coreDir, tname, stmt, binPath, binFile, parent;
    size_t pos1, pos2;
    vector<vector<int>> border;
    vector<double> positionPNG, positionGPS;
    double scale;
    for (int ii = 0; ii < stmtsTMI.size(); ii++)
    {
        pos1 = stmtsTMI[ii].find("VALUES");
        pos1 = stmtsTMI[ii].find('\'', pos1) + 1;
        pos2 = stmtsTMI[ii].rfind('\'');
        coreDir = stmtsTMI[ii].substr(pos1, pos2 - pos1);

        pos1 = coreDir.find('$');
        binPath = sroot + "\\mapsBIN" + coreDir.substr(pos1) + ".bin";
        jf.clean(binPath, dirt, soap);
        binFile = wf.load(binPath);

        scale = readBinScale(binFile);
        if (scale < 0.0) { jf.err("readBinScale-sc.makeInsertTMap"); }
        tname = coreDir + "$scale";
        stmt = "INSERT OR IGNORE INTO \"" + tname + "\" (\"Pixels Per KM\")";
        stmt += " VALUES (" + to_string(scale) + ");";
        stmts.push_back(stmt);

        parent = readBinParent(binFile);
        if (parent.size() < 1) { jf.err("readBinParent-sc.makeInsertTMap"); }
        jf.clean(parent, soap, dirt);  // Double quotes.
        tname = coreDir + "$parent";
        stmt = "INSERT OR IGNORE INTO \"" + tname + "\" (\"Region Name\")";
        stmt += " VALUES ('" + parent + "');";
        stmts.push_back(stmt);

        positionPNG = readBinPositionPNG(binFile);
        if (positionPNG[0] < 0.0) { jf.err("readBinPositionPNG-sc.makeInsertTMap"); }
        positionGPS = readBinPositionGPS(binFile);
        if (positionGPS[0] < -180.0) { jf.err("readBinPositionGPS-sc.makeInsertTMap"); }
        tname = coreDir + "$position";
        stmt = "INSERT OR IGNORE INTO \"" + tname + "\" (PNG, GPS) VALUES (";
        stmt += to_string(positionPNG[0]) + ", " + to_string(positionGPS[0]) + ");";
        stmts.push_back(stmt);
        stmt = "INSERT OR IGNORE INTO \"" + tname + "\" (PNG, GPS) VALUES (";
        stmt += to_string(positionPNG[1]) + ", " + to_string(positionGPS[1]) + ");";
        stmts.push_back(stmt);

        border = readBinBorder(binFile);
        if (border.size() < 1) { jf.err("readBinBorder-sc.makeInsertTMap"); }
        tname = coreDir + "$border";
        for (int jj = 0; jj < border.size(); jj++)
        {
            stmt = "INSERT OR IGNORE INTO \"" + tname + "\" (xCoord, yCoord) VALUES (";
            stmt += to_string(border[jj][0]) + ", " + to_string(border[jj][1]) + ");";
            stmts.push_back(stmt);
        }
    }
    return stmts;
}
vector<string> STATSCAN::makeInsertTMapIndex(string& geoFile, string cataName, vector<string>& geoLayers)
{
    vector<string> stmts;
    string gid, temp, name, coreDir, stmt;
    int indent;
    vector<string> dirt = { "'" }, soap = { "''" };
    size_t pos1 = geoFile.find_first_of("1234567890");
    size_t pos2 = geoFile.find('$', pos1);
    size_t pos3 = geoFile.rfind("@@Geo Layers");
    while (pos2 < pos3)
    {
        gid = geoFile.substr(pos1, pos2 - pos1);
        pos1 = pos2 + 1;
        pos2 = geoFile.find('$', pos1);
        name = geoFile.substr(pos1, pos2 - pos1);
        jf.clean(name, dirt, soap);
        pos1 = pos2 + 1;
        pos2 = geoFile.find_first_not_of("1234567890", pos1);
        temp = geoFile.substr(pos1, pos2 - pos1);
        try { indent = stoi(temp); }
        catch (invalid_argument) { jf.err("stoi-sc.makeInsertTMap"); }
        coreDir = "TMap";
        for (int ii = 0; ii <= indent; ii++)
        {
            if (geoLayers[ii] == "canada") { continue; }
            coreDir += "$" + geoLayers[ii];
        }
        coreDir += "$" + name;
        stmt = "INSERT OR IGNORE INTO \"TMapIndex$" + cataName + "\" (GID, ";
        stmt += "\"Map Path\") VALUES (" + gid + ", '" + coreDir + "');";
        stmts.push_back(stmt);
        pos2 = geoFile.find('\n', pos2);
        pos1 = pos2 + 1;
        pos2 = geoFile.find('$', pos1);
    }
    return stmts;
}

vector<string> STATSCAN::makeGeoLayers(string& geoTemp)
{
    geoTemp.append("/ ");
    vector<string> geoLayers, dirt;
    size_t pos1 = 0;
    size_t pos2 = geoTemp.find('/');
    string temp, geoLayer;
    do
    {
        temp = geoTemp.substr(pos1, pos2 - pos1);
        jf.clean(temp, dirt);  // To remove extra spaces only.
        try { geoLayer = mapGeoLayers.at(temp); }
        catch (out_of_range& oor) { err("mapGeoLayers-sc.makeGeoLayers"); }
        if (geoLayer == "fsa")  // fsa zones currently have no maps on Stats Can.
        {
            geoLayers = { "fsa" };
            return geoLayers;
        }
        geoLayers.push_back(geoLayer);
        pos1 = pos2 + 1;
        pos2 = geoTemp.find('/', pos1);
    } while (pos2 < geoTemp.size());
    return geoLayers;
}
string STATSCAN::makeGeoList(vector<string>& geoLinkNames, vector<string>& geoLayers, string geoURL)
{
    // GID$[Region Name]$indentation
    string geoList, temp, can0 = "Canada", can1 = "CANADA", can2 = "canada";
    size_t pos1, pos2;
    vector<int> indents = { -1 };
    int spaces, indent, sizeGL = geoLayers.size();
    vector<string> dirt = { "/",  "  or  " };
    vector<string> soap = { " or ", " or " };
    for (int ii = 0; ii < geoLinkNames.size(); ii++)
    {
        jf.clean(geoLinkNames[ii], dirt, soap);
        pos1 = geoLinkNames[ii].find("GID=") + 4;
        pos2 = geoLinkNames[ii].find('&', pos1);
        geoList += geoLinkNames[ii].substr(pos1, pos2 - pos1) + "$";
        pos1 = geoLinkNames[ii].rfind('>') + 1;
        geoList += geoLinkNames[ii].substr(pos1) + "$";
        pos1 = geoLinkNames[ii].find('"');
        temp = geoLinkNames[ii].substr(0, pos1);
        try { spaces = stoi(temp); }
        catch (invalid_argument& ia) { err("stoi-sc.makeGeoList"); }
        if (ii == 0 && spaces != -1) 
        {
            pos2 = geoList.rfind('$');
            pos1 = geoList.rfind('$', pos2 - 1) + 1;
            temp = geoList.substr(pos1, pos2 - pos1);
            if (temp == can0 || temp == can1 || temp == can2) { spaces = -1; }
            else { indents[0] = spaces; }
        }
        for (int jj = 0; jj < indents.size(); jj++)
        {
            if (indents[jj] == spaces)
            {
                indent = jj;
                break;
            }
            else if (jj == indents.size() - 1)
            {
                indent = indents.size();
                indents.push_back(spaces);
            }
        }
        if (indent >= sizeGL)
        {
            pos2 = geoList.rfind('$');
            pos1 = geoList.rfind('$', pos2 - 1) + 1;
            temp = geoList.substr(pos1, pos2 - pos1);
            pos1 = temp.find("part");
            pos2 = temp.find("partie");
            if (pos1 < temp.size() && pos2 < temp.size()) { indent--; }
            else { err("geoLayer indent mismatch-sc.makeGeoList"); }
        }
        geoList += to_string(indent) + "\n";
    }
    geoList += "\n@@Geo Layers\n";
    for (int ii = 0; ii < sizeGL; ii++)
    {
        geoList += geoLayers[ii] + "\n";
    }
    geoList += "\n@@Geo URL\n" + geoURL + "\n";
    return geoList;
}
string STATSCAN::make_csv_path(int gid_index)
{
    string csv_path = cata_path + "\\" + cata_name + " " + csv_branches[gid_index];
    return csv_path;
}
wstring STATSCAN::make_csv_wpath(int gid_index)
{
    string csv_path = cata_path + "\\" + cata_name + " " + csv_branches[gid_index];
    wstring csv_wpath = jf.utf8to16(csv_path);
    return csv_wpath;
}

string STATSCAN::make_create_csv_table_statement(string& stmt0, string gid, string& tname_template)
{
    // Returns the table name, after it completes the SQL statement by reference.

    string tname = tname_template;
    size_t pos1 = tname.find("!!!");
    tname.replace(pos1, 3, gid);
    pos1 = stmt0.find("!!!");
    stmt0.replace(pos1, 3, tname);
    return tname;
}
string STATSCAN::make_create_csv_table_template(vector<string>& column_titles)
{
    string stmt = "CREATE TABLE IF NOT EXISTS [!!!] (";
    for (int ii = 0; ii < column_titles.size(); ii++)
    {
        stmt += "[" + column_titles[ii];
        if (ii == 0)
        {
            stmt += "] TEXT, ";
        }
        else
        {
            stmt += "] NUMERIC, ";
        }
    }
    stmt.erase(stmt.size() - 2, 2);
    stmt += ");";
    return stmt;
}

string STATSCAN::make_insert_damaged_csv(string cata_name, string gid, int damaged_val)
{
    string stmt = "INSERT OR IGNORE INTO TDamaged ([Catalogue Name], GID, [Number of Errors]) ";
    stmt += "VALUES (?, ?, ?);";
    vector<string> param = { cata_name, gid, to_string(damaged_val) };
    jf.bind(stmt, param);
    return stmt;
}
void STATSCAN::make_insert_csv_row_statement(string& stmt0, string tname, vector<string>& row_vals)
{
    size_t pos1 = stmt0.find("!!!");
    stmt0.replace(pos1, 3, tname);
    string stmt = jf.bind(stmt0, row_vals);
    stmt0 = stmt;
}
string STATSCAN::make_insert_csv_row_template(vector<string>& column_titles)
{
    string stmt = "INSERT INTO [!!!] (";
    for (int ii = 0; ii < column_titles.size(); ii++)
    {
        stmt += "[" + column_titles[ii] + "], ";
    }
    stmt.erase(stmt.size() - 2, 2);
    stmt += ") VALUES (";
    for (int ii = 0; ii < column_titles.size(); ii++)
    {
        stmt += "?, ";
    }
    stmt.erase(stmt.size() - 2, 2);
    stmt += ");";
    return stmt;
}
void STATSCAN::make_insert_primary_statement(string& stmt0, string gid, vector<vector<string>>& text_vars, vector<vector<string>>& rows)
{
    vector<string> params;
    params.push_back(gid);
    for (int ii = 0; ii < text_vars.size(); ii++)
    {
        params.push_back(text_vars[ii][1]);
    }
    for (int ii = 0; ii < rows.size(); ii++)
    {
        for (int jj = 1; jj < rows[ii].size(); jj++)
        {
            params.push_back(rows[ii][jj]);
        }
    }
    string stmt = jf.bind(stmt0, params);
    stmt0 = stmt;
}
string STATSCAN::make_insert_primary_template(string cata_name, vector<vector<string>>& text_vars, vector<string>& linearized_titles)
{
    string stmt = "INSERT INTO [" + cata_name + "] (GID, ";
    for (int ii = 0; ii < text_vars.size(); ii++)
    {
        stmt += "[" + text_vars[ii][0] + "], ";
    }
    for (int ii = 0; ii < linearized_titles.size(); ii++)
    {
        stmt += "[" + linearized_titles[ii] + "], ";
    }
    stmt.erase(stmt.size() - 2, 2);
    stmt += ") VALUES (";
    for (int ii = 0; ii < (1 + text_vars.size() + linearized_titles.size()); ii++)
    {
        stmt += "?, ";
    }
    stmt.erase(stmt.size() - 2, 2);
    stmt += ");";
    return stmt;
}

string STATSCAN::make_subtable_name(int& num_param, string cata_name, string gid, vector<int>& my_tree_st, vector<string>& tree_pl)
{
    string tname = cata_name + "$" + gid;
    int pos, inum;
    for (int ii = 0; ii < my_tree_st.size(); ii++)
    {
        if (my_tree_st[ii] < 0)
        {
            pos = ii;
            break;
        }
        else if (ii == my_tree_st.size() - 1)
        {
            pos = 0;
        }
    }
    int cheddar = 2;
    int max_param = 0;
    for (int ii = 0; ii <= pos; ii++)
    {
        for (int jj = 0; jj < cheddar; jj++)
        {
            tname += "$";
        }
        if (cheddar > max_param) { max_param = cheddar; }
        cheddar++;
        if (ii < pos) { inum = my_tree_st[ii]; }
        else { inum = -1 * my_tree_st[ii]; }
        tname += tree_pl[inum];
    }
    num_param = max_param + 1;
    return tname;
}
int STATSCAN::make_tgr_statements(vector<string>& tgr_stmts, string syear, string sname)
{
    // For a given catalogue, read its local 'geo list' bin file, then make all statements
    // to add a complete TG_Region table to the database. Tree structure contained in param columns.
    // Tables have name  TG_Region$cata_name  and form { GID PRIMARY, Region Name, param2, param3, ... }
    // The list of params is the ancestry of this region, in order of trunk->leaf.
    // Returned integer is the max number of columns in the table. 
    // NOTE: The first statement in the vector must be executed BEFORE declaring a transaction !

    string geo_list_path = sroot + "\\" + syear + "\\" + sname + "\\" + sname + " geo list.bin";
    string geo_list = jf.load(geo_list_path);
    vector<vector<string>> tgr;
    vector<string> vtemp(3);
    int tgr_index, inum1, inum2;
    vector<string> hall_of_fame;

    int num_col = 2;
    size_t pos1 = geo_list.find_first_of("1234567890");
    size_t pos2 = geo_list.find('$');
    do
    {
        vtemp[0] = geo_list.substr(pos1, pos2 - pos1);
        pos1 = pos2 + 1;
        pos2 = geo_list.find('$', pos2 + 1);
        vtemp[1] = geo_list.substr(pos1, pos2 - pos1);
        pos1 = pos2 + 1;
        pos2 = geo_list.find_first_not_of("1234567890", pos2 + 1);
        vtemp[2] = geo_list.substr(pos1, pos2 - pos1);
        try
        {
            inum1 = stoi(vtemp[0]);
            inum2 = stoi(vtemp[2]);
        }
        catch (invalid_argument& ia)
        {
            err("stoi-sc.make_tgr_statements");
        }
        if (inum2 + 2 > num_col)
        {
            num_col = inum2 + 2;
        }

        while (hall_of_fame.size() <= inum2)
        {
            hall_of_fame.push_back(vtemp[0]);
        }
        hall_of_fame[inum2] = vtemp[0];

        tgr_index = tgr.size();
        tgr.push_back(vector<string>(2));
        tgr[tgr_index][0] = vtemp[0];
        tgr[tgr_index][1] = vtemp[1];
        for (int ii = 0; ii < inum2; ii++)
        {
            tgr[tgr_index].push_back(hall_of_fame[ii]);
        }

        pos1 = geo_list.find('\n', pos1 + 1) + 1;
        pos2 = geo_list.find('$', pos2 + 1);
    } while (pos2 < geo_list.size());

    string temp;
    tgr_stmts.clear();
    vtemp.clear();
    string stmt = "CREATE TABLE IF NOT EXISTS [TG_Region$" + sname + "] (GID INTEGER PRIMARY KEY, ";
    stmt += "[Region Name] TEXT, ";
    for (int ii = 2; ii < num_col; ii++)
    {
        temp = "param" + to_string(ii);
        vtemp.push_back(temp);
        stmt += temp + " INT, ";
    }
    stmt.erase(stmt.size() - 2, 2);
    stmt += ");";
    tgr_stmts.push_back(stmt);

    for (int ii = 0; ii < tgr.size(); ii++)
    {
        stmt = "INSERT OR IGNORE INTO [TG_Region$" + sname + "] (GID, [Region Name], ";
        for (int jj = 0; jj < tgr[ii].size() - 2; jj++)
        {
            stmt += vtemp[jj] + ", ";
        }
        stmt.erase(stmt.size() - 2, 2);
        stmt += ") VALUES (";
        for (int jj = 0; jj < tgr[ii].size(); jj++)
        {
            if (jj == 1)
            {
                sclean(tgr[ii][jj], 1);
                stmt += "'" + tgr[ii][jj] + "', ";
            }
            else
            {
                stmt += tgr[ii][jj] + ", ";
            }
        }
        stmt.erase(stmt.size() - 2, 2);
        stmt += ");";
        tgr_stmts.push_back(stmt);
    }
    return num_col;
}
int STATSCAN::make_tgrow_statements(vector<string>& tgrow_stmts)
{
    // For a given catalogue, scan its sample CSV file, then make all statements
    // to add a complete TG_Row table to the database. Tree structure contained in param columns.
    // Tables have name  TG_Row$cata_name  and form { Row Index, Row Title, param2, param3, ... }
    // The list of params is the ancestry of this row, in order of trunk->leaf.
    // Returned integer is the max number of columns in the table. 
    // NOTE: The first statement in the vector must be executed BEFORE declaring a transaction !

    int num_rows = rows.size();
    if (num_rows == 0) { err("Cannot sc.make_tgrow_statements before sc.cata_init"); }
    vector<vector<string>> tgrow(num_rows, vector<string>());
    vector<int> hall_of_fame;
    string temp, row;
    int indent;
    int tg_row_col = 0;
    for (int ii = 0; ii < num_rows; ii++)
    {
        row = rows[ii][0];
        indent = 0;
        while (row[0] == '+')
        {
            indent++;
            row.erase(row.begin());
        }
        while (hall_of_fame.size() <= indent)
        {
            hall_of_fame.push_back(ii);
        }
        hall_of_fame[indent] = ii;
        tgrow[ii].push_back(to_string(ii));
        tgrow[ii].push_back(row);
        for (int jj = 0; jj < indent; jj++)
        {
            tgrow[ii].push_back(to_string(hall_of_fame[jj]));
        }
        if (tg_row_col < indent + 2)
        {
            tg_row_col = indent + 2;
        }
    }

    tgrow_stmts.resize(num_rows + 1);
    string stmt = "CREATE TABLE IF NOT EXISTS [TG_Row$" + cataName + "] (";
    stmt += "[Row Index] INTEGER PRIMARY KEY, [Row Title] TEXT, ";
    for (int ii = 2; ii < tg_row_col; ii++)
    {
        temp = "param" + to_string(ii);
        stmt += temp + " INT, ";
    }
    stmt.erase(stmt.size() - 2, 2);
    stmt += ");";
    tgrow_stmts[0] = stmt;

    for (int ii = 0; ii < num_rows; ii++)
    {
        stmt = "INSERT OR IGNORE INTO [TG_Row$" + cataName + "] ([Row Index], [Row Title], ";
        for (int jj = 2; jj < tgrow[ii].size(); jj++)
        {
            temp = "param" + to_string(jj);
            stmt += temp + ", ";
        }
        stmt.erase(stmt.size() - 2, 2);
        stmt += ") VALUES (";
        for (int jj = 0; jj < tgrow[ii].size(); jj++)
        {
            if (jj == 1)
            {
                sclean(tgrow[ii][jj], 1);
                stmt += "'" + tgrow[ii][jj] + "', ";
            }
            else
            {
                stmt += tgrow[ii][jj] + ", ";
            }
        }
        stmt.erase(stmt.size() - 2, 2);
        stmt += ");";
        tgrow_stmts[ii + 1] = stmt;
    }

    return tg_row_col;
}

string STATSCAN::mapLinkToPDFUrl(string& urlMap, string& mapLink)
{
    size_t pos1 = urlMap.find('/') + 1;
    pos1 = urlMap.find('/', pos1) + 1;
    string mapUrl = urlMap.substr(0, pos1);
    mapUrl += "geo/maps-cartes/pdf/";
    pos1 = mapLink.find("[%22") + 4;
    size_t pos2 = mapLink.find("%22", pos1);
    string dguid = mapLink.substr(pos1, pos2 - pos1);
    string temp = dguid.substr(4, 5);
    mapUrl += temp + "/" + dguid + ".pdf";
    return mapUrl;
}
vector<vector<string>> STATSCAN::navAsset()
{
    vector<vector<string>> nA = {
        {"<select name=\"Temporal\"", "<select name=\"Temporal\"", "</select>", "<option value=\"", "\" "},
        {"<select name=\"Temporal\"", "<tbody>", "</tbody>", "&ips=", "\" targ"},
        {"ndex</h3>", "<div id=\"geo-download\"", "</ol>", "=\"indent-", "</a>"},
        {"Download data as displayed", "Download data as displayed", "comma-separated values", "href=\"", "\">CSV"},
        {"id=\"tablist\"", "<a title=\"Download\"", "Download</span>", "href=\"", "\"><"},
        {"displayed in the Data table tab</h4>", "displayed in the Data table tab</h4>", "comma-separated values", "href=\"", "\">CSV"},
        {">Geographic index</h3>", "<p><strong>", "</strong> <span", "<strong>", "</strong>"},
        {"id=\"tablist\"", "<a title=\"Map\"", "Map</span>", "href=\"", "\"><i"},
        {"<div id=\"map-cont", "<iframe", "</iframe>", "https://", "]}]}"}
    };
    return nA;
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
    if (posEnd > binFile.size()) { jf.err("No posEnd found-sc.readBinBorder"); }

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

vector<vector<string>> STATSCAN::readGeo(string& geoPath)
{
    // First row is the geo layer data (province, cmaca, etc).
    vector<vector<string>> geo(1, vector<string>());
    size_t pos1, pos2, pos3;
    string sfile = jf.load(geoPath);
    pos1 = sfile.rfind("@@Geo");
    if (pos1 > sfile.size()) { err("No geo layers-sc.readGeo"); }
    pos3 = sfile.find_last_not_of('\n') + 1;
    pos1 = sfile.find('\n', pos1) + 1;
    pos2 = sfile.find('\n', pos1);
    while (pos2 <= pos3)
    {
        if (pos1 == pos2)
        {
            geo[0].push_back("");
        }
        else
        {
            geo[0].push_back(sfile.substr(pos1, pos2 - pos1));
        }
        pos1 = pos2 + 1;
        pos2 = sfile.find('\n', pos1);
    }

    pos1 = sfile.find_first_of("1234567890");
    pos2 = sfile.find('$', pos1);
    while (pos2 < sfile.size())
    {
        pos3 = geo.size();
        geo.push_back(vector<string>(3));
        geo[pos3][0] = sfile.substr(pos1, pos2 - pos1);
        pos1 = pos2 + 1;
        pos2 = sfile.find('$', pos1);
        geo[pos3][1] = sfile.substr(pos1, pos2 - pos1);
        pos1 = pos2 + 1;
        pos2 = sfile.find('\n', pos1);
        geo[pos3][2] = sfile.substr(pos1, pos2 - pos1);
        pos1 = pos2 + 1;
        pos2 = sfile.find('$', pos1);
    }
    return geo;
}
vector<vector<string>> STATSCAN::readGeo(string& geoPath, unordered_map<string, string>& mapGeo)
{
    // Normal rows have the form [GID, Region Name, indentation].
    vector<vector<string>> geo;
    size_t pos1, pos2, pos3;
    int index;
    string sfile = jf.load(geoPath), temp;
    pos1 = sfile.find_first_of("1234567890");
    pos2 = sfile.find('$', pos1);
    while (pos2 < sfile.size())
    {
        index = geo.size();
        geo.push_back(vector<string>(3));
        geo[index][0] = sfile.substr(pos1, pos2 - pos1);
        pos1 = pos2 + 1;
        pos2 = sfile.find('$', pos1);
        geo[index][1] = sfile.substr(pos1, pos2 - pos1);
        pos1 = pos2 + 1;
        pos2 = sfile.find('\n', pos1);
        geo[index][2] = sfile.substr(pos1, pos2 - pos1);
        pos1 = pos2 + 1;
        pos2 = sfile.find('$', pos1);
        temp = jf.utf8ToAscii(geo[index][1]);
        mapGeo.emplace(geo[index][0], temp);
    }

    // First supplementary row is the geo layer data (province, cmaca, etc).
    index = geo.size();
    geo.push_back(vector<string>());
    pos1 = sfile.rfind("@@Geo Layers");
    if (pos1 > sfile.size()) { err("No geo layers-sc.readGeo"); }
    pos1 = sfile.find('\n', pos1) + 1;
    pos2 = sfile.find('\n', pos1);
    pos3 = sfile.find("@@Geo", pos1);
    pos3 = sfile.find_last_not_of('\n', pos3 - 1) + 1;
    while (pos2 <= pos3)
    {
        if (pos1 == pos2)
        {
            geo[index].push_back("");
        }
        else
        {
            geo[index].push_back(sfile.substr(pos1, pos2 - pos1));
        }
        pos1 = pos2 + 1;
        pos2 = sfile.find('\n', pos1);
    }

    // Second supplementary row is the geo page's URL.
    index = geo.size();
    geo.push_back(vector<string>(1));
    pos1 = sfile.find("@@Geo URL", pos3);
    pos1 = sfile.find('\n', pos1) + 1;
    pos2 = sfile.find('\n', pos1);
    geo[index][0] = sfile.substr(pos1, pos2 - pos1);

    return geo;
}
string STATSCAN::regionLinkToMapUrl(string& urlRegion, string& regionLink)
{
    size_t pos1 = urlRegion.rfind('/') + 1;
    string mapUrl = urlRegion.substr(0, pos1);
    mapUrl += regionLink;
    cleanURL(mapUrl);
    return mapUrl;
}

int STATSCAN::sclean(string& sval, int mode)
{
    int count = 0;
    int pos1, pos2, pos3, inum = -1;
    string temp;
    pos1 = sval.find('[');
    if (pos1 > 0)
    {
        pos2 = sval.find(']', pos1);
        sval.erase(pos1, pos2 - pos1 + 1);
    }
    if (mode == 1)
    {
        pos1 = sval.find('\'');
        while (pos1 > 0)
        {
            sval.replace(pos1, 1, "''");
            pos1 = sval.find('\'', pos1 + 2);
        }
    }
    else if (mode == 2)
    {
        pos1 = sval.find('(');
        while (pos1 > 0)
        {
            pos2 = sval.find(')', pos1);
            temp = sval.substr(pos1 + 1, pos2 - pos1 - 1);
            pos3 = temp.find("Sample Data");
            try { inum = stoi(temp); }
            catch (invalid_argument) { inum = -1; }
            if (inum >= 0 || pos3 >= 0)
            {
                sval.erase(sval.begin() + pos1, sval.begin() + pos2 + 1);
                pos1 = sval.find('(', pos1 - 1);
            }
            else { pos1 = sval.find('(', pos2); }
        }
    }
    while (1)
    {
        if (sval.front() == ' ') { sval.erase(0, 1); count++; }
        else { break; }
    }
    while (1)
    {
        if (sval.back() == ' ') { sval.erase(sval.size() - 1, 1); }
        else { break; }
    }
    return count;
}
void STATSCAN::set_path(string path)
{
	cata_path = path;
}
int STATSCAN::skimGeoList(string& filePath, vector<string>& geoLayers)
{
    string sfile = jf.load(filePath);
    int numRegions = 0;
    size_t pos1 = sfile.find('\n');
    size_t pos2 = sfile.rfind("@@Geo Layers");
    if (pos2 > sfile.size()) { err("Geo list lacking geo layers-sc.skimGeoList"); }
    while (pos1 < pos2)
    {
        numRegions++;
        pos1 = sfile.find('\n', pos1 + 1);
    }
    numRegions--;

    size_t pos3 = sfile.rfind("@@Geo URL");
    pos3 = sfile.find_last_not_of('\n', pos3 - 1) + 1;
    pos2 = sfile.find('\n', pos2);
    while (pos2 < pos3)
    {
        pos1 = pos2 + 1;
        pos2 = sfile.find('\n', pos1);
        if (pos2 <= pos3)
        {
            if (pos1 == pos2) { geoLayers.push_back(""); }
            else { geoLayers.push_back(sfile.substr(pos1, pos2 - pos1)); }
        }
    }
    return numRegions;
}
vector<vector<string>> STATSCAN::splitLinkNames(vector<string>& linkNames)
{
    vector<vector<string>> split(linkNames.size(), vector<string>(4));
    size_t pos1, pos2;
    string gid, temp;
    int inum;
    for (int ii = 0; ii < linkNames.size(); ii++)
    {
        pos1 = linkNames[ii].find('"');
        split[ii][0] = linkNames[ii].substr(0, pos1);
        pos1 = linkNames[ii].find("href=\"") + 6;
        pos2 = linkNames[ii].find('"', pos1);
        temp = linkNames[ii].substr(pos1, pos2 - pos1);
        cleanURL(temp);
        split[ii][1] = temp;
        pos1 = linkNames[ii].rfind('>') + 1;
        split[ii][2] = linkNames[ii].substr(pos1);
        pos1 = linkNames[ii].find("GID=") + 4;
        pos2 = linkNames[ii].find('&', pos1);
        gid = linkNames[ii].substr(pos1, pos2 - pos1);
        try
        {
            inum = stoi(gid);
        }
        catch (out_of_range& oor) { err("stoi-sc.splitLinkNames"); }
        split[ii][3] = gid;
    }
    return split;
}
bool STATSCAN::testCanadaOnly(string& geoLayer)
{
    size_t pos1 = geoLayer.find("Canada Only");
    if (pos1 < geoLayer.size()) { return 1; }
    return 0;
}
bool STATSCAN::testFileNotFound(string& webpage)
{
    // Returns TRUE if the given webpage's title contains 'File Not Found'.
    size_t pos1 = webpage.find("<title>");
    size_t pos2 = webpage.find("</title>", pos1);
    if (pos1 > webpage.size() || pos2 > webpage.size()) { err("No HTML title-sc.testFileNotFound"); }
    string temp = webpage.substr(pos1, pos2 - pos1);
    pos1 = temp.find("File Not Found");
    if (pos1 < temp.size()) { return 1; }
    return 0;
}
bool STATSCAN::testGeoList(string& filePath)
{
    string sfile = jf.load(filePath);
    size_t pos = sfile.find("@@Geo Layers");
    if (pos > sfile.size()) { return 0; }
    pos = sfile.find("@@Geo URL");
    if (pos > sfile.size()) { return 0; }
    pos = sfile.find("gc.caor", pos);
    if (pos < sfile.size()) { return 0; }
    return 1;
}
void STATSCAN::tree_from_indent(vector<vector<int>>& tree_st, vector<vector<string>>& rows)
{
    // Note that the tree structure is of the form 
    // [node_index][ancestor1, ancestor2, ... , (neg) node value, child1, child2, ...]

    // Genealogy's indices are indent magnitudes, while its values are the last list index
    // to have that indentation.

    vector<string> row_list(rows.size());
    for (int ii = 0; ii < rows.size(); ii++)
    {
        row_list[ii] = rows[ii][0];
    }

    vector<int> genealogy, vtemp;
    int indent, delta_indent, node, parent;
    tree_st.resize(row_list.size(), vector<int>());

    for (int ii = 0; ii < row_list.size(); ii++)
    {
        // Determine this row title's indentation.
        indent = 0;
        while (row_list[ii][indent] == '+')
        {
            indent++;
        }

        // Update genealogy.
        delta_indent = indent - genealogy.size() + 1;  // New indent minus old indent.
        if (delta_indent == 0)
        {
            genealogy[genealogy.size() - 1] = ii;
        }
        else if (delta_indent > 0)
        {
            for (int jj = 0; jj < delta_indent; jj++)
            {
                genealogy.push_back(ii);
            }
        }
        else if (delta_indent < 0)
        {
            for (int jj = 0; jj > delta_indent; jj--)
            {
                genealogy.pop_back();
            }
            genealogy[genealogy.size() - 1] = ii;
        }

        // Populate the current node with its ancestry and with itself, but no children.
        tree_st[ii] = genealogy;
        node = tree_st[ii].size() - 1;  // Genealogical position of the current node.
        tree_st[ii][node] *= -1;

        // Determine this node's parent, and add this node to its list of children.
        if (node == 0)
        {
            continue;  // This node has no parents.
        }
        parent = genealogy[node - 1];
        tree_st[parent].push_back(ii);
    }
}

string STATSCAN::urlCatalogue(int iYear, string sCata)
{
    string urlCata, urlTemp;
    wstring wPage, wTemp, wCata;
    size_t pos1, pos2;
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
string STATSCAN::urlCataDownload(int iyear, string& geoPage, string gid)
{
    int mode;
    size_t pos1, pos2;
    string temp, pid, url;
    if (iyear >= 2016) { mode = 0; }
    switch (mode)
    {
    case 0:
    {
        pos1 = geoPage.find("geo-download");
        if (pos1 > geoPage.size()) { err("geo-download-sc.urlCataDownload"); }
        temp = "GID=" + gid;
        pos1 = geoPage.find(temp, pos1);
        pos1 = geoPage.find("PID=", pos1) + 4;
        pos2 = geoPage.find('&', pos1);
        pid = geoPage.substr(pos1, pos2 - pos1);
        url = "www12.statcan.gc.ca/census-recensement/2016/dp-pd/dt-td/File.cfm?S=0&LANG=E&A=R&PID=";
        url += pid + "&GID=" + gid + "&D1=0&D2=0&D3=0&D4=0&D5=0&D6=0&OFT=CSV";
        break;
    }
    }
    return url;
}
string STATSCAN::urlCataList(int iyear, string scata)
{
    string temp;
    return temp;
}
string STATSCAN::urlEntireTableDownload(int iYear, string& urlCata)
{
    string url, pid;
    size_t pos1, pos2;
    if (iYear >= 2016)
    {
        pos1 = urlCata.rfind('/');
        url = urlCata.substr(0, pos1);
        pos1 = urlCata.find("PID=", pos1) + 4;
        pos2 = urlCata.find('&', pos1);
        pid = urlCata.substr(pos1, pos2 - pos1);
        url += "/CompDataDownload.cfm?LANG=E&PID=";
        url += pid + "&OFT=CSV";
    }
    return url;
}
string STATSCAN::urlGeoList(int iyear, string urlCata)
{
    int mode;
    size_t pos1, pos2;
    string temp, syear, pid, ptype, theme, urlGeo;
    if (iyear <= 1996) { mode = 0; }
    else if (iyear == 2001) { mode = 1; }
    else if (iyear == 2006) { mode = 2; }
    else if (iyear >= 2011) { mode = 3; }
    switch (mode)
    {
    case 0:
    {
        syear = to_string(iyear);
        temp = syear.substr(2);
        pos1 = urlCata.find("PID=") + 4;
        pos2 = urlCata.find('&', pos1);
        pid = urlCata.substr(pos1, pos2 - pos1);
        pos1 = urlCata.find("PTYPE=") + 6;
        pos2 = urlCata.find('&', pos1);
        ptype = urlCata.substr(pos1, pos2 - pos1);
        pos1 = urlCata.find("THEME=") + 6;
        pos2 = urlCata.find('&', pos1);
        theme = urlCata.substr(pos1, pos2 - pos1);
        urlGeo = "www12.statcan.gc.ca/English/census" + temp;
        urlGeo += "/data/tables/Geo-index-eng.cfm?TABID=5&LANG=E&APATH=3&";
        urlGeo += "DETAIL=1&DIM=0&FL=A&FREE=1&GC=0&GID=0&GK=0&GRP=1&PID=" + pid;
        urlGeo += "&PRID=0&PTYPE=" + ptype + "&S=0&SHOWALL=No&SUB=0&Temporal=";
        urlGeo += syear + "&THEME=" + theme;
        urlGeo += "&VID=0&VNAMEE=&VNAMEF=&D1=0&D2=0&D3=0&D4=0&D5=0&D6=0";
        break;
    }
    case 1:
    {
        syear = to_string(iyear);
        temp = syear.substr(2);
        pos1 = urlCata.find("PID=") + 4;
        pos2 = urlCata.find('&', pos1);
        pid = urlCata.substr(pos1, pos2 - pos1);
        pos1 = urlCata.find("PTYPE=") + 6;
        pos2 = urlCata.find('&', pos1);
        ptype = urlCata.substr(pos1, pos2 - pos1);
        pos1 = urlCata.find("THEME=") + 6;
        pos2 = urlCata.find('&', pos1);
        theme = urlCata.substr(pos1, pos2 - pos1);
        urlGeo = "www12.statcan.gc.ca/English/census" + temp;
        urlGeo += "/products/standard/themes/Geo-index-eng.cfm?TABID=5&LANG=E"; 
        urlGeo += "&APATH=3&DETAIL=1&DIM=0&FL=A&FREE=1&GC=0&GID=0&GK=0&GRP=1&PID=";
        urlGeo += pid + "&PRID=0&PTYPE=" + ptype + "&S=0&SHOWALL=No&SUB=0"; 
        urlGeo += "&Temporal=2006&THEME=" + theme;
        urlGeo += "&VID=0&VNAMEE=&VNAMEF=&D1=0&D2=0&D3=0&D4=0&D5=0&D6=0";
        break;
    }
    case 2:
    {
        syear = to_string(iyear);
        pos1 = urlCata.find("PID=") + 4;
        pos2 = urlCata.find('&', pos1);
        pid = urlCata.substr(pos1, pos2 - pos1);
        pos1 = urlCata.find("PTYPE=") + 6;
        pos2 = urlCata.find('&', pos1);
        ptype = urlCata.substr(pos1, pos2 - pos1);
        pos1 = urlCata.find("THEME=") + 6;
        pos2 = urlCata.find('&', pos1);
        theme = urlCata.substr(pos1, pos2 - pos1);
        urlGeo = "www12.statcan.gc.ca/census-recensement/2006/dp-pd/prof";
        urlGeo += "/rel/Geo-index-eng.cfm?TABID=5&LANG=E&APATH=3";
        urlGeo += "&DETAIL=1&DIM=0&FL=A&FREE=1&GC=0&GID=0&GK=0&GRP=1&PID=";
        urlGeo += pid + "&PRID=0&PTYPE=" + ptype + "&S=0&SHOWALL=No&SUB=0"; 
        urlGeo += "&Temporal=" + syear + "&THEME=" + theme;
        urlGeo += "&VID=0&VNAMEE=&VNAMEF=&D1=0&D2=0&D3=0&D4=0&D5=0&D6=0";
        break;
    }
    case 3:
    {
        pos1 = urlCata.find("LANG=E");
        urlGeo = urlCata;
        urlGeo.insert(pos1, "TABID=6&");
        urlGeo.append("&D1=0&D2=0&D3=0&D4=0&D5=0&D6=0");
        break;
    }
    }
    return urlGeo;
}
string STATSCAN::urlYear(string syear)
{
    string url = "www12.statcan.gc.ca/datasets/index-eng.cfm?Temporal=" + syear;
    return url;
}
