#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/string_path.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>
#include <fstream>
#include <sstream>
#include <string>
#include <queue>
#include <set>


const char* json_data_1 =
    "  \t {\n"
    "  \t \"name 0\" : \"value\", \t // comment \n"
    "  \t \"name 1\" : \"\", // comment \n"
    "  \t \"name 2\" : true, // comment \n"
    "  \t \"name 3\" : false, // comment \n"
    "  \t \"name 4\" : null, // comment \n"
    "  \t \"name 5\" : 0, // comment \n"
    "  \t \"name 6\" : -5, // comment \n"
    "  \t \"name 7\" : 1.1, // comment \n"
    "  \t \"name 8\" : -956.45e+4, // comment \n"
    "  \t \"name 8\" : 5956.45E-11, // comment \n"
    "  \t \"name 9\" : [1,2,3,4], // comment \n"
    "  \t \"name 10\" : {\"a\":\"b\"} // comment \n"
    "  \t } // comment \n";

const char* json_data_2 =
    "{\n"
    "\"MyOldSection\" : {\n"
    "\t\"old-value\" : 3.14\n"
    "},\n"
    "\t\"update_me\" : 33,\n"
    "\t\"do_not_touch_value\" : 0\n"
    "},\n"
    "\"myArray\" : [1,2,3,4]\n"
    "}\n";

const char* json_data_3 =
    "{\n"
    "\"MySection\" : {\n"
    "\t\"update_me\" : 44, \n"
    "\t\"new-value\" : 2 \n"
    "}\n"
    "}\n";

const char* json_data_4 =
    "{\n"
    "\"frame\" : {\n"
    "  \"index\" : 2, \n"
    "  \"dt\" : 0.16 \n"
    "},\n"
    "\"frame\" : {\n"
    "  \"index\" : 3, \n"
    "  \"dt\" : 0.17 \n"
    "}\n"
    "}\n";

const char* json_data_5 =
    "{\n"
    "\"empty_array\" : [],\n"
    "\"non_empty_array\" : [2, 3, 4, 5]\n"
    "}\n";

const char* json_data_6 =
    "{\n"
    "  \"a1\" : {\n"
    "    \"a2\" : {\n"
    "      \"a3\" : 42,\n"
    "      \"a3\" : 42\n"
    "    },\n"
    "    \"b2\" : { \n"
    "      \"a3\" : 42\n"
    "    }\n"
    "  },\n"
    "  \"b1\" : {\n"
    "  }\n"
    "}\n";

const char* json_data_7a =
    "{\n"
    "  \"a1\" : [1, 2, 3],\n"
    "  \"b1\" : \n"
    "  {\n"
    "    \"value\" : 42"
    "  }\n"
    "}\n";

const char* json_data_7b =
    "{\n"
    "  \"a1\" : [4, 5],\n"
    "  \"c1\" : {},\n"
    "  \"d1\" : [\n"
    "    [1, 2, 42],\n"
    "    {\n"
    "      \"idx\" : 4\n"
    "    }\n"
    "  ]\n"
    "}\n";


void
write_json(const std::string& filename, const char* json_data)
{
    using namespace std;

    ofstream ofs;
    ofs.open(filename);
    ofs << json_data;
    ofs.close();
    cout << "Wrote: '" << filename << "'" << endl;
}




/**
 * @brief Check if @a pt is a leaf, i.e. has no children.
 * @return True if @a pt has no children, otherwise false.
 */
bool isLeafTree(const boost::property_tree::ptree& pt)
{
    // size function returns number of childred.
    return pt.size() == 0;
}


/**
 * @ Checks if @a pt is empty, i.e. has no data and no children.
 * @return True if @a pt is empty, otherwise false.
 */
bool isEmptyTree(const boost::property_tree::ptree& pt) {
    return pt.data().empty() && isLeafTree(pt);
}


/**
 * @brief Check if @pt is an array, i.e. all its child keys are empty.
 *        Note that a leaf-tree (which has no keys) is not an array.
 * @return True if @pt is an array, otherwise false.
 */
bool isArrayTree(const boost::property_tree::ptree& pt)
{
    using boost::property_tree::ptree;

    if (isLeafTree(pt))
    {
        return false;
    }

    const ptree::const_iterator iend = pt.end();
    for (ptree::const_iterator iter = pt.begin(); iter != iend; ++iter)
    {
        const ptree::key_type& key = iter->first;
        if (key != "")
        {   // Found non-empty key.
            return false;
        }
    }

    return true;
}


/**
 * @brief Check if the direct children of @a pt have unique keys.
 * @return True if all direct children of @a pt have unique keys,
 *         otherwise false. Returns true for leaf trees, which have no children.
 */
