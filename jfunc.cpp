#include "jfunc.h"

using namespace std;

int JFUNC::tree_from_indent(vector<int>& ind_list, vector<vector<int>>& tree_st)
{
	// Note that the tree structure is of the form 
	// [node_index][ancestor1, ancestor2, ... , (neg) node value, child1, child2, ...]

	// genealogy's indices are indent magnitudes, while its 
	// values are the last list index to have that indentation.
	vector<int> genealogy, vtemp;
	int delta_indent, node, parent;
	tree_st.resize(ind_list.size(), vector<int>());

	for (int ii = 0; ii < ind_list.size(); ii++)
	{
		// Update genealogy.
		delta_indent = ind_list[ii] - genealogy.size() + 1;  // New indent minus old indent.
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
		else { return 1; }

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
	return 0;
}
int JFUNC::tree_from_marker(vector<vector<int>>& tree_st, vector<string>& tree_pl)
{
	vector<vector<string>> sections;
	unordered_map<string, int> map_sections;  // RESUME HERE. 

}
