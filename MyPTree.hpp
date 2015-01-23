#ifndef MY_PTREE_HPP_INCLUDED
#define MY_PTREE_HPP_INCLUDED

#include <iostream>
#include <queue>
#include <sstream>
#include <utility>
#include <vector>

#include <boost/optional/optional.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include "PTreeUtils.hpp"

// Forward declarations.
struct StringToMyData;
struct MyDataToString;

class MyData
{
public:
    MyData()
            : data_("")
            , hits_(std::make_shared<size_t>(0)) {
    }

    MyData(const std::string& data)
            : data_(data)
            , hits_(std::make_shared<size_t>(0)) {
    }

    std::string data() const {
        ++(*hits_);
        return data_;
    }

    std::size_t hits() const {
        return *hits_;
    }

private:
    std::string data_;
    mutable std::shared_ptr<std::size_t> hits_;

    friend struct StringToMyData;
    friend struct MyDataToString;
};


struct StringToMyData
{
    typedef std::string internal_type;
    typedef MyData      external_type;

    // Converts a string to bool
    boost::optional<external_type> get_value(const internal_type& t)
    {
        //std::cout << "StringToMyData::get_value" << std::endl;
        return boost::optional<external_type>(t);
    }

    // Converts a bool to string
    boost::optional<internal_type> put_value(const external_type& d)
    {
        //std::cout << "StringToMyData::put_value" << std::endl;
        return boost::optional<internal_type>(d.data_);
    }
};

struct MyDataToString
{
    typedef MyData      internal_type;
    typedef std::string external_type;

    // Converts a string to bool
    boost::optional<external_type> get_value(const internal_type& d)
    {
        //std::cout << "MyDataToString::get_value" << std::endl;

        if (!d.data_.empty()) {
            return boost::optional<external_type>(d.data_);
        }
        return boost::optional<external_type>("");
    }

    // Converts a bool to string
    boost::optional<internal_type> put_value(const external_type& t)
    {
        //std::cout << "MyDataToString::put_value" << std::endl;
        return boost::optional<internal_type>(t);
    }
};

namespace boost {
namespace property_tree {

template<typename Ch, typename Traits, typename Alloc>
struct translator_between<std::basic_string<Ch, Traits, Alloc>, MyData>
{
    typedef StringToMyData type;
};

template<typename Ch, typename Traits, typename Alloc>
struct translator_between<MyData, std::basic_string<Ch, Traits, Alloc>>
{
    typedef MyDataToString type;
};

} // namespace property_tree
} // namespace boost

typedef boost::property_tree::basic_ptree<std::string, MyData> MyPTree;

std::vector<MyPTree::key_type> untouchedKeys(const MyPTree& pt)
{
    using namespace std;

    vector<MyPTree::key_type> untouched_keys;

    // Setup a queue of children for traversal of tree.
    queue<pair<MyPTree::path_type, MyPTree>> children;
    children.push(make_pair(MyPTree::path_type(), pt));

    while (!children.empty()) {
        // Setup keys and corresponding values
        const auto child = children.front();
        children.pop();
        const auto& path = child.first;
        const auto& tree = child.second;

        const auto iend = end(tree);
        for (auto iter = begin(tree); iter != iend; ++iter)
        {
            const auto& sub_key = iter->first;
            const auto& sub_tree = iter->second;

            const auto sub_path = path / MyPTree::path_type(sub_key);

            if (isLeafTree(sub_tree))
            {
                const auto data = sub_tree.get_value<MyPTree::data_type>();
                if (data.hits() == 0)
                {
                    untouched_keys.push_back(sub_path.dump());
                }
            }
            else if (isArrayTree(sub_tree))
            {
                size_t index = 0;
                const auto sub_iend = end(sub_tree);
                for (auto sub_iter = begin(sub_tree); sub_iter != sub_iend;
                     ++sub_iter, ++index)
                {
                    const auto data = sub_tree.get_value<MyPTree::data_type>();
                    if (data.hits() == 0)
                    {
                        stringstream ss;
                        ss << sub_path.dump() << "[" << index << "]";
                        untouched_keys.push_back(ss.str());
                    }
                }
            }
            else
            {
                // Actual sub-tree with non-array element children.
                // Store sub_tree for further processing.
                children.push(make_pair(sub_path, sub_tree));
            }
        }
    }

    return untouched_keys;
}

#endif // MY_PTREE_HPP_INCLUDED
