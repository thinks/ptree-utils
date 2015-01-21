#include <boost/property_tree/ptree.hpp>
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
    "\"MySection\" : {\n"
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
 * @return true if @a pt has no children, otherwise false.
 */
bool is_leaf(const boost::property_tree::ptree& pt)
{
    return pt.size() == 0;
}


/**
 *
 */
bool is_array(const boost::property_tree::ptree& pt)
{
    using boost::property_tree::ptree;

    if (is_leaf(pt)) {
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
 * @return true if all children of @a pt have unique keys, otherwise false.
 *         If @a pt is a leaf, return true.
 */
bool has_unique_child_keys(const boost::property_tree::ptree& pt)
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




void
print(const boost::property_tree::ptree& pt,
      std::ostream& os,
      const std::string& path = "",
      const std::string& indent = "")
{
    using namespace std;
    using boost::property_tree::ptree;

    ptree::const_iterator end = pt.end();
    for (ptree::const_iterator it = pt.begin(); it != end; ++it) {
        const string key = (it->first != "") ? it->first : "<empty>";
        const string value = it->second.get_value<string>();
        const string local_path = (path != "") ? path + "." + key : key;
        os << indent << "'" << local_path << "' : '" << value << "'" << endl;
        print(it->second, os, local_path, indent + "  "); // Recursive!
    }
}


// get_child("path.to.child")
// put_child

boost::property_tree::ptree merge(const boost::property_tree::ptree& pt1,
                                  const boost::property_tree::ptree& pt2) {
    using namespace std;
    using boost::property_tree::ptree;

    // Initialize to first tree.
    ptree merged = pt1;

    // ...
    queue<string> keys;
    queue<ptree> values;
    values.push(pt2);

    while (!values.empty()) {
        // Setup keys and corresponding values
        ptree pt = values.front();
        values.pop();

        string path = "";
        if (!keys.empty())
        {
          path = keys.front();
          keys.pop();
        }
        cout << "path: " << path << endl;

        const ptree::const_iterator iend = pt.end();
        for (ptree::const_iterator iter = pt.begin(); iter != iend; ++iter)
        {
            const string& key = iter->first;
            const ptree& sub_tree = iter->second;

            if (is_leaf(sub_tree))
            {
                // No "." for top-level entries.
                string local_path;
                if (path == "")
                {
                    local_path = key;
                }
                else
                {
                    local_path = path + "." + key;
                }

                // Put into combined property tree
                merged.put(local_path, sub_tree.data());
                cout << "put: " << local_path << " | " << sub_tree.data() << endl;
            }
            else
            {   // Sub-tree.
                if (path == "")
                {
                    keys.push(key);
                }
                else
                {
                    keys.push(path + "." + key);
                }

                // Put values (the subtrees) aside too.
                values.push(sub_tree);
            }
#if 0
            const string key = (it->first != "") ? it->first : "<empty>";
            const string value = it->second.get_value<string>();
            const string local_path = (path != "") ? path + "." + key : key;
            cout << indent << "'" << local_path << "' : '" << value << "'" << endl;
            print(it->second, local_path, indent + "  "); // Recursive!
#endif
        }

    }

    return merged;
}

#if 0
boost::property_tree::ptree
mergePropertyTrees(const boost::property_tree::ptree& rptFirst,
                   const boost::property_tree::ptree& rptSecond)
{
  using namespace std;

  // Take over first property tree
  boost::property_tree::ptree ptMerged = rptFirst;

  // Keep track of keys and values (subtrees) in second property tree
  queue<string> qKeys;
  queue<boost::property_tree::ptree> qValues;
  qValues.push(rptSecond);

  // Iterate over second property tree
  while (!qValues.empty())
  {
    // Setup keys and corresponding values
    boost::property_tree::ptree ptree = qValues.front();
    qValues.pop();
    string keychain = "";
    if (!qKeys.empty())
    {
      keychain = qKeys.front();
      qKeys.pop();
    }

    // Iterate over keys level-wise
    BOOST_FOREACH (const boost::property_tree::ptree::value_type& child, ptree)
    {
      // Leaf
      if (child.second.size() == 0)
      {
        // No "." for first level entries
        string s;
        if (keychain != "")
        {
          s = keychain + "." + child.first.data();
        }
        else
        {
          s = child.first.data();
        }

        // Put into combined property tree
        ptMerged.put(s, child.second.data());
      }
      // Subtree
      else
      {
        // Put keys (identifiers of subtrees) and all of its parents (where present)
        // aside for later iteration. Keys on first level have no parents.
        if (keychain != "")
        {
          qKeys.push(keychain + "." + child.first.data());
        }
        else
        {
          qKeys.push(child.first.data());
        }

        // Put values (the subtrees) aside too.
        qValues.push(child.second);
      }
    }
  }

  return ptMerged;
}
#endif

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

#if 1
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

#if 1
        {
            write_json("data_4.json", json_data_4);
            ptree config;
            read_json("data_4.json", config);
            cout << "-----" << endl;
            print(config, cout);
            cout << "-----" << endl;

            cout << "is_leaf <root>: " << is_leaf(config) << endl;
            cout << "has_unique_child_keys <root>: " << has_unique_child_keys(config) << endl;

            cout << "is_leaf <frame>: " << is_leaf(config.get_child("frame")) << endl;
            cout << "has_unique_child_keys <frame>: " << has_unique_child_keys(config.get_child("frame")) << endl;

            cout << "is_leaf <frame.index>: " << is_leaf(config.get_child("frame.index")) << endl;
            cout << "has_unique_child_keys <frame.index>: " << has_unique_child_keys(config.get_child("frame.index")) << endl;

            // Cannot set data for non-leaf tree when using JSON!
            //config.get_child("frame").data() = "test42";
            //boost::property_tree::write_json("data_4b.json", config); // ERROR!
        }
#endif

#if 1
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

    }
    catch (boost::property_tree::json_parser_error& e) {
        cerr << "Exception: " << e.what() << endl;
    }
    catch (...) {
        cerr << "Unknown exception" << endl;
    }

    return 0;
}
