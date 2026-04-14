#include "allocator.hpp"
#include "utils.hpp"
#include "validator.hpp"

#include <algorithm>
#include <stdexcept>
#include <unordered_set>
#include <map>

namespace
{

struct NodeInfo
{
    std::string name;
    int         width       = 0;
    bool        expandable  = false;
    int         degree      = 0;
    int         usage_count = 0;
};

bool overlaps_any_conflict(
    const std::string&                                               node,
    const Interval&                                                  candidate,
    const std::unordered_map<std::string, Interval>&                 placed,
    const std::unordered_map<std::string, std::vector<std::string>>& graph)
{
    auto it = graph.find(node);
    if (it == graph.end())
        return false;

    for (const auto& neighbor : it->second)
    {
        auto p = placed.find(neighbor);
        if (p == placed.end())
            continue;

        if (candidate.overlaps(p->second))
            return true;
    }
    return false;
}

bool place_nodes_backtracking(
    size_t                                                           idx,
    const std::vector<NodeInfo>&                                     nodes,
    int                                                              instruction_length,
    const std::unordered_map<std::string, std::vector<std::string>>& graph,
    std::unordered_map<std::string, Interval>&                       placed)
{
    if (idx == nodes.size())
        return true;

    const auto& node    = nodes[idx];
    const int   width   = node.width;
    const int   max_lsb = instruction_length - width;

    for (int lsb = max_lsb; lsb >= 0; --lsb)
    {
        Interval candidate{lsb + width - 1, lsb};
        if (overlaps_any_conflict(node.name, candidate, placed, graph))
            continue;

        placed[node.name] = candidate;
        if (place_nodes_backtracking(idx + 1, nodes, instruction_length, graph, placed))
            return true;

        placed.erase(node.name);
    }

    return false;
}

std::unordered_map<std::string, std::vector<bool>> initialize_occupancy(
    const InputSpec&                                 spec,
    const std::unordered_map<std::string, Interval>& global_fields,
    int                                              format_bits,
    int                                              opcode_bits)
{
    std::unordered_map<std::string, std::vector<bool>> occ;

    for (const auto& group : spec.instructions)
    {
        occ[group.format] = std::vector<bool>(spec.length, false);

        auto mark = [&](const std::string& name)
        {
            auto it = global_fields.find(name);
            if (it == global_fields.end())
                return;
            for (int b = it->second.lsb; b <= it->second.msb; ++b)
            {
                occ[group.format][b] = true;
            }
        };

        if (format_bits > 0)
            mark("F");
        if (opcode_bits > 0 && group.uses_opcode)
            mark("OPCODE");

        for (const auto& op : group.operands)
        {
            mark(op);
        }
    }

    return occ;
}

bool try_expand_one_direction(
    const std::string&                                               field_name,
    bool                                                             downward,
    std::unordered_map<std::string, Interval>&                       global_fields,
    std::unordered_map<std::string, std::vector<bool>>&              occ,
    const std::unordered_map<std::string, std::vector<std::string>>& field_usage,
    int                                                              instruction_length)
{
    auto it = global_fields.find(field_name);
    if (it == global_fields.end())
        return false;

    const Interval current = it->second;

    if (downward)
    {
        if (current.lsb == 0)
            return false;
        int bit = current.lsb - 1;
        for (const auto& fmt : field_usage.at(field_name))
        {
            if (occ[fmt][bit])
                return false;
        }

        for (const auto& fmt : field_usage.at(field_name))
        {
            occ[fmt][bit] = true;
        }

        it->second.lsb -= 1;
        return true;
    }
    else
    {
        if (current.msb == instruction_length - 1)
            return false;

        int bit = current.msb + 1;
        for (const auto& fmt : field_usage.at(field_name))
        {
            if (occ[fmt][bit])
                return false;
        }

        for (const auto& fmt : field_usage.at(field_name))
        {
            occ[fmt][bit] = true;
        }

        it->second.msb += 1;
        return true;
    }
}

void expand_expandable_fields(
    const InputSpec&                           spec,
    std::unordered_map<std::string, Interval>& global_fields,
    int                                        format_bits,
    int                                        opcode_bits)
{
    auto field_usage = build_field_usage(spec);
    auto occ         = initialize_occupancy(spec, global_fields, format_bits, opcode_bits);

    bool progress = true;
    while (progress)
    {
        progress = false;

        for (const auto& field : spec.fields)
        {
            if (!field.expandable)
                continue;
            if (field_usage.find(field.name) == field_usage.end())
                continue;

            if (try_expand_one_direction(field.name, true, global_fields, occ, field_usage, spec.length))
                progress = true;
            else if (try_expand_one_direction(field.name, false, global_fields, occ, field_usage, spec.length))
                progress = true;
        }
    }
}

std::vector<ReserveField> build_reserves_for_format(
    const InputSpec&                                 spec,
    const InstructionGroup&                          group,
    const std::unordered_map<std::string, Interval>& global_fields,
    int                                              format_bits,
    int                                              opcode_bits)
{
    std::vector<bool> occ(spec.length, false);

    auto mark = [&](const std::string& name)
    {
        auto it = global_fields.find(name);
        if (it == global_fields.end())
            return;

        for (int b = it->second.lsb; b <= it->second.msb; ++b)
        {
            occ[b] = true;
        }
    };

    if (format_bits > 0)
        mark("F");
    if (opcode_bits > 0 && group.uses_opcode)
        mark("OPCODE");

    for (const auto& op : group.operands)
    {
        mark(op);
    }

    std::vector<ReserveField> reserves;
    int                       res_index = 0;

    int bit = spec.length - 1;
    while (bit >= 0)
    {
        if (occ[bit])
        {
            --bit;
            continue;
        }

        int msb = bit;
        while (bit >= 0 && !occ[bit])
        {
            --bit;
        }

        int lsb = bit + 1;

        reserves.push_back(ReserveField{
            "RES" + std::to_string(res_index++),
            Interval{msb, lsb}
        });
    }

    return reserves;
}

} // namespace

