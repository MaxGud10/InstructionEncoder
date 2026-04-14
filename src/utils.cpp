#include "utils.hpp"

#include <algorithm>
#include <cctype>
#include <set>
#include <stdexcept>
#include <unordered_set>

int ceil_log2_int(int value)
{
    if (value <= 1)
        return 0;

    int bits = 0;
    int x    = 1;
    while (x < value)
    {
        x <<= 1;
        ++bits;
    }

    return bits;
}

std::string to_binary_string(int value, int width)
{
    if (width <= 0)
        return "";

    std::string s(width, '0');
    for (int i = width - 1; i >= 0; --i)
    {
        s[i] = (value & 1) ? '1' : '0';
        value >>= 1;
    }

    return s;
}

std::string to_upper(std::string s)
{
    for (char& c : s)
    {
        c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    }

    return s;
}

std::vector<std::string> collect_all_operand_names(const InputSpec& spec)
{
    std::set<std::string> names;
    for (const auto& group : spec.instructions)
    {
        for (const auto& op : group.operands)
        {
            names.insert(op);
        }
    }

    return std::vector<std::string>(names.begin(), names.end());
}

std::unordered_map<std::string, std::vector<std::string>>
build_field_usage(const InputSpec& spec)
{
    std::unordered_map<std::string, std::vector<std::string>> usage;
    for (const auto& group : spec.instructions)
    {
        for (const auto& op : group.operands)
        {
            usage[op].push_back(group.format);
        }
    }

    return usage;
}

std::unordered_map<std::string, std::vector<std::string>>
build_format_to_fields(const InputSpec& spec, int format_bits, int opcode_bits)
{
    std::unordered_map<std::string, std::vector<std::string>> result;

    for (const auto& group : spec.instructions)
    {
        std::vector<std::string> fields;
        if (format_bits > 0)
            fields.push_back("F");
        if (opcode_bits > 0 && group.uses_opcode)
            fields.push_back("OPCODE");

        for (const auto& op : group.operands)
        {
            fields.push_back(op);
        }

        result[group.format] = std::move(fields);
    }

    return result;
}

std::unordered_map<std::string, std::vector<std::string>>
build_conflict_graph(const InputSpec& spec, int format_bits, int opcode_bits)
{
    auto format_fields = build_format_to_fields(spec, format_bits, opcode_bits);

    std::unordered_map<std::string, std::unordered_set<std::string>> temp;

    for (const auto& [format_name, fields] : format_fields)
    {
        for (size_t i = 0; i < fields.size(); ++i)
        {
            temp[fields[i]];
            for (size_t j = i + 1; j < fields.size(); ++j)
            {
                temp[fields[i]].insert(fields[j]);
                temp[fields[j]].insert(fields[i]);
            }
        }
    }

    std::unordered_map<std::string, std::vector<std::string>> graph;
    for (auto& [node, neigh] : temp)
    {
        graph[node] = std::vector<std::string>(neigh.begin(), neigh.end());
    }

    return graph;
}