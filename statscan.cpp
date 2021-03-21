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

    jf_sc.tree_from_indent(csv_tree, rows);
    int max_param;
    subtable_names_template = make_subtable_names_template(cata_name, max_param, csv_tree, rows);

    return max_param;
}
void STATSCAN::err(string func)
{
    jf_sc.err(func);
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
    string temp1;
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
    int spaces, indent;
    do
    {
        pos1 = sfile.find('"', pos_nl1);
        pos2 = sfile.find('"', pos1 + 1);
        temp1 = sfile.substr(pos1 + 1, pos2 - pos1 - 1);
        spaces = sclean(temp1, 0);
        if (temp1 == "Note") { break; }  // Primary loop exit.
        rows.push_back(vector<string>(1));

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
        rows[rows.size() - 1][0] = temp1;

        pos2 = sfile.find(',', pos2);
        do
        {
            pos1 = pos2;
            pos2 = sfile.find(',', pos1 + 1);
            if (pos2 > pos_nl2)  // If we have reached the last value on this line...
            {
                pos3 = sfile.find(' ', pos1 + 1);  // ... check for a space before newline.
                if (pos3 > pos_nl2)
                {
                    pos3 = sfile.find('\r', pos1 + 1);  // ... check for Windows' carriage feed return.
                    if (pos3 > pos_nl2)
                    {
                        pos3 = sfile.find('\n', pos1 + 1);  // ... final option, get the newline char. 
                    }
                }
                temp1 = sfile.substr(pos1 + 1, pos3 - pos1 - 1);
                if (temp1 == "..") { damaged++; }
                rows[rows.size() - 1].push_back(temp1);
            }
            else
            {
                temp1 = sfile.substr(pos1 + 1, pos2 - pos1 - 1);
                if (temp1 == "..") { damaged++; }
                rows[rows.size() - 1].push_back(temp1);
            }
        } while (pos2 < pos_nl2);

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
string STATSCAN::make_csv_path(int gid_index)
{
    string csv_path = cata_path + "\\" + cata_name + " " + csv_branches[gid_index];
    return csv_path;
}
wstring STATSCAN::make_csv_wpath(int gid_index)
{
    string csv_path = cata_path + "\\" + cata_name + " " + csv_branches[gid_index];
    wstring csv_wpath = jf_sc.utf8to16(csv_path);
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
    string tname = cata_name + "$Damaged";
    string stmt = "INSERT INTO [" + tname + "] (GID, [Number of Missing Data Entries]) ";
    stmt += "VALUES (?, ?);";
    vector<string> param = { gid, to_string(damaged_val) };
    jf_sc.bind(stmt, param);
    return stmt;
}
void STATSCAN::make_insert_csv_row_statement(string& stmt0, string tname, vector<string>& row_vals)
{
    size_t pos1 = stmt0.find("!!!");
    stmt0.replace(pos1, 3, tname);
    string stmt = jf_sc.bind(stmt0, row_vals);
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
vector<string> STATSCAN::make_insert_csv_subtable_statements(string gid, vector<vector<int>>& tree_st, vector<vector<string>>& rows)
{
    vector<string> tree_pl(rows.size());
    for (int ii = 0; ii < rows.size(); ii++)
    {
        tree_pl[ii] = rows[ii][0];
    }

    vector<string> stmts, vtemp;
    int num_param, pos, child;
    vector<int> children;
    string tname, stmt0, stmt;
    for (int ii = 0; ii < tree_st.size(); ii++)
    {
        // Locate this node's position within its tree vector.
        children.clear();
        for (int jj = 0; jj < tree_st[ii].size(); jj++)
        {
            if (tree_st[ii][jj] < 0)
            {
                pos = jj;
                break;
            }
            else if (jj == tree_st[ii].size() - 1)
            {
                pos = 0;
            }
        }

        // Determine this node's children.
        for (int jj = pos + 1; jj < tree_st[ii].size(); jj++)
        {
            children.push_back(tree_st[ii][jj]);
        }

        // If this node is a parent, generate insert statements for it and its children.
        if (children.size() > 0)
        {
            tname = make_subtable_name(num_param, cata_name, gid, tree_st[ii], tree_pl);

            stmt0 = "INSERT INTO [" + tname + "] (";
            for (int jj = 0; jj < column_titles.size(); jj++)
            {
                stmt0 += "[" + column_titles[jj] + "], ";
            }
            stmt0.erase(stmt0.size() - 2, 2);
            stmt0 += ") VALUES (";
            for (int jj = 0; jj < column_titles.size(); jj++)
            {
                stmt0 += "?, ";
            }
            stmt0.erase(stmt0.size() - 2, 2);
            stmt0 += ");";

            stmt = stmt0;
            vtemp.assign(rows[ii].begin(), rows[ii].end());
            jf_sc.bind(stmt, vtemp);
            stmts.push_back(stmt);

            for (int jj = 0; jj < children.size(); jj++)
            {
                child = children[jj];
                stmt = stmt0;
                vtemp.assign(rows[child].begin(), rows[child].end());
                jf_sc.bind(stmt, vtemp);
                stmts.push_back(stmt);
            }
        }
    }
    return stmts;
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
    string stmt = jf_sc.bind(stmt0, params);
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
string STATSCAN::make_tg_insert_statement(vector<string>& row)
{
    string stmt = "INSERT INTO TGenealogy (Name, ";
    string temp;
    for (int ii = 1; ii < row.size(); ii++)
    {
        temp = "param" + to_string(ii);
        stmt += temp + ", ";
    }
    stmt.erase(stmt.size() - 2, 2);
    stmt += ") VALUES (";
    for (int ii = 0; ii < row.size(); ii++)
    {
        temp = "'" + row[ii] + "'";
        stmt += temp + ", ";
    }
    stmt.erase(stmt.size() - 2, 2);
    stmt += ");";
    return stmt;
}
int STATSCAN::make_tgr_statements(vector<string>& tgr_stmts, string syear, string sname)
{
    // For a given catalogue, read its local 'geo list' bin file, then make all statements
    // to add a complete TG_Region table to the database. Tree structure contained in param columns.
    // Tables have name  TG_Region$cata_name  and form { GID PRIMARY, Region Name, gid0, gid1, ... }
    // Returned integer is the max number of columns in the table. 
    // NOTE: The first statement in the vector must be executed BEFORE declaring a transaction !

    string geo_list_path = root + "\\" + syear + "\\" + sname + "\\" + sname + " geo list.bin";
    string geo_list = jf_sc.load(geo_list_path);
    vector<vector<string>> tgr;
    vector<string> vtemp(3);
    int tgr_index, inum1, inum2;
    vector<string> hall_of_fame;

    int num_col = 2;
    size_t pos1 = 0;
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
    for (int ii = 0; ii < num_col - 2; ii++)
    {
        temp = "gid" + to_string(ii);
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