bool hasUniqueKeys(const boost::property_tree::ptree& pt)
{
    using namespace std;
    using boost::property_tree::ptree;

    set<ptree::key_type> keys;
    const ptree::const_iterator iend = pt.end();
    for (ptree::const_iterator iter = pt.begin(); iter != iend; ++iter)
    {
        const ptree::key_type& key = iter->first;
        if (key != "")
        {   // Ignore empty keys.
            const auto p = keys.insert(key);
            if (!p.second)
            {   // Duplicate key found!
                return false;
            }
        }
    }

    return true;
}


/**
 * @brief Check if the all (direct and indirect) children of @a pt have unique keys.
 * @return True if all children of @a pt have unique keys, otherwise false.
 *         If @a pt is a leaf, return true.
 */
bool hasUniquePaths(const boost::property_tree::ptree& pt)
{
    using boost::property_tree::ptree;

    if (!hasUniqueKeys(pt)) {
        return false;
    }

    const auto iend = pt.end();
    for (auto iter = pt.begin(); iter != iend; ++iter) {
        const ptree& child = iter->second;
        if (!hasUniquePaths(child)) { // Recursive!
            return false;
        }
    }

    return true;
}


/**
 *
 */
boost::property_tree::ptree merge(const boost::property_tree::ptree& pt1,
                                  const boost::property_tree::ptree& pt2) {
    using namespace std;
    using boost::property_tree::ptree;

    // Initialize to merge result to first tree.
    ptree merged = pt1;

    // Setup a queue of children for traversal of tree.
    queue<pair<ptree::path_type, ptree>> children;
    children.push(make_pair(ptree::path_type(), pt2));

    while (!children.empty()) {
        // Setup keys and corresponding values
        const auto child = children.front();
        children.pop();
        const ptree::path_type& path = child.first;
        const ptree& tree = child.second;

        const auto iend = tree.end();
        for (auto iter = tree.begin(); iter != iend; ++iter)
        {
            const ptree::key_type& sub_key = iter->first;
            const ptree& sub_tree = iter->second;

            const ptree::path_type sub_path = path / ptree::path_type(sub_key);

            if (isLeafTree(sub_tree) || isArrayTree(sub_tree))
            {
                // Put sub-tree into merged property tree.
                merged.put_child(sub_path, sub_tree);
            }
            else
            {
                // Actual sub-tree with non-array element children.
                // Store sub_tree for further processing.
                children.push(make_pair(sub_path, sub_tree));
            }
        }
    }

    return merged;
}


std::string nextIndent(const std::string& indent)
{
    return indent + "  ";
}

void print(
        const boost::property_tree::ptree& pt,
        std::ostream& os,
        const boost::property_tree::ptree::path_type& path =
                boost::property_tree::ptree::path_type(),
        const std::string& indent = "",
        const bool in_array = false)
{
    using namespace std;
    using boost::property_tree::ptree;

    if (in_array)
    {
        size_t i = 0;
        const auto end = pt.end();
        for (auto it = pt.begin(); it != end; ++it, ++i)
        {
            // Array element keys are empty, use [i] notation instead
            stringstream ss;
            ss << "[" << i << "]";
            const string key = ss.str();
            const ptree& tree = it->second;
            const string& data = tree.data();
            const bool tree_is_array = isArrayTree(tree);
            const bool tree_is_leaf = isLeafTree(tree);
            const bool tree_is_empty = isEmptyTree(tree);


            const ptree::path_type local_path = path / ptree::path_type(key);

            os << indent << "'" << local_path.dump() << "'";
            if (!data.empty())
            {
                os << " : '" << data << "'";
            }
            if (tree_is_leaf)
            {
                os << " <leaf>";
            }
            else
            {
                os << " <internal>";
            }
            if (tree_is_array)
            {
                os << " <array>";
            }
            if (tree_is_empty)
            {
                os << " <empty>";
            }
            os << endl;

            // Recursive!
            print(tree, os, local_path, nextIndent(indent), tree_is_array);
        }
    }
    else
    {
        const auto end = pt.end();
        for (auto it = pt.begin(); it != end; ++it)
        {
            const string& key = it->first;
            const ptree& tree = it->second;
            const string& data = tree.data();
            const bool tree_is_array = isArrayTree(tree);
            const bool tree_is_leaf = isLeafTree(tree);
            const bool tree_is_empty = isEmptyTree(tree);

            const ptree::path_type local_path = path / ptree::path_type(key);

            os << indent << "'" << local_path.dump() << "'";
            if (!data.empty())
            {
                os << " : '" << data << "'";
            }
            if (tree_is_leaf)
            {
                os << " <leaf>";
            }
            else
            {
                os << " <internal>";
            }
            if (tree_is_array)
            {
                os << " <array>";
            }
            if (tree_is_empty)
            {
                os << " <empty>";
            }
            os << endl;

            // Recursive!
            print(tree, os, local_path, nextIndent(indent), tree_is_array);
        }
    }
}


