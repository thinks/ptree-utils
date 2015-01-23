#ifndef PTREE_UTILS_HPP_INCLUDED
#define PTREE_UTILS_HPP_INCLUDED

#include <iostream>
#include <sstream>

#include <boost/property_tree/ptree.hpp>

namespace std {

template<class K, class D, class C>
ostream& operator<<(
        ostream& os,
        const boost::property_tree::basic_ptree<K, D, C>& pt) {
    boost::property_tree::write_json(os, pt);
    return os;
}

} // namespace std

template<class K, class D, class C>
void readJsonString(
        const char* json_data,
        boost::property_tree::basic_ptree<K, D, C>& pt)
{
    std::stringstream ss;
    ss << json_data;
    boost::property_tree::read_json(ss, pt);
}

/**
 * @brief Check if @a pt is a leaf, i.e. has no children.
 * @return True if @a pt has no children, otherwise false.
 */
template<class K, class D, class C>
bool isLeafTree(const boost::property_tree::basic_ptree<K, D, C>& pt)
{
    // size function returns number of childred.
    return pt.size() == 0;
}


/**
 * @ Checks if @a pt is empty, i.e. has no data and no children.
 * @return True if @a pt is empty, otherwise false.
 */
template<class K, class D, class C>
bool isEmptyTree(const boost::property_tree::basic_ptree<K, D, C>& pt) {
    return pt.data().empty() && isLeafTree(pt);
}

/**
 * @brief Check if @pt is an array, i.e. all its child keys are empty.
 *        Note that a leaf-tree (which has no keys) is not an array.
 * @return True if @pt is an array, otherwise false.
 */
template<class K, class D, class C>
bool isArrayTree(const boost::property_tree::basic_ptree<K, D, C>& pt)
{
    using boost::property_tree::ptree;

    if (isLeafTree(pt))
    {
        return false;
    }

    const auto iend = pt.end();
    for (auto iter = pt.begin(); iter != iend; ++iter)
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
template<class K, class D, class C>
bool hasUniqueKeys(const boost::property_tree::basic_ptree<K, D, C>& pt)
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

    if (!hasUniqueKeys(pt))
    {
        return false;
    }

    const auto iend = pt.end();
    for (auto iter = pt.begin(); iter != iend; ++iter)
    {
        const ptree& child = iter->second;
        if (!hasUniquePaths(child))
        { // Recursive!
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

#endif // PTREE_UTILS_HPP_INCLUDED
