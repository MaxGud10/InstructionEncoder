#include "validator.hpp"
#include "utils.hpp"

#include <stdexcept>
#include <unordered_set>

void validate_input_spec(InputSpec& spec)
{
    if (spec.length <= 0)
        throw std::runtime_error("Instruction length must be positive");

    if (spec.instructions.empty())
        throw std::runtime_error("At least one instruction group is required");

    std::unordered_set<std::string> field_names;
    for (const auto& field : spec.fields)
    {
        if (field.name.empty())
            throw std::runtime_error("Field name must not be empty");
        if (!field_names.insert(field.name).second)
            throw std::runtime_error("Duplicate field name: " + field.name);

        spec.field_map[field.name] = field;
    }

    std::unordered_set<std::string> all_insns;
    std::unordered_set<std::string> format_names;

    for (auto& group : spec.instructions)
    {
        if (group.format.empty())
            throw std::runtime_error("Format name must not be empty");
        if (!format_names.insert(group.format).second)
            throw std::runtime_error("Duplicate format name: " + group.format);

        if (group.insns.empty())
            throw std::runtime_error("Each group must contain at least one instruction");

        std::unordered_set<std::string> local_operands;
        for (const auto& op : group.operands)
        {
            if (!local_operands.insert(op).second)
                throw std::runtime_error("Duplicate operand '" + op + "' in format " + group.format);
            if (spec.field_map.find(op) == spec.field_map.end())
                throw std::runtime_error("Operand '" + op + "' is not declared in fields");
        }

        for (const auto& insn : group.insns)
        {
            if (insn.empty())
                throw std::runtime_error("Instruction name must not be empty");
            if (!all_insns.insert(insn).second)
                throw std::runtime_error("Duplicate instruction name: " + insn);
        }

        group.uses_opcode       = group.insns.size() > 1;
        group.local_opcode_bits = group.uses_opcode  ? ceil_log2_int(static_cast<int>(group.insns.size())) : 0;
    }
}