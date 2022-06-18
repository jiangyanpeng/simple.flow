#include <sstream>
#include <set>
#include <unordered_map>
#include <iostream>
#include <regex>

using topology_t = std::unordered_map<std::string, std::set<std::string>>;

bool graphviz(topology_t &target, std::stringstream &ss, std::string tag)
{
    ss << "\ndigraph " << tag << "{" << std::endl;
    ss << "rankdir=BT;" << std::endl;
    ss << "size=\"8,5\";" << std::endl;
    for (auto &k_v : target)
    {
        for (auto input : k_v.second)
        {
            ss << "\"" << k_v.first << "\" -> \"" << input << "\""
               << ";" << std::endl;
        }
    }
    ss << "}";
    return true;
}

int main()
{
    std::cout << "graphviz" << std::endl;
    return 0;
}