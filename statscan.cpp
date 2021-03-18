#include "statscan.h"

using namespace std;

void STATSCAN::cata_init(string& sample_csv)
{
    size_t pos1 = cata_path.rfind('\\');
    cata_name = cata_path.substr(pos1 + 1);
    cata_desc = extract_description(sample_csv);
    text_vars = extract_text_vars(sample_csv);
    column_titles = extract_column_titles(sample_csv);
    rows = extract_rows(sample_csv);
    linearized_titles = linearize_row_titles(rows, column_titles);

    insert_primary_template = make_insert_primary_template(cata_name, text_vars, linearized_titles);
    create_csv_table_template = make_create_csv_table_template(column_titles);
    insert_csv_row_template = make_insert_csv_row_template(column_titles);
     
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
vector<vector<string>> STATSCAN::extract_rows(string& sfile)
{
    // Returns a 2D vector of the form [row index][row title, row val1, row val2, ...].

    vector<vector<string>> rows;
    string temp1;
    size_t pos1, pos2, pos3, pos_nl1, pos_nl2;
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
                rows[rows.size() - 1].push_back(temp1);
            }
            else
            {
                temp1 = sfile.substr(pos1 + 1, pos2 - pos1 - 1);
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
string STATSCAN::get_create_csv_table_template()
{
    return create_csv_table_template;
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
string STATSCAN::make_create_csv_table_template(vector<string>& column_titles)
{
    string stmt = "CREATE TABLE IF NOT EXISTS [!!!] (";
    for (int ii = 1; ii < column_titles.size(); ii++)
    {
        stmt += "[" + column_titles[ii] + "] NUMERIC, ";
    }
    stmt.erase(stmt.size() - 2, 2);
    stmt += ");";
    return stmt;
}
string STATSCAN::make_insert_csv_row_template(vector<string>& column_titles)
{
    string stmt = "INSERT INTO [!!!] (";
    for (int ii = 1; ii < column_titles.size(); ii++)
    {
        stmt += "[" + column_titles[ii] + "], ";
    }
    stmt.erase(stmt.size() - 2, 2);
    stmt += ") VALUES (";
    for (int ii = 1; ii < column_titles.size(); ii++)
    {
        stmt += "?, ";
    }
    stmt.erase(stmt.size() - 2, 2);
    stmt += ");";
    return stmt;
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