int
main(int argc, char* argv[])
{
    using namespace boost::property_tree;
    using namespace std;

    try {
#if 0
        {
            const string filename("json_data_1.json");
            ofstream ofs;
            ofs.open(filename);
            ofs << json_data_1;
            ofs.close();

            cout << "Data: " << endl
                 << json_data_1 << endl;

            ptree config;
            read_json(filename, config);
            print(config);
        }
#endif

#if 0
        {
            cout << endl << "==========" << endl << endl;

            write_json("data_2.json", json_data_2);
            write_json("data_3.json", json_data_3);

            ptree config2;
            read_json("data_2.json", config2);
            cout << "-----" << endl;
            print(config2, cout);
            cout << "-----" << endl;

            ptree config3;
            read_json("data_3.json", config3);
            cout << "-----" << endl;
            print(config3, cout);
            cout << "-----" << endl;

            print(merge(config2, config3), cout);
            cout << "-----" << endl;
            //print(merge(config3, config2), cout);
            //cout << "start merge" << endl;
            //print(mergePropertyTrees(config2, config3));

            cout << endl << "==========" << endl;

            // merge(pt1, pt2) == merge(pt1, pt2)
        }
#endif

#if 0
        {
            write_json("data_4.json", json_data_4);
            ptree config;
            read_json("data_4.json", config);
            cout << "-----" << endl;
            print(config, cout);
            cout << "-----" << endl;

            cout << "is_leaf <root>: " << is_leaf(config) << endl;
            cout << "has_unique_child_keys <root>: "
                 << has_unique_child_keys(config) << endl;

            cout << "is_leaf <frame>: " << is_leaf(config.get_child("frame")) << endl;
            cout << "has_unique_child_keys <frame>: "
                 << has_unique_child_keys(config.get_child("frame")) << endl;

            cout << "is_leaf <frame.index>: " << is_leaf(config.get_child("frame.index")) << endl;
            cout << "has_unique_child_keys <frame.index>: "
                 << has_unique_child_keys(config.get_child("frame.index")) << endl;

            ptree empty_config;
            cout << "is_leaf <empty>: " << is_leaf(empty_config) << endl;
            cout << "has_unique_child_keys <empty>: "
                 << has_unique_child_keys(empty_config) << endl;

            // Cannot set data for non-leaf tree when using JSON!
            //config.get_child("frame").data() = "test42";
            //boost::property_tree::write_json("data_4b.json", config); // ERROR!
        }
#endif

#if 0
        {
            write_json("data_5.json", json_data_5);
            ptree config;
            read_json("data_5.json", config);
            cout << "-----" << endl;
            print(config, cout);
            cout << "-----" << endl;

            cout << "is_array <root>: " << ::is_array(config) << endl;
            cout << "is_array <root.empty_array>: " << ::is_array(config.get_child("empty_array")) << endl;
            cout << "is_array <root.non_empty_array>: " << ::is_array(config.get_child("non_empty_array")) << endl;
        }
#endif

#if 0
        {
            write_json("data_6.json", json_data_6);
            ptree config;
            read_json("data_6.json", config);
            cout << "-----" << endl;
            print(config, cout);
            cout << "-----" << endl;

            cout << "has_unique_paths <root>: " << has_unique_paths(config) << endl;
            cout << "has_unique_paths <root.a1.b2>: " << has_unique_paths(config.get_child("a1.b2")) << endl;
        }
#endif

#if 1
        {
            ::write_json("data_7a.json", json_data_7a);
            ptree config_a;
            read_json("data_7a.json", config_a);
            cout << "----- a" << endl;
            cout << "hasUniquePaths(a): " << hasUniquePaths(config_a) << endl;
            print(config_a, cout);
            cout << "----- " << endl;

            ::write_json("data_7b.json", json_data_7b);
            ptree config_b;
            read_json("data_7b.json", config_b);
            cout << "----- b" << endl;
            cout << "hasUniquePaths(b): " << hasUniquePaths(config_b) << endl;
            print(config_b, cout);
            cout << "----- " << endl;

            cout << "----- merge(a, b)" << endl;
            ptree merged_ab = merge(config_a, config_b);
            print(merged_ab, cout);
            cout << "----- merge(b, a)" << endl;
            ptree merged_ba = merge(config_b, config_a);
            print(merged_ba, cout);

            write_json("merged_7ab.json", merged_ab);
            write_json("merged_7ba.json", merged_ba);
        }
#endif


#if 0
        {
            ptree pipeline_config;
            read_json("pipeline.json", pipeline_config);
            cout << "-----" << endl;
            print(pipeline_config, cout);
            cout << "-----" << endl;

            ptree pipeline_config_override;
            read_json("pipeline_override.json", pipeline_config_override);
            cout << "-----" << endl;
            print(pipeline_config_override, cout);
            cout << "-----" << endl;

        }
#endif

    }
    catch (boost::property_tree::json_parser_error& e) {
        cerr << "Exception: " << e.what() << endl;
    }
    catch (...) {
        cerr << "Unknown exception" << endl;
    }

    return 0;
}
