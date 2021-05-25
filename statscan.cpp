#include "statscan.h"

using namespace std;

int STATSCAN::cata_init(string& sample_csv)
{
    size_t pos1 = cata_path.rfind('\\');
    cata_name = cata_path.substr(pos1 + 1);
    cata_desc = extract_description(sample_csv);
    text_vars = extract_text_vars(sample_csv);
    column_titles = extract_column_titles(sample_csv);
    int damaged_val;  // Unneeded for init.
    rows = extract_rows(sample_csv, damaged_val);
    linearized_titles = linearize_row_titles(rows, column_titles);

    insert_primary_template = make_insert_primary_template(cata_name, text_vars, linearized_titles);
    create_csv_table_template = make_create_csv_table_template(column_titles);
    insert_csv_row_template = make_insert_csv_row_template(column_titles);

    jf.tree_from_indent(csv_tree, rows);
    int tg_row_col;
    subtable_names_template = make_subtable_names_template(cata_name, tg_row_col, csv_tree, rows);

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
            jf.unzip(zipPath);
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
vector<string> STATSCAN::extract_column_titles(string& sfile)
{
    vector<string> column_titles;
    string temp1;
    size_t pos1, pos2, pos_nl1, pos_nl2;
    int spaces, indent;
    vector<int> space_history = { 0 };
    char math;

    if (final_text_var == 0)
    {
        pos1 = 0;
        while (1)
        {
            pos1 = sfile.find('=', pos1 + 1);
            if (pos1 > sfile.size()) { break; }  // Loop exit.
            math = sfile[pos1 - 1];
            if (math == '<' || math == '>') { continue; }  // Ignore cases where '=' is used in row titles.
            final_text_var = pos1;
        }
    }

    pos_nl1 = sfile.find('\n', final_text_var);
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
vector<vector<string>> STATSCAN::extract_rows(string& sfile, int& damaged)
{
    // Returns a 2D vector of the form [row index][row title, row val1, row val2, ...].

    vector<vector<string>> rows;
    string line, temp1;
    size_t pos1, pos2, pos3, pos_nl1, pos_nl2;
    char math;
    damaged = 0;

    if (final_text_var == 0)
    {
        pos1 = 0;
        while (1)
        {
            pos1 = sfile.find('=', pos1 + 1);
            if (pos1 > sfile.size()) { break; }  // Loop exit.
            math = sfile[pos1 - 1];
            if (math == '<' || math == '>') { continue; }  // Ignore cases where '=' is used in row titles.
            final_text_var = pos1;
        }
    }
    
    pos_nl1 = sfile.find('\n', final_text_var);
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

                pos3 = line.find_last_of("1234567890") + 1;
                pos1 = line.find_last_of(" ,", pos3 - 1) + 1;
                temp1 = line.substr(pos1, pos3 - pos1);
                rows[rindex].push_back(temp1);
            }
            else
            {
                temp1 = line.substr(pos1 + 1, pos2 - pos1 - 1);
                if (temp1 == "..") { damaged++; }
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
vector<vector<string>> STATSCAN::extract_text_vars(string& sfile)
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
		
        final_text_var = pos1;
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
int STATSCAN::get_num_subtables()
{
    return subtable_names_template.size();
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
        for (int jj = 1; jj < rows[0].size(); jj++)
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
    string stmt = "CREATE TABLE IF NOT EXISTS [TG_Row$" + cata_name + "] (";
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
        stmt = "INSERT OR IGNORE INTO [TG_Row$" + cata_name + "] ([Row Index], [Row Title], ";
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
    int pos1, pos2;
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
    return 1;
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
