#include "generator.hpp"
#include "utils.hpp"

#include <algorithm>
#include <fstream>
#include <stdexcept>
#include <tuple>

namespace
{

struct OutputField
{
    std::string name;
    Interval    interval;
    std::string value;
};

std::vector<OutputField> build_fields_for_instruction(const InstructionGroup& group,
                                                      int                     insn_index,
                                                      const LayoutResult&     layout)
{
    std::vector<OutputField> fields;

    if (layout.format_bits > 0)
    {
        fields.push_back(OutputField{
            "F",
            layout.global_fields.at("F"),
            to_binary_string(group.format_id, layout.format_bits)
        });
    }

    if (layout.opcode_bits > 0 && group.uses_opcode)
    {
        fields.push_back(OutputField{
            "OPCODE",
            layout.global_fields.at("OPCODE"),
            to_binary_string(insn_index, layout.opcode_bits)
        });
    }

    for (const auto& op : group.operands)
    {
        fields.push_back(OutputField{
            to_upper(op),
            layout.global_fields.at(op),
            "+"
        });
    }

    auto res_it = layout.reserves_by_format.find(group.format);
    if (res_it != layout.reserves_by_format.end())
    {
        for (const auto& res : res_it->second)
        {
            fields.push_back(OutputField{
                res.name,
                res.interval,
                std::string(res.interval.width(), '0')
            });
        }
    }

    std::sort(fields.begin(), fields.end(), [](const OutputField& a, const OutputField& b)
    {
        if (a.interval.msb != b.interval.msb)
            return a.interval.msb > b.interval.msb;

        return a.name < b.name;
    });

    return fields;
}

} // namespace

nlohmann::json generate_output_json(const InputSpec& spec, const LayoutResult& layout)
{
    nlohmann::json result = nlohmann::json::array();

    for (const auto& group : spec.instructions)
    {
        for (size_t i = 0; i < group.insns.size(); ++i)
        {
            nlohmann::json insn_json;
            insn_json["insn"]   = group.insns[i];
            insn_json["fields"] = nlohmann::json::array();

            auto fields = build_fields_for_instruction(group, static_cast<int>(i), layout);

            for (const auto& field : fields)
            {
                nlohmann::json outer;
                outer[field.name] =
                {
                    {"msb",   field.interval.msb},
                    {"lsb",   field.interval.lsb},
                    {"value", field.value}
                };
                insn_json["fields"].push_back(outer);
            }

            result.push_back(insn_json);
        }
    }

    return result;
}

void write_output_json(const std::string& path, const nlohmann::json& j)
{
    std::ofstream out(path);
    if (!out)
        throw std::runtime_error("Cannot open output file: " + path);

    out << j.dump(2) << "\n";
}