LayoutResult allocate_layout(InputSpec spec)
{
    validate_input_spec(spec);

    LayoutResult result;
    result.instruction_length = spec.length;
    result.format_bits        = ceil_log2_int(static_cast<int>(spec.instructions.size()));

    int max_opcode_bits = 0;
    for (auto& group : spec.instructions)
    {
        group.uses_opcode       = group.insns.size() > 1;
        group.local_opcode_bits = group.uses_opcode ? ceil_log2_int(static_cast<int>(group.insns.size())) : 0;
        max_opcode_bits         = std::max(max_opcode_bits, group.local_opcode_bits);
    }

    result.opcode_bits = max_opcode_bits;

    for (size_t i = 0; i < spec.instructions.size(); ++i)
    {
        spec.instructions[i].format_id = static_cast<int>(i);
    }

    auto graph = build_conflict_graph(spec, result.format_bits, result.opcode_bits);

    std::unordered_map<std::string, int> usage_count;
    for (const auto& group : spec.instructions)
    {
        std::unordered_set<std::string> seen;
        if (result.format_bits > 0)
            usage_count["F"]++;
        if (result.opcode_bits > 0 && group.uses_opcode)
            usage_count["OPCODE"]++;
        for (const auto& op : group.operands)
        {
            if (seen.insert(op).second)
                usage_count[op]++;
        }
    }

    std::vector<NodeInfo> nodes;

    if (result.format_bits > 0)
    {
        nodes.push_back(NodeInfo{
            "F", result.format_bits, false,
            static_cast<int>(graph["F"].size()), usage_count["F"]
        });
    }
    if (result.opcode_bits > 0)
    {
        nodes.push_back(NodeInfo{
            "OPCODE", result.opcode_bits, false,
            static_cast<int>(graph["OPCODE"].size()), usage_count["OPCODE"]
        });
    }

    for (const auto& field : spec.fields)
    {
        nodes.push_back(NodeInfo{
            field.name,
            field.min_width,
            field.expandable,
            static_cast<int>(graph[field.name].size()),
            usage_count[field.name]
        });
    }

    nodes.erase(
        std::remove_if(nodes.begin(), nodes.end(), [](const NodeInfo& n) {
            return n.width <= 0; }),
        nodes.end()
    );

    std::sort(nodes.begin(), nodes.end(), [](const NodeInfo& a, const NodeInfo& b)
    {
        if (a.name == "F"     ) return true;
        if (b.name == "F"     ) return false;
        if (a.name == "OPCODE") return true;
        if (b.name == "OPCODE") return false;

        if (a.degree      != b.degree     ) return a.degree      > b.degree;
        if (a.usage_count != b.usage_count) return a.usage_count > b.usage_count;
        if (a.width       != b.width      ) return a.width       > b.width;
        return a.name < b.name;
    });

    std::unordered_map<std::string, Interval> placed;
    if (!place_nodes_backtracking(0, nodes, spec.length, graph, placed))
        throw std::runtime_error("Unable to construct a valid layout for the given constraints");

    expand_expandable_fields(spec, placed, result.format_bits, result.opcode_bits);
    result.global_fields = placed;

    for (const auto& group : spec.instructions)
    {
        FormatLayout fl;
        fl.format      = group.format;
        fl.insns       = group.insns;
        fl.operands    = group.operands;
        fl.format_id   = group.format_id;
        fl.uses_opcode = group.uses_opcode;
        result.formats.push_back(fl);

        result.reserves_by_format[group.format] =
            build_reserves_for_format(spec, group, result.global_fields, result.format_bits, result.opcode_bits);
    }

    return result;
